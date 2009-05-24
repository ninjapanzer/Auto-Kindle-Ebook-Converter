$output = StringTrimRight($CmdLine[1],4)
$runThis = "lib\Aut2exe.exe /in "& $CmdLineRaw &" /out """& $output & ".exe"" /icon ""lib\Icons\Onibari\Ico\Library folder.ico"""
;Msgbox(0, "", $runThis)
RunWait(@ComSpec & " /c " & $runThis, @ScriptDir, @SW_ENABLE)