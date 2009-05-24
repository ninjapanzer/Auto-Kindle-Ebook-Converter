;$configFile = @ScriptDir &"\Config.pnz"

;ReadConfig($configFile)
func GenConfig($configfile)
		If(Not FileExists($configfile))Then
			ShellExecuteWait(@ScriptDir &"\Auto Kindle Config.exe")
		EndIf
EndFunc



func ReadConfig($configfile)
	dim $configResults[2]
	If (FileExists($configfile)) Then
		$configFile = FileOpen($configfile, 0)
		While 1
			$configInput = FileReadLine($configfile)
			If @error = -1 Then ExitLoop
			$configElement = StringSplit($configInput, ":" , 2)
			if ($configElement[0] == "OutputLoc") Then
                        If ($configElement[1] == "UserDefine") Then
							$configResults[0] = FileReadLine($configfile)
                        ElseIf ($configElement[1] == "InputLocation") Then
							$configResults[0] = "input"
                        ElseIf ($configElement[1] == "Default") Then
							$configResults[0] = "default"
						EndIf
			ElseIf ($configElement[0] == "PDFImage") Then
                        If ($configElement[1] == "true") Then
							$configResults[1] = true
                        Else
							$configResults[1] = false
						EndIf
			EndIf
		WEnd
				
	EndIf
	return $configResults 
EndFunc