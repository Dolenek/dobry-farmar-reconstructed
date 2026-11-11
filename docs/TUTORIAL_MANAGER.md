# Správce tutoriálu a propojení s HUD

V obou celých hrách je doloženo všech 48 běžně vyvolávaných kroků. `analysis/current-tutorial-runtime-coverage.json` odkazuje na skutečné průchody. `analysis/tutorial-native-callers-audit.json` prověřil 44 původních volání pěti metod a nezávisle je vyhledal v instrukcích E8. Indexy 0 a 10 nemají běžného kompilovaného volajícího. Všech 50 datových záznamů prošlo samostatným porovnáním.

Při následném auditu pozorovacího nástroje se opravilo pomocné označení úrovně skladu: původní instrukce00468045/0046804d/00468056 předávají Upgrade propertyId32 a itemType12; studna používá33/13 podle0046b394/0046b39c/0046b3a5. Produkční třídy jsou správné. Starší warning logy omylem pojmenovaly hodnotu studny jako warehouse_level; u full-carry checkpointu tedy zapsaly2 místo skutečné skladové3. Neupravené XML dokazují obě hodnoty. Výpočty kapacity sekačky16, skutečné převody a pixelové výsledky tím nejsou dotčené; ostatní prověřené warning fixtures mají obě úrovně0. Původní raw logy se nepřepisují; opravu a dopad obsahuje `analysis/tutorial-warning-property-role-correction.json`.

`src/game/ui/TutorialManager.cpp` obnovuje správce se singletonem `0065f73c`, původní velikostí `0xc20` a padesáti 60bajtovými záznamy. Konstruktor vytváří Common, Arrow, Action, DisabledAnchor a RepeatedAction v původním pořadí, načítá jejich skutečné XML a `Tutorial/Tutorial.xml`, potom instaluje 29 podmínek. Herní svět musí dodávat skutečné objekty a primitivní operace prostřednictvím povinného `TutorialWorld`; rozhraní nemá výchozí implementace.

| Adresy | Chování |
| --- | --- |
| `004cfce0`, `004d0490` | Konstrukce, singleton, pole kroků a skutečná destruction pravidla |
| `004cdf70`, `004cdfd0`, `004ce160` | Defaulty, XML záznamu a celého správce |
| `004cdee0`, `004cdef0`, `004cdf10` | Aktivní krok, porovnání čísla a cancellation |
| `004cfbe0`, `004d0990`, `004d0a20`, `004d0ac0` | Klíče profilu, processed/canShow/complete |
| `004ce650`, `004ceb00`, `004cef60`, `004cf3c0`, `004cf7d0` | Pět způsobů otevření skutečného okna |
| `004d0c30` | Dokončení, obnovení výběrů a normálního/absolute enable HUD |
| `004d1160` | Automatický počátek zavlažování, návaznost na sklad a dokončení výroby |
| `004c9fc0`..`004ca610` | Všech 29 původních predicate funkcí |

Klíč je doslova `tip` + unsigned decimal, takže opakované dokončení kroku −1 zapisuje `tip4294967295`. `complete` zapisuje boolean, `justProcessed` a integer `tutJustProc`; krok 2 zapíná `TFastSpeed`, krok 4 jej vypíná. Zakázaný tutoriál nečte ani nevkládá profile boolean. Aktivní krok blokuje `canShow` před profile access. `justProcessed` a uložené enable příznaky jsou v native konstruktoru neurčené, dokud příslušná operace nezapíše hodnotu; diagnostika tyto počáteční neurčené bajty explicitně inicializuje.

Common okno zachytí enable dvou výběrů a HUD, poté zakáže absolute HUD, normální HUD, seed selection a building selection v tomto pořadí. Arrow a DisabledAnchor zachytí pouze HUD. Action/RepeatedAction nastaví gameplay pause, ale HUD nevypínají. `finishTip` validuje identitu, zpracuje aktuální číslo a nastaví −1; pouze první tři typy obnovují ovládání. RepeatedAction může zpracovat −1 při druhém dokončení ve stejném kroku.

Původní predicate 2 dokončuje krok při **rovnosti** stavu zavlažení a hodnoty objektu na `+8a8`, případně při aktivitě postavy 10. Getter `00437d90` čte float profilu `FFW` + id; nejde o odhadnutou růstovou mechaniku. Predicates 4/21 čtou aktivitu dvakrát, predicate 44 ji nejprve čte a zahodí a teprve pak může číst další dvě hodnoty. Tyto dotazy a short-circuit pořadí jsou zachované. Predicates 6/7 požadují pohyb obou příslušných scén.

Native `004d0490` zakáže a odloží pouze Common, Arrow, DisabledAnchor a Action, s origins `.\Game\Tutorial\TutorialManager.cpp122`..`125`. Páté RepeatedAction okno neodloží ani nezakáže; zůstává v registru View. Zdroj tuto zvláštnost zachovává. Diagnostika samostatně ověřuje jeho přežití a teprve potom jej ručně maže pro úklid testu. Další používání tohoto okna po zániku správce a jeho dangling kontext není ověřené.

Delší propojené vykreslování doplnilo přesnost samostatných oken: jejich pole na `+34` je původní **hloubka**, nikoli alpha. Umístění vždy volá skutečný setter depth (`004f1080`, virtual `+d4`), včetně aktualizace potomků. Samostatný test nyní také porovnává uloženou hloubku.

`python tools/validate_tutorial_manager.py` ověřuje PC53 a PC24: 2 500 stavů, 334 explicitních startů, 280 kroků správce/akčních oken a 14 848 vyhodnocení podmínek. Všech 50 záznamů se otevírá přes skutečné common/arrow/disabled-anchor metody. Predicate matrix porovnává výsledky i přesné pořadí world dotazů, včetně měnící se aktivity mezi dotazy. 256 enabled-step scénářů porovnává automatické otevírání, elapsed a profil.

204 skutečných D3D8 snímků / 97 920 000 shodných RGB pixelů propojuje původní i zdrojový TutorialManager, GameMenu a všechny typy oken v dispatcheru. Zahrnuje hover/press/release OK, obnovení HUD, cancellation a dvojí dokončení RepeatedAction. Runtime adaptér světa dodává řízené hodnoty; viditelnost dialogů se testuje pomocí skutečných View stavů. Farm/Warehouse/Manufacture/character a plné dialogové factories musí být ještě propojeny se skutečnými herními objekty. Manager ve frame scénářích má automatické scheduling vypnuté; jeho enabled-step body se samostatně vykonává a porovnává v uvedené matici.

Původní programový kód běží pouze v diagnostickém oracle a entry point se nespouští. XML/resources, typed profile a staré CRT jsou explicitní adaptéry. Reverse vector destruction volá každý native record destructor; exception cleanup není tímto adaptérem ověřené.

Zbývá skutečný svět a cGame, celý průchod tutoriálem při hraní a obousměrné saves, extrémní/NaN predicate hodnoty, XML reload/default override/parent-disabled matice, invalid-step/null-predicate/constructor výjimky. Celý projekt včetně hlavního EXE, úplného Squallu a izolovaného odinstalátoru není dokončený.
