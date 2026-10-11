# GameMenu: herní HUD a propojení controllerů

`src/game/ui/GameMenu.cpp` obnovuje 232bajtový původní HUD se singletonem `0065f740`. Vlastní čtyři ExtendedTextAnimationButton, JournalButton, MoneyText, skladové tlačítko, progress, lighter, tři ContractStatus a samostatný GamePauseMenu. Používá skutečné `UI/gameUI/GameMenu.xml`, profil a již obnovené UI; implementace nepoužívá původní programový kód.

| Původní adresy | Obnovené chování |
| --- | --- |
| `004c3430`, `004c31f0` | Vlastnictví, pořadí konstrukce, singleton, obnova tří uložených zakázek, destruction |
| `004c1f90` | XML, skutečné texty a zvuky, posun substrate zakázek |
| `004c02c0`, `004c06b0` | Aktivace prvního volného statusu, přesný guard count == 3, odstranění podle identity |
| `004c4390`, `004c44a0` | Vlastněné 32bajtové zprávy, příznaky přečtení, journal a zvuk |
| `004bef70`, `004bf4d0`, `004bf9e0` | Farm/Market/Ware přechody skutečných StateView, enable tlačítek, overlays a warehouse tip |
| `004bd950`, `004be740`, `004c2cc0` | Dostupnost scén, skutečné unsigned UpAvail0..10 a počáteční Farm |
| `004bdf00`, `004be340` | Uložení a obnovení dostupnosti tlačítek |
| `004beb40`, `004bfee0` | Skladové procento a signed runtime Money text |
| `004c30a0`, `004c3130` | Escape binding, zachycení pěti nástrojů, stop/resume v pořadí 2,6,1,0,4 |
| `004c0750`, `004c1b60`, `004c0b50`, `004c0f70`, `004c1350`, `004c1770` | Pause, Escape a callbacky scén/journal včetně tutoriálu |
| `004c3920` | RunTime, GameTime, wealth/level triggers, blikání zpráv/skladu, tutorial anchors |
| `004bd830`, `004bd8a0`, `004bd8f0`, `004bd920` | Původní vlastní Button pro sklad; hover/enable otevírá tip a lighter |
| `004bd9d0`, `004bb330` | Stock notifikace pro všechny aktivní statusy, actual ContractPassConfirm |
| `00413c50` | Control color setter, původní float rovnost a dirty příznak |

Skladový ovládací prvek je obyčejný `Button` s vlastními hooky, nikoli AnimationButton. `ContractStatusPlaces` posouvají pouze substrate. Původní XML používá unsigned formatter `0041db60`, průběžný Money update signed `004bda10`. `UpAvail` jsou uints; použití ints měnilo dostupnost vylepšení a bylo opraveno proti instrukcím.

Callback Farm `004c0f70` nemá globální blokovací test `0065f720/721`, který mají Market a Ware. Journal jej rovněž nemá. Escape navíc odmítá pohybující se scénu; MainMenuButton toto omezení nemá. Zprávy deep-copy texty a read flag; prázdný vector sám neschová journal. Odstranění statusu neověřuje active a unsigned count se může podtočit. Inconsistent stav se třemi active statusy a count != 3 používá v originálu neurčený pointer; zdroj tuto neověřenou větev explicitně odmítá.

`ContractStatus::stockChanged` vždy obnoví text požadavků. Preview nemění initialStocks ani dialog. Readiness používá unsigned zásoby, porovnání nové zásoby s předchozí je původní signed `jge`. Při existujícím, skrytém ContractPassConfirm otevře skutečný dialog; při již viditelném dialogu pouze označí status dirty.

GameMenu vyžaduje skutečné světové rozhraní `GameMenuWorld` a `ContractStatusWorld`. Neobsahují výchozí implementace ani náhradní mechaniky. Bohatství, sklad, výroba, postava, tutoriál, gameplay a Messages musí poskytovat rekonstruovaný svět. GamePauseMenu je skutečný controller; jeho celá navigace je ověřena samostatně.

`python tools/validate_game_menu.py` porovnává PC53 a původní device PC24. Celkem 4 778 stavů, 69 954 písmen, 1 150 akcí a 336 explicitních HUD kroků. Všech 131 skutečných kontraktů se aktivuje do tří skutečných ContractStatus; stock změny otevírají skutečný potvrzovací dialog. Znovuvytvoření HUD obnoví task/time z profilu. Porovnávají se všechny profilové mapy, zprávy, scalars, kontrolní prvky, scény a přesné pořadí světových požadavků. Součástí jsou wealth/level gates, warehouse capacity nula, tool flags a tutorial short-circuit.

242 GPU snímků / 116 160 000 shodných RGB pixelů zahrnuje skutečný timer/cursor, původní frame dispatcher, myšové press/release přechody mezi scénami, hover skladu a Escape/Enter pause. Native instrukce controllerů a widgetů se vykonávají pouze v diagnostickém programu; world/old CRT/XML resource hranice jsou explicitní testovací adaptéry. Použití skutečných XML ve společném StateView neověřuje odvozené Farm/Manufacture/Ware controllery.

Zbývá skutečný cGame a svět, tutoriál/Messages controller, všechny tool/transition kombinace, HUD pause button mouse cesta, extrémní float/error/XML reload větve a fyzický vstup/zvuk. Hlavní herní EXE, obousměrné saves, kompletní Squall a izolovaný odinstalátor nejsou dokončené. `analysis/game-menu-validation.json` eviduje přenositelný rozsah a hash zdrojů; dokončení tohoto controlleru není dokončením projektu.
