# cGameInput a původní textové příkazy

`GameInput` obnovuje singleton0065f6e4: constructor004030e0 registruje character callback00402400 s vlastním object payload, destructor00401de0 volá Input.vtable005ffc48+18 =00524e60. To odstraní všechny character handlers, následně vyprázdní procesový WString0065f704 a uvolní singleton. Kapacita bufferu zůstává zachovaná. Constructor buffer nemaže.

Callback přidá jednu UTF16 code unit; při délce>20 odstraní právě první. Helper00402240 zkouší konec bufferu, zkopíruje jej a přes00401140/00580415 převádí dočasnou kopii na malá písmena. V původním C locale se mění jen ASCII A..Z; celý buffer uchovává původní case. Po úspěchu se celý buffer vyprázdní. Nejde o case-sensitive porovnání naznačené dřívějšími poznámkami. Native CRT locale TLS je výslovná diagnostická hranice; skutečné původní towlower tělo se vykonává pro všech65536 code units.

| Suffix | Původní tělo | Skutečný zdrojový účinek |
| --- | --- | --- |
| cheatmoney |00412630| Money+1000 s32bitovým wrap, potom skutečný setter00410d60 s clamp na0 a navazujícími UI/upgrades/effects. |
| cheatupgrades |00412650|15 typed UInt UpAvail=3, pouze index13=2; skutečné refreshUpgrades a HUD.refreshWareAvailability. |
| cheatseeds |004126b0|8 typed UInt SeedAvail=1; žádný dodatečný refresh nebyl do původní funkce přidán. |
| cheatproduct |0040c360|19 skutečných Warehouse.give(kind,2,true,false), omezených kapacitou skladu; poté HUD.refreshMarketAvailability. |
| cheatall |00402400| Win32 Beep(1000,200), následně uvedené čtyři funkce v témže pořadí. |
| a01..a21 |00402400 disjoint tail do00402d7a| Nezávisle na přítomnosti cGame nastaví skutečný GameAwardManager.requested=0..20, pokud tento singleton existuje. |

Prvních pět příkazů se testuje jen při existujícím cGame; award manager má samostatnou podmínku. Produkční constructor používá skutečné Game/Award singletons a Win32 Beep. V hlavním EXE se GameInput vytvoří před Startup a odstraní po Game/Profile před engine vstupem.

## Přímé porovnání

`game-input-reference-test` mapuje původní EXE mimo preferovanou adresu bez entry pointu. Skutečné původní callback/string/lifetime funkce mají80 doložených testovacích operand fixupů. Šest životních cyklů porovná33345 stavů /33333 znaků a1038 přesných service událostí. Ověřené jsou všechny21 award suffixes, všechny5 game příkazy, mixed/upper case, nepřítomné singletons, rolling limit, UTF16 nula/surrogate/ffff, kapacita bufferu a odstranění i cizího handleru v destruktoru.

Native game action entry points jsou v tomto scalar testu výslovné service hranice, nikoli obnovená implementace světa. Zdrojové produkční metody pracují se skutečným ProfileProperties/Warehouse/HUD. Celý proces má samostatné porovnání; řízené frame/effect/award lifecycle a úplné herní chování nejsou prokázané samotným tímto testem.

Metadata: `analysis/game-input-validation.json`; další plné runtime a regresní výsledky budou doplněné po jejich dokončení.

## Skutečné účinky v celých hrách

`validate_game_input_runtime.py` vytváří čtyři samostatné procesy: originál/zdroj pro běžných20 peněz a pro2147483300. Pracuje s nezávislými kopiemi skutečně původním EXE vytvořeného profilu; v overflow variantě jsou již získaná ocenění výslovně nastavena v testovací fixture, aby neotevřela nesouvisející panel. Skutečný Alt+Pause zastaví Logic stepping, ale textový vstup zůstává aktivní. Příkazy jsou zaslané do Win32 WM_CHAR fronty vlastního foreground okna a projdou skutečným InputDevices/GameInput, nikoli přímým voláním cheat metod.

Sekvence `CHEATALLcheatmoneyCHEATUPGRADEScHeAtSeEdScheatproduct` končí2020 nebo1000 penězi; v druhém případě se první add wrapne do záporné hodnoty a skutečný setter ji omezí na0. Všech15 upgrade/8 osiv hodnot souhlasí; původní sklad s kapacitou15 obsahuje2 kusy produktů0..6 a1 kus produktu7. Zbytek19 produktů je0. Všech790 profilových vlastností souhlasí kromě RunTime/GameTime; obě1981byte binární saves jsou přesně stejné. Dlouhé běžící efekty peněz a růst nejsou součástí tohoto pozastaveného scénáře.

`validate_game_awards_runtime.py` navíc projde21 celých native/source cyklů:20 původních textových požadavků a automatické collect-all po potvrzení prvních20 podmínek. Každý vykoná skutečný AwardEffect → Awards → AwardPanel → OK; po návratu se obnoví skutečný Game (mezi20. a21. se čeká přímo na automatický další panel). Všech21 condition flags a awardShow0..20 jsou skutečně commitnuté;790 profilových hodnot souhlasí mimo hodiny a1981byte save je shodný. Statická oblast panelu s textem `[250,370,550,465]` souhlasí ve všech598500 RGB pixelech. Celé časované snímky ani přirozené splnění ostatních20 podmínek se tím neprokazují; zvlášť přetrvává kontrola přirozených světových podmínek7/9/18.

## Regrese

Úplné Win32 Release sestavení a101/101 CTest za471.82s prošly. Obě aktuální audio sestavení a PersonSounds kontroly, Squall runtime a obě loaded-DLL kontroly prošly. Celý zdrojový proces znovu ověřil profil, farmu, restart/restore, pause/Help/Journal; dále prošly úvodní i bohatší obousměrné saves, fyzické zkratky a změny režimu a nové čtyři GameInput / dva21-award procesy. Striktní hash kontrola všech současných zdrojů, binárek, výstupů a17 originálů prošla. Souhrn je v `analysis/game-input-regression.json`; úplný projekt nadále pokračuje.
