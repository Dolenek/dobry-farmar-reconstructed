# Ověření shody s originálem

## Referenční běhy

Používat vlastní pracovní kopii původní instalace a oddělenou složku rekonstruované verze. Před každým scénářem zaznamenat verzi/hash binárních souborů, data, nastavení, profil, výchozí uloženou hru a posloupnost vstupů. Původní uživatelský profil se pro testování nepřepisuje.

| Scénář | Co porovnat |
| --- | --- |
| První spuštění, loga, hlavní menu | Pořadí obrazovek, české texty, fonty, rozmístění, klikací oblasti, zvuk. |
| Profily a nastavení | Vytvoření/výběr profilu, hodnoty nastavení, jejich uchování po restartu. |
| Tutoriál | Podmínky přechodů, instrukce, zpřístupněné akce a stav farmy. |
| Pěstování a zavlažování | Náklady, časování, růst, sklizeň, kapacita, chování postavy. |
| Výroba a budovy | Vstupy/výstupy, fronty, ceny, časy, sklad, animace. |
| Trh a zakázky | Ceny, nabídka/poptávka, odměny, časové limity, úspěch/neúspěch. |
| Odměny a postup | Podmínky odemčení a zobrazení statistik. |
| Uložení a načtení | Originál načte naši uloženou hru a naše verze načte původní; hodnoty a postup zůstanou stejné. |
| Ztráta fokusu a změny zobrazení | Klávesnice/myš, pauza, obnovení grafických prostředků a zvuku. |

Stabilní obrazovky porovnávat na stejném systému a nastavení. U animací, zvuku, času a náhodnosti zaznamenat rozdíly více referenčních běhů, než se určí přípustná odchylka. Nepoužívat libovolnou toleranci k zakrytí chyby.

## Knihovny

### Ověřeno nativně na Windows

Win32 Release sestavení v `build/windows` prošlo 35 CTest kontrolami. Python reference testy kontrolují všech 17 původních hashů, CRC archivů a nezávisle porovnávají PE importy/exporty s objdump. Binutils dostává ASCII kopii souboru v dočasném adresáři, protože jeho Windows sestavení neumí zdejší české cesty; původní soubory se nemění. Výchozí test dekodéru na Windows vybírá nové EXE, nikoli starý linuxový soubor.

- JNGLoad: všechny rozměry a BGRA bajty 416 JNG souhlasí s původní DLL; PE architektura, názvy a ordinály jsou ověřené.
- String: 20 000 deterministických scénářů × find/rfind/compare = 60 000 shod s původními rutinami. Objektové rozložení, kapacity a aliasování navíc ověřuje lokální C++ test.
- Color: 121 538 bitových porovnání s originálem; matice: 140 004; sprite geometrie: 40 000. Nativní sprite draw má 108 scénářů, shodná API volání a 51 840 000 shodných RGB pixelů včetně dvanácti lokalizovaných fontových snímků. Podrobnosti a omezení v [SPRITE_RENDERING.md](SPRITE_RENDERING.md) a [UI_FONTS.md](UI_FONTS.md).
- WString: dalších 20 000 přímých porovnání původního unsigned UTF-16 compare; testy SSO, aliasování a self-append.
- Integrované indexování → iFile → XML/JNG: 552 bajtově shodných souborů, 66 XML stromů proti nezávislému Python Expat modelu, 416 pixelově shodných obrázků proti originální DLL. Přímé ověření původního cXML parseru zatím neprošlo kvůli kolizi jeho pevné adresy; kandidát Expat není označen za prokázanou původní verzi. Osm indexovacích fixture scénářů a 12 kontrol zápisu/mazání používá oddělenou složku.
- cRealFile: Windows I/O scénáře ověřené pouze pro rekonstruovanou implementaci v dočasné složce; přímé porovnání původních rutin a engine výjimek ještě chybí.
- Nastavení: 14 integračních kontrol, přesné UTF-16 fixture bajty, 24 položek původních dat; původní cSettings runtime a mezní CRT konverze ještě neporovnané.
- Okno: 18 nativních kontrol vytvoření a zpráv ve skrytém okně, zatím bez skutečných input/main/sound objektů.
- cResource/cTexture/Direct3D8: rozšířené ověření zahrnuje 425 JNG (devět má příponu .dds) i obnovení po uvolnění, 81 014 421 pixelů přesně shodných s původní JNGLoad.dll. Dále deset nativních DDS/TGA/BMP s 86 346 source pixely; původní raw readery souhlasí v 345 368 bajtech, úplný D3DX GPU loader čeká. Podrobnosti v [NATIVE_IMAGES.md](NATIVE_IMAGES.md). GPU diagnostika je samostatná `python tools/validate_textures.py`, ne součást výchozího CTest.
- Fonty: 393 216 přímých lookup porovnání a nezávislá kontrola binárních záznamů; TextLine: 12 000 scénářů a 88 583 geometrií i spritů proti původním metodám. Testové adaptéry neověřují resource lookup/aktivaci prvků. Podrobnosti a zbývající UI integrace v [UI_FONTS.md](UI_FONTS.md).
- StringManager: 10 464 přímých getString porovnání pro 461 českých překladů a chybějící/prázdné vstupy; čtyři vlastní dump fixtures mají evidované chybějící původní runtime porovnání.
- Control/Picture: 400 020 stavů, 24 546 callbacků, 1 822 sprite pairs, 10 764 hit testů, 1 788 původních depth průchodů a dvacet destruktorů ve dvou precision kontextech. TextLine/Letter: 18 000 scénářů, 169 056 vlastněných letter/sprite porovnání a 4 800 RTL porovnání ve dvou precision kontextech, s 600 TextLine destruktory. Testové hranice a neověřené loadXML jsou v UI_FONTS.md a UI_CONTROLS.md.
- Control sprity: 200 000 bitově shodných geometrií/UV/barev, 40 000 úplných animovaných stavů ve dvou x87 precision kontextech a dvojic spritů. cView: 384 024 stavů, 35 826 callback událostí, 32 000 registračních snímků a 24 vlastněných deletion pořadí. Omezení v [UI_CONTROLS.md](UI_CONTROLS.md).

