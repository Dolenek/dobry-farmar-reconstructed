# Ověření shody s originálem

[Aktivní trackerový mixer](SQUALL.md): obě sestavení po17856 call párech,8303616 guarded výstupních bajtech a28449792 skalárních bajtech kanálů. Všechny128 tabulkové položky a celý1/2/33kanálový dispatch se smyčkami a rampami souhlasí. Konkrétní DirectToDisk scénář prokazuje skutečnou výjimku z32kanálového limitu; adresní pole jsou ověřená odděleně, nikoli normalizací PCM.

[Skutečné dlouhodobé zavlažení](NO_DRY_AWARD.md): oba celé EXE71 přesunů,60 polí a přirozené ocenění po1200 sekundách bez resetu;480000 přesných RGB pixelů. Všech15808 save bajtů souhlasí s nezávisle zakódovanou původní deníkovou událostí. Čtyři další celé obousměrné přenosy prošly; obě source obnovy bez AV.

[Skutečné souborové callbacky](SQUALL.md): každé ze dvou sestavení24576 kombinací,98304 přesných uložených slotů,30720 guarded PCM čtení/1683456 payload bajtů a18 řízených selhání. PC53/PC24 a tři lifecycle stavy mají samostatná měření FPU; původní neúspěšné diagnostiky se nevydávají za úspěch.

[Skutečný konec příběhu a časová smyčka](CAMPAIGN_LOOP.md): dva celé normálně ukončené procesy,13 kontrolních bodů na EXE, doložený přechod14400→11190 a383 bitově odvozených nových koeficientů v každém save. Sedm peněžních definic a celý trh jsou nezávisle kontrolované; čtyři další obousměrné15605bajtové přenosy souhlasí, obě source obnovy bez AV.

[Pozdní časová osa a tříhodinové ocenění](LATE_TIMELINE.md): dva normálně ukončené celé procesy,13 kontrolních bodů na EXE,24 samostatně doložených peněžních událostí, shodný trh a480000 RGB pixelů ocenění. Saves14616/15906 bajtů uchovávají odlišné náhodné výsledky. Čtyři další obousměrné obnovy mají14616/15906/14616/14616 celých přesných bajtů; obě source obnovy skončily bez AV. Produkce je shodná s úplnou tracker-loaders regresí102/102; tato etapa neoznačuje starý CTest za nový běh.

Aktuální tracker-loader reporty se zapisují jako UTF-8 bajty s LF, stejně jako jejich Git pravidla. To opravuje konkrétní chybu přenositelnosti: Windows textový zápis zaváděl CRLF, které Git při commitu odstranil a tím změnil mezireportové SHA-256. Všech13 scoped a14 regresních odkazů bylo po novém zápisu ověřeno nad skutečnými LF bajty. Starší záznamy ponechávají své původní místní byte hashe; nejsou zpětně vydávané za nově spuštěné testy. Vrchní hash zdrojového stromu je znovu spočítaný a označený příslušným reportem.

[Trackerové loadery a nenulový mixer](SQUALL.md) nově odstranily doložené rozdíly XM/IT/S3M polí, vynechané AGC, nesprávně zapisovaný útlum a odlišné záporné PCM/VU. Obě sestavení mají9088 přesných AGC/mixer případů a288 skutečných ukončení kanálu. Nová úplná regrese102/102 za499,88s, všech45 zvuků /122527752 PCM bajtů na sestavení, game-audio i PersonSounds prošly. Čtyři celé6943/7784/6943/6943bajtové saves souhlasí; obě source obnovy bez AV. Rozsah, historické neshody i zbývající oblasti jsou v `analysis/squall-tracker-loaders-regression.json`; celý projekt pokračuje.

