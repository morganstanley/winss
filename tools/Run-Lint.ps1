$srcDir = Resolve-Path $(Join-Path "$PSScriptRoot" "..")
$buildDir = Join-Path $srcDir "build"

if ($args[0])
{
    $items = @($args[0])
}
else
{
    $items = @("$srcDir\bin", "$srcDir\lib\winss", "$srcDir\test")
}

Remove-Item build\lint.txt -ErrorAction Ignore
try
{
    [string] (& "cpplint" "--output=vs7" "--quiet" "--extensions=cpp,hpp" "--headers=hpp" "--filter=-build/c++11" "--recursive" $items 2>&1) | Tee-Object -file "$buildDir\cpplint.out"
    Exit $LastExitCode
}
catch
{
    Exit 1
}
