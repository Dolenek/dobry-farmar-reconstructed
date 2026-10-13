# MowBuilding – nástroj pro sklizeň

`src/game/ui/MowBuilding.h/.cpp` obnovují skutečný cMowBuilding nad ActionArea/Upgrade a vlastněným UI. Oblast type8, singleton `0065f754`, item11/property31. Povinné MowBuildingWorld rozhraní vyžaduje skutečné Farm/Person operace bez náhradního provideru. Samotný controller je propojen s profilem, GameConfiguration, shared Upgrade, TutorialManager a skutečnými UI základy.

PC53/PC24 porovnání: 1 748 stavů, 236 explicitních actions, šest XML reloadů a 120 GPU snímků / 57 600 000 shodných RGB pixelů. Actual native Person getter a PCC sum běží proti uint profilu. Je porovnáno 1 024 held/level/PCC patterns, wrap32 sum, všech osm product icons, null/present Person, mask data, Money/availability, mouse upgrade, tooltip/input, nápovědy27/47 a resource reference deltas. Person display/animation/refresh a skutečný Farm zůstávají výslovné diagnostické hranice.

Native size2244 / `0x8c4`: singleton MI+504, Upgrade+508, item+50c, property+52c, levelCount+530; čtyři224b level records od+534. ProductionNum+0 a MowTime+4 ctor neinicializuje; actual XML definuje všechny čtyři. Normal+8, hovered+48, mask String+88, bare+116, bare-hover+156, bare mask+196. Upgrade+8b4, Price+8b8, WotInScythSign Picture+8bc, jeho child WotSubstrate Picture+8c0.

| Adresa | Obnovené chování |
| --- | --- |
| `00449560` | ActionArea8 → singleton → Upgrade11/31 → levels → upgrade/sign/substrate/price → XML → callback → refresh. |
| `004473b0` / `00447490` | Levels destructor, Upgrade, clear singleton, ActionArea owned UI; scalar wrapper. |
| `00447260` / `004472f0` | Level ctor/dtor; numeric fields unspecified do XML. |
| `004474b0` | ActionArea/Upgrade XML, UpgradeButton/CostText/WotInScythSign/WotSubstrate. |
| `004483e0` | ProductionNum → mask → MowTime → normal/hover → optional bare mask/appearances. |
| `00448b60` | Held6 zvolí bare. Jinak normal; level<3 a held count!=0 vybere product icon podle prvního nenulového PCC0..7 a ukáže sign. Jinak sign hide. |
| `00449050` | Level−1 return, refresh appearance; existující Person s held6: aktuální sum, hide Person+2850, display(true,count,ProductionNum); price/availability/visibility. |
| `00447b40` | Investment costs i=1..<level, wrap32. |
| `00447f30` | Farm64/32 gates → ordinary Button.clicked, jinak actual ActionArea.clicked. |
| `00448330` | Shown upgrade a Farm shown/not moving: level0 canShow27 nebo level2 canShow47 → actual arrow. |
| `004471a0..00447255` | Money minus next cost, setLevel(next,true), při held6 animator from Person to Mow, kind=currentLevel+4,false → Person.refreshHeldAppearance. |
| `0044ec00` / `0044ed10` | Uint PCarry getter a wrap32 součet osmi uint PCC. |
| `004036e0` | ProductInfo pointer, nikoli SeedInfo. Mow kopíruje small icon na ProductInfo+16. |

Primární vtable `005f2988` má91 položek, MI `005f297c` dvě a Upgrade `005f296c` tři. Upgrade callback není indexován Ghidrou; skutečný RET konec je ověřen disassembly.

Diagnostický Person má dostatečnou původní velikost `0x2b30`, vlastní skutečný Source/native View pro +2850 hide a zaznamenává original display/animator arguments. Celý Person display `0044ee70` tím není obnovený. Farm je stále StateView fixture a tutorial jej dočasně odpojí od hidden game owner; HUD při resume čte skutečnou investici Mow. CGame Money clamp a disabled audio backend jsou samostatné hranice. Zbývá full Farm/Person/cGame, actual harvest/task controllers, celý source EXE/playthrough a bidirectional saves; malformed XML/invalid levels/exception ABI zatím nejsou doložené.
