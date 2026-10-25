# Skutečný Farm controller

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
