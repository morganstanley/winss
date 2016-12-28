if ($args[0])
{
    $items = ($args[0])
}
else
{
    $items = ("bin","lib\winss","test")
}

Remove-Item build\lint.txt -ErrorAction Ignore
try
{
    [string] (& "cpplint" "--extensions=cpp,hpp" "--headers=hpp" "--filter=-legal/copyright,-build/c++11" "--recursive" $items 2>&1) | Tee-Object -file build\lint.txt
    Exit $LastExitCode
}
catch
{
    Exit 1
}
