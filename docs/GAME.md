# Rekonstrukce hlavní hry

Herní EXE zatím nelze sestavit. Sestavitelná knihovna `recovered_game_utilities` obsahuje základ původního `String` a UTF-16 `WString`, souborovou vrstvu, indexování balíků a XML strom. Diagnostika `resource-inspect` propojuje tyto části s JNGLoad a načítá původní data. Integrace do celé hry a původní engine výjimky nejsou hotové. Adresy, zvláštnosti a výsledky celé načítací cesty jsou v [RESOURCE_LOADING.md](RESOURCE_LOADING.md).

Datová část nastavení, vytváření Windows okna a jeho message pump jsou navázané na tuto načítací cestu. Grafická a resource knihovna přidávají cache, cResource/cTexture/cShader/cFont, skutečný upload/reload textur a obnovené spritové dávky včetně letter geometrie. StringManager načítá české překlady a cView propojuje vlastnictví, viditelnost a registraci UI. Podrobnosti jsou v [SETTINGS.md](SETTINGS.md), [WINDOW_GRAPHICS.md](WINDOW_GRAPHICS.md), [SPRITE_RENDERING.md](SPRITE_RENDERING.md), [NATIVE_IMAGES.md](NATIVE_IMAGES.md), [UI_FONTS.md](UI_FONTS.md) a [UI_CONTROLS.md](UI_CONTROLS.md); celý engine a herní EXE ještě čekají.

## String

`src/game/core/GameString.h` a `String.cpp` obnovují úzký řetězec s 28bajtovým rozložením: první čtyři bajty na +0 mají zatím neověřenou úlohu, 16bajtová unie lokálního textu / ukazatele je na +4, délka na +20 a kapacita na +24. Obnovené rutiny první slot nečtou. Kapacita 15 znamená lokální buffer, od 16 se používá halda. Název hlavičky nekolídá s CRT `string.h` na Windows.

| Adresa | Obnovená operace |
| --- | --- |
| `00401250` | Erase, kontrola offsetu, omezení délky a zachování kapacity. |
| `00401350`–`00401458` | Růst kapacity: zaokrouhlení pomocí OR 15, případně 1,5násobek původní kapacity; při selhání první alokace pokus o přesnou požadovanou velikost. |
| `004016d0` | Přiřazení podřetězce, včetně přiřazení ze stejného objektu. |
| `004019c0` | Přiřazení bajtů, rozpoznání ukazatele dovnitř vlastního obsahu. |
| `00403960` | Porovnání unsigned bajtů s výsledkem -1, 0, +1. |
| `00548260`, `00548330` | Find/rfind s explicitní délkou a původním 32bitovým npos. |
| `005484f0` | Vytvoření podřetězce. |

Rozložení, růst, vlastnictví, překryv při přiřazení, vložené NUL a mezní offsety ověřuje `recovered-string`. Instrukce původního EXE byly kontrolovány pomocí objdump, nejen podle automatického pseudokódu. Zprávy `invalid string position` a `string too long` jsou doložené na `0057edd5` a `0057ee15`.

Windows nástroj `string-reference-test` mapuje původní EXE pomocí `DONT_RESOLVE_DLL_REFERENCES`, nespouští jeho vstupní bod a volá pouze původní find/rfind/compare na našich objektech. `python tools/validate_string.py` nejprve ověří hash konkrétní edice. Prošlo 20 000 deterministických scénářů, celkem 60 000 porovnání, včetně krátkých i haldových řetězců, všech bajtových hodnot, prázdných hledání a vložených NUL. Výsledek je v `.local/string-validation.json`; při přítomnosti lokálního originálu se kontrola spouští i přes CTest.

Jde o obnovený základ, nikoli celý původní typ: další operátory, formátování, úplné engine konverze a kompatibilita výjimek mezi starým a novým MSVC ještě nejsou hotové. Změna velikosti CRT runtime ani shoda objektového rozložení samy neprokazují kompletní binární ABI. Lowercase je dosud ověřen pouze pro ASCII.

## Názvy souborů

