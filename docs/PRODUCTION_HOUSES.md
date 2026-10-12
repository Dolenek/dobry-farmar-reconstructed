# Výrobní budovy

Tato etapa navazuje na commit `01649da`. Společný controller všech jedenácti výrobních budov je nyní upravitelný zdrojový kód připojený do Win32 sestavení. Skutečné Manufacture, warehouse a cGame vazby dále pokračují. Porovnání controlleru používá explicitně řízené world hranice, takže není důkazem fungujícího celého herního světa. Souhrnný verifier je `tools/validate_production_houses.py`, přenositelný záznam `analysis/production-houses-validation.json`.

Manufacture `00499730` je 256bajtový StateView, singleton `0065f758`. Vlastní dvacet slotů budov na `+a4` a počet `+f4`. `00494390` vybírá volný slot, rozlišuje jedenáct skutečných derived constructors a obnovuje uložené budovy podle UpgradeN/ProdTypeN. Kontext ve starším HUD oracle měl tuto scénu pouze jako základ StateView, takže neprokazuje žádnou výrobu ani umístění.

Společný production-house constructor `0049e9f0` nejprve konstruuje ExtendedPictureButton `0049dc20` (868 native bajtů), potom Upgrade na `+364`. Budova má celkem 1 588 bajtů. Dva nákupní symboly z panelu odpovídají skutečnému vstupnímu a výstupnímu produktu z následujících constructor polí, nikoli odhadnutému receptu.

| Typ | Constructor | XML | Direct input +568 | Manufactured input +56c | Manufactured output +570 |
| --- | --- | --- | --- | --- | --- |
| 0 | `004a7850` | SheepFold.xml | 3 | −1 | 1 |
| 1 | `004a54b0` | HenHouse.xml | 7 | −1 | 0 |
| 2 | `004a42e0` | CowShed.xml | 0 | −1 | 2 |
| 3 | `004a6f60` | PigPen.xml | 1 | −1 | 3 |
| 4 | `004a39f0` | CheeseDairy.xml | −1 | 5 | 4 |
| 5 | `004a4bc0` | Creamery.xml | −1 | 2 | 5 |
| 6 | `004a8a30` | Weavery.xml | −1 | 1 | 6 |
| 7 | `004a5d90` | Incubator.xml | −1 | 0 | 7 |
| 8 | `004a8140` | TextileDep.xml | −1 | 6 | 8 |
| 9 | `004a3100` | Bakery.xml | −1 | 9 | 10 |
| 10 | `004a6670` | Mill.xml | 7 | −1 | 9 |

Manufactured index se převádí na index v devatenácti ProductionInfo přičtením osmi. Přesná tabulka je v `ProductionKinds.h`. Všech jedenáct derived XML a enabled-step wrappers bylo přečteno v instrukcích: XML pouze volá `004a0330`, enabled-step pouze `0049cd50` → `0049b7c0`, navíc CRT/EH prolog a epilog. Skutečné native constructors a jejich wrappers vykonává oracle. Společná zdrojová třída proto používá přesnou constructor tabulku; žádná dodatečná odhadnutá mechanika jednotlivých typů není přidána.

ExtendedPictureButton má PictureButton overlay a vlastněný tooltip image, čtyři directional appearances a stejné tooltip text/picture/multitext arrays jako ostatní extended varianty. Constructor `0049dc20`, XML `0049ddc0`, step `0049b7c0`, placement `0049a650`, enabled/disabled `0049c880`/`0049c490`, hover/unhover `0049bcc0`/`0049c0b0`, destructor `0049a270`. Režim `extended-picture-buttons` / `extended-picture-buttons-device` používá skutečných jedenáct XML budov: v každé přesnosti 23 749 stavů, 33 reloadů, 23 628 step calls, 819 850 letters a 44 D24S8 GPU frames / 21 120 000 shodných pixelů. Je zahrnutý v souhrnném verifieru odděleně od celé budovy.

Čtyři records na `+398` mají krok 116 bajtů: ProductionNum uint +0, MaxProductionTime float +4, appearance +8, hovered appearance +30, mask String +58. Constructor/destructor `0049a120`/`0049a180` neinicializují dvě numerické hodnoty. Instrukce `0049cd60` načtou čísla a primary appearance, kopírují primary do hovered appearance, zapisují mask name včetně empty assign a načtou případný Hovered. Původní neúplné čtení dekompilace nesprávně přisoudilo tento copy samotné budově; přímý oracle odhalil rozdíl v inherited duration. Zdrojové `AnimableAppearance::copyAppearance` zachovává playing a padding, stejně jako `004d2160`. Všech 44 skutečných records je porovnáno v celých controllers.

StartStopButton `00499ad0` má 720 bajtů: AnimationButton, owner pointer a dvě AnimableAppearance. XML `0049ab60` čte StateStopSign/StateStartSign; `00499b90` pouze kopíruje sign a nemění dirty ani profil. Pouze byte přesně 1 vybere StateStartSign, ostatní bytes StateStopSign. Callback `0049fdb0` čte nonzero ProdStop, normalizuje nový flag a mění sign/profil. Skutečná XML budov sekci vynechávají; distinct signs v explicitní diagnostické XML fixture navíc ověřují osm byte hodnot a zachování dirty flagu.

