# Skutečný cGame a binární ukládání

Obnovený `src/game/engine/Game.cpp` skládá původní hlavní herní objekt, ne diagnostické náhrady jeho světových služeb. Konstruktor `00413210` vytváří Gameplay, Tutorial, načítá game.xml a následně WarehouseTip, HUD, Farm, Market, Manufacture, FieldTip, čtyři nástroje, binární obnovu, Person, ElasticAnimation, nákupní panely, Journal, čtyři smluvní dialogy, Help a GameAwardManager. HUD nakonec inicializuje scénu. Vnější engine musí dodat skutečný pause kontext s delší životností kvůli odloženému návratu do menu.

Přímé vazby propojují příkazy Farm s postavou, nástroje se skladem a penězi, HUD s reálnými zvukovými kanály Person, výrobu/trh se skladem, smlouvy s profily/dialogy a Tutorial s konkrétními ovládacími prvky. Události Gameplay vykonávají původních deset dispatchů podle vtabulek `005f98d0`–`005f9914` a `005f9c60`; DestroyDepot má původní skutečný RET `004bd820`. Dostupnost UpAvail a SeedAvail patří do uint mapy, nikoli int mapy.

`00410d60` nastavuje peníze s dolním omezením, provádí `0040be10`, refresh HUD, opakovaně refreshuje Dig/Mow/Warehouse/Water, obnovuje tlačítko Farm a vytvoří skutečný výdajový/příjmový efekt. `0040be10` jedenáctkrát čte tentýž aktuálně zaměřený Upgrade výroby, nikoli jednotlivé sloty; zachovává i tutoriálovou výjimku 34. Rutina `00404170` spouští zvuk upgradu, proto byla odpovídající služba Farm přejmenována na playUpgradeSound.

## Binární .save

`GamePersistence` obnovuje zápis `0040fbb0` a čtení `00412850`. Jméno používá adresář profilu a jeho osmimístné ID z `004e4110`, nikoli zobrazované jméno hráče. Pole v souboru:

1. uint32 počet sáčků a 24 bajtů na každý: relativní float XY, uint32 kind/count/status, byte unpaid a tři ignorované výplňové bajty.
2. uint32 počet prodejů, float cutoff a záznamy float time + uint32 amount. Obnova volá skutečný SalesHistory.record, včetně prune a awEarn, potom vrací uložený cutoff.
3. uint32 počet zpráv, byte read, uint32 počet UTF16 jednotek a jejich bajty bez terminátoru.
4. uint32 počet koeficientů a raw float pole pro skutečný Gameplay.restoreCoefficients.

Chybějící soubor se před čtením vytvoří stejnou zápisovou cestou. Sáčky vznikají před Person. Instrukce nevracejí relativní X do světa: StateView konstruktor `004c48e0` nastaví pozadí na -1, zápis tento počátek odečte a obnova předává výsledek beze změny do `0045e130` / `004192a0`. Zdroj zachovává posun X o +1 při načtení; pro testované X123 vznikne zápis124, načtení124 a další zápis125. Celý původní program ještě musí tuto instrukční stopu potvrdit. Zdroj zapisuje tři ignorované bajty výplně jako nuly; originál je nechával nespecifikované. Krátké/chybné soubory a výjimky nejsou prohlášené za ověřené.

## Ověření a hranice

`game-source-runtime-test` a `python tools/validate_game_source_world.py` spouštějí skutečný zdrojový svět s původními daty, vlastním JNGLoad a vlastní sestavenou Squall. Každý proces má nový izolovaný profil, skutečný Direct3D8 a řízený clock/input. V PC53 i device PC24 se konstruují dva úplné světy. Skutečné kliknutí na maskované tělo rýče spustí frontu postavy, chůzi a převzetí nástroje; tutorial20 se dokončí. Další kliknutí na skutečné pole spustí práci a tutorial21. Následuje Journal/read/return, pauza/Help/return a obnovení celého Game. Binární save/restore/save se porovnává po bajtech s jedinou předem doloženou změnou float X uvedenou výše; ostatní bajty musí být identické.

Jde o zdrojovou integrační kontrolu, nikoli porovnání celého cGame s originálem. Nativní constructor/destructor, úplný frame/input/event dispatch, obousměrné saves mezi programy a samostatný interaktivní hlavní EXE čekají. Zvlášť čeká první uživatelská obrazovka UI/NewUser.xml, skutečný engine startup/reset/cleanup a srovnání zbývajících větví. Původní Tutorial destructor ponechává nezávislý repeated tip; úplný engine cleanup a životnosti jeho vazeb dále vyžadují audit. Stav celého projektu zůstává nedokončený.

Úplné Win32 sestavení a 95/95 CTest prošly za 472.04 s. Oba aktuální game-audio a oba PersonSounds buildy prošly; striktní zápis ověřil source/binary hashe a17 zachovaných originálů. Celou regresi eviduje `analysis/game-controller-regression.json`; nativní celý svět tím není prohlášený za ověřený.
