@ECHO OFF

pushd %~dp0

REM Command file for building documentation

curl --output doxygen.zip https://www.doxygen.nl/files/doxygen-1.9.2.windows.x64.bin.zip
tar -x -k -f doxygen.zip

py -m pip install --user virtualenv
py -m venv _env

py .\docs\build.py

popd
