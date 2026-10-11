# Datové tabulky cGame

`GameConfiguration` obnovuje datovou část `00413210` a XML loader `0040e6e0`. Původní hra má 19 záznamů produktu po 100 bajtech od +30, 8 semen po 44 bajtech od +79c, 15 × 4 uint cen od +8fc a 5 WString po 28 bajtech od +a10. Moderní třídy netvrdí shodu objektového ABI. Úplný cGame/world ještě není obnoven; engine musí před datovým loaderem provést skutečný `View::loadXML`.

| Původní funkce | Obnovené chování |
| --- | --- |
| 00403610 / 00403660 | Konstrukce/destrukce dvou nezávislých cAnimable na product +10/+38. Skaláry nejsou v původním konstruktoru inicializované. |
| 004036b0 / 004036c0 | Jedna nezávislá appearance na seed +4; Price konstruktor nenastavuje. |
| 0040af00 | CreationTime float +60, DieTime float +0, PriceRadius int +4, GoodAttPriceRadius int +8, MaxSellFrequency float +c; Icon a IconLarge. |
| 0040b560 | Price uint +0 a Icon. |
| 0040e6e0 | View XML první; ProductionInfo, Costs, Seeds, UpgradeStrings, UpgradeSound v tomto pořadí. |
| 004d2080 / 004d2130 / 004d22a0 | Samostatný vlastník 2D texture, frames/length/loop/atlas/playing, release/get na XML výměně a frames omezené součinem columns × rows. |

Produkty a semena se přiřazují podle seřazené XML enumerace; původní loader nečte `Id`. Costs omezuje počet řádků na 15, počet hledaných úrovní na menší z počtu sekcí a 4. Klíče tvoří `Up` + index, tedy samotná přítomnost Up3 při jediné sekci nezapíše cenu třetí úrovně. Chybějící sekce a prázdné atributy zachovají předchozí hodnoty. UpgradeStrings vezme nejvýše 5 potomků a jejich atribut `string` jako WString.

UpgradeSound při neprázdném jméně získá další cache referenci bez uvolnění předchozí. `00410830` na začátku destrukce uvolňuje pouze současnou +a08; zdroj zachovává pozorované původní chování. Starší reference zničí teardown ResourceManageru. Herní objekty ani dynamické systémy se tím nenahrazují.

`python tools/validate_game_configuration.py` porovnal PC53/PC24: 16 diagnostických objektů, 144 stavů celé tabulky, 96 XML reloadů, 6 624 stavů appearance a 8 640 cen. Používá skutečný game.xml, prázdný/částečný XML, přímé record loadery a skutečné D3D8 textury; všechny reference na textury se porovnávají při každé výměně i zničení. Native fixture provádí skutečné konstruktory/destruktory záznamů a původní XML instrukce, nikoli celý konstruktor cGame. Původní neinitializované skaláry se pro missing-field testy explicitně osévají stejnými hodnotami.

Typové numeric čtení, XML tree a Up formátování jsou uvedené diagnostické hranice starého CRT. SoundManager je disabled; jde o skutečné resource vlastnění, nikoli důkaz přehrávání. Kompletní singleton/virtual hooks cGame, gameplay, tutoriál, svět, save/load a herní EXE pokračují. Původní product/seed loader neomezuje počet položek; pro nepůvodní overflow vstup moderní komponenta explicitně vyhodí chybu. Mesh/Animation větve explicitně odmítá do obnovení jejich továren. Rozsah a zbývající oblasti eviduje `analysis/game-configuration-validation.json`.