[Původní MOD tracker](SQUALL.md) má nově přesné loader/order, wrapper a skutečné konstruktorové testy v obou sestaveních; žádné původní heap selhání není převzaté jako úspěšné přehrávání. Aktuální celá regrese102/102 prošla za510,02s, všech45 herních zvuků má122527752 shodných PCM bajtů na sestavení a game-audio/PersonSounds také prošly. Čtyři celé6943/7784/6943/6943bajtové přenosy uložených her souhlasí; oba source procesy mají normální exit bez AV. `analysis/squall-tracker-regression.json` odkazuje na konkrétní buildy, protokoly a zbývající rozsahy. Celý původní cíl pokračuje.

[Obnovená původní x87 aritmetika MPEG](SQUALL.md) prošla v obou sestaveních přesnými tabulkami,40960 kernel případy,640 čteními a56 bezpečnými fyzickými buffery na sestavení. Oba původní MPEG2/Layer2 přesahy jsou prokázané odděleně; nebezpečné přehrávání se nepovažuje za ověřené. Aktuální celá regrese102/102 za543,07s, obě audio regrese a čtyři celé6943/7784bajtové herní obnovy prošly. Dvě source obnovy jsou bez AV, minutí kurzoru prvního GUI pokusu je zachované. Celý původní projekt dále pokračuje.

[Skutečné číselné profily](PROFILES.md) nově porovnávají4324 položek s nezměněnými původními CRT streamy v PC24 a odděleném FP fixture PC53. Same-seed bity/text i všechny přiřazené převody souhlasí; skutečné adresně závislé neplatné výsledky zůstávají výslovně odlišné. Nezávislé sestavení také prošlo. [Celé EXE s plným skladem](MANUFACTURE.md) mají devět shodných pracovních kontrol, správné blokování při100 %, skutečné uvolnění dvou míst přes trh a4089 přesných save bajtů. Nový úplný CTest po produkční opravě profilů:102/102,1062,64s. Celý původní cíl pokračuje.

[Pozice a další kanálové exporty Squall](SQUALL.md) mají obnovenou původní adresně závislou větev a2542 přesných porovnání v obou přesnostech, zopakovaných ve druhém sestavení. Nová zvuková regrese a čtyři celé2650bajtové save přenosy prošly; úplná sada101/102 a tři focused opakování opraveného testovacího driveru jsou evidované odděleně. Ostatní exporty a celý projekt pokračují.

[Skupiny kanálů Squall](SQUALL.md) mají nové skutečné DLL porovnání840 veřejných výsledků v PC53/PC24, zopakované ve druhém sestavení. Příprava/přehrávání/pauza, skupinová hlasitost/frekvence a ukončení workerů prošly. Ostatní rozsahy97 exportů a celý projekt pokračují.

[Přirozené osivo v celých EXE](SEED_RUNTIME.md) nově prošlo odemčením, druhou sklizní/prodejem, deníkem a zaplaceným sáčkem rajčat. Čtyři skutečné obousměrné2523bajtové přenosy zachovaly obsah i prodejní hodiny; původní X+1 a padding jsou doložené zvlášť. Navazující skutečné rytí/setí/zalévání má143000 přesně shodných tutorial RGB pixelů a tři zbývající semena. Skutečný celý cyklus rajčat, prodej72, obnova staré historie a první rozšíření na20 polí také prošly; další systémy a celý projekt pokračují.

[První přirozená zakázka v celých EXE](CONTRACT_RUNTIME.md) nově prošla i skutečným zaléváním/sklizní, kapacitou nákladu3, splněním za60 a dalšími třemi úplnými2186bajtovými save přenosy dokončeného kontraktu i dalšího dopisu. Následující herní systémy pokračují.

[Přirozený tutoriál v celých EXE](TUTORIAL_RUNTIME.md) nově ověřil deset kroků, skutečnou sklizeň/sklad/trh a1981 shodných save bajtů. Oprava alokací překrývajících se obrázků polí nově obnovila286000 přesně shodných RGB pixelů deseti oblastí. Celý tutoriál a projekt pokračují; neúspěšné průzkumné běhy jsou odděleně evidované.

