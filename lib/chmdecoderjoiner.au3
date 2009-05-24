;$inputfile = FileOpenDialog("File Open", @MyDocumentsDir & "\", "ebook (*.pdf;*.html;*.htm;*.lit;*.pdb)", 0)
;$inputdirectory = @WorkingDir
;$inputfilename = StringReplace($inputfile, "|", @CRLF)
;$inputfilename = StringReplace($inputfilename, $inputdirectory, "")
;$inputfilename = StringTrimRight($inputfilename, 9)
;MsgBox(0, "", $inputdirectory)
;$outputfile = FileOpenDialog("File Open", @MyDocumentsDir & "\", "HTML (*.html)", 1 + 8)

Func decodechmjoiner ($inputfileName, $tempFolder)
;$inputdirectory = @WorkingDir
;$inputfilename = StringReplace($inputfile, "|", @CRLF)
;$inputfilename = StringReplace($inputfilename, $inputdirectory, "")
;$inputfilename = StringTrimRight($inputfilename, 9)
$inputfilestring = $tempFolder & $inputfileName &"\6555final\LiB"
$outputfileName = $tempFolder & $inputfileName &".html"
;msgbox(0, "", $outputfileName)
$outputfile = FileOpen($outputfileName, 1)
FileWriteLine($outputfile, "<body>")
; Check if file opened for reading OK
;msgbox(0, "", $inputfilestring)
Exit
; Read in 1 character at a time until the EOF is reached
$inputfilecounter = 1
$inputfilecounter2 = 1
$inputfilenumber = 1
ProgressOn("","Joining HTML", "", -1, -1, 1)
While 1
	$positionstring = numberofzeros(StringLen($inputfilecounter2))
	$inputfile = FileOpen($inputfilestring & $positionstring& $inputfilecounter2 &".html", 0)
	If $inputfile = -1 Then ExitLoop
		$inputfilenumber = $inputfilenumber + 1
	FileClose($inputfile)
	$inputfilecounter2 = $inputfilecounter2 + 1
WEnd
$filenumpercent = 100 / $inputfilenumber
$line = "Joining HTML"
While 1
	$positionstring = numberofzeros(StringLen($inputfilecounter))
	;MsgBox(0, "",$inputdirectory & $inputfilename & $positionstring& $inputfilecounter &".html")
	ProgressSet($filenumpercent, $inputfilename & $positionstring& $inputfilecounter &".html", $line)
	$inputfile = FileOpen($inputfilestring & $positionstring& $inputfilecounter &".html", 0)
	If $inputfile = -1 Then ExitLoop
	While 1
		$line = FileReadLine($inputfile)
		;$line = StringRegExpReplace($line, "", "")
		If @error = -1 Then ExitLoop
		$line = StringRegExpReplace($line, "(<body>)", "")
		$line = StringRegExpReplace($line, "(</body>)", "")
		FileWriteLine($outputfile, $line)
		;MsgBox(0, "Char read:", $chars)
	WEnd
FileClose($inputfile)
$inputfilecounter = $inputfilecounter + 1
$filenumpercent = $filenumpercent + $filenumpercent
WEnd
FileWriteLine($outputfile, "</body>")
FileClose($outputfile)
ProgressOff()
EndFunc

Func numberofzeros($digitlength)
$zerostring = ""
$digitlength = 4 - $digitlength
for $i = 1 to $digitlength Step 1
		$zerostring = $zerostring &"0"
Next
Return ($zerostring)
EndFunc

;$inputfile = FileOpenDialog("File Open", @MyDocumentsDir & "\", "ebook (*.pdf;*.html;*.htm;*.lit;*.pdb)", 0)
;$outputfile = FileOpenDialog("File Open", @MyDocumentsDir & "\", "ebook (*.pdf;*.html;*.htm;*.lit;*.pdb)", 1 + 8)
