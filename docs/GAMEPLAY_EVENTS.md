# Definice Gameplay a herní čas

`GameplayEventDefinitions` obnovuje datový loader `004b50d0`, všech deset typů událostí a jejich původní XML loadery. Jde o skutečná data pro navazující runtime; vykonávání událostí a celý svět ještě nejsou obnovené.

| Typ | Loader | Data |
| --- | --- | --- |
| 0 DepotUpgrade | 004b1710 | Upgrade, DepotType |
| 1 SeedAvailable | 004b1bd0 | SeedType |
| 2 Money | 004b2040 | RandomFactor, Value |
| 3 DestroyField | 004b2500 | RandomFactor, Value |
| 4 DestroyDepot | 004b29c0 | RandomFactor, Value |
| 5 Contract | 004b3300 | WinTime, Bonus, Penalty, RandomFactor, nejvýše 2 Requirements, WinNote, **LosNote**, LetterTime |
| 6 Popularity | 004b4b60 | Nejvýše 19 ProductType/Value záznamů, současný a výchozí vektor |
| 7 FieldUpgrade | 004b3a00 | UpgradeIndex |
| 8 Trigger | 004b3e70 | Money, WareLevel (loader vždy nejprve nastaví -1) |
| 9 End | 004b4340 | Pouze společná data |

Společný loader `004b1240` načte Time, Note a Data.Range. Time se porovnává lexikograficky po třech unsigned složkách. Nový záznam se vloží před první přísně pozdější čas; shodné časy zachovají XML pořadí. Reload nevymaže starý seznam. Při neprázdném seznamu se aktualizuje konečný čas, příznak nulového plánu a všechny indexy; poslední nalezený typ 9 určí endEventIndex.

Contract kopíruje bonus/penalty a oba 12b požadavky do výchozího stavu i při chybějícím Data. LetterTime je na původním +a0 a před čtením atributu dostane 60 sekund. Popularity při loadu přidává do současného vektoru, poté jej vždy kopíruje do výchozího. Nepřítomné atributy ponechají hodnoty; původní konstruktory některé skaláry neinicializují. Zdroj z nich nevyrábí odhadnuté herní defaulty.

`GameplayTime::fromSeconds` obnovuje `004adac0`, včetně původních float konstant, nepřerušeného x87 zbytku, wrap32 násobení unsigned hodin/minut a truncace na dolní DWORD. PC53 i PC24 prošly celkem 417 286 přímými porovnáními původních instrukcí: 200 000 libovolných float reprezentací a 8 643 hraničních vstupů na kontext. Porovnány jsou hodiny/minuty/sekundy a zachování řídicího slova; další rounding modes a exception flags zatím nejsou doložené. Source ASM je MSVC Win32; portable fallback nemá tuto úplnou shodu pro NaN/inf ověřenou.

`python tools/validate_gameplay.py`: skutečný Gameplay/Gameplay.xml obsahuje 383 událostí, počty typů 0–9 jsou 22/7/43/7/4/131/155/3/10/1. Oba kontexty porovnaly 4 772 celých událostí, 1 588 kontraktů, 1 876 popularity záznamů a 20 stavů plánu při 16 reloadech. Dodatečné XML ověřuje obrácené pořadí, shodné časy, ignorovaný typ 33, limity požadavků a popularity. Native diagnostika provádí skutečné konstruktory záznamů, loadery, třídění a destrukci timeline, nikoli celý konstruktor Gameplay.

XML tree, typed numeric čtení a string-time parser `004b0b70` jsou explicitní hranice starého CRT. Parser času používá nezávislý diagnostický adaptér; jeho vlastní nativní kód není tímto scénářem ověřen. Native testovací heap je zeroed, proto chybějící neinicializované skaláry nejsou tvrzené jako původní defaulty normálního malloc. Původní `004aeb10` uvolní společný Note a událost; odvozené contract strings/popularity storage zde ponechá. Source RAII je při destrukci uvolní; nejde o tvrzení shodného allocation ABI.

Pokračuje runtime `004b59c0`, `004b5c60`, randomizace `004ae570`, napojení skutečných profilů a všech světových callbacků. Celý cGame, tutoriál, save/load a hlavní herní EXE stále čekají. Přenositelný výsledek je v `analysis/gameplay-validation.json`.