[Zdrojový odinstalátor](UNINSTALL.md) je nyní sestavitelný přes FarmarUninstall z přesného NSIS2.28; nezávislé sestavení má shodných všech sedm ověřovaných binárek. Všech27 aplikačních mazacích instrukcí a tři stránky mají statické porovnání. Nově20 skutečných silent odinstalačních procesů ověřilo deset source/reference párů, souborové/registry účinky a504 shodných aplikačních operací. Dalších16 UI procesů souhlasí v osmi párech a268 záznamech prvků; dalších32 procesů ověřilo vlastní bootstrap, kopie A/B a odložené záměry v testovacím backendu. Skutečný reboot a pixelové snímání zůstávají neověřené. Další neověřené rozsahy a podmínky celé hry zůstávají evidované v dokumentaci.

[Obsluha textových příkazů GameInput](GAME_INPUT.md) je propojená se skutečným herním světem. Native callback porovnání pokrývá33333 znaků; celé procesy ověřily peníze včetně wrap32,15 upgrade hodnot,8 osiv a kapacitně omezený sklad. Úplné herní chování a ostatní podmínky projektu pokračují.

[Zkratky cLogic a grafický reset](LOGIC_SHORTCUTS.md) jsou nově zapojené do hlavního EXE:3661 nativně porovnaných stavů a skutečné přepnutí okno/fullscreen. Dva resetové snímky mají2553600 přesně shodných RGB pixelů; původní texture-suppression zvláštnost i DPI hranice jsou evidované. Úplný runtime a ostatní podmínky projektu pokračují.

[Porovnání celých procesů a saves](SAVE_INTEROP.md) nyní ověřuje úvodní profily v obou směrech a připravený bohatší stav se sáčkem, prodejem, zprávami a383 koeficienty. Původní EXE běží pouze jako reference s izolovanou systémovou datovou složkou; produkce používá vlastní zdroje. Rozsah a neověřené stavy jsou výslovně evidované.

Hlavní [zdrojové herní EXE](APP_RUNTIME.md) se nyní sestaví a spustí s původními daty: nový profil → menu → skutečná farma, normální ukončení, opětovné načtení save, pauza, nápověda i deník. Obě načtené herní DLL pocházejí z našeho sestavení. [Startup](STARTUP.md) navíc přímo souhlasí s originálem ve čtyřech cyklech / 786 snímcích / 377 280 000 RGB pixelech (PC53/PC24). Úplný nativní runtime, obousměrné saves, všechny systémy, úplná Squall a izolovaný odinstalátor stále pokračují.

[Původní JPEG loga](JPEG.md) mají obnovený reader a MMX barevnou aritmetiku: všechna3 loga souhlasí v každém dekódovaném bajtu i celé GPU textuře, PC53/PC24; vyčerpávající porovnání16777216 barevných trojic také prošlo. Celé spuštění a hlavní herní EXE dál pokračují.

[První profil NewUser](NEW_USER.md) je obnovený a porovnaný s přechodem do skutečného menu: PC53/PC24, čtyři životní cykly, 88 stavů a 56 snímků / 26 880 000 shodných RGB pixelů. Zdrojové profily se skutečně ukládají; nativní Profile služba má výslovný testovací adaptér. Celé spuštění a herní EXE pokračují.

[Skutečný cGame a binární save/load](GAME_CONTROLLER.md) už propojuje všechny hlavní obnovené objekty. Zdrojové porovnání save/restore/save a skutečné mouse/tutoriálové cykly ověřuje nový integrační běh v PC53/PC24; nativní úplný svět, obousměrné saves mezi programy a úplné porovnání hlavního interaktivního EXE dál čekají.

[Řídicí objekt herních ocenění](GAME_AWARD_MANAGER.md) propojuje automatické vyhodnocení se skutečným efektem, obrazovkou, panelem a návratem. PC53/PC24: 18 podmínek, 36 celých cyklů a 2012 snímků / 965 760 000 shodných RGB pixelů. Tři podmínky nad úplným světem a celý cGame/EXE zůstávají neověřené; source cleanup Help/Journal při chybě konstrukce prošel osmi kontrolami.

