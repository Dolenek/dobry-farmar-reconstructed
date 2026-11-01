# Skutečný Farm controller

## Skutečné nákupy zbývajících rozšíření a všech60 aktivních polí

`tools/validate_game_farm_expansions.py` navazuje na skutečný profil po neúspěchu zakázky37: fcu1/fua3, Money4504, tři zaplacené postřikovače. Původní tlačítka na572/152 a572/102 skutečně zaplatí500 a1000 z XML; Money postupuje4504→4004→3004. Nejde o cheat rozšíření ani zápis profilových klíčů. Už dříve původním průběhem dosažená dostupnost3 zůstává zachovaná.

Pět snapshotů na EXE čte vždy všech60 skutečných FarmField objektů podle vtable. Původní konstruktor0043c120 zapisuje vtable005f1870 a id/upgrade/active na508/50c/510; dosavadní field oracle stejná pole porovnává. Samostatný zdrojový metadata nástroj poskytuje Win32 Release offsety1120/1124/1128 a velikost2096. Zdrojové ASLR adresy se odvozují z map symbolů; do herní paměti se nezapisuje. Příznaky aktivace a XML skupiny souhlasí při20→38→60. Překryvy unavailable/ready-to-buy mají22/18, potom0/22 a nakonec0/0; při maximální úrovni zmizí nákupní tlačítko.

Původní `setCurrentUpgrade` přepočítá dosah všech tří už instalovaných postřikovačů. Skutečná závlaha tak postupuje20→28→30 polí. Deset nově zalévaných polí10/19/28/37/49/50/51/52/56/57 mělo před nákupem vodu0 a active=false; po nákupu má vodu>118 a jejich skutečný růstový čas klesá. Ostatní nově odkrytá pole mimo dosah zůstávají suchá.300 živých záznamů na EXE, všechny diskrétní profilové hodnoty a4563 save bajtů souhlasí. Nezávislé časové, vodní, hnojivé a růstové floaty jsou explicitně evidované; žádné bajty se nenormalizují.

Čtyři další celé obnovy ověřily všechny240 skutečných příznaků aktivace,30 zalévaných polí a každý4563bajtový save. Oba zdrojové procesy mají readonly exception trace s normálním výstupem a bez AV. Počáteční diagnostika skončila před nákupem, protože MSVC map byl nesprávně čtený jako UTF-8. Oprava používá bezztrátové Latin-1 pro ASCII dekorované symboly; původní report/driver zůstávají zachované. Nejde o chybu hry.

Reprodukce:

```powershell
cmake -S tools/live_farm_field_layout -B build/live-farm-field-layout-vs -G "Visual Studio 17 2022" -A Win32 -T "v143,version=14.43.34808"
cmake --build build/live-farm-field-layout-vs --config Release
py -3 tools/validate_game_farm_expansions.py
py -3 tools/validate_farm_expansions_state.py
py -3 tools/validate_game_farm_expansions_interop.py --crash-trace build/source-crash-trace-vs/Release/source-crash-trace.exe
py -3 tools/record_farm_expansions_verification.py
```

Přenositelné důkazy: `analysis/farm-expansions-state-validation.json`, `analysis/farm-expansions-interop-validation.json`, `analysis/farm-expansions-regression.json`. Produkční zdroje stále odpovídají úplné102/102 regresi a všech17 originálů je beze změny. Pozdější přirozené události ničení polí, další ocenění, úplné DLL a ostatní podmínky celého cíle pokračují. Následující text popisuje starší izolované hranice původní rekonstrukce controlleru.

`src/game/ui/Farm.h/.cpp` obnovuje vlastní farmu, její UI a konkrétní rozhraní polí, sáčků, zavlažovačů, postavy, nástrojových budov a dopisů. Constructor `00435c40` vytvoří StateView, šedesát skutečných FarmField, tři ContractLetterSlot a HoseEmitter. XML `004305c0` načte původní ceny, místa tlačítek a profilový stav. Destructor `00434b80` uvolní singleton před UI a vlastní potomky smaže, dokud jejich vypůjčené kontexty a virtuální služby žijí.

Obnovené jsou také absoluteStep `004331b0`, input lifecycle `00434cc0/004350d0`, nákup polí `004357f0`, upgrade dostupnosti/úrovně `004354d0/00435660`, osivo, přidání a odstranění dopisů `0042ec20/004341b0`, kapacita a prostorové dotazy `0042d700/0042db30`.

Výběr zničených kultur `0042f080` zachovává původní zvláštnosti: čte FFC (kulturu), nikoli FFS (pracovní stav), nepřeskakuje neaktivní pole, pro pravděpodobnost používá 1/32767 a pro index 1/32768. Horní mez indexu je předem snížená; při více kandidátech se poslední nemůže vybrat. Nulový počet nespotřebuje RNG. Hranice obdélníku `00417910` zachovává neuložený pravý okraj v x87.

## Přímé porovnání

`python tools/validate_farm_scene.py` spouští oddělené PC53 a PC24 scénáře. Dohromady prošlo 188 stavů, 132 výslovných CPU operací a 64 úplných D3D8 snímků / 30 720 000 přesně shodných RGB pixelů. Čtyři životní cykly Farm vlastní celkem 240 polí, 12 dopisů a šest obnovených zavlažovačů.

Porovnání zahrnuje tři skutečné nákupy rozšíření, 36 variant výběru kultur na každou přesnost, kapacitu tří dopisů, přílet/přečtení a skutečný ContractProvide.accept → Farm.removeLetter → platba → GameMenu/ContractStatus. Druhá konstrukce obnovuje všechny dopisy i instalované zavlažovače z profilu. Kontrolují se profilové mapy, RNG, životnost GameAction, seznamy zasažených polí, UI/částice a uvolnění textur. Všech 17 původních souborů zůstává shodných s inventářem.

Původní strojové instrukce běží pouze v diagnostickém porovnání. Produkční Farm nevolá původní kód. Diagnostické fixupy nyní zahrnují obě skutečné prostorové funkce; jejich starší náhradní hranice se v tomto scénáři nepoužívají.

## Přesné hranice

Celý cGame stále chybí. Povinné `FarmGameServices` nemá produkční náhradní implementaci; test pozoruje jeho peněžní a upgrade operace a dodává lookup zakázek. Úplné důsledky obnovy ostatních scén nejsou tímto testem prokázané. Skutečný původní Warehouse getter čte zásoby profilu pro ContractStatus; celá skladba Warehouse/Manufacture/Market dále pokračuje.

Vstupy FFC/FFS/RNG selector testů jsou výslovně řízené a před vykreslením vrácené. Obnovené zavlažovače jsou porovnané před konstrukcí WaterSource a bez zavlažovacího ticku. Konkrétní vazby Farm na Person/tools existují, avšak celý pracovní cyklus postavy na této skutečné Farm ještě není vykázaný jako porovnaný. Zvuk zde používá vypnutý UI backend; fyzické audio má samostatné audity.

Další práce: úplný cGame a vstupní cykly nad touto farmou, spuštění herního EXE, obousměrné celé save soubory, zbývající Squall a izolovaný odinstalátor. Tento celek neoznačuje dokončení projektu.

Úplné Win32 sestavení a 91/91 CTest prošly za 417.72 s. Oba aktuální game-audio a oba PersonSounds buildy prošly; striktní zápis ověřil source/binary hashe a 17 nezměněných originálů. Celou regresi eviduje `analysis/farm-scene-regression.json`.
