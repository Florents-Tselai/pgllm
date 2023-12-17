import psycopg
import binascii
from collections import namedtuple
from collections.abc import Mapping
import contextlib
import datetime
import decimal
import inspect
import itertools
import json
import os
import pathlib
import re
import secrets
from sqlite_fts4 import rank_bm25  # type: ignore
import textwrap
from typing import (
    cast,
    Any,
    Callable,
    Dict,
    Generator,
    Iterable,
    Union,
    Optional,
    List,
    Tuple,
)
import uuid
from dataclasses import dataclass


@dataclass
class Column:
    name: str
    typename: str


class Database:
    """
    Wrapper for a Postgres database connection that adds a variety of useful utility methods.
    """

    def __init__(self, conninfo: str = "", **kwargs):
        self.conn = psycopg.connect(conninfo, **kwargs)

    def query(
        self, sql: str, params: Optional[Union[Iterable, dict]] = None
    ) -> Generator[dict, None, None]:
        """
        Execute ``sql`` and return an iterable of dictionaries representing each row.

        :param sql: SQL query to execute
        :param params: Parameters to use in that query - an iterable for ``where id = ?``
          parameters, or a dictionary for ``where id = :id``
        """
        cursor = self.execute(sql, params or tuple())
        keys = [d[0] for d in cursor.description]
        for row in cursor:
            yield dict(zip(keys, row))

    def execute(
        self, sql: str, parameters: Optional[Union[Iterable, dict]] = None
    ) -> psycopg.Cursor:
        """
        Execute SQL query and return a ``psycopg.Cursor``.

        :param sql: SQL query to execute
        :param parameters: Parameters to use in that query - an iterable for ``where id = ?``
          parameters, or a dictionary for ``where id = :id``
        """
        if parameters is not None:
            return self.conn.execute(sql, parameters)
        else:
            return self.conn.execute(sql)

    def executescript(self, sql: str) -> psycopg.Cursor:
        """
        Execute multiple SQL statements separated by ; and return the ``sqlite3.Cursor``.

        :param sql: SQL to execute
        """
        return self.conn.executescript(sql)

    def table_names(self) -> List[str]:
        pass

    def view_names(self) -> List[str]:
        pass

    @property
    def name(self):
        return self.execute("SELECT current_database()").fetchone()[0]

    def close(self):
        self.conn.close()

    def table(self, table_name: str, **kwargs) -> Union["Table", "View"]:
        """
        Return a table object, optionally configured with default options.

        See :ref:`reference_db_table` for option details.

        :param table_name: Name of the table
        """
        klass = View if table_name in self.view_names() else Table
        return klass(self, table_name, **kwargs)

    def __getitem__(self, table_name: str) -> Union["Table", "View"]:
        """
        ``db[table_name]`` returns a :class:`.Table` object for the table with the specified name.
        If the table does not exist yet it will be created the first time data is inserted into it.

        :param table_name: The name of the table
        """
        return self.table(table_name)