[Skutečný Journal](JOURNAL.md) už otevírá a zavírá skutečný HUD, rozvrhuje zprávy a označuje je za přečtené. PC53/PC24: 430 stavů / 102 operací a 362 celých snímků / 173 760 000 shodných RGB pixelů, včetně obou směrů, indexových hranic a XML reloadu. Obnovené jsou i potřebné textové copy/measure metody; celý cGame/EXE dále pokračuje.

[Skutečná nápověda](HELP_VIEW.md) je propojená s pauzovacím menu: otevření, stránky, návrat i XML reload probíhají skutečnou myší. PC53/PC24: 30 stavů / 24 operací a 376 celých snímků / 180 480 000 shodných RGB pixelů včetně ostatních pauzovacích obrazovek. Pokračují Journal, award manager a celý cGame/EXE.

[Skutečný Farm controller](FARM_SCENE.md) propojuje šedesát polí, dopisy, rozšíření a profilovou obnovu zavlažovačů. PC53/PC24: 188 stavů, 132 CPU operací a 64 celých snímků / 30 720 000 shodných RGB pixelů, včetně skutečného přijetí zakázky a odstranění dopisu. Celý cGame, pracovní vstup nad touto farmou a herní EXE pokračují; přesné hranice jsou v dokumentaci.


[ContractLetterSlot](CONTRACT_LETTER.md) obnovuje příchozí dopisy Farm: přílet, odpočet, blikání, profilovou obnovu a skutečný tutoriál17 → mouse → přečtení → ContractProvide. PC53/PC24:3404 stavů,548 přiřazení,2660 CPU operací a160 celých snímků /76 800 000 shodných RGB pixelů. Úplný Farm/cGame a herní EXE pokračují.


[Skutečné seed/hose cykly](PERSON_FLOWS.md): dalších426 stavů /144 výslovných CPU operací a272 celých snímků /130 560 000 shodných RGB pixelů v PC53/PC24. Vstup sám spustí sebrání sáčku, setí, odložení při převzetí rýče i zalévání hadicí včetně živých částic. Opravený je cíl hadice podle instrukcí a vlastní původní prázdné mouse callbacky postavy. Celý Farm/cGame/EXE a ostatní podmínky cíle pokračují.

Person controller:462 stavů /284 výslovných CPU operací v PC53/PC24 a128 kompletních D3D8 snímků /61 440 000 shodných RGB pixelů. Plný actor sám spotřebuje úkol z reálného Field vstupu, vykoná pohyb, arrival, Dig a návrat do idle. Sedm časovaných workerů také používá skutečné Field/Warehouse/tool objekty. Přesné hranice a dosud neověřené větve uvádí [PERSON_CONTROLLER.md](PERSON_CONTROLLER.md); celá hra tím není dokončená.

ElasticAnimation je porovnaná v PC53/PC24:1524 stavů /1354 akcí,168 celých D3D8 snímků /80 640 000 shodných RGB pixelů. Skutečné tool upgrady vykonají platbu a profilovou změnu, původní let a appearance completion. Skutečné Mow mouse press/release spouští stejný controller. Deset kinds, Euler/x87 arithmetic, zero-acceleration unordered větev, XML reload a teardown resource deltas jsou evidované v [ELASTIC_ANIMATION.md](ELASTIC_ANIMATION.md) a `analysis/elastic-animation-validation.json`. Úplná hra se tím nevykazuje jako hotová.

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

