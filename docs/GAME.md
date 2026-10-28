# Rekonstrukce hlavní hry

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

[Person controller](PERSON_CONTROLLER.md) už v porovnané scéně vykonává skutečný řetězec Field mouse → PersonTasks → pohyb → arrival → Dig → změna pole → idle. Plný constructor/lifecycle a sedm časovaných pracovních větví mají v PC53/PC24 dalších462 stavů /284 výslovných CPU operací a128 GPU snímků /61 440 000 shodných RGB pixelů. Seed/hose nově ověřuje navazující porovnání; ostatní arrival a tutorial větve jsou evidované jako neověřené; vlastní Farm/cGame kompozice dále zbývá.

Nově obnovená [ElasticAnimation](ELASTIC_ANIMATION.md) přímo propojuje skutečné tool upgrady s původním přeletem a obnovou vzhledu Dig/Mow/WaterSource. V obou přesnostech prošlo1524 stavů /1354 akcí a168 celých snímků /80 640 000 přesně shodných RGB pixelů, včetně skutečného mouse upgrade řetězce. Person world pozorování a kind9 View jsou výslovné diagnostické hranice; celé pracovní akce postavy a celý Farm controller pokračují.

Herní EXE už lze sestavit; dosavadní stav základních knihoven a jejich hranice jsou popsané níže. Sestavitelná knihovna `recovered_game_utilities` obsahuje základ původního `String` a UTF-16 `WString`, souborovou vrstvu, indexování balíků a XML strom. Diagnostika `resource-inspect` propojuje tyto části s JNGLoad a načítá původní data. Integrace do celé hry a původní engine výjimky nejsou hotové. Adresy, zvláštnosti a výsledky celé načítací cesty jsou v [RESOURCE_LOADING.md](RESOURCE_LOADING.md).

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

[SeedPackage a ProgressDiscrete](SEED_PACKAGE.md) nyní propojují skutečný BuySeeds callback, čtyřpoužitelný sáček, platbu při umístění, myš, Farm masky a odstranění odmítnutého nákupu. Skutečné pole spouští tutorial22 s existujícím sáčkem. PC53/PC24:550 SeedPackage stavů /504 akcí a1038 ProgressDiscrete stavů,64 GPU snímků /30 720 000 shodných RGB pixelů. Úplné sestavení a81/81 CTest prošlo za338,02 s, obě game-audio sestavení a striktní source/binary hashe také; všech17 originálů zůstalo beze změny. V tomto SeedPackage režimu je Person queue pozorovaná hranice; navazující PersonTasks ji už vykonává skutečně; tutorial20 a neurčený původní počáteční status na recyklované haldě mají výslovně evidované omezení. Celý Farm/Person, herní EXE, obousměrné saves, úplná Squall a izolovaný uninstaller nadále zbývají.

[PersonTasks](PERSON_TASKS.md) nyní obnovuje skutečnou frontu postavy a značky napojené na myší vstup Field/SeedPackage:102 stavů /84 akcí a16 dalších GPU snímků /7 680 000 shodných RGB pixelů v PC53/PC24. Dokončení přesouvá značky, zachovává uložené cíle a používá původní deferred discard; FIFO, vyřazení i unsigned hraniční indexy jsou přímo porovnané. Úplné Win32 sestavení a82/82 CTest prošlo za341,53 s, obě game-audio sestavení a striktní hashe také; všech17 originálů je beze změny. Nativní Person je v tomto porovnání explicitní adaptér příslušných dat. Celý jeho constructor, pohyb, carrying/work, animace/zvuk a úplná Farm/EXE dále zbývají.

[PersonSounds](PERSON_SOUNDS.md) obnovuje výběr a vlastnictví zvukových kanálů postavy přes skutečný SoundManager/cChannel a fyzické DLL. PC53/PC24 na vlastní i původní knihovně:332 stavů /328 akcí /616 API událostí,40 kontrol postupu smyček a8 kontrol paused pozice; samostatné sestavení prošlo stejným scénářem. Původní sound metody běží jen jako reference na explicitních datech; actual replay/stop pořadí, resource reference, cached hose resume i null sloty jsou porovnané. Úplné sestavení a86/86 CTest prošlo za386.41 s, oba game-audio audity a striktní source/binary hashe také; všech17 originálů je beze změny. Plný actor/action/Farm/EXE, celé saves, zbývající Squall a izolovaný uninstaller pokračují; audible/refill kvalita tím není vykázaná jako hotová.

[PersonVehicleWalk](PERSON_VEHICLE.md) obnovuje přesun s nástroji včetně přepnutí os a směru:8660 stavů /8532 kroků,128 GPU snímků /61 440 000 shodných RGB pixelů v PC53/PC24. Kliknutí na skutečné pole vytvoří PersonTasks úkol a vykreslená postava s Dig/Mow nástrojem dojde po původní trase; následuje actual marker cleanup. Porovnané jsou hranice2/10, direct/L trasa, clipped čas, profilová pozice, axis appearance, cancel a skutečné SeedPackage stavy. Úplné sestavení a85/85 CTest prošlo za359.55 s, obě aktuální audio sestavení a striktní hashe také;17 originálů je beze změny. Výslovný drawing/input a world adaptér nadále platí. Skutečný arrival/work dispatcher, plný Person/Farm/EXE, obousměrné saves, zbytek Squall a izolovaný uninstaller zůstávají další prací.

[PersonWalk](PERSON_WALK.md) obnovuje běžný pěší přesun k úkolu:1306 stavů /1210 kroků,96 GPU snímků /46 080 000 přesně shodných RGB pixelů v PC53/PC24. Skutečné kliknutí na Field vytvoří PersonTasks úkol a značku; obnovený pohyb spotřebuje cíl, posune vykreslenou postavu a dojde k poli. Porovnané jsou také profilové XPersPos/YPersPos, posunutý Farm.movingPicture, cancel, unordered souřadnice a skutečný SeedPackage status2/0. Úplné sestavení a84/84 CTest prošlo za350.78 s, oba game-audio buildy a striktní source/reference hashe také; všech17 originálů je beze změny. Explicitní drawing/input a world adaptéry nadále platí. Arrival/work dispatcher, vozidlová větev, úplný Person/Farm/EXE, saves, zbývající Squall a izolovaný odinstalátor pokračují.

[PersonVisuals](PERSON_VISUALS.md) obnovuje skutečná appearance/XML data postavy, výběr idle/work/move/hose animací, hotspoty a UI nesených věcí. PC53/PC24:9014 stavů /8928 akcí,10 XML reloadů a40 GPU snímků /19 200 000 shodných RGB pixelů včetně skutečného pole a značek fronty. Pořadí aktualizací, původní x87 přesnost i retained UI/sound reference jsou přímo porovnané. Úplné sestavení a83/83 CTest prošlo za371,54 s, oba game-audio buildy a striktní hashe také; všech17 originálů je beze změny. Drawing/input base a některé world hodnoty mají výslovné adaptéry. Celý Person constructor, pracovní akce a úplný actor lifecycle dále zbývají; běžný přesun nyní popisuje PersonWalk; stejně jako kompletní Farm/EXE a ostatní podmínky celého cíle.
