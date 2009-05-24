Func chmtocparser ($inputfileName, $tempFolder)
	$filecount = 0
	$indexfile = "___left.htm"
	$inputfileloc = $tempFolder &"\"& $inputfileName &"\"& $indexfile
	$tocfile = $tempFolder &"\temptoc.txt"
	$inputfile = FileOpen($inputfileloc, 0)
	$outputfile = FileOpen($tocfile, 2)
	While 1
	$line = FileReadLine($inputfile)
	If @error = -1 Then ExitLoop
	$line = StringRegExpReplace($line, "<.*.>", "")
	$line = StringRegExpReplace($line,"<!--","")
	$line = StringRegExpReplace($line,"//-->","")
	$line = StringReplace($line, "d = new dTree(""d"");","")
	$line = StringReplace($line, "document.write(d);","")
	$line = StringReplace($line, "%20", " ")
	if $line Not = "" Then
	$line = StringSplit($line, """")
	if $line[4] Not = "" Then
	$line = $line[4]
	If StringInStr($line, "#") Not = 0 Then 
	;$stringlength = StringLen($line)
	;$anchorpos = StringInStr($line, "#")
	;$line = StringTrimRight($line, $stringlength - $anchorpos)
	Else
	FileWriteLine($outputfile, $line)
	$filecount = $filecount + 1
	EndIf
	EndIf
	EndIf
	WEnd
	FileClose($outputfile)
	FileClose($inputfile)
	chmtocjoiner($inputfilename, $tocfile, $filecount, $tempFolder)
EndFunc

Func chmtocjoiner ($inputfilename, $tocfile, $filecount, $tempFolder)
	$inputfile = FileOpen($tocfile, 0)
	$outputfile = FileOpen($tempFolder &"\"& $inputfilename &".html", 2)
	FileWriteLine($outputfile, "<body>")
	While 1
		$line = FileReadLine($inputfile)
		If @error = -1 Then ExitLoop
		$interinput = FileOpen($tempFolder &"\"& $inputfilename &"\"& $line, 0)
		While 1
			$interline = FileReadLine($interinput)
			If @error = -1 Then ExitLoop
			$interline = StringRegExpReplace($interline, "(<body>)", "")
			$interline = StringRegExpReplace($interline, "(</body>)", "")
			If StringInStr($interline, "<head>") Not = 0 Then
			While StringInStr($interline, "</head>") == 0
			$interline = FileReadline($interinput)
			WEnd
			else
			FileWriteLine($outputfile, $interline)
			EndIf
		WEnd
		FileClose($interinput)
	WEnd
	FileWriteLine($outputfile, "</body>")
	FileClose($outputfile)
EndFunc