class Queryable:
    def exists(self) -> bool:
        "Does this table or view exist yet?"
        return False

    def __init__(self, db, name):
        self.db = db
        self.name = name

    def count_where(
        self,
        where: Optional[str] = None,
        where_args: Optional[Union[Iterable, dict]] = None,
    ) -> int:
        """
        Executes ``SELECT count(*) FROM table WHERE ...`` and returns a count.

        :param where: SQL where fragment to use, for example ``id > ?``
        :param where_args: Parameters to use with that fragment - an iterable for ``id > ?``
          parameters, or a dictionary for ``id > :id``
        """
        sql = "select count(*) from [{}]".format(self.name)
        if where is not None:
            sql += " where " + where
        return self.db.execute(sql, where_args or []).fetchone()[0]

    def execute_count(self):
        # Backwards compatibility, see https://github.com/simonw/sqlite-utils/issues/305#issuecomment-890713185
        return self.count_where()

    @property
    def count(self) -> int:
        "A count of the rows in this table or view."
        return self.count_where()

    @property
    def rows(self) -> Generator[dict, None, None]:
        "Iterate over every dictionaries for each row in this table or view."
        return self.rows_where()

    def rows_where(
        self,
        where: Optional[str] = None,
        where_args: Optional[Union[Iterable, dict]] = None,
        order_by: Optional[str] = None,
        select: str = "*",
        limit: Optional[int] = None,
        offset: Optional[int] = None,
    ) -> Generator[dict, None, None]:
        """
        Iterate over every row in this table or view that matches the specified where clause.

        Returns each row as a dictionary. See :ref:`python_api_rows` for more details.

        :param where: SQL where fragment to use, for example ``id > ?``
        :param where_args: Parameters to use with that fragment - an iterable for ``id > ?``
          parameters, or a dictionary for ``id > :id``
        :param order_by: Column or fragment of SQL to order by
        :param select: Comma-separated list of columns to select - defaults to ``*``
        :param limit: Integer number of rows to limit to
        :param offset: Integer for SQL offset
        """
        if not self.exists():
            return
        sql = "select {} from [{}]".format(select, self.name)
        if where is not None:
            sql += " where " + where
        if order_by is not None:
            sql += " order by " + order_by
        if limit is not None:
            sql += " limit {}".format(limit)
        if offset is not None:
            sql += " offset {}".format(offset)
        cursor = self.db.execute(sql, where_args or [])
        columns = [c[0] for c in cursor.description]
        for row in cursor:
            yield dict(zip(columns, row))

    def pks_and_rows_where(
        self,
        where: Optional[str] = None,
        where_args: Optional[Union[Iterable, dict]] = None,
        order_by: Optional[str] = None,
        limit: Optional[int] = None,
        offset: Optional[int] = None,
    ) -> Generator[Tuple[Any, Dict], None, None]:
        """
        Like ``.rows_where()`` but returns ``(pk, row)`` pairs - ``pk`` can be a single value or tuple.

        :param where: SQL where fragment to use, for example ``id > ?``
        :param where_args: Parameters to use with that fragment - an iterable for ``id > ?``
          parameters, or a dictionary for ``id > :id``
        :param order_by: Column or fragment of SQL to order by
        :param select: Comma-separated list of columns to select - defaults to ``*``
        :param limit: Integer number of rows to limit to
        :param offset: Integer for SQL offset
        """
        column_names = [column.name for column in self.columns]
        pks = [column.name for column in self.columns if column.is_pk]
        if not pks:
            column_names.insert(0, "rowid")
            pks = ["rowid"]
        select = ",".join("[{}]".format(column_name) for column_name in column_names)
        for row in self.rows_where(
            select=select,
            where=where,
            where_args=where_args,
            order_by=order_by,
            limit=limit,
            offset=offset,
        ):
            row_pk = tuple(row[pk] for pk in pks)
            if len(row_pk) == 1:
                row_pk = row_pk[0]
            yield row_pk, row

    @property
    def columns(self) -> List["Column"]:
        "List of :ref:`Columns <reference_db_other_column>` representing the columns in this table or view."
        if not self.exists():
            return []
        rows = self.db.execute("PRAGMA table_info([{}])".format(self.name)).fetchall()
        return [Column(*row) for row in rows]

    @property
    def columns_dict(self) -> Dict[str, Any]:
        "``{column_name: python-type}`` dictionary representing columns in this table or view."
        return {column.name: column_affinity(column.type) for column in self.columns}

    @property
    def schema(self) -> str:
        "SQL schema for this table or view."
        return self.db.execute(
            "select sql from sqlite_master where name = ?", (self.name,)
        ).fetchone()[0]


class View(Queryable):
    def exists(self):
        return True

    def __repr__(self) -> str:
        return "<View {} ({})>".format(
            self.name, ", ".join(c.name for c in self.columns)
        )

    def drop(self, ignore=False):
        """
        Drop this view.

        :param ignore: Set to ``True`` to ignore the error if the view does not exist
        """

        try:
            self.db.execute("DROP VIEW [{}]".format(self.name))
        except sqlite3.OperationalError:
            if not ignore:
                raise


class Table(Queryable):
    def __init__(self, db: Database, name: str, pk: Optional[Any] = None):
        super().__init__(db, name)
