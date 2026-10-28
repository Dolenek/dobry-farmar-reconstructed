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
6. Odstraní klíč `HKCU\Software\Alawar\VirtualFarm` pokud nemá podklíče; samotné hodnoty jej před smazáním nechrání. Odstraní odinstalační registraci hry v HKLM.

Další instrukce obsahují nastavení UI, inicializaci plug-in adresáře a jazykového výběru. Řetězce obsahují také nabídku TrialPay. Samotná přítomnost těchto řetězců neprokazuje, že se tato nabídka skutečně zobrazí nebo že program otevírá síťové spojení.

## Zdrojové sestavení

`FarmarUninstall` nyní sestaví `build/windows/Release/Uninstall.exe`. Aplikační kód je upravitelný v `src/uninstall/reconstructed_uninstall.nsi` a `original_removal.nsh`; INI je samostatný textový prostředek. Udržujeme původní názvy/klíče a pořadí operací pro věrnou rekonstrukci. Deset silent scénářů má nativně porovnané účinky v omezených testovacích procesech; původní instalace se nepoužívá jako cíl. Interaktivní UI a self-copy/bootstrap pokračují.

Sestavení používá přesné oficiální zdroje [NSIS2.28](https://github.com/nsis-dev/nsis/tree/2473ff63ba8dce55dfc14c48e1fe68112c30a05d): commit2473ff63ba8dce55dfc14c48e1fe68112c30a05d, archiv SHA256d098aa0ec1730a498f85b89c51df6309603083e6f96d242bb9eb2a54fb8186ec a734 jednotlivě ověřovaných souborů v `analysis/nsis-source.lock.json`. Makensis, LZMA/Zlib runtime, základní a header-bitmap Modern UI i LangDLL se skutečně kompilují v MSVC Win32; původní programová binárka se nekopíruje jako implementace. Upstream COPYRIGHT/COPYING zůstávají u připnutých zdrojů. Config odpovídá výchozím Win32 přepínačům2.28; kompatibilitní MFC resource include obsahuje pouze standardní Windows definice. Původní ikona je výtvarný ICO prostředek se šesti bitmapami, žádný strojový kód; původ je v `analysis/uninstall-icon.json`. Header `win.bmp` je bajtově shodný s upstream výtvarným prostředkem.

```powershell
python tools/fetch_nsis.py
cmake -S . -B build/windows -G 'Visual Studio 17 2022' -A Win32 -DBUILD_SQUALL_CANDIDATE=ON
cmake --build build/windows --config Release --target FarmarUninstall --parallel 4
python tools/validate_uninstaller_build.py
```

Makensis vytvoří pouze generovací PE s `WriteUninstaller`. Nástroj `emit_nsis_uninstaller.py` tento příkaz realizuje nad daty: vlastní zdrojový PE header, vlastní compiler-generated icon patches a vlastní uninstall payload. Odpovídá `Source/exehead/exec.c` EW_WRITEUNINSTALLER; nespouští instalátor, odinstalační instrukce, registry ani UI. Tím není při sestavení potřeba spouštět instalační pomocník přes Windows elevation heuristiku. Aplikační texty se při build převádějí z editovatelného UTF8 na původní CP1250 a prostředkům se zachovají doložené FILETIME. Kompilace/link mají `/Brepro`.

## Porovnání sestavení

Finální EXE má65748 bajtů, Win32 PE a pouze systémové importy. Stejný zdroj v nezávislé konfiguraci `build/nsis228-repro` vytvořil všech sedm sledovaných binárek bajtově shodných, včetně `Uninstall.exe` se SHA2562deed97623f957fd07dba370d67c07ee6d5889f69f5f91c9eaf931f6c64308d1. Postup:

```powershell
cmake -S . -B build/nsis228-repro -G 'Visual Studio 17 2022' -A Win32 -DBUILD_SQUALL_CANDIDATE=ON
cmake --build build/nsis228-repro --config Release --target FarmarUninstall --parallel 4
python tools/validate_uninstaller_build.py --fresh-build-dir build/nsis228-repro
```

Po odstranění rozdílů v číslech proměnných a adresách skoků souhlasí všech27 skutečných mazacích instrukcí i jejich řídicí graf. Souhlasí také všech devět dosažitelných init/UI/page callback grafů včetně volaných helper funkcí, jejich řídicího toku a rozlišených operandů. Souhlasí tři typy stránek, jejich příznaky, české popisky a tlačítka; skrytí Zpět na potvrzovací stránce je obnovené z callback104..109. Původní137 instrukcí zahrnuje14 nulových nedosažitelných míst; source126 má navíc wrapper Call/Return kolem stejné odstraňovací funkce. Tyto počty nejsou zaměňované za identitu celé binárky. Header bitmap a679byte INI souhlasí v každém bajtu, source LangDLL payload odpovídá našemu sestavení a liší se od původní DLL. Porovnání callbacků zachytilo a opravilo i rozdíl o jednu mezeru v branding labelu: NSIS k BrandingText přidává úvodní mezeru. Metadata: `analysis/uninstaller-source-build-validation.json`.

Při kontrole původního NSIS exec.c se opravilo dekódování GETFULLPATHNAME: vstup je argument0, destination variable argument1; původní zobrazení argument1 jako string ukazovalo nesouvisející CommonFilesDir. Doložený mazací skript se tím neměnil. Doplněné jsou také typované UI operandy, page table a header callbacky. Čtečka i offline emitor mají testy, včetně odmítnutí původního odinstalátoru jako vstupu pro generování implementace.

## Izolované nativní účinky pokračují

`tools/uninstall_sandbox/containment_probe.cpp` a `tools/validate_uninstall_containment.py` ověřily tři skutečné životní cykly vlastního Win32 kontrolního procesu. Každý používá úplně restricted primary token (vlastní SID, Restricted Code, Builtin Users, odstraněná privilegia, Administrators deny-only) a Mandatory Integrity Untrusted/RID0. Pouze nový vlastní testovací root, soukromé registry a desktop mají Untrusted label. Zápis, DELETE, WRITE_DAC a WRITE_OWNER vně jsou odmítnuté jak pro běžný Medium kontrolní soubor, tak i pro další vlastní Low kontrolní soubor. Skutečné vnější DeleteFile je také odmítnuté; markery zůstávají nezměněné.

Původní varianta WRITE_RESTRICTED byla zamítnutá, protože umožnila vnější smazání. Pouhé přidání Builtin Users do restricting listu by kvůli zděděnému Users Modify v pracovním adresáři také nebylo dostatečné; nový běh spoléhá současně na Untrusted MIC. [Restricted token](https://learn.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-createrestrictedtoken) vyžaduje průchod normálním i restricted access check. Přísnější unique+TrustedInstaller token prošel impersonation testem, ale skutečný primární proces skončil před vlastní funkcí C0000022, po přidání Restricted Code C0000135. Untrusted+Users pak dovolil loaderu číst potřebné prostředky, ale KernelBase attach nejprve selhal C0000142: výchozí ACL nových objektů tokenu muselo výslovně povolit vlastní restricting SID. Po této opravě loader i vytvoření skutečného USER32 okna na oddělené ploše prošly. Neúspěšné varianty nespouštěly odinstalátor.

Pojmenovanou novou Windows station [smí vytvořit administrátorský token](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowstationw); na tomto hostiteli vrací5. Používáme proto existující systémem přidělenou **neinteraktivní** station, výslovně odmítáme WinSta0 a vytváříme unikátní soukromý desktop. Nejde o novou privátní station. Její původní DACL se uloží, dočasně doplní pouze nepředvídatelný testovací SID a následně se obnoví a bajtově ověří. Pracovní plocha uživatele ani její ACL se nemění. Job omezuje běh na jediný proces a ukončí jej při uzavření vlastníka.

Kontrolnímu procesu se přes přesný inherited-handle whitelist předají jen dva privátní registry handle. [RegOverridePredefKey](https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regoverridepredefkey) lokálně mapuje HKCU i HKLM. Skutečné child zápisy do obou úlů byly přečtené pod dočasnou testovací cestou; RegOpenCurrentUser pro změnu skutečného HKCU je odmítnutý. Dočasné klíče byly uklizené. Všech17 původních souborů má původní hash.

```powershell
cmake -S tools/uninstall_sandbox -B build/uninstall-sandbox -G 'Visual Studio 17 2022' -A Win32
cmake --build build/uninstall-sandbox --config Release --parallel 2
python tools/validate_uninstall_containment.py
```

Výsledky a hashe tohoto předpokladu: `analysis/uninstaller-containment-validation.json`. Na hlavní konfiguraci prošlo všech102 CTest (580,88s). Samotné markerové testy nebyly vydávané za odinstalační porovnání; následné skutečné běhy jsou popsány níže.

## Skutečné silent odinstalace: deset párů

`tools/validate_uninstaller_runtime.py` nyní spustil20 úplných odinstalačních procesů: deset scénářů se zdrojovým EXE a stejných deset s nezměněným originálem, pokaždé nad novými vlastními soubory a soukromými registry. Originál je pouze diagnostická reference; naše distribuce jej nekopíruje ani nevolá jako implementaci. Všechny běhy mají stejný ověřený omezený primární token a oddělený desktop. Vlastní DLL nastaví procesově lokální HKCU/HKLM a IAT hranice při zastaveném PE entry pointu; teprve potom se původní vstupní instrukce uvolní. Registrované produkční registry se vůbec nepoužívají. Každý běh má uložený entry proof, úplný raw OS trace a soukromý registry snapshot.

Aplikační ANSI importy pro soubory, registry, load/export, special folders, spuštění procesů, shell/COM a systémové nastavení mají výslovné hranice. Zápisy dovolují jen vlastní root, odmítají parent traversal, alternativní streamy a reparse cesty; WriteFile a SetFileTime kontrolují i skutečnou cestu již otevřeného handle. Special-folder PIDL mají privátní cíle pro user/common Programs a Desktop. Shell/COM spuštění a další procesy jsou pro tyto silent případy zakázané. MoveFileEx s DELAY_UNTIL_REBOOT pouze zaznamená soukromý záměr, nikdy nezapisuje systémovou frontu po restartu.

Tři vlastní guard klienty ověřily také odmítnutí zápisu do zvláštního **Untrusted souboru mimo root**: samotná integrita jej nechrání, takže test ověřuje skutečnou aplikační hranici. Test záměrně získal vnější handle přes vlastní diagnostický W import a následné ANSI WriteFile/SetFileTime jej odmítly. Tento W import původní NSIS nemá; import audit ověřuje pokrytí známé aplikace. Nejde o univerzální sandbox pro libovolný cizí program.

```powershell
cmake --build build/uninstall-sandbox --config Release --parallel 2
python tools/validate_uninstaller_runtime.py
```

Scénáře: chybějící registrace; chybějící i prázdná menu hodnota; výchozí i vlastní český vnořený menu název; odstranění více prázdných předků a zachování neprázdného menu; PlayerName s prázdnou/neprázdnou Games složkou nebo chybějícím menu; VirtualFarm jen s hodnotami a VirtualFarm s podklíčem. Každý payload obsahuje rekurzivní Data strom i hidden/read-only soubor. Všechny ověřují čtyři skutečně odstraněné desktop soubory, zachované nesouvisející soubory/registry, odstranění či nepřítomnost HKLM stromu a správné HKCU chování. Pojem `/ifempty` byl zpřesněný podle `Source/exehead/exec.c::myRegDeleteKeyEx`: smyčka testuje RegEnumKey, nikoli hodnoty. Testy potvrzují smazání values-only klíče i zachování klíče s dítětem; rekonstruovaný runtime už používal tentýž zdrojový NSIS algoritmus, změnila se vysvětlující dokumentace.

Ve všech deseti párech souhlasí konečný souborový strom a obsah soukromých registrů po výslovně evidované normalizaci tří hodnot Windows inicializace: `ZonesSecurityUpgrade` FILETIME a dva dočasné `InstallINFFile` názvy. Ostatní hodnoty zůstávají porovnané bajtově, raw snímky a původní normalizované hodnoty jsou uchované. Souhlasí i všech504 sledovaných aplikačních mazacích/registry/shell operací v pořadí, po nahrazení samotného testovacího rootu. Přípravné zápisy dekomprimovaného NSIS streamu mají rozdílné členění do WriteFile bloků, které se nevydává za shodu; konečný prázdný temp strom souhlasí. Žádná skutečná odinstalace nenarazila na zamítnutou aplikační operaci ani nepotřebovala delayed-delete.

Windows u historického EXE bez moderního elevation manifestu nejprve odmítly CreateProcessAsUser chybou740. Pouze prostředí testovaného dítěte proto obsahuje `__COMPAT_LAYER=RunAsInvoker`; oba programy běží bez elevation pod stejnými omezeními. Běhy mají `/S _?=<vlastní payload>`, čímž vědomě obcházejí bootstrap copy fázi a umožní samostatné porovnání mazací logiky. Nejde o test zvýšení oprávnění, běhu proti Program Files ani přístupů do skutečného HKLM.

Výsledky: `analysis/uninstaller-runtime-validation.json`. Všech17 původních souborů zůstalo bajtově beze změny. Interaktivní potvrzení/zrušení/dokončení, LangDLL při nesilent inicializaci a úplná bootstrap/self-copy/delayed-delete posloupnost dále pokračují. Celý projekt není dokončený.

## Interaktivní porovnání a PE platformní pole

Dalších16 skutečných procesů ověřuje osm source/reference párů: zrušení v LangDLL dialogu, zrušení v potvrzení, úplné dokončení, uložený jazyk1029 a neplatný uložený jazyk65535, vlastní neprázdné menu a neprázdné hráčské Games. Jazykový dialog se skutečně načítá z příslušné vlastní/referenční LangDLL; při uloženém jazyce se přeskočí. Návratové kódy souhlasí: language cancel2, confirmation cancel1, completion0. Zrušení ponechá všechny fixture soubory včetně atributů a všechny aplikační registry beze změny. Dokončení má totožné mazací účinky, soukromé registry a pořadí aplikačních operací.

Všech268 párově sledovaných záznamů ovládacích prvků souhlasí přesně: třída, ID, text (s normalizací testovacího rootu), style, enabled/visible hodnoty a relativní geometrie včetně rozměru celého okna. Observer běží na vlastním vlákně připojeném ke konkrétnímu privátnímu desktopu; posílá běžné WM_COMMAND pouze ověřenému HWND téhož procesu. Desktop se nikdy nepřepíná do uživatelova zobrazení. IsWindowVisible na této neinteraktivní station zahrnuje neviditelného desktop předka, proto readiness používá vlastní WS_VISIBLE dialogu. PrintWindow i WM_PRINT vracejí jednobarevné snímky. Tyto snímky nejsou vydávané za pixelové porovnání; to zůstává výslovně neověřené. Texty, řídicí tok, skutečné prvky a účinky jsou nativně porovnané.

První UI měření zachytilo rozdíl12 pixelů v rozměru okna a6 pixelů v posunech potomků. Moderní LINK vytvořil OS/subsystem verzi6.0 a příznak Terminal Server Aware, zatímco originál má4.0/4.0 a DLL characteristics0. Diagnostická změna právě těchto tří bajtů ve vlastním zdrojovém EXE obnovila přesnou nativní geometrii všech tří dialogů. Sestavení nyní tyto explicitní platformní hodnoty zapisuje pouze do vlastních zdrojově linkovaných LZMA/Zlib stubů před makensis. Vlastní generátor i offline EW_WRITEUNINSTALLER tak dál používají svůj vlastní PE header bez výjimky. Všechny ostatní bajty kódu, prostředků a NSIS payloadu zůstaly totožné. Výsledek má stále65748 bajtů a SHA2562deed97623f957fd07dba370d67c07ee6d5889f69f5f91c9eaf931f6c64308d1; všech sedm binárek v nezávislé konfiguraci je znovu shodných. Pole linker version nadále pravdivě uvádí náš MSVC; původní strojový kód ani hlavička se nekopírují.

Příprava pluginů NSIS zkouší CreateDirectory i pro již existující rodiče. Hranice pro známé existující předky vlastního nově vytvořeného rootu vrací normální ERROR_ALREADY_EXISTS bez jakéhokoli vnějšího mutujícího API volání. Skutečné vytvoření mimo root zůstává odmítnuté. RichEd20/RichEd32 jsou dovolené systémové komponenty, načítané explicitně ze systémové složky. IAT patchování vychází ze skutečné cesty načteného modulu a týká se jen aplikačních modulů uvnitř rootu, nikoli systémových DLL.

```powershell
python tools/validate_uninstaller_ui.py
```

Doklady: `analysis/uninstaller-ui-validation.json`, `analysis/uninstaller-platform-validation.json`. Plný bootstrap bez `_?=` (vlastní copy, další proces a jejich bezpečné delayed-delete záměry) nadále pokračuje. Celkový projekt zůstává nedokončený.
