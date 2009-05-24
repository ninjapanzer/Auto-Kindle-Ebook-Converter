Func SetPath($appPath, $method)
if ($method == "add") Then
	if (FindInPath($appPath) == 0) Then
		$pathEnv = EnvGet("PATH")
		$pathEnv = $pathEnv & "\;" & $appPath
		EnvSet("Path",$pathEnv)
	EndIf
EndIf
if ($method == "del") Then
	if (FindInPath($appPath) == 1) Then
	$pathEnv = EnvGet("PATH")
	$pathEnv = StringRegExpReplace($pathEnv, ";"&$appPath, "")
	MsgBox(1, "set path del", $pathEnv)
	EnvSet("Path",$pathEnv)
	EndIf
EndIf
EnvUpdate();
EndFunc

Func FindInPath($appPath)
$pathEnv = EnvGet("Path")
$test = StringRegExp($pathEnv, $appPath)
if ($test == 1) Then
	return 1
ElseIf ($test == 0) Then
	return 0
EndIf	

EndFunc