| Adresa | Původní metoda | Obnovené chování |
| --- | --- | --- |
| `00549500` | `getFileExt` | Text za poslední tečkou, převedený na malá písmena. Chybějící tečka vrací prázdný text. |
| `00549a60` | `getFileName` | Text za posledním `/`, převedený na malá písmena. Pokud `/` chybí, vrátí celý vstup převedený na malá písmena. |

Identifikaci dokládají původní podpisy metod v řetězcích na `00600e70` a `00600eb8`, jejich odkazy z přilehlých handlerů výjimek a kontrola instrukcí hlavních těl. Vyhledávání odzadu provádí společná rutina `00548330`, podřetězec `005484f0`, převod na malá písmena `00508bb0` s CRT `tolower`.

Metody nepoužívají běžná pravidla moderních filesystem knihoven: `getFileExt("Folder.DOT/File")` vrací `dot/file`; `getFileName("Data\\FARM.JNG")` vrací `data\\farm.jng`. Každá metoda má vlastní sdílený návratový buffer (`00660610`, `00660630`). Následující volání téže metody změní hodnotu dříve vrácené reference. Tato pravidla jsou zachovaná a ověřená testem `recovered-path-names`.

Dosavadní ověření pokrývá ASCII názvy a návratové reference. Chování znaků mimo ASCII při změně CRT locale a porovnání běhu s originálním EXE ještě nejsou ověřené.

## Soubory Windows

`src/game/file_manager/RealFile.cpp` obnovuje úspěšné otevření a I/O jádro `cRealFile`. Vtabulka `00601e40`–`00601e54` dokládá pořadí destruktor, velikost, seek, read, write, close; 32bitový objekt má 8 bajtů a handle na +4.

- Konstruktor `00564820` volá `CreateFileA`, předává přístup a způsob vytvoření beze změny; sdílení READ/WRITE odvozuje ze stejných bitů přístupu. Primární chybový text je zachovaný.
- `00564c90` a `00564ce0` používají 32bitové `GetFileSize` a `SetFilePointer` bez vyššího slova. Chybový sentinel není převáděn na výjimku.
- `00564d40` a `00564da0` ignorují BOOL návrat API a vracejí počet přenesených bajtů. `00564e00` zavře a vynuluje handle; zavřené operace vracejí nulu.

`recovered-real-file` ověřuje binární zápis, seek, EOF/krátké čtení, chybný seek, read-only a write-only soubory, pravidla sdílení, dvojí close a uvolnění handle destruktorem v dočasném adresáři. Tyto metody zatím nebyly dynamicky porovnány s původním EXE. Původní engine balení výjimek a kontextové catch handlery konstruktoru zůstávají neobnovené; dočasný `std::runtime_error` proto není označen za původní exception ABI.

Další rekonstrukce vyžaduje úplné řetězcové operace a engine výjimky, správce prostředků a nastavení, vytvoření okna, inicializaci Direct3D 8 a vstupu, menu, profily a uložené hry, všechny herní systémy a jejich propojení se zvukem Squall. Export funkcí sám o sobě tyto části nenahrazuje.

Pole nyní obsahují zdrojové Growing/Ready/Dead/Ploughed akce a společný ActionProcessor, s FarmAreas prostorovými dotazy a skutečným zavlažováním přes Sprinkle/WaterSource. Přímé porovnání obou x87 kontextů prošlo 4 460 stavy a 128 GPU snímky / 61 440 000 pixely. Větve tutorial21/22/23 a mouse input mají konkrétní nativní porovnání. Úplné Farm UI, FieldTip/SeedPackage, Person a herní EXE dál čekají; adresy, opravy a diagnostické hranice jsou v [FARM_FIELDS.md](FARM_FIELDS.md).

Navazující skutečný [FieldTip](FIELD_TIP.md) je nyní zapojený do pole a přímo porovnaný v PC53/PC24:4 998 tip stavů a společně4 472 field stavů,140 GPU snímků /67 200 000 shodných pixelů, čtyři XML reloady. Úplná regrese prošla80/80 za721,22 s a oba game-audio buildy; současné source/binary hashe i všech17 originálů jsou ověřené. Farm indication a Person zůstávají výslovné hranice.
