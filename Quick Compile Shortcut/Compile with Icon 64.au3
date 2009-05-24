$output = StringTrimRight($CmdLine[1],4)
$runThis = "lib\Aut2exe.exe /in "& $CmdLineRaw &" /out """& $output & "64.exe"" /icon ""lib\Icons\Onibari\Ico\Library folder.ico"" /x64"
;Msgbox(0, "", $runThis)
RunWait(@ComSpec & " /c " & $runThis, @ScriptDir, @SW_ENABLE)