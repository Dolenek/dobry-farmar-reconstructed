; Recovered original removal function, instructions 47..74 of NSIS 2.28.
; Evidence: analysis/uninstall-program.json; see docs/UNINSTALL.md.
; Included by the source Uninstall.exe build. Its destructive effects may run
; only inside the separately verified native test sandbox during reconstruction.

Var OriginalRegStartMenu
Var OriginalMenuPath
Var OriginalPlayerName
Var OriginalPlayerMenu

Function un.RecoveredOriginalRemoval
  ; 47: recursively removes the original installation directory.
  RMDir /r "$INSTDIR"
  ; 48..52: registry start menu folder, fallback to the game's name.
  ReadRegStr $OriginalRegStartMenu HKCU "Software\Alawar\VirtualFarm" "Start Menu Folder"
  StrCmp $OriginalRegStartMenu "" original_default_menu
  StrCpy $OriginalMenuPath $OriginalRegStartMenu
  Goto original_remove_shortcuts
original_default_menu:
  StrCpy $OriginalMenuPath "Dobrý farmář"
original_remove_shortcuts:
  ; 53..58: program folder links and four desktop links.
  Delete "$SMPROGRAMS\$OriginalMenuPath\*.lnk"
  Delete "$DESKTOP\Dobrý farmář.lnk"
  Delete "$DESKTOP\Špidla Data Processing, s.r.o..lnk"
  Delete "$DESKTOP\Hry zdarma ke stažení.lnk"
  Delete "$DESKTOP\Pohádky zdarma ke stažení.lnk"
  StrCpy $OriginalMenuPath "$SMPROGRAMS\$OriginalMenuPath"
original_remove_parent:
  ; 59..63: remove empty program menu directories up to, excluding, SMPROGRAMS.
  ClearErrors
  RMDir "$OriginalMenuPath"
  GetFullPathName $OriginalMenuPath "$OriginalMenuPath\.."
  IfErrors original_player_menu
  StrCmp $OriginalMenuPath "$SMPROGRAMS" original_player_menu original_remove_parent
original_player_menu:
  ; 64..71: optional Alawar player shortcut and its empty Games directory.
  ClearErrors
  ReadRegStr $OriginalPlayerName HKCU "Software\Alawar\VirtualFarm" "PlayerName"
  IfErrors original_remove_registry
  ReadRegStr $OriginalPlayerMenu HKCU "Software\Alawar\$OriginalPlayerName" "Start Menu Folder"
  IfErrors original_remove_registry
  Delete "$SMPROGRAMS\$OriginalPlayerMenu\Games\Dobrý farmář.lnk"
  RMDir "$SMPROGRAMS\$OriginalPlayerMenu\Games"
  ClearErrors
original_remove_registry:
  ; 72: /ifempty; 73: unconditional deletion of the HKLM uninstall registration.
  DeleteRegKey /ifempty HKCU "Software\Alawar\VirtualFarm"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Dobrý farmář"
  Return
FunctionEnd
