Remove-Item build\testresults.xml -ErrorAction Ignore
Remove-Item build\coverage.xml -ErrorAction Ignore
try
{
    & OpenCppCoverage.exe --modules=winss --sources=lib\winss\* --export_type=cobertura:build\coverage.xml -q -- build\bin\x64\Debug\winss-test.exe --gtest_output=xml:build\testresults.xml
    Exit $LastExitCode
}
catch
{
    Exit 1
}
