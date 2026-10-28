; Reconstructed NSIS application script; editable UTF8, build converts to CP1250.
; NSIS framework/compiler/UI/plugin all built from exact2.28 sources.
SetCompressor /SOLID lzma
CRCCheck off
Name "Dobrý farmář"
OutFile "UninstallSourceBuilder.exe"
InstallDir "$EXEDIR"
SilentInstall silent
BrandingText " " ; NSIS prepends one space; original runtime label is two spaces
!include "MUI.nsh"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\win.bmp"
!define MUI_UNICON "uninstall-artwork.ico"
!define MUI_ICON "uninstall-artwork.ico" ; only the offline generation helper needs matching icon slots
!define MUI_LANGDLL_REGISTRY_ROOT HKCU
!define MUI_LANGDLL_REGISTRY_KEY "Software\Alawar\VirtualFarm"
!define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"
!define MUI_PAGE_CUSTOMFUNCTION_SHOW un.RecoveredConfirmShow
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "Czech"
!include "original_removal.nsh"
Section
 WriteUninstaller "$EXEDIR\Uninstall.exe"
SectionEnd
Section "Uninstall"
 Call un.RecoveredOriginalRemoval
SectionEnd
Function un.onInit
 IfSilent init_done
 InitPluginsDir
 File /oname=$PLUGINSDIR\special_offer_page.ini "special_offer_page.ini"
 WriteINIStr "$PLUGINSDIR\special_offer_page.ini" "Settings" "RTL" "$(^RTL)"
 !insertmacro MUI_UNGETLANGUAGE
 init_done:
FunctionEnd

; Original callback104..109, called by the confirmation page show callback6.
Function un.RecoveredConfirmShow
 StrCmp "" "0" confirm_show_done
 Push $R0
 GetDlgItem $R0 $HWNDPARENT 3
 ShowWindow $R0 0
 Pop $R0
 confirm_show_done:
FunctionEnd
