from setuptools import setup
import os

VERSION = "0.0.1"


def get_long_description():
    with open(
            os.path.join(os.path.dirname(os.path.abspath(__file__)), "README.md"),
            encoding="utf8",
    ) as fp:
        return fp.read()


setup(
    name="pgllm",
    description="Easily create LLM vectors for existing Postgres data",
    long_description=get_long_description(),
    long_description_content_type="text/markdown",
    author="Florents Tselai",
    url="https://github.com/Florents-Tselai/pgllm",
    entry_points="""
        [console_scripts]
        pgllm=pgllm.cli:cli
    """,
    project_urls={
        "Issues": "https://github.com/Florents-Tselai/pgllm/issues",
        "CI": "https://github.com/Florents-Tselai/pgllm/actions",
        "Changelog": "https://github.com/Florents-Tselai/pgllm/releases",
    },
    license="Apache License, Version 2.0",
    version=VERSION,
    packages=["pgllm"],
    install_requires=["click", "setuptools", "pip", "pypgconfig", "psycopg[binary,pool]", "llm", "tqdm"],
    extras_require={"test": ["pytest", "pytest-cov", "black", "ruff", "click", "pgxnclient"]},
    python_requires=">=3.7"
)
