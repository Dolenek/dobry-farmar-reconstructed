# DigBuilding – rekonstrukce kultivátoru

`src/game/ui/DigBuilding.h/.cpp` obnovují skutečný cDigBuilding nad ActionArea, Upgrade, ExtendedTextAnimationButton a TextMoney. Oblast má type16, singleton `0065f74c`, item14/property34. Ověřené Warehouse/Market implementace zůstávají zachované. Povinné rozhraní DigBuildingWorld nemá fallback; skutečný Farm/Person a celý cGame se dále obnovují.

`python tools/validate_dig_building.py` porovnává původní těla a rekonstruované zdroje v PC53 i PC24: 698 stavů, 236 explicitních actions, osm XML reloadů a 104 GPU snímků / 49 920 000 shodných RGB pixelů. Obsahuje skutečný mouse upgrade, held types 0/1/2/6/−1, všechny úrovně, každou mask column/bit, Money/availability kombinace, Farm cursor gates, nápovědu28 a její HUD stop/resume účinky. Společný warehouse/production scénář probíhá před DigBuilding. Jde o porovnání propojených controllerů, nikoli důkaz celé hratelné hry.

Native size2220 / `0x8ac`: ActionArea do `+503`, singleton MI `+504`, Upgrade `+508`, item `+50c`, property `+52c`, levelCount `+530`; čtyři records od `+534` se stride220. Level: DigTime+0 (ctor jej neinicializuje), appearance+4, hovered+44, mask String+84, noInstrument+112, noInstrumentHovered+152, bare mask String+192. UpgradeButton `+8a4`, CostText `+8a8`. Moderní source layout se může lišit.

| Adresa | Obnovená operace |
| --- | --- |
| `00429fe0` | ActionArea16 → singleton → Upgrade14/34 → čtyři levels → button/price → skutečné XML → callback → refresh. |
| `00426470` / `00426550` | Levels destructor, Upgrade, singleton clear, ActionArea owned UI; scalar deleting wrapper. |
| `00426320` / `004263b0` | Level appearance/String konstrukce a destrukce. |
| `004265e0` | ActionArea XML → Upgrade XML → UpgradeButton → CostText, bez automatického refresh na konci. |
| `00427bf0` | ECX Upgrade; DigTime, normal appearance/mask, optional Hovered a NoInstrument; chybějící NoInstrument zachová bare fields. |
| `00426b10` | Held type2 zvolí bare appearance/mask; jiný typ normal. Empty mask pouze vypne mask flag, retained data nemaže. |
| `00427740` | Level−1 return; appearance, cena dalšího level, visibility, Money/availability enable a hideTips ve skutečném pořadí. |
| `00426f50` | Unsigned součet upgrade costs pro i=1..<currentLevel. |
| `00448b20` / `0045f210` | Sdílený upgradeStrings[level+1] tooltip hover/unhover. |
| `00427340` | Farm cursor types64/32 → ordinary Button.clicked, jinak skutečný ActionArea.clicked. |
| `00428310` | Field tooltip step, shown upgrade, Farm shown/not moving a canShow28 → actual arrow28. |
| `00426260` | canUpgrade11 → Money minus cost → setLevel(next,true); held2 pak animator `0042b7a0` (Person.position, Dig.position, currentLevel,false) → Person appearance `00450f90`. |

Primární vtable `005f0970` má 91 položek, MI `005f0968` dvě a Upgrade `005f0958` tři. Callback `00426260..00426312` není v původním function indexu; diagnostika používá ověřené instrukční hranice.

V diagnostice jsou Person held getter, queue, animator a appearance refresh povinné pozorované hranice. Farm je stále StateView fixture; tutorial scénář jej dočasně odpojí od hidden game owner, aby skutečně provedl původní visibility guards. HUD dostává stejnou informaci o přítomnosti postavy jako reference. Native/source investice Dig jsou při obnově HUD skutečné. cGame Money clamp a physical audio zůstávají samostatné oblasti práce; disabled SoundManager fixture ověřuje argumenty, nikoli slyšitelný výstup. Malformed XML, další ctor scénáře, invalid levels a exception ABI dosud nejsou doložené.
