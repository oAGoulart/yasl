@ECHO OFF

pushd %~dp0

REM Command file for building documentation

py -m pip install --user virtualenv
py -m venv _env

py .\docs\build.py

popd
