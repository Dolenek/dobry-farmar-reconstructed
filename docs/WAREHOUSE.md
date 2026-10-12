# Warehouse – probíhající rekonstrukce

`src/game/ui/Warehouse.h/.cpp` jsou sestavitelné zdroje skutečného cWareHouse, navazující na [ActionArea](ACTION_AREAS.md), Upgrade, Manufacture a GameMenu. Není to náhradní sklad v service fixture. Profilové zásoby, transfery, vlastní ukazatel zaplnění, vlastněné UI a přímo volané Manufacture/HUD metody jsou implementované. Farm/Person, obchod, tooltip WareDetail a celý cGame se dále obnovují; jejich povinné produkční rozhraní nemá fallback provider.

Přímé porovnání PC53/PC24 prošlo: 1 608 stavů, 20 skutečných konstrukcí/destrukcí, 1 024 transferů a 48 GPU snímků / 23 040 000 shodných pixelů přes všechny čtyři úrovně. Používá skutečné Warehouse.xml a UI/HUD, Goods/Free/Investment a Give/Take včetně nulových transferů, neplatných produktů, obou boolean flags a uint wrap. Obsazená skutečná Manufacture s SheepFold odebírá vstupy ze skutečného skladu, vrací je a po absoluteStep uloží vyrobený produkt zpět; při každém transferu souhlasí skutečné popisky i HUD. Upgrade callbacks zahrnují 216 kombinací availability/Money, sdílený sound context a actual Upgrade. Snímky procházejí skutečným hover/press/release/leave. Další saved-level constructors a missing/partial/full XML reloads ověřují vlastnění a kumulativní levelCount. `python tools/validate_warehouse.py` tvoří CTest `original-warehouse`; plná regrese prošla všech 67 kontrol za 286,22 s. Portable report v `analysis/warehouse-validation.json` přesně vymezuje skutečné controllery a zbylé diagnostické world hranice.

Native size 1800 / `0x708`; singleton `0065f748`. ActionArea zabírá `+0..+503`, singleton MI word `+504`, Upgrade `+508`, itemType `+50c`, propertyId `+52c` (32), levelCount `+530`, occupied `+534`. Čtyři 112bajtové level records od `+538`: capacity +0, primary appearance +4, hovered appearance +44, mask String +84. Numeric capacity constructor neinicializuje. Root UI: LoadingSubstrain Picture `+6f8`, Loading ProgressBar `+6fc`, UpgradeButton ExtendedTextAnimationButton `+700`, CostText TextMoney `+704`.

| Původní adresa | Obnovené chování |
| --- | --- |
| `00467fe0` | ActionArea type 2, singleton před Upgrade, UI construction, XML, upgrade callback, initial refresh, součet devatenácti WGoodCount, umístění dvou progress controls, loading/HUD refresh. |
| `00466020` | Destrukce levels, Upgrade, clear singleton, ActionArea tooltip vlastnění. Source `WarehouseLifetime` zachovává toto pořadí. |
| `00467220` | ActionArea/Upgrade XML, čtyři UI sections, updateLoading; reload zachovává původní kumulativní Upgrade.levelCount. |
| `00467910` | ECX je Upgrade subobject. Capacity, mask, primary appearance a případný Hovered; chybějící Hovered nepřebírá primary appearance. |
| `00466530` | ECX Upgrade: level −1 return, primary/mask/dirty, loading/HUD, price a upgrade visibility/availability podle původního pořadí. |
| `00467fa0` | WGoodCount0..18; jiné uint product vrací nulu. |
| `00465e70` | Capacity minus occupied jako unsigned odečtení; bez dodatečného clampu. |
| `00467ec0` / `004682d0` | Give omezuje free, Take goods; return skutečné množství. Zero nemá notifikace. Give zapíše profil a pak occupied; Take sníží occupied a pak profil. |
| `00466110` | Unsigned occupied×100/capacity podle x87 a skutečné ProgressBar.updateLid. |
| `00466a40` | Součet upgradeCost(12, i) pro i=1 až level−1; aktuální level se nepočítá. |
| `00465ee0` | canUpgrade(true,true), cena next level, Money/SetMoney, setLevel(next,true) včetně společného upgrade sound. |
| `00448b20` / `0045f210` | Text upgradeStrings[level+1], skutečný Field tooltip hover; unhover je přímý jump do Field tooltip unhover. |
| `00466e30` | Farm rectangle gates pro area types 64 a 32 (excluded=null); při zásahu ordinary Button.clicked, jinak ActionArea.clicked. |
| `004678b0` | Field tooltip step, shown upgrade + Farm shown/not moving + canShow(26) → skutečný showArrow(26), nikoli showAction. |

Nenulový transfer vyvolá: loading → Manufacture.refreshHouseControls `004953e0` → HUD.refreshWarehouse `004beb40` → volitelný obchodní refresh `0046f9e0` na `0065ffc8` → HUD.stockChanged(preview) `004bd9d0` → WareDetail tooltip `00464b40` na `0065ffc4`. Poslední příjemce je samostatný tooltip (`004658b0`, UI/gameUI/WareDetail.xml), ne Warehouse. Diagnostika pozoruje zbylé obchodní/tooltip boundaries a jejich receiver i snapshot occupied/obou progress hodnot; neprohlašuje je za skutečný herní svět.

Skutečný sklad vyžádal A8R8G8B8 TGA masku `farm/ActionAreas/Warehouse/house_mask1.tga` (164×164). ClickMask nyní připouští native obrazy, které již mají požadovaný BGRA formát; převody jiných formátů zůstávají explicitně nepodporované. Samostatný původní mask oracle prošel v PC53/PC24: 432 masek / 81 099 735 alfa bitů na běh, včetně sedmi native BGRA DDS/TGA přes skutečný D3DX loader, texture creation a surface filtering. Referenční modul je nuceně načten mimo 00400000, aby původní adresy nezakryly chybějící diagnostické relocace. Warehouse oracle má nadále výslovný GPU-mask service adapter, stejně jako předchozí controller testy; samostatné mask porovnání dokládá tuto závislost. Aktualizovaný validator a plná regrese celku pokračují.

Zbývá dokončit přímé porovnání controlleru včetně snímků/obsazené výroby a skutečných dalších world providers, zvláštní/neplatné level/empty XML paths, celý herní runtime a save/load kompatibilitu. Samostatné úspěšné controller testy nedokládají dokončenou hru.
