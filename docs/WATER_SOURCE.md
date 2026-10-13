# WaterSource – studna a ovládání postřikovače

`src/game/ui/WaterSource.h/.cpp` obnovují skutečný cWaterSource, ActionArea type4, singleton `0065f750`, Upgrade item13/property33. Povinné rozhraní WaterSourceWorld vyžaduje skutečné Farm/Person operace, nemá výchozí náhradní implementaci. Samotná studna je propojená s původními UI základy, shared Upgrade, profilem a TutorialManager; celý Farm/Person se dále rekonstruuje.

Přímé PC53/PC24 porovnání studny: 1 040 stavů, 534 explicitních actions, šest XML reloadů a 128 GPU snímků / 61 440 000 přesně shodných RGB pixelů. Původní Person held getter/setter běží skutečně proti uint mapě PCarry. Scénář zahrnuje upgrade callbacks, held snapshot, reset drženého nástroje, podmíněný task discard, kapacitu/investici, sprinkler UI a hloubku při přesunu, skutečný mouse upgrade, nápovědy25/46/29, HUD stop/resume a destrukci/refcounts. Jeho původní factory pozorování zůstává diagnostickou hranicí. Nový propojený scénář v [SPRINKLE.md](SPRINKLE.md) už vykonává skutečnou source/native Farm factory00433980 přes actual WaterSource callback a obnovuje vlastní controller i emiter postřikovače. Celý Farm/Person/cField stále čeká.

Native size2360 / `0x938`: ActionArea do +503, singleton MI+504, Upgrade+508, item+50c/property+52c/levelCount+530, SprinkleCost+534. Čtyři level records od +538 mají stride220: capacity+0, normal+4, hover+44, mask String+84, bare+112, bare-hover+152, bare mask+192. Actual XML definuje tři úrovně; čtvrtý capacity word ctor neinicializuje a test jej nečte. UI: upgrade+8a8, price+8ac, sprinklePrice+8b0, sprinkleButton+8b4. Tři SprinkleButtonStates od +8b8 stride40; moving depth+930, XML button depth+934.

| Adresa | Obnovené chování |
| --- | --- |
| `0046b330` | ActionArea4, singleton, Upgrade13/33, levels, tři sprinkle states; button/button/price/price, XML, sprinkle price string, dva callbacks, refresh. |
| `00468640` / `00468730` | Sprinkle states → levels → Upgrade → clear singleton → ActionArea owned UI; scalar wrapper. |
| `004684f0` / `00468580` | Level ctor/dtor; capacity zůstává unspecified. |
| `00468750` | ActionArea/Upgrade XML, SprinkleCost, čtyři UI sections, uložená default depth a moving depth, nejvýše tři states. |
| `0046a560` | WaterCapacity, normal mask/appearance/hover a optional bare mask/appearances. |
| `00469bd0` | Held2 zvolí bare; jiné slovo normal; mask flag, retained mask data a dirty. |
| `0046acb0` | Level−1 přeskočí hlavní refresh, ale ještě volá refreshSprinkle. Cena/availability/visibility; při currentLevel!=0 a held1 setHeldType0, Person appearance update a cancel. |
| `00468f60` | Jen level2. Count<3 copy/show; jinak hideTips/price hide. Moving flag volí hloubku; seed/sprinkle move nebo nedostatek Money disable. Cost se porovnává jako signed word. |
| `004683a0` / `00469400` | Kapacita (level−1 nula, x87 výsledek) a investment i=1..<level. |
| `0046a010` / `004684c0` | Upgrade button rectangle bez masky, potom Farm64/32 guards; actual ActionArea click. Release má při level!=2 ještě tipsDirection=1. |
| `0046a470` | Farm shown/not moving, shown upgrade a úrovně0/1 → arrow25/46; shown sprinkle → arrow29. |
| `00468420..004684bb` | Upgrade callback zachytí heldType před změnou, odečte Money a setLevel(next,true). Původně held1 spustí animator kind0/false, i když refresh mezitím změnil heldType. |
| `004697f0` | Actual Farm.createSprinkle(cost); nenulový návrat dostane ButtonState3. |
| `0044ec00` / `0044ec80` | Person held getter/setter uint PCarry. |

Primární vtable `005f47e0` má91 položek, MI `005f47d8` dvě a Upgrade `005f47c8` tři. Callback68420 není v function indexu; konec RET byl ověřen disassembly.

Refresh při held1 nastaví Person+284c=1 a +2838=0; pokud +283c obsahuje task, vyžádá discard s řetězcem `d:\work\farmer\src\farmer\game\farm\Person.h157`. Původní held accessors mají skutečná těla; ostatní Person update/animator/discard jsou povinné pozorované hranice. Diagnostický Person rezervuje dost místa pro tyto skutečné offsety a source HUD dostává shodnou přítomnost postavy.

Zbývá skutečný Farm/Sprinkle a Person controller, celý cGame/Money effects, audible audio, full game EXE/playthrough a obousměrné saves. Test fixture nepředstavuje produkční náhradu těchto částí. Malformed XML, invalid level/exception ABI a další constructor scénáře dosud nejsou ověřené.
