#requireadmin
#NoTrayIcon
;#include "lib\chmdecoderjoiner.au3"
#include "lib\chmtocparser.au3"
#include "lib\envhandler.au3"
#include "lib\ConfigHandler.au3"
DirRemove("Temp", 1)
DirCreate("Temp")
DirRemove(@ScriptDir & "\app\History", 1)
SetPath(@ScriptDir & "\app\", "add")
Global $configFile = @ScriptDir &"\Config.pnz"
GenConfig($configFile)
$message = "Select an ebook file PDF HTML LIT PDB or CHM to convert."
$clitProgram = "app\clit.exe -d"
$mobigenProgram = "app\mobigen.exe "
If (FileExists($configFile)) Then
	$configRead = ReadConfig($configFile)
	If ($configRead[1] == true) Then
		$pdftohtmlProgram = "app\pdftohtml.exe -c -noframes "
	Else
		$pdftohtmlProgram = "app\pdftohtml.exe -i -c -noframes "
	EndIf
EndIf
;$pdftohtmlProgram = "app\pdftohtml.exe -c -noframes "
$pdbtotxtProgram = "app\abcpalm.exe "
$chmdecoder = "app\chmdecoder.exe -menu "
$metadataProgram = "app\mobi2mobi.exe --title "
If $CmdLine[0] = 0 Then
$fileIn = FileOpenDialog($message, @MyDocumentsDir & "\", "ebook (*.pdf;*.html;*.htm;*.lit;*.pdb;*.chm;*.mobi)", 0 )
If @error Then
MsgBox(4096,"Error","No File Chosen - Exitting without conversion.")
exit
EndIf
$fileInDir = @WorkingDir
;this is the full filename with location
$fileIn = StringReplace($fileIn, "|", @CRLF)
$fileExt = StringRight($fileIn, 4)
$fileExt = StringReplace($fileExt , ".", "")
;This is the filename with location minus the extension
$fileName = StringTrimRight($fileIn, StringLen($fileExt)+1)
$fileName2 = StringReplace($fileIn, $fileInDir, "")
;This is just the filename no extension
$fileName2 = StringTrimRight($fileName2, StringLen($fileExt)+1)
$fileExt = StringLower($fileExt)
$fileOut = GetFileOut($fileInDir)
ElseIf $CmdLine[0] = 2 Then
$fileIn = $CmdLine[1]
;this is the full filename with location
$fileIn = StringReplace($fileIn, """", "")
$fileExt = StringRight($fileIn, 4)
$fileExt = StringReplace($fileExt , ".", "")
;This is the filename with location minus the extension
$fileName = StringTrimRight($fileIn, StringLen($fileExt)+1)
$fileNoQuoteIn = $fileName
$fileReadCount = 1
$fileNameRead = StringRight($fileNoQuoteIn, $fileReadCount)
$fileCharRead = $fileNameRead
While $fileCharRead <> "\"
$fileNameRead = StringRight($fileNoQuoteIn, $fileReadCount)
$fileCharRead = StringLeft($fileNameRead, 1)
$fileReadCount = $fileReadCount + 1
WEnd
; at the end of the loop we will always be one away from the goal.
; this will put us back into position
$fileReadCount = $fileReadCount - 1
$fileInDir = StringTrimRight($fileNoQuoteIn, $fileReadCount)
$fileName2 = StringReplace($fileIn, $fileInDir, "")
;This is just the filename no extension
$fileName2 = StringTrimRight($fileName2, StringLen($fileExt)+1)
$fileExt = StringLower($fileExt)
$fileOut = $CmdLine[2]
If $fileExt == "pdf" OR $fileExt == "pdb" OR $fileExt == "chm" OR $fileExt == "html" OR $fileExt == "htm" OR $fileExt == "lit" Then
Else
ConsoleWriteError ("Sorry The File Type !! "& $fileExt &" !! Is Not Supported Yet" @CRLF)
exit
EndIf
ConsoleWrite("Converting File " & $fileIn & "\n")
ConsoleWrite("To .mobi \n")
ConsoleWrite("Output is " & $fileOut & "\\" & $fileName2 & ".mobi")
Else
$fileIn = $CmdLineRaw
;this is the full filename with location
$fileIn = StringReplace($fileIn, """", "")
$fileExt = StringRight($fileIn, 4)
$fileExt = StringReplace($fileExt , ".", "")
;This is the filename with location minus the extension
$fileName = StringTrimRight($fileIn, StringLen($fileExt)+1)
$fileNoQuoteIn = $fileName
$fileReadCount = 1
$fileNameRead = StringRight($fileNoQuoteIn, $fileReadCount)
$fileCharRead = $fileNameRead
While $fileCharRead <> "\"
$fileNameRead = StringRight($fileNoQuoteIn, $fileReadCount)
$fileCharRead = StringLeft($fileNameRead, 1)
$fileReadCount = $fileReadCount + 1
WEnd
; at the end of the loop we will always be one away from the goal.
; this will put us back into position
$fileReadCount = $fileReadCount - 1
$fileInDir = StringTrimRight($fileNoQuoteIn, $fileReadCount)
$fileName2 = StringReplace($fileIn, $fileInDir, "")
;This is just the filename no extension
$fileName2 = StringTrimRight($fileName2, StringLen($fileExt)+1)
$fileExt = StringLower($fileExt)
$fileOut = GetFileOut($fileInDir)
If $fileExt == "mobi" OR $fileExt == "pdf" OR $fileExt == "pdb" OR $fileExt == "chm" OR $fileExt == "html" OR $fileExt == "htm" OR $fileExt == "lit" Then
Else
MsgBox(0, "Sorry", "Sorry The File Type !! "& $fileExt &" !! Is Not Supported Yet")
exit
EndIf
EndIf

$tempFolder = @ScriptDir & "\Temp"
Select
Case $fileExt == "html" OR $fileExt == "htm"
		ProgressOn("","Converting Ebook", "", -1, -1,1)
		ProgressSet( 10, "Converting From HTML to Mobi", "Converting Ebook")
		RunWait(@ComSpec & " /c " & $mobigenProgram &""""&$fileIn&"""", @ScriptDir, @SW_HIDE)
		ProgressSet( 50, "Moving Files", "Converting Ebook")
		Sleep(1000)
		FileMoveTest($fileName, $fileOut)
		;FileMove($fileName&".mobi", $fileOut, 1)
		ProgressSet( 100, "Complete", "Converting Ebook")
		Sleep(2000)
		ProgressOff()
	Case $fileExt == "lit"
		;MsgBox(0,"LIT Files", "Warning some lit files will change their names to the title from inside the file.  If this happens it will generate an html file in the same directory as the source file. Just run the converter again on this html file to complete the process",10)
		ProgressOn("","Converting Ebook", "", -1, -1,1)
		ProgressSet( 10, "Converting From .lit to HTML", "Converting Ebook")
		$runThis = $clitProgram & " " & chr(34) & $fileIn & chr(34) & " " & chr(34) & @ScriptDir &"\Temp" & chr(34)
		RunWait(@ComSpec & " /c " & $runThis, @ScriptDir, @SW_HIDE)
		ProgressSet( 30, "Converting from HTML to Mobi", "Converting Ebook")
		;MobiIt(@ScriptDir &"\Temp"&$fileName2&".htm", @ScriptDir &"\Temp"&$fileName2)
		$htmlFile = @ScriptDir &"\Temp"&$fileName2&".htm"
		$htmlFileName = @ScriptDir &"\Temp"&$fileName2
		RunWait(@ComSpec & " /c " & $mobigenProgram &""""&$htmlFile&"""", @ScriptDir, @SW_HIDE)
		ProgressSet( 70, "Moving Files", "Converting Ebook")
		FileMoveTest($htmlFileName, $fileOut)
		Sleep(1000)
		ProgressSet( 100, "Complete", "Converting Ebook")
		Sleep(2000)
		ProgressOff()
	Case $fileExt == "pdf"
		ProgressOn("","Converting Ebook", "", -1, -1,1)
		ProgressSet( 10, "Converting From PDF to HTML", "Converting Ebook")
		RunWait(@ComSpec & " /c " & $pdftohtmlProgram &""""&$fileIn&""" "&""""&$tempFolder&$fileName2&".html""", @ScriptDir, @SW_HIDE)
		$htmlFile = $tempFolder&$fileName2&".html"
		ProgressSet( 30, "Converting From HTML to Mobi", "Converting Ebook")
		RunWait(@ComSpec & " /c " & $mobigenProgram &""""&$htmlFile&"""", @ScriptDir, @SW_HIDE)
		ProgressSet( 70, "Moving Mobi File To Target", "Converting Ebook")
		Sleep(1000)
		FileMoveTest($tempFolder&$fileName2, $fileOut)
		ProgressSet( 90, "Moving Intermediate Files to Temp", "Converting Ebook")
		Sleep(500)
		;FileMove($fileName&"_ind.html", $tempFolder)
		;FileMove($fileName&"s.html", $tempFolder)
		;FileMove($filename&".html", $tempFolder)
		ProgressSet( 100, "Complete", "Converting Ebook")
		Sleep(2000)
		ProgressOff()
	Case $fileExt =="pdb"
		ProgressOn("","Converting Ebook", "", -1, -1,1)
		ProgressSet( 10, "Converting From PDB to HTML Using ABC Palm Converter", "Converting Ebook")
		$htmlFile = @ScriptDir &"\Temp"&$fileName2&".html"
		$htmlFileName = @ScriptDir &"\Temp"&$fileName2
		;MsgBox(0,"",@ComSpec & " /c " & $pdbtotxtProgram &""""&$fileIn&""" "&""""&$htmlFile&"""")
		RunWait(@ComSpec & " /c " & $pdbtotxtProgram &""""&$fileIn&""" "&""""&$htmlFile&""" "&"1", @ScriptDir, @SW_HIDE)
		ProgressSet( 30, "Converting From HTML to MOBI", "Converting Ebook")
		RunWait(@ComSpec & " /c " & $mobigenProgram &""""&$htmlFile&"""", @ScriptDir, @SW_HIDE)
		ProgressSet( 70, "Moving Mobi File To Target", "Converting Ebook")
		Sleep(1000)
		HandleTitle($fileName &".mobi", $fileName2, $metadataProgram, "n")
		FileMoveTest($htmlFileName, $fileOut)
		Sleep(1000)
		ProgressSet( 100, "Complete", "Converting Ebook")
		Sleep(2000)
		ProgressOff()
	Case $fileExt =="chm"
		ProgressOn("","Converting Ebook", "", -1, -1,1)
		ProgressSet( 10, "Converting From Chm to Multiple Html Files Using GrindinSoft CHM Decoder", "Decoding")
		$htmlFile = @ScriptDir &"\Temp"
		$htmlFileName = @ScriptDir &"\Temp"&$fileName2
		RunWait(@ComSpec & " /c " & $chmdecoder &""""&$fileIn&""" "&""""&$htmlFile&"""", @ScriptDir, @SW_DISABLE) 
		;decodechmjoiner($fileName2, $tempFolder)
		ProgressSet( 30, "Creating Ordered Index of Book", "Processing")
		chmtocparser($fileName2, $tempFolder)
		ProgressSet( 30, "Joining Files Based on Index", "Joining")
		RunWait(@ComSpec & " /c " & $mobigenProgram &""""&$htmlFileName&".html"&"""", @ScriptDir, @SW_HIDE)
		ProgressSet( 70, "Converting File to Mobi and Moving to Target", "Converting Ebook")
		Sleep(1000)
		FileMoveTest($htmlFileName, $fileOut)
		Sleep(2000)
		Progressoff()
	Case $fileExt == "mobi"
		HandleTitle($fileIn, $fileName2, $metadataProgram, "!")
EndSelect


Func FileMoveTest($outPutFile, $fileOut)
; Check if file opened for reading OK
$fileCheck = FileOpen($outPutFile&".mobi", 0)
If $fileCheck = -1 Then
	ProgressOff()
	If $CmdLine[0] = 2 Then
		ConsoleWrite ("Output File not Created Document may have Copy Protection and Cannot be Processed" & @CRLF)
	Else
		MsgBox(0, "Output File not Created", "Document may have Copy Protection and Cannot be Processed")
	;MsgBox(0, $fileCheck, $fileCheck)
	EndIf
Else
	FileClose($fileCheck)
	$filemoveCheck = FileMove($outPutFile&".mobi", $fileOut, 1)
	if $filemoveCheck = 0 Then
		ProgressOff()
		If $CmdLine[0] = 2 Then
			ConsoleWrite("Output File Cannot be Moved Output Filename may be Incorrect. Before Trying Again Look Here "&$outPutFile & @CRLF)
		Else
		;MsgBox(0, "Error", $filemoveCheck)
		MsgBox(0, "Output File Cannot be Moved", "Output Filename may be Incorrect. Before Trying Again Look Here "&$outPutFile)
		;MsgBox(0, $outPutFile&".mobi", $fileOut, 0)
	EndIf
	Else
		FileMove($outPutFile&".mobi", $fileOut, 1)
		;MsgBox(0, $outPutFile&".mobi", $fileOut, 0)
	EndIf
EndIf	
FileClose($fileCheck)
EndFunc

Func GetFileOut($fileInDir)
	If (FileExists($configFile)) Then
		$configRead = ReadConfig($configFile)
		If ($configRead[0] == "input") Then
			$fileOut = $fileInDir
		ElseIf ($configRead[0] == "default") Then
			$fileOut = FileSelectFolder ( "Select The Destination (Kindle) Folder", "",1)
			If @error Then
				if Msgbox(1, "Default Location", "Do you want to use the same directory as the source file?") == 2 Then
					MsgBox(4096,"Error","No folder chosen - Exitting without conversion")
					exit
				Else
					$fileOut = $fileInDir
				EndIf
			EndIf
		Else
			$fileOut = $configRead[0]
		EndIf
	EndIf
	Return $fileOut
EndFunc

Func HandleTitle($mobiFileLoc, $title, $metadataProgram, $prompt)
	If $prompt == "!" Then
		; the default $filename string has a leading / this removes it
		If StringLeft($title , 1) == "\" Then
			$title = StringTrimLeft($title, 1)
		EndIf
		$title = InputBox("Adjust Metadata Title", "Enter a New Title (Will Show Up This Way on your Kindle)", $title);
	EndIf
	$title = StringReplace($title, "_", " ")
	RunWait(@ComSpec & " /c " & $metadataProgram &"""" & $title & """" & " """ & $mobiFileLoc & """ " & "--outfile """ & $mobiFileLoc & """", @ScriptDir, @SW_HIDE)
	MsgBox(0,"Title", "Title Changed", 2)
EndFunc