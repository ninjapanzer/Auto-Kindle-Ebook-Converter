DirRemove("Temp", 1)
DirCreate("Temp")
$message = "Select an ebook file PDF HTML or LIT to convert."
$clitProgram = "app\clit.exe -d"
$mobigenProgram = "app\mobigen.exe "
$pdftohtmlProgram = "app\pdftohtml.exe -i "
$fileIn = FileOpenDialog($message, @ScriptDir & "\", "ebook (*.pdf;*.html;*.htm;*.lit)", 1 + 4 )
If @error Then
MsgBox(4096,"Error","No File Chosen - Exitting without conversion.")
exit
EndIf
$fileIn = StringReplace($fileIn, "|", @CRLF)
$fileExt = StringRight($fileIn, 4)
$fileExt = StringReplace($fileExt , ".", "")
$fileName = StringTrimRight($fileIn, StringLen($fileExt)+1)
;MsgBox(0,$fileIn,$fileName)
$fileExt = StringLower($fileExt)
;MsgBox(0,$fileExt,$fileIn)
$fileOut = FileSelectFolder ( "Select The Destination (Kindle) Folder", "",1)
If @error Then
MsgBox(4096,"Error","No folder chosen - Exitting without conversion")
exit
EndIf
$tempFolder = "Temp"
ProgressOn("","Converting Ebook", "", -1, -1,1)
Select
	Case $fileExt == "html" OR $fileExt == "htm"
		ProgressSet( 10, "Converting From HTML to Mobi", "Converting Ebook")
		RunWait(@ComSpec & " /c " & $mobigenProgram &""""&$fileIn&"""", @ScriptDir, @SW_HIDE)
		ProgressSet( 50, "Moving Files", "Converting Ebook")
		Sleep(1000)
		FileMove($fileName&".mobi", $fileOut, 1)
		ProgressSet( 100, "Complete", "Converting Ebook")
		Sleep(2000)
		ProgressOff()
	Case $fileExt == "lit"
		MsgBox(0,"LIT Files", "Warning some lit files will change their names to the title from inside the file.  If this happens it will generate an html file in the same directory as the source file. Just run the converter again on this html file to complete the process")
		ProgressSet( 10, "Converting From .lit to HTML", "Converting Ebook")
		$runThis = $clitProgram & " " & chr(34) & $fileIn & chr(34) & " " & chr(34) & @ScriptDir & chr(34)
		RunWait(@ComSpec & " /c " & $runThis, @ScriptDir, @SW_HIDE)
		ProgressSet( 30, "Converting from HTML to Mobi", "Converting Ebook")
		MobiIt($fileName&".htm", $fileName)
		ProgressSet( 70, "Moving Files", "Converting Ebook")
		Sleep(1000)
		ProgressSet( 100, "Complete", "Converting Ebook")
		Sleep(2000)
		ProgressOff()
	Case $fileExt == "pdf"
		ProgressSet( 10, "Converting From PDF to HTML Pictures Removed", "Converting Ebook")
		RunWait(@ComSpec & " /c " & $pdftohtmlProgram &""""&$fileIn&"""", @ScriptDir, @SW_HIDE)
		$htmlFile = $fileName&"s.html"
		ProgressSet( 30, "Converting From HTML to Mobi", "Converting Ebook")
		RunWait(@ComSpec & " /c " & $mobigenProgram &""""&$htmlFile&"""", @ScriptDir, @SW_HIDE)
		ProgressSet( 70, "Moving Mobi File To Target", "Converting Ebook")
		Sleep(1000)
		FileMove($fileName&"s.mobi", $fileOut, 1)
		ProgressSet( 90, "Moving Intermediate Files to Temp", "Converting Ebook")
		Sleep(500)
		FileMove($fileName&"_ind.html", $tempFolder)
		FileMove($fileName&"s.html", $tempFolder)
		FileMove($filename&".html", $tempFolder)
		ProgressSet( 100, "Complete", "Converting Ebook")
		Sleep(2000)
		ProgressOff()
EndSelect

Func MobiIt($htmlFile, $htmlFileName)
	;msgbox(0,"",$htmlFile)
     RunWait(@ComSpec & " /c " & $mobigenProgram &""""&$htmlFile&"""", @ScriptDir, @SW_HIDE)
	 FileMove($htmlFileName&".mobi", $fileOut, 1)
EndFunc
