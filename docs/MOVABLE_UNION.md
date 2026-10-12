# cMovableUnion: základ nákupních panelů

`src/game/ui/MovableUnion.cpp` obnovuje skutečný 636bajtový `cMovableUnion`, ze kterého vycházejí BuySeeds (`0065ff64`) a BuyBuilding (`00660010`). Je odvozený z Picture; pohybuje celou svojí geometrií a potomky. Vlastní Control origin a list Picture pozadí. Zdroje nepoužívají původní programový kód.

| Adresy | Operace |
| --- | --- |
| `00413780`, `00413fa0` | Konstruktor origin, list pozadí a vlastnictví při destruction |
| `00414d60` | Picture XML, append Background, MoveSpeed, HiddenPlace a původní pozice |
| `00414070`, `00414430` | Původní rise/unriseControls: show/hide origin |
| `004147f0` | Stav pohybu, from/to, x87 duration, velocity a počátek přesunu |
| `004148e0` | Absolute krok, dokončení, hide při stavu 1 a obnovení origin/enable |

`+248` je integer stav pohybu. Tutoriálové predicates 13 a 32 čtou právě tento stav, nikoli pointer vybraného itemu. Světové rozhraní správce je proto upřesněné na `seedSelectionMoving` a `buildingSelectionMoving`. Původní defaulty MoveSpeed a remaining jsou neurčené do XML/pohybu; diagnostická alokace je explicitně inicializuje.

Opakované XML přidává další pozadí. Origin i pozadí jsou vlastněné stromem View, jejich separátní seznam je pouze evidence. Move(0) mění stav bez změny from/to či geometrie. Při přesunu origin zůstává skrytý, po dokončení se ukáže a Union se zapne. Na rozdíl od StateView se zde nevolá derived enterState hook.

`python tools/validate_movable_union.py` porovnává PC53/PC24: 11 604 stavů, 1 036 pohybů, 8 192 kroků a čtyři XML reloady. Kontroluje skutečné `BuySeeds.xml` a `BuyBuilding.xml`, vlastnictví, celé controly, origin, pozadí a přesné float hodnoty. Zahrnuté jsou nulový stav, nulová rychlost i nulová vzdálenost. Degenerate probes mohou zanechat NaN geometrii potomků; před viditelnými scénáři diagnostika výslovně obnoví zachycené počáteční pozice. Toto obnovení není vydáváno za automatickou opravu v produkční hře.

144 D3D8 snímků / 69 120 000 shodných RGB pixelů používá skutečný HUD, scény a oba obnovené základy jako jeho overlay objekty. Původní i zdrojové HUD callbacky během scénáře skutečně pracují s jejich enable stavem. Porovnává se pořadí spritů i draw API, konečný obraz a referenční počty textur při zániku.

Dodaná XML mají Background prázdné; neprázdné přidávání pozadí dosud není ověřené samostatným fixture.

Zbývají celé BuySeeds/BuyBuilding controllery, jejich záznamy, tlačítka a skutečný nákup do Farm/warehouse/character, úplná parent-disabled/error matice a extrémní NaN payloads. Herní EXE, obousměrné saves, kompletní Squall a izolovaný odinstalátor nejsou dokončené. Dokončení společného základu není dokončením nákupu ani celé hry.