Win32 Release sestavení v `build/windows` má nyní 80 úspěšných CTest kontrol. Poslední úplný běh prošel 79/80 za 702,63 s; jediná kontrola masek skončila před porovnáním na diagnostické rezervaci adresy. Po zmenšení rezervace původní image base z 3 MiB na jeden 64 KiB granule tato kontrola prošla za 34,09 s. Produkční kód masek se neměnil. Původní JUnit i opakovaný JUnit jsou lokálně zachované; metadata sloučených výsledků a původní chyba jsou v `analysis/regression80-execution.json`. Python reference testy kontrolují všech 17 původních hashů, CRC archivů a nezávisle porovnávají PE importy/exporty s objdump. Binutils dostává ASCII kopii souboru v dočasném adresáři, protože jeho Windows sestavení neumí zdejší české cesty; původní soubory se nemění. Výchozí test dekodéru na Windows vybírá nové EXE, nikoli starý linuxový soubor.

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

AwardPanel nyní používá skutečné vlastněné texty a OK input, borrowed AwardEffect a Awards factory/confirmation/deletion: 882 stavů, 42 input callbacků a 168 GPU snímků / 80 640 000 shodných RGB pixelů. Původní adresy, Text0/Text zvláštnost XML a sound hranice jsou v [AWARD_PANEL.md](AWARD_PANEL.md). Gameplay condition detection a skutečný herní svět pokračují.

Obnovené pole a akce (`original-farm-field`) mají přímé PC53/PC24 porovnání: 4 460 stavů, 80 objektů, 4 224 kroků/dotazů a 128 GPU snímků / 61 440 000 přesně shodných RGB pixelů. Skutečné Farm query bodies vybírají pole a ověřují masku Warehouse/Sprinkle, skutečný postřikovač mění vodu a preview pole a běží s emitterem/částicemi. Porovnané jsou všechny kultury a čtyři počáteční saved stavy, dostupnost upgrade, hover/down/release/leave a osm tutorial21/22/23 flag kombinací. FieldTip/Person/celý Farm mají výslovné hranice; úplný svět ani obousměrný save tím nejsou prokázané. Detaily: [FARM_FIELDS.md](FARM_FIELDS.md).

Navazující skutečný [FieldTip](FIELD_TIP.md) je nyní zapojený do pole a přímo porovnaný v PC53/PC24:4 998 tip stavů a společně4 472 field stavů,140 GPU snímků /67 200 000 shodných pixelů, čtyři XML reloady. Úplná regrese prošla80/80 za721,22 s a oba game-audio buildy; současné source/binary hashe i všech17 originálů jsou ověřené. Farm indication a Person zůstávají výslovné hranice.

[SeedPackage a ProgressDiscrete](SEED_PACKAGE.md) nyní propojují skutečný BuySeeds callback, čtyřpoužitelný sáček, platbu při umístění, myš, Farm masky a odstranění odmítnutého nákupu. Skutečné pole spouští tutorial22 s existujícím sáčkem. PC53/PC24:550 SeedPackage stavů /504 akcí a1038 ProgressDiscrete stavů,64 GPU snímků /30 720 000 shodných RGB pixelů. Úplné sestavení a81/81 CTest prošlo za338,02 s, obě game-audio sestavení a striktní source/binary hashe také; všech17 originálů zůstalo beze změny. V tomto SeedPackage režimu je Person queue pozorovaná hranice; navazující PersonTasks ji už vykonává skutečně; tutorial20 a neurčený původní počáteční status na recyklované haldě mají výslovně evidované omezení. Celý Farm/Person, herní EXE, obousměrné saves, úplná Squall a izolovaný uninstaller nadále zbývají.

[PersonTasks](PERSON_TASKS.md) nyní obnovuje skutečnou frontu postavy a značky napojené na myší vstup Field/SeedPackage:102 stavů /84 akcí a16 dalších GPU snímků /7 680 000 shodných RGB pixelů v PC53/PC24. Dokončení přesouvá značky, zachovává uložené cíle a používá původní deferred discard; FIFO, vyřazení i unsigned hraniční indexy jsou přímo porovnané. Úplné Win32 sestavení a82/82 CTest prošlo za341,53 s, obě game-audio sestavení a striktní hashe také; všech17 originálů je beze změny. Nativní Person je v tomto porovnání explicitní adaptér příslušných dat. Celý jeho constructor, pohyb, carrying/work, animace/zvuk a úplná Farm/EXE dále zbývají.

