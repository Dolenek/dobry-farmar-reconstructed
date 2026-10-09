# Obnovená cesta načítání prostředků

Win32 knihovna nyní propojuje index souborů, cRealFile/cZipFile, UTF-16 řetězce,
XML strom a JNGLoad. Diagnostika `resource-inspect` není hlavní herní EXE.

## Původní funkce a zachované chování

| Adresy hry | Obnovená část |
| --- | --- |
| `00565510`, `00565ce0`, `00565cf0` | cZipFile: otevření balíku a položky, uložená velikost, zápis vrací 0. Objekt 12 bajtů. |
| `00565d00`, `00565da0`, `00565e00` | ZIP seek, čtení, close. Seek používá offset centrálního adresáře přes unzGetOffset/unzSetOffset, nikoli seek v rozbaleném streamu; close ponechá uloženou velikost. |
| `00565e60`, `005662b0`, `005666b0`, `00566b20` | ZIP finder, společné buffery názvů po 256 bajtech. |
| `0054bd80`, indexování v `0054d720` | Rekurzivní index, ignorování všech jmen začínajících tečkou, pořadí balíků a přednost volných souborů. |
| `005489b0`, `00548de0`, `0054a890` | Existence a otevření s lowercase; dotaz na společný soubor velikost písmen nemění. |
| `0054bbb0`, `0054d5c0` | Překryv společným adresářem, odstranění a obnovení předchozího mapování. `0054bbb0` vkládá chybějící záznamy, nemaže původní mapu. |
| `0054c7d0`, `00549f70`, `0054b4b0`, `00564800` | ANSI vytváření souborů, adresáře pro prefixy končící lomítkem, enumerace společných souborů, převzetí již otevřeného HANDLE. |
| `00401490`, `00401505`–`0040159c`, `004010e0` | Růst WString: SSO 7, maska 7, růst o polovinu při stejných mezích, přesná velikost po první neúspěšné alokaci. |
| `004017c0`, `00401ab0`, `004012d0`, `004d9dc0`, `004df820`, `00401050` | Přiřazení, erase, append, aliasování a unsigned UTF-16 porovnání. Objekt 28 bajtů. |
| `00564010`, `005637b0`, `00563910`, `0055f140`, `0055f1e0` | Načtení XML, vytvoření dětí, callbacky start/end/text. První element představuje dokument, jeho tag se neukládá; děti jsou v seřazené multimapi, opakované sekce zůstávají zachované. Whitespace se netrimuje. |
| `0055df00`, `0055df40`, `0055e860`, `00561320` | Přístup k první pojmenované sekci, enumerace, atribut (chybějící vrací prázdný WString) a text. |

Pořadí názvů balíků je původní byte comparison: `Data.pack` před `cz.pack`.
Pozdější balík přepíše dřívější. Proto český balík nahrazuje mimo jiné
`ui/mainmenu.xml`, `ui/helpview.xml` a `ui/gameui/buyseeds.xml`.
WindowsPath v Pythonu řadí bez rozlišení velikosti písmen, takže validační nástroj
používá explicitní ASCII klíč, nikoli implicitní pořadí Path.

Původní indexář ukládá u nalezeného balíku pouze basename i v podadresáři.
Tato zvláštnost je zachovaná. Pro původní data jsou oba balíky přímo v Data.

ANSI create používá původní GENERIC_READ|GENERIC_WRITE, nulové sdílení,
CREATE_ALWAYS, dědičný handle a NULL DACL. Zapisuje lowercase název po nahrazení
zpětných lomítek. Úspěšné vytvoření obnoví záznam #common# a vyvolá připojenou
invalidaci prostředku. Vlastní celý resource manager ještě není obnoven.

## Závislosti a sestavení

Minizip 1.01 je identifikovaný původním copyright řetězcem `00602c58` a funkcemi
unzGetOffset/unzSetOffset. Šest původních zdrojů z archivu zlib 1.2.3 má připnuté
SHA-256 v `analysis/minizip-source.lock.json`; nemodifikují se.

Expat 1.95.8 je zatím kandidát: původní XML_ErrorString přijímá chyby 1..37,
parser používá XML_UNICODE_WCHAR_T a jeho tvorba odpovídá sledovanému zdroji.
Nelze tím prohlásit úplnou shodu verze. Oficiální commit a 29 Git blob hashů
jsou v `analysis/expat-source.lock.json`, licence zůstává v lokálním snapshotu.
Sestavení používá XML_STATIC, XML_UNICODE_WCHAR_T, COMPILED_FROM_DSP.

```powershell
python tools/fetch_minizip.py
python tools/fetch_expat.py
cmake -S . -B build/windows -A Win32
cmake --build build/windows --config Release --parallel 4
ctest --test-dir build/windows -C Release --output-on-failure
python tools/validate_string.py --wide
python tools/validate_resources.py --reference-decoder build/windows/Release/jng-reference-decode.exe --file-index-test build/windows/Release/file-index-test.exe
```

Ve zdejším omezeném prostředí MSBuild uvízl při kontrole globů a sandbox odmítal
přístup k některým sestaveným EXE. Sestavení a nativní testy proto používají
schválené spuštění mimo sandbox. Není to nedostupnost MSVC či SDK.

## Ověření a zbývající práce

- Všechny 552 dostupné soubory přes nový index a iFile mají přesně stejné bajty
  jako rozbalení původních archivů. Adresáře a tři překryté XML položky se
  nepovažují za další dostupné soubory.
- Všech 66 dostupných XML vytvoří shodný strom proti nezávislému Python Expat
  event modelu. To není dynamické porovnání s původním cXML.
- Všech 416 obrázků načtených přes nový index/ZIP mají stejné rozměry a BGRA
  bajty jako původní JNGLoad.dll.
- WString compare: 20 000 scénářů s celým rozsahem 16bitových jednotek, SSO i
  heap, prázdnými úseky a vloženými NUL souhlasí s původní čistou rutinou.
- Osm nezávislých index/XML fixtures a 12 kontrol vytvoření, invalidace,
  čtení, enumerace a odstranění souborů prošlo v izolované lokální složce.

Experimentální `xml-reference-test` ještě nefunguje v tomto adresním prostoru.
Originál má odstraněné relokace; LoadLibraryEx jej mapuje jinam. Přímé mapování
na 00400000 selhalo s Win32 487, protože tam už leží MEM_MAPPED oblast. Pokus
rezervovat adresu testovací DLL selhal s NTSTATUS C0000018. Tato diagnostika je
vypnutá ve výchozím sestavení (`BUILD_EXPERIMENTAL_XML_ORACLE=OFF`) a není
započtená mezi úspěšná ověření. Importy ani vstupní bod původní hry se v ní
nespouštějí; pomocná DLL není součástí rekonstruované implementace.

XML retry po chybě v souboru #common# původně nečistí částečný strom ani sdílený
zásobník callbacků. Ověřený fixture selže před začátkem prvního elementu a
načte původní položku po smazání poškozeného překryvu. Chyby uvnitř částečně
otevřených elementů, dangling parser pointers a další chybové vstupy nejsou
ověřené. Tento problém nesmí být zakryt tvrzením o úplné shodě parseru.

FileIndex není celý cFileManager singleton: výběr COMMON_APPDATA, celé engine
ABI, Unicode create overload a původní exception wrapping zbývají. WString
má obnovený základ, nikoli všechny operátory a engine konverze. Další práce
vede přes settings/resource manager, typed XML přístup, engine/okno, Direct3D
8, input a skutečné menu; současná diagnostika tento běh nenahrazuje.