ProdCount jsou uints (`0049f010`), ProdPct floats (`0049f0c0`, `0049f300`), ProdStop uints interpretované jako nonzero (`0049f170`, `0049f210`). Property id je `+388`, vlastní stavové bity `+574`, již postaveno `+578`, původní depth `+57c`. `0049fdf0` omezuje zásobu podle současné úrovně, aktualizuje slider/progress/oba texty a výrobní bit 2; nula vykoná skutečný Button mouse callback, ne bezpodmínečný hover. `004a1c90` řeší drag, jeho zahájení při squared mouse distance >32, pozastavení při tutoriálu nebo pohybu kterékoliv ze tří scén, průběh výroby, volné místo ve skladu, dokončení a uint awQProcess. TProdCompl je nastavena pouze ve větvi posledního kusu s nenulovým JobDoneSound. Tyto metody a jejich UI/profile důsledky jsou přeložené a přímo porovnané.

Skutečné dětské pole `+5a4` je SensorPanel Button s owner a hover callbacks (`0049a050`, `00499ea0`); `+5ac` je LoadedSubst Picture. XML offset loop posune právě Picture `+5ac`, nikoli SensorPanel. `lighter_mask` se přiřazuje PictureButton overlay `+27c`, nikoli Podlogka Picture `+620`. Tyto vazby opravilo přímé porovnání constructoru; vlastní RGB/stencil rendering je také porovnaný. AddDepotButton `+5a0` volá taking callback `004a0230`, AddStockButton `+59c` return callback `004a02c0`. Slider `004a0000` zachovává původní unsigned capacity, signed low-word `_ftol2` target, oba warehouse transfer flags true/true a následné limity podle dostupného zboží a volného skladu.

ASM navíc opravuje dva automatické podpisy/přetypování: `0049fdf0` přijímá unsigned počet (čte celé EDI a používá FILD pro přepočet procent), nikoli float uvedený v pseudokódu. Výrobní tick přičítá float elapsed do float animation time na +1ac a ukládá FSTP, nikoli integer cast z pseudokódu. Click `0049f8c0` poprvé postavené budově nastaví `awManDepotBuild{itemType}`, level 0, BuildSound a odečte cenu; zdarma je tento krok pouze při skutečně aktivním návodu 37. Zobrazení nuly v BuyBuilding během 35/36 tudíž není samo o sobě pravidlem odečtu peněz.

Umístění `00494a20` vyžaduje obsah celé budovy v allowed rectangle, vylučuje forbidden rectangles a ověřuje průnik s maskami ostatních dvaceti slotů. `00417d10`, `00417b90`, `004178c0` testují rohy včetně hranic. Intersects nepoužívá obecný AABB overlap; křížení obdélníků bez uvnitř ležícího rohu může původně vrátit false. `004f0510` používá pixelové masky, clamping overlap na menší rozměr, původní float spill pořadí a truncation souřadnic. `ProductionPlacement` je sestavená průběžná implementace těchto metod; úplné porovnání maskových/spill/edge případů zbývá.

Source mimo platný rozsah souřadnic a s chybějící/empty maskou hlásí výjimku namísto neurčeného paměťového čtení. Shoda původních error/undefined větví tím prokázaná není. Aktuální podklad obsahuje neprázdné masky a normální rozměry; jeho skutečné placement matrix porovnání se musí provést před označením metod za ověřené.

Controller oracle v každé přesnosti vytvoří 55 skutečných typ/level stavů (jedenáct typů × čtyři saved levels a nová budova). Ověřuje 8 094 stavů a 7 829 akcí: uint zásoby a limity, taking/return/slider, nonzero stop value 3, reálné setLevel 0/1/2/upper-clamped 3, mask/appearance/capacity/speed refresh, awDepotUpg, výrobní tick, plný sklad, tutorial a všechny scene pause gates, negative/near/exact/NaN progress, novou placenou/free stavbu a invalid guide-37 click, drag/distance/position/colour/release/focus. Skutečný mouse/frame/render traversal má 44 GPU snímků na přesnost. Dohromady controller a extended-picture základ: 176 snímků / 84 480 000 shodných pixelů a ordered draw calls.

Při novém levelu −1 čte původní setProductionCount kapacitu z Root+324 (levels[-1]), což překrývá loop byte a padding třetí tooltip appearance. Zdroj zachovává tři neurčené padding bytes u loop a čte tento raw word, nikoli odhadnutou kapacitu. Oracle má čerstvý zero monotonic heap a navíc se oběma implementacím dosadí words 0/100/10000/1000000 hex; nula dává stejné NaN slider/progress a pozdější platná úroveň je opraví. Chování při jiné historii/reused heap není tímto prokázané a zůstává výslovně neověřené.

World adapters v oracle pouze zaznamenávají původní hranice Manufacture/warehouse/Money/SetMoney a řízené observations. Neobnovují slot ownership, collision validation, warehouse perishables/capacity ani skutečné money/HUD notification účinky. Ověřený branch aktivního tutorial 37 dosazuje step field; úplný průchod návodem pokračuje. HUD/ostatní scény jsou pro isolated house frames suspendované. Audible playback, jiné hardware formáty, malformed/repeated XML, invalid levels/masks a exception branches nejsou pokryté.

Další práce: skutečný Manufacture controller/factory/lifetime s dvaceti slots, geometrie a pixelové collisions, remove/destroy/upgrade UI a BuyBuilding propojení, skutečný warehouse, Farm/character/cGame, celý tutorial a bidirectional world saves. Hlavní EXE, úplná shoda Squallu a odinstalátor se dál dokončují. Tato etapa není dokončením projektu.