[PersonSounds](PERSON_SOUNDS.md) obnovuje výběr a vlastnictví zvukových kanálů postavy přes skutečný SoundManager/cChannel a fyzické DLL. PC53/PC24 na vlastní i původní knihovně:332 stavů /328 akcí /616 API událostí,40 kontrol postupu smyček a8 kontrol paused pozice; samostatné sestavení prošlo stejným scénářem. Původní sound metody běží jen jako reference na explicitních datech; actual replay/stop pořadí, resource reference, cached hose resume i null sloty jsou porovnané. Úplné sestavení a86/86 CTest prošlo za386.41 s, oba game-audio audity a striktní source/binary hashe také; všech17 originálů je beze změny. Plný actor/action/Farm/EXE, celé saves, zbývající Squall a izolovaný uninstaller pokračují; audible/refill kvalita tím není vykázaná jako hotová.

[PersonVehicleWalk](PERSON_VEHICLE.md) obnovuje přesun s nástroji včetně přepnutí os a směru:8660 stavů /8532 kroků,128 GPU snímků /61 440 000 shodných RGB pixelů v PC53/PC24. Kliknutí na skutečné pole vytvoří PersonTasks úkol a vykreslená postava s Dig/Mow nástrojem dojde po původní trase; následuje actual marker cleanup. Porovnané jsou hranice2/10, direct/L trasa, clipped čas, profilová pozice, axis appearance, cancel a skutečné SeedPackage stavy. Úplné sestavení a85/85 CTest prošlo za359.55 s, obě aktuální audio sestavení a striktní hashe také;17 originálů je beze změny. Výslovný drawing/input a world adaptér nadále platí. Skutečný arrival/work dispatcher, plný Person/Farm/EXE, obousměrné saves, zbytek Squall a izolovaný uninstaller zůstávají další prací.

[PersonWalk](PERSON_WALK.md) obnovuje běžný pěší přesun k úkolu:1306 stavů /1210 kroků,96 GPU snímků /46 080 000 přesně shodných RGB pixelů v PC53/PC24. Skutečné kliknutí na Field vytvoří PersonTasks úkol a značku; obnovený pohyb spotřebuje cíl, posune vykreslenou postavu a dojde k poli. Porovnané jsou také profilové XPersPos/YPersPos, posunutý Farm.movingPicture, cancel, unordered souřadnice a skutečný SeedPackage status2/0. Úplné sestavení a84/84 CTest prošlo za350.78 s, oba game-audio buildy a striktní source/reference hashe také; všech17 originálů je beze změny. Explicitní drawing/input a world adaptéry nadále platí. Arrival/work dispatcher, vozidlová větev, úplný Person/Farm/EXE, saves, zbývající Squall a izolovaný odinstalátor pokračují.

[PersonVisuals](PERSON_VISUALS.md) obnovuje skutečná appearance/XML data postavy, výběr idle/work/move/hose animací, hotspoty a UI nesených věcí. PC53/PC24:9014 stavů /8928 akcí,10 XML reloadů a40 GPU snímků /19 200 000 shodných RGB pixelů včetně skutečného pole a značek fronty. Pořadí aktualizací, původní x87 přesnost i retained UI/sound reference jsou přímo porovnané. Úplné sestavení a83/83 CTest prošlo za371,54 s, oba game-audio buildy a striktní hashe také; všech17 originálů je beze změny. Drawing/input base a některé world hodnoty mají výslovné adaptéry. Celý Person constructor, pracovní akce a úplný actor lifecycle dále zbývají; běžný přesun nyní popisuje PersonWalk; stejně jako kompletní Farm/EXE a ostatní podmínky celého cíle.
