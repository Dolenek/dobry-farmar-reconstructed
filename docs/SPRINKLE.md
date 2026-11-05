# Postřikovač a jeho vytvoření ve Farm

[Navazující přirozené zavlažovací ocenění](NO_DRY_AWARD.md) už ověřilo všech60 aktivních polí, opakované skutečné přesuny jednoho kusu a nepřerušených1200 sekund. Oba celé panely a čtyři15808bajtové save přenosy souhlasí. Níže uvedená suchá neaktivní pole se vztahují k dřívější20polové etapě.

Aktuální celý runtime už má skutečný Farm/Person/cField. `validate_game_irrigation.py` v obou celých EXE koupí tři postřikovače za600: Money5134→4534, pozice400/270,530/260,390/380 a všechny tři SPExist=true/SPStatus0. Jejich kruhy radius100 mají13/8/6 skutečných aktivních polí a sjednocení pokryje všech20 dostupných. Vláha se skutečně doplní nad118; po třetím nákupu původní UI další nabídku skryje a fyzické kliknutí už nic nekoupí.

Další skutečný drag přesune první kus na stodolu120/100. Obě hry mají stejný červený spriteARGBff804040; neplatné uvolnění zachová SPStatus2 a původní uloženou pozici400/270. Kliknutí na platném130/280 jej položí s prázdným dosahem, bez další ceny. Původní pole14 skutečně ztrácí vodu. Normální drag/release zpět400/270 obnoví zavlažování. Všech17 pracovních kontrol na EXE má stejné peníze, držený nástroj, prázdný náklad a všechny příznaky/pozice postřikovačů. awNoDry zůstává false: dotaz zahrnuje i40 suchých neaktivních polí.

Při časovém řezu1070 bylo pole16 v native ještě těsně před dozráním, v source už zralé. Konkrétní zbývající čas i jediný diskrétní rozdílFFS16 jsou evidované; růst se nevydává za synchronní frame replay. Oba celé4089bajtové saves jsou přesné bez normalizace. Čtyři další obousměrné obnovy zachovají tři postřikovače, automatické zalévání všech20 polí a všech4089 save bajtů. Doběhnutí jednotlivých téměř zralých polí se ověřuje jako skutečná změna0→1, nevyjímá se libovolná diskrétní vlastnost. Dva source debug traces končí0 bez AV. Produkční implementace se neměnila.

Metadata: `analysis/irrigation-regression.json`, state/interop reports a samostatná počáteční chyba driveru při čtení seznamu polí. Kontrolované stejné frame intervaly celého živého částicového světa zatím nejsou ověřené; níže uvedené144 řízené snímky zůstávají samostatným přímým testem. Následující odstavce popisují historickou izolovanou etapu, nikoli současnou absenci Farm/Person.

`src/game/ui/Sprinkle.h/.cpp` obnovuje skutečný cSprinkle, ActionArea type64, vlastní SprinkleEmitter a původní Farm factory `00433980`. Factory je zapojená do porovnání callbacku skutečné studny: Money/count gate, první volný index SPExist, skutečný postřikovač vlastněný movingPicture a jeho Pressed state. Celý Farm controller a cField zatím nejsou obnovené; geometrické dotazy Farm a watering/preview polí mají povinná rozhraní, bez produkční náhradní implementace.

`python tools/validate_sprinkle.py` porovnává PC53 a PC24: celkem1 254 stavů,18 objektů (dvanáct přímých a šest přes tlačítko studny),856 explicitních akcí,24 částečných XML reloadů a144 GPU snímků /69 120 000 shodných RGB pixelů. Vedle polí profilu a UI jsou porovnané skutečné částice, jejich počet/pořadí/parent links, velocities/lifetime/elapsed, shared RNG, sprite submissions a D3D8 draw calls. Mouse input skutečně stiskne postřikovač, táhne jej o5 pixelů, projde odmítnutým preview a uvolní tlačítko. Originální program je pouze diagnostickou referencí; implementace jeho kód nevolá.

Native object1380 /`0x564`: ActionArea do+503, index+504, radius+508, installed byte+50c, press position+510/+514, drag offset+518/+51c, effect+520, vector allocator+524/begin+528/end+52c/capacity+530, Install/Take/Drop sounds+534/+538/+53c, normalColor+540 a unfitColor+550. Primary vtable`005f3c68` má91 položek, MI`005f3c60` dvě.

| Adresa | Obnovené chování |
| --- | --- |
| `00460180` | ActionArea64, XML, indexované SPXPos/SPYPos, owned SprinkleEffect, infinite start, effect y−10/deep+1, SPStatus mode a SPExist. |
| `0045e9d0` / `0045ea90` | Install/Drop/Take reference releases, affected-field vector, ActionArea owned cleanup / scalar wrapper. |
| `0045eb40` | ActionArea XML, snapshot normalColor, Radius, UnfitColor a tři optional sound acquisitions. |
| `0045f340` | Přímá metoda posunu a persistence SPXPos/SPYPos. **Není** virtuální setter: slot+c0 zůstává FieldObject`004192a0`. |
| `0045f3d0` | Uint SPStatus, affected fields a preview reset pro0; pro2 disable Farm+9c a stop emise; jinak enable Farm+9c a effect saved-position y−10/deep+1. |
| `0045f580` | Click: select/topmost area, begin press; nebo valid placement, installed drop / signed-wrap Money purchase; reject/remove dosud nenainstalovaného kusu. |
| `0045fb40` | Release: umístění nebo návrat do state0. Původní valid release při nové instalaci **neodečítá Money**. |
| `00460450` | ExtendedField normal step, squared cursor distance **>=16** pro begin drag, moving geometry/colors/field preview, jinak elapsed×WaterSource.capacity do affected fields. |
| `0045f230` / `0045f210` | Building/topmost area hover gate, overlap tooltip direction, ExtendedField tooltip / unhover. |
| `0045ceb0` | State-change thunk do AnimationButton`004f1ce0`; nezobrazuje ActionArea overlay. |
| `004bd820` | Skutečný prázdný Sprinkle absolute-step slot; vodní emiter/částice mají vlastní absolute processing. |
| `00433980` | Farm factory: Money>=signed cost a unsigned count<3, první neexistující index, mode2, count++, virtuální posun na cursor bez zápisu SPXPos/SPYPos. |
| `004697f0` | Actual WaterSource callback, factory result přepne do Pressed. |

Instrukce i přímé porovnání potvrzují zachování původního argumentu mode v konstruktoru. Dekompilace nesprávně slučuje argument a dočasný float pro deep; při čtení instrukcí je nutné započítat také push před fstp. Podezření na přepsání mode bylo vyvráceno skutečným native během.

Diagnostic field fixtures používají skutečný ActionArea základ; native cField watering/preview a Farm geometric services jsou pozorované hranice. Neprokazují vývoj vláhy, růst, sklizeň ani úplnou herní scénu. Pokračuje skutečný Farm a cField. Celé sound XML reload retention, další overlap candidates, allocation/exception ABI, save files, celý Person, slyšitelné audio a celý game EXE/playthrough stále zbývají; celý projekt není dokončený.
