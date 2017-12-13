$srcDir = Resolve-Path $(Join-Path "$PSScriptRoot" "..")
$buildDir = Join-Path $srcDir "build"

Remove-Item "$buildDir\testresults.xml" -ErrorAction Ignore
Remove-Item "$buildDir\coverage.xml" -ErrorAction Ignore

try
{
    & "OpenCppCoverage.exe" "--modules=winss" "--sources=lib\winss\*" "--export_type=cobertura:$buildDir\coverage.xml" "-q" "--" "$buildDir\bin\x64\Debug\winss-test.exe" "--gtest_output=xml:$buildDir\testresults.xml"
    Exit $LastExitCode
}
catch
{
    Exit 1
}
