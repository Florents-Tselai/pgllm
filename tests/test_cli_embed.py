import pytest
from pgllm.cli import cli
from click.testing import CliRunner


def test_cli_simple(db):
    runner = CliRunner()
    result = runner.invoke(cli, ["hello", "flo"])
    assert result.exit_code == 0
    assert result.stdout == "Hello, flo\n"


def test_cli_embed_simple(db):
    runner = CliRunner()
    result = runner.invoke(cli, ["hello", "flo"])
