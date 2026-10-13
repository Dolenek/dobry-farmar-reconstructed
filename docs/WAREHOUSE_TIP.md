# WareDetail – skutečný tooltip zásob

`WarehouseTip` obnovuje cWareDetail, singleton `0065ffc4`, z `UI/gameUI/WareDetail.xml`. Jde o přesouvaný panel zásob s devatenácti zobrazovacími řádky. Samostatná obchodní scéna na `0065ffc8` se obnovuje zvlášť. Controller používá skutečné Warehouse/Profile, GameConfiguration product appearances, Picture/TextLine a MovableUnion. Nemá stock provider ani default náhradu herního světa.

Přímé PC53/PC24 porovnání prošlo: 2 100 stavů, 40 GPU snímků / 19 200 000 přesně shodných RGB pixelů a dvanáct XML reloadů. Integrovaný scénář současně obsahuje 2 098 Warehouse stavů, 1 024 transferů a jeho samostatných 48 snímků / 23 040 000 pixelů. Skutečný Warehouse nyní vyvolá skutečný tooltip refresh; skutečný HUD WarehouseButton jej otevře při hover a při leave pouze nastaví časovač. Manufacture a skutečná výrobní budova zůstávají propojené přes Warehouse. Market, Farm/Person a cGame Money/world služby stále mají jasně evidované diagnostické hranice.

| Adresa | Chování |
| --- | --- |
| `004658b0` / `00465a10` | MovableUnion, singleton, VisibleTime=2, opened=false, skutečné XML. Singleton se ruší před MovableUnion/owned-control teardown. |
| `004645b0` | MovableUnion XML, VisibleTime, ProductInfo count s clampem 19, nové Picture/TextLine pro každý řádek, setPosition(HiddenPlace), hide. |
| `00464410` | Řádek: Icon/Text XML a IconType. Name attr není dodatečná mapa výrobků. |
| `00464b40` | Jen opened: queries Warehouse.goods pro produkt 0..productCount−1, nenulové zásoby kompaktně do řádků 0..N−1, x:uint quantity, malé/velké appearance podle řádku, dirty, icon.show → quantity.show; zbytek icon.hide → quantity.hide. |
| `00465050` | remainingVisible=1e9, opened=true, refresh, incoming move(2) ze současné position do originalPosition. |
| `00465460` | Kopie VisibleTime do remainingVisible; opened zůstává true, nezačíná pohyb. |
| `004654b0` | opened=false, outgoing move(1) ze současné position do HiddenPlace. |
| `00465a70` | MovableUnion.absoluteStep, další skutečný float timer query, x87 subtraction, float store a porovnání původního unspilled výsledku proti nule. Timeout znovu volá close i během outgoing/hidden stavů. NaN porovnání close nevyvolá. |
| `004bd8f0` / `004bd920` | Skutečný HUD hover otevře panel pouze pokud není opened; unhover při opened zavolá armHide. |

Native size 884 / `0x374`: MovableUnion 636 / `0x27c`, singleton MI word `+27c`, 19 × 12-byte rows od `+280` (Picture, TextLine, IconType), productCount `+364`, VisibleTime `+368`, remainingVisible `+36c`, opened byte `+370`. Primary vtable `005f43c8` má přesně 65 slots; MI `005f43c0` dvě a `005f43bc` jednu.

Reload přepisuje row pointers a ponechá staré děti ve vlastnění View; nic neodmazává odhadem. Při otevření po změně počtu mohou být vidět i staré children. Whole-frame oracle porovnává tento skutečný strom. Missing ProductInfo count zachovává, prázdné dává nulu, oversized se omezí na 19. Ověřené jsou také 64 zero/all/sparse/random uint stock patternů na přesnost, oba icon types a timer words +0/−0/kladné/záporné/±infinity/NaN. Owned texture reference delta souhlasí při destrukci celého skutečného Warehouse/Manufacture/HUD scénáře.

`remainingVisible` constructor neinicializuje. Diagnostika obě strany před absolute dispatch výslovně seeduje; není to tvrzení o běžném původním alokátoru. Chybějící počáteční XML/ProductInfo, malformed/exception ABI a hidden startup s reused allocator storage zůstávají neověřené. Celý engine/world, původní tutorial playthrough a bidirectional saves se musí dále obnovit.

Reprodukce: `python tools/validate_warehouse_tip.py --oracle build/windows/Release/text-buttons-reference-test.exe`; CTest `original-warehouse-tip`. Celá regrese prošla všech 68 kontrol za 285,20 s. Portable report `analysis/warehouse-tip-validation.json` doplňuje přesné source hashe a hranice; fyzické game-audio porovnání v běžné i samostatné cache se obnovuje pro stejný snapshot.