- Tlačítka: 480 024 stavů a 480 000 mask hits; animovaný text: 18 000 scénářů; vlastněné parent/text XML: 2 160 scénářů a 9 786 letters/sprites. Klikací JNG masky: 162 028 842 alpha bitů. DirectInput: 5 340 000 handler setů a 500 snímků přes řízené COM fixtures. Propojená diagnostická scéna menu: 96 snímků, 46 080 000 RGB pixelů a shodná draw API volání včetně hide/show přechodů. Skutečná navigace menu, zvuk a fyzický vstup tím ověřené nejsou; hranice a původní adresy v [UI_CONTROLS.md](UI_CONTROLS.md).

- Časovač: 120 000 původních snímků, 120 200 bitových stavových porovnání, 200 konstruktorů/destruktorů a 30 358 volání Sleep ve dvou přesnostech x87. Menu již používá obnovené vyhlazování; clock/WinMM jsou v oracle řízené. Podrobnosti v [GAME_TIMER.md](GAME_TIMER.md).

- Kurzor: 40 000 původních frame aktualizací, 40 000 bounds a 80 000 transformací, dva vlastní Win32 testovací windows. Menu: 48 vstupních snímků a 24 shodných sound požadavků přes najetí/stisk/uvolnění/odjetí šesti tlačítek. Přenositelné reporty analysis/cursor-validation.json a analysis/text-buttons-validation.json; hranice v [GAME_CURSOR.md](GAME_CURSOR.md).

- Hlavičky profilů: 200 původních konstruktorů s prázdným vybraným jménem, 16 000 header loads, 160 000 lookupů a 800 append-list kontrol. Typované XML konverze a ID formatter mají testové adaptéry; lifecycle je ověřen samostatně; přepínání/mazání a celé uložené hry čekají. Podrobnosti v [PROFILES.md](PROFILES.md).

Souhrn a konkrétní omezení jsou v `analysis/verification.json`, `docs/GAME.md` a `docs/JNGLOAD.md`. Celá hra, Squall a odinstalátor tímto nejsou označené za dokončené.

JNGLoad: porovnat rozměry a dekódované pixely všech obrazových souborů použitých hrou, pořadí kanálů, alfa kanál, nakládání s výstupní pamětí, chybové vstupy a opakovaná volání.

Squall: ověřit všech 97 exportů včetně ABI, chybových kódů, WAV/OGG, start/stop/pauza, hlasitost, frekvence, pozice, smyčky, kanály/skupiny a úklid prostředků. Pro výslednou shodu nestačí pouze zvuk v hlavním menu.

## Odinstalátor

Původní ani rekonstruovaný odinstalátor nespouštět proti původní instalaci. Testovat v odděleném Windows prostředí s připravenými soubory, zástupci a položkami registru. Zaznamenat změny a ověřit, že rekonstruovaná distribuce pracuje pouze se svou instalační cestou a identifikátorem.

## Vydání

Čistý klon musí umožnit sestavení ze zdrojů. Balíček obsahuje pouze rekonstruované binární soubory a dokumentaci; původní data se dodávají lokálně. Kontrola nesmí původní EXE nebo DLL zaměnit za sestavený výsledek. Dokud tyto scénáře neprojdou, není celý projekt označen za funkční rekonstrukci 1:1.

Vlastnosti profilů: všech třináct nativních map, 62 400 přístupů, 67 200 typových volání, 128 492 mapových porovnání a 284 clear operací ve dvou x87 kontextech. Dále 240 celých serializací, 160 native lifecycle scénářů a 544 UTF-16 zápisů; 220 původních inicializací porovnalo všech 790 herních defaultů. Source menu otevírá skutečné profily přes ResourceManager. Starý CRT/XML a celý herní save/load mají evidované hranice v [PROFILES.md](PROFILES.md).

Awards a všech 21 AwardButton nyní propojují obnovované nápovědy, efekty, input a skutečné profilové booleans: 16 632 stavů, 574 992 písmen, 126 potvrzení podmínek ocenění a 156 GPU snímků / 74 880 000 shodných RGB pixelů. Podrobnosti, původní adresy, shodné retained reference a servisní hranice jsou v [AWARDS.md](AWARDS.md). Gameplay award manager/AwardPanel a MainMenu navigation pokračují; hlavní herní EXE ještě není sestavený.
