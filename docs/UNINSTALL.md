# Obnovení původního odinstalátoru

Původní Uninstall.exe je NSIS 2.28 ANSI s výchozím pořadím instrukcí a solid-LZMA kompresí. Hlavička je na offsetu `0xd800`, rozbalená hlavička má 7848 bajtů; celý solid blok 37747 bajtů. Obsahuje 137 instrukcí, 120 řetězců, jeden odinstalační oddíl a českou jazykovou tabulku (1029).

```powershell
python tools/nsis_extract.py
```

Úplný dekódovaný program se šestiparametrovými instrukcemi a interpretovanými řetězci je v `analysis/uninstall-program.json`. Escape kódy NSIS jsou dekódované včetně českého `ý`; nelze je číst jako prosté nulou oddělené CP1250 řetězce.

Struktury, pořadí instrukcí, expanze řetězců a význam mazacích flagů byly porovnány s původními zdroji NSIS 2.28: [fileform.h](https://github.com/kichik/nsis/blob/v228/Source/exehead/fileform.h), [util.c](https://github.com/kichik/nsis/blob/v228/Source/exehead/util.c), [exec.c](https://github.com/kichik/nsis/blob/v228/Source/exehead/exec.c).

## Co původní program dělá

Oddíl začíná instrukcí 47. Rekonstruovaná aplikační logika je v `src/uninstall/original_removal.nsh`:

1. Rekurzivně odstraní instalační adresář `$INSTDIR`.
2. Z registru HKCU načte jméno složky nabídky Start; při prázdné hodnotě použije „Dobrý farmář“.
3. Odstraní zástupce této složky a zástupce hry, Špidly, her a pohádek na ploše.
4. Odstraňuje prázdné nadřazené složky nabídky Start až k `$SMPROGRAMS`, který samotný nemaže.
5. Pokud jsou nalezeny údaje PlayerName a jeho nabídky Start, odstraní i příslušný hráčský zástupce v podadresáři Games.
6. Odstraní klíč `HKCU\Software\Alawar\VirtualFarm` pouze pokud je prázdný; odstraní odinstalační registraci hry v HKLM.

Další instrukce obsahují nastavení UI, inicializaci plug-in adresáře a jazykového výběru. Řetězce obsahují také nabídku TrialPay. Samotná přítomnost těchto řetězců neprokazuje, že se tato nabídka skutečně zobrazí nebo že program otevírá síťové spojení.

## Zdrojové sestavení

`FarmarUninstall` nyní sestaví `build/windows/Release/Uninstall.exe`. Aplikační kód je upravitelný v `src/uninstall/reconstructed_uninstall.nsi` a `original_removal.nsh`; INI je samostatný textový prostředek. Udržujeme původní názvy/klíče a pořadí operací pro věrnou rekonstrukci. Účinky zatím nejsou nativně ověřené a program se nespouští proti původní instalaci.

Sestavení používá přesné oficiální zdroje [NSIS2.28](https://github.com/nsis-dev/nsis/tree/2473ff63ba8dce55dfc14c48e1fe68112c30a05d): commit2473ff63ba8dce55dfc14c48e1fe68112c30a05d, archiv SHA256d098aa0ec1730a498f85b89c51df6309603083e6f96d242bb9eb2a54fb8186ec a734 jednotlivě ověřovaných souborů v `analysis/nsis-source.lock.json`. Makensis, LZMA/Zlib runtime, základní a header-bitmap Modern UI i LangDLL se skutečně kompilují v MSVC Win32; původní programová binárka se nekopíruje jako implementace. Upstream COPYRIGHT/COPYING zůstávají u připnutých zdrojů. Config odpovídá výchozím Win32 přepínačům2.28; kompatibilitní MFC resource include obsahuje pouze standardní Windows definice. Původní ikona je výtvarný ICO prostředek se šesti bitmapami, žádný strojový kód; původ je v `analysis/uninstall-icon.json`. Header `win.bmp` je bajtově shodný s upstream výtvarným prostředkem.

```powershell
python tools/fetch_nsis.py
cmake -S . -B build/windows -G 'Visual Studio 17 2022' -A Win32 -DBUILD_SQUALL_CANDIDATE=ON
cmake --build build/windows --config Release --target FarmarUninstall --parallel 4
python tools/validate_uninstaller_build.py
```

Makensis vytvoří pouze generovací PE s `WriteUninstaller`. Nástroj `emit_nsis_uninstaller.py` tento příkaz realizuje nad daty: vlastní zdrojový PE header, vlastní compiler-generated icon patches a vlastní uninstall payload. Odpovídá `Source/exehead/exec.c` EW_WRITEUNINSTALLER; nespouští instalátor, odinstalační instrukce, registry ani UI. Tím není při sestavení potřeba spouštět instalační pomocník přes Windows elevation heuristiku. Aplikační texty se při build převádějí z editovatelného UTF8 na původní CP1250 a prostředkům se zachovají doložené FILETIME. Kompilace/link mají `/Brepro`.

## Porovnání sestavení

Finální EXE má65748 bajtů, Win32 PE a pouze systémové importy. Stejný zdroj v nezávislé konfiguraci `build/nsis228-repro` vytvořil všech sedm sledovaných binárek bajtově shodných, včetně `Uninstall.exe` se SHA2564dcaa48d4c063c9e28f5052b6b9890a1c451340f80a929b014961c270ad28261. Postup:

```powershell
cmake -S . -B build/nsis228-repro -G 'Visual Studio 17 2022' -A Win32 -DBUILD_SQUALL_CANDIDATE=ON
cmake --build build/nsis228-repro --config Release --target FarmarUninstall --parallel 4
python tools/validate_uninstaller_build.py --fresh-build-dir build/nsis228-repro
```

Po odstranění rozdílů v číslech proměnných a adresách skoků souhlasí všech27 skutečných mazacích instrukcí i jejich řídicí graf. Souhlasí také všech devět dosažitelných init/UI/page callback grafů včetně volaných helper funkcí, jejich řídicího toku a rozlišených operandů. Souhlasí tři typy stránek, jejich příznaky, české popisky a tlačítka; skrytí Zpět na potvrzovací stránce je obnovené z callback104..109. Původní137 instrukcí zahrnuje14 nulových nedosažitelných míst; source126 má navíc wrapper Call/Return kolem stejné odstraňovací funkce. Tyto počty nejsou zaměňované za identitu celé binárky. Header bitmap a679byte INI souhlasí v každém bajtu, source LangDLL payload odpovídá našemu sestavení a liší se od původní DLL. Porovnání callbacků zachytilo a opravilo i rozdíl o jednu mezeru v branding labelu: NSIS k BrandingText přidává úvodní mezeru. Metadata: `analysis/uninstaller-source-build-validation.json`.

Při kontrole původního NSIS exec.c se opravilo dekódování GETFULLPATHNAME: vstup je argument0, destination variable argument1; původní zobrazení argument1 jako string ukazovalo nesouvisející CommonFilesDir. Doložený mazací skript se tím neměnil. Doplněné jsou také typované UI operandy, page table a header callbacky. Čtečka i offline emitor mají testy, včetně odmítnutí původního odinstalátoru jako vstupu pro generování implementace.

## Izolované nativní účinky pokračují

`tools/uninstall_sandbox/containment_probe.cpp` zatím ověřil jen omezený token nad vlastními kontrolními soubory a dočasnými HKCU klíči. První WRITE_RESTRICTED varianta dovolila smazat vnější kontrolní soubor, a proto nebyla použita pro odinstalátor. Úplná restrikce s vlastním test SID a průnikem existujících čtecích oprávnění systémových souborů již odmítla zápis i smazání vně a dovolila změny uvnitř. Systémové ACL se neměnily; dočasné registry byly uklizené. Oprávnění se [vyhodnocují dvěma současně nutnými access checks](https://learn.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-createrestrictedtoken); přidaný restricting SID nezvyšuje normální práva tokenu.

Tento proof není spuštěný izolovaný uninstall test. Následuje ověření skutečného restricted child procesu, soukromé Windows station/desktop, Job a procesu lokálně mapovaných HKCU/HKLM ([RegOverridePredefKey](https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regoverridepredefkey)). Poté je třeba skutečně porovnat soubory, zástupce, prázdné/neprázdné registry, PlayerName, fallback menu, UI/cancel/completion a bootstrap/self-copy/delayed-delete hranice. Celková odinstalace ani celý projekt nejsou dokončené.
