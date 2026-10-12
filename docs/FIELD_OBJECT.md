# FieldObject a základ akčních oblastí farmy

Zdrojový `FieldObject` doplňuje AnimationButton o HeightOffset. Je zapojený do `recovered_resource_utilities` a nové `ExtendedFieldButton` specializace, kterou používá původní cActionArea. Nejde o náhradu celého cActionArea nebo Warehouse; jejich skutečné závislosti se obnovují následně.

| Původní adresa | Obnovené chování |
| --- | --- |
| `00419680` | AnimationButton XML, potom HeightOffset float. |
| `004192a0` | Běžný Control.setPosition, potom depth = HeightOffset + nové y s float spill. |
| `00419a90` | Odečíst HeightOffset od root y, Control transform, přičíst offset zpět; obě operace spill. |
| `00417fe0` | Stejné odečtení/přičtení kolem původního Button.pointInRect. |
| `0041be20` / `004182b0` | Vlastní constructor/destructor 868bajtové tooltip specializace. |
| `0041c0e0` / `0041b010` | Field XML a skutečné tooltip umístění. |
| `0041ab10` / `0041b520` / `00419e70` | Wait/fade step a hover/unhover. |
| `0041b910` / `0041a250` | Enable/disable a převzetí vstupu. |

HeightOffset je na native `+27c`; tooltip fields jsou posunuté o čtyři bajty proti 864bajtovému ExtendedAnimationButton. Moderní zdrojové rozložení se za původní ABI nevydává. Vykreslení a hit test úmyslně neobnovují uložené y, nýbrž provedou původní float subtract/add, které mohou při opakování změnit zaokrouhlení. XML samotné zapisuje pozici přímo; nelze přesměrovat jeho původní base load na běžný Field setPosition.

PC53/PC24 porovnání: 38 862 tooltip stavů, 18 objects, 54 reloadů, 1 346 280 letter comparisons, 38 664 steps a 72 GPU snímků / 34 560 000 shodných RGB pixelů. Test vykonává vlastní původní specializaci, dvě XML fixtures a sedm skutečných Farm/ActionAreas XML roots. Dalších 3 072 field geometry states pokrývá šest offsetů (kladné/záporné, zlomkové a velké), 64 pozic a čtyři opakované transforms/hits. Source/native root geometrie, drift a celé sprite records souhlasí.

PC24 edge scénář doložil chybu společného `Control.hitTest`: původní `00413a50` uchovává half extents i bounds v x87, a tedy v device precision zaokrouhlí i přesný levý okraj před porovnáním. Obyčejná moderní double aritmetika to nezachovala. Metoda nyní používá již obnovené originalMultiply/Subtract/Add helpers. Tato změna vyžaduje celou regresi dotčených controllerů; úspěch samotného Field oracle neprokazuje úplnost herního světa.

```powershell
cmake --build build/windows --config Release --target text-buttons-reference-test
python tools/validate_field_object.py
```

Native old CRT/strings, resource/XML rozhraní a tutorial state jsou výslovné diagnostické adapters. Production nepoužívá původní kód. Zvukové requesty jsou porovnané na fixture; audible world behavior zůstává neověřený. Další práce: vlastní ActionArea offsets, overlay a klikací tasky, registrace Farm a teardown postavy, skutečný Warehouse a notifikace, cGame a hlavní EXE.

Plná regrese prošla 65/65 CTest (267,66 s), běžné i samostatné game-audio sestavení po 378 stavech / 36 playback progress / 16 pause kontrolách. Strict record ověřil source hashe a všech 17 nezměněných originálů. Přenositelný záznam: [field-object-validation.json](../analysis/field-object-validation.json).
