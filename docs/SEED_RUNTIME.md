# Přirozený nákup osiva v celých EXE

## Dokončený růst, současně zralých osm druhů a skutečné sklizně

`validate_game_remaining_crops_growth.py` obnoví pět rozpracovaných druhů v opačném EXE: originál čte source save a source čte originální save. Normální kbelík znovu zalije původní rajče, okurku a trávu. Pouze růst používá fyzický původní Alt+Insert15×; poté se vrací1×. Obě celé hry mají skutečně současně zralých všech osm druhů, nenulovou vláhu a masku255. Původní správce sám udělí13 při RunTime1800 a11 po dozrání posledního ananasu. Oba celé panely souhlasí ve960000 RGB pixelech.5142bajtové saves souhlasí mimo původní padding pěti sáčků; obě nové zprávy z1335/1490 jsou stejné.

Časové řezy růstu nejsou vydávané za stejné hodiny: native skončil1502.81 a source1491.53. Native proto už provedl cenu mléka16 v1500 a návod49 po vyčerpání úrodnosti starého pole; source ještě měl cenu8 a nevyřízený49. Tyto tři diskrétní rozdíly jsou přesně rozepsané v `analysis/remaining-crops-growth-state-validation.json`. Záznam všech osmi skutečně zralých polí v obou procesech je samostatný důkaz, nespoléhá jen na masku ocenění.

`validate_game_remaining_crops_harvest.py` znovu načte zralý stav v opačném programu, převezme základní kosu a sklidí typy2/4/5/6/7. Po každém kusu se skutečně vrací do skladu; kosa0 nedovoluje smíšený náklad. awQCollect se zvýší o5, zásoba každého nového druhu je1. Původní událost1515 odečte25 a přidá skutečnou zprávu o sousedově poškození plotu. Dvanáct pracovních kontrol na EXE dokládá produkty a tento časový odpočet. Při1598 souhlasí všechny diskrétní properties a všech5439 save bajtů mimo15 neurčených padding bajtů.

Další čtyři celé obousměrné obnovy zachovají pět zásob, historii výroben, ocenění, třináct zpráv a383 koeficientů. Každá zvlášť vyžaduje původní X+1 u všech pěti sáčků; kromě této prokázané změny a paddingu jsou všechny save bajty přesné. Dva source debug traces končí0 bez AV. Celkem jde o osm nových skutečných procesů růstu/sklizní/finálních přenosů, navazujících na dva nákupní.

První sklizňový pokus měl už při otevření návod49, který zablokoval kliknutí kosy; driver nyní vyřizuje návod před vstupem. Další původní konce1590 ležely na expiraci dopisu: jeden už vypršel, ve druhém zbývalo0.59s. Úspěšný průchod skutečné akce zopakoval a počkal do1598. Tyto původní výsledky zůstávají v `analysis/remaining-crops-completion-initial-observations.json`, nejsou skryté normalizací času/profilu. Souhrn `analysis/remaining-crops-completion-regression.json`; produkční kód zůstává shodný s posledním102/102 během. Pokračují skutečná automatická závlaha, další zakázky/počasí/ocenění, úplné DLL a celý původní cíl.

## Pět dalších druhů koupených, zasetých a zalitých

`validate_game_remaining_crops_sowing.py` navazuje na skutečný nativní save z historie11 výroben. Dostupnost sedmi osiv pochází z dřívějšího původního vstupního příkazu v tomto profilu; tato etapa netvrdí přirozené časové odemčení osiv. Normální UI koupí po jednom sáčku typů2/4/5/6/7 za70/50/40/90/25. Skutečná postava pokaždé vezme lopatu, vyryje pole24/25/26/33/35, převezme zaplacený sáček, zaseje jednu rostlinu, doplní kbelík480 a zalije120. Obě hry mají35 shodných pracovních kontrol, Money5234→4959, všechny diskrétní profilové hodnoty stejné a v každém z pěti sáčků zbývají tři semena. Čas není zrychlený a profilové hodnoty se nezapisují z testu.

Úplné4372bajtové saves souhlasí po vyjmutí pouze15 původně neurčených padding bajtů v pěti záznamech sáčků. Souřadnice, počet/druh/stav/zaplacení, staré prodejní hodiny, deset zpráv/read flags a383 raw koeficientů jsou bitově shodné. Rozdíly růstu, vody, úrodnosti a normálních hodin zůstávají rozepsané. Na disku se nic nenormalizuje. První pokus skončil chybou převodu XML float souřadnice na integer v driveru ještě před rytím; není vydávaný za herní chybu.

Audit a metadata: `validate_remaining_crops_sowing_state.py`, `analysis/remaining-crops-sowing-state-validation.json` a `analysis/remaining-crops-sowing-regression.json`. Produkční implementace zůstává shodná s posledním úplným102/102 během. Navazující obnova růstu v opačném programu, dozrání a sklizeň těchto pěti druhů, přirozené ocenění11 a ostatní celý cíl pokračují.

## Tři skutečné sklizně okurek a peněžní brána

`tools/validate_game_cucumber_harvest.py` pokračuje z Money64 a čtyř skutečně zalitých okurkových ploch. V každém ze tří cyklů originál i zdroj vyčkají na skutečné dozrání, sklidí tři plody do nákladu, uloží je, vrátí se pro čtvrtý a znovu uloží. Na trhu osm skutečných cenových kliknutí poprvé zvýší cenu2→10. Vše přenese čtyři kusy a skutečné prodeje dávají postupně Money104/144/184. Dvanáct plodů znamená120 získaných mincí. Pouze růst a prodejní odpočet používají fyzický původní Alt+Insert15×, před každou sklizní a po prodeji se vrací1×.

Skutečná událost720 nastaví mTrigger140. Při první sklizni, prvním prodeji za104 i druhém dozrání se GameTime v rámci každého vlastního procesu drží na přesně téže float hodnotě, přesto pole a prodejní odpočty pokračují. Druhý prodej za144 vyčistí bránu a GameTime znovu postupuje. V obou EXE prošlo27 pracovních kontrol. Nedoplňují se peníze ani profilové/časové hodnoty.

Nezávislé konce `.local/cucumber-harvest/20261029T204929619217Z` leží na opačných stranách původní popularity události810: originál816.582 má ceny−1/4/6, zdroj808.986 má předchozí791s ceny8/6/20. Auditor tyto konkrétní hodnoty odvozuje z původního Gameplay.xml a netvrdí rovnost všech diskrétních properties. Všechny795 klíče/typy a ostatní diskrétní hodnoty souhlasí. Další rozdíly jsou evidované hodiny, dopis a růst/voda/úrodnost.3557bajtové saves mají stejné čtyři poslední prodejní částky10, sedm zpráv/read flags a383 koeficientů; ostatní bajty souhlasí po oddělení pouze pěti čtyřbajtových prodejních hodin/cutoff. Původní soubory ani diagnostické saves se nenormalizují na disku. Audit `python tools/validate_cucumber_harvest_state.py`, metadata `analysis/cucumber-harvest-state-validation.json`. Přesný frame replay, další save transfery této historie a placená výroba pokračují.

## Skutečný nákup a čtyři vyseté okurky

Po dokončení povinné výrobní výuky pokračuje `tools/validate_game_cucumber_sowing.py` z nativního uloženého stavu Money94, držený rýč2 a zbořený výukový kravín. Obě EXE skutečně koupí druhý dostupný sáček, typ3 za30. Staré vyryté plochy17/18/23 už mají téměř vyčerpaných původních60s do zarostení; průchod počká na skutečnou obnovu trávy, znovu vyryje17/18/23/27 a teprve poté převezme připravený sáček. Čtyři skutečná setí jej spotřebují, deferred odstranění vrátí PCarry10→0, studna naplní480 a čtyři skutečná zalití je spotřebují do0. Tato etapa nepoužívá časovou akceleraci.

Sedmnáct pracovních kontrol prošlo v obou celých EXE `.local/cucumber-sowing/20261029T180721717655Z`. Money je64, čtyři nové okurky mají FFC3/FFS0 a vodu i úrodnost nad118; čtyři stávající rajčata jsou zralá. Všech795 profilových klíčů/typů a diskrétních hodnot souhlasí, včetně beze změny převzaté neobvyklé původní Upgrade vlastnosti. Rozdíly jsou výslovně pouze GameTime/RunTime a růst/voda/úrodnost polí. Celé3557bajtové saves jsou přesně stejné bez normalizace: nula sáčků, sedm zpráv/read flagů, čtyři původní prodeje s hodinami a383 raw koeficientů. Audit `python tools/validate_cucumber_sowing_state.py`, metadata `analysis/cucumber-sowing-state-validation.json`. Sklizeň/prodej okurek, obousměrné přenosy tohoto stavu s osmi pěstovanými plochami a ostatní systémy pokračují.

Předchozí průzkumy jsou zachované: povinný návod30 otevřel skutečné zpracování (`analysis/cucumber-sowing-initial-probe.json`), další příliš dlouho spoléhal na starou vyrytou půdu (`analysis/cucumber-restored-till-probe.json`). Nákup se tehdy povedl, avšak pole17 zarostlo před prvním setím; tento failed driver se nevydává za splněnou etapu. Následující změna upravila pouze pořadí skutečných hráčských akcí, nikoli produkční herní data.

## Navazující čtyři sklizně a ruční cena

`tools/validate_game_tomato_batch.py` pokračuje z nativního skutečně vyčerpaného sáčku a čtyř zasetých polí14/15/16/34. Pouze při čekání na růst použije fyzickou původní zkratku Alt+Insert pro15× čas; před prací obnoví1×. Toto je výslovný rozdíl proti dřívějším průchodům bez akcelerace. Není vložený žádný čas ani profilový stav. Skutečná postava sklidí první tři plody do kapacity3, uloží je, dojde pro čtvrtý a znovu jej uloží. Na trhu šest skutečných kliknutí zvýší cenu rajčat2→8, jedno tlačítko Vše přesune čtyři kusy do nabídky a skutečné prodeje zvýší Money2→34. Farmář dále drží sekačku a rostliny znovu rostou. Přirozeně se odemknou upgrady11/14 a dopis16; na farmě se zpracuje návod28.

Oba celé procesy `.local/tomato-batch/20261029T165206018774Z` prošly shodnými devíti pracovními kontrolami i všemi šesti cenovými přírůstky. Všech794 profilových klíčů/typů a diskrétních hodnot souhlasí. Liší se evidované nezávislé hodiny, zbývající čas dopisu a růst/voda/úrodnost polí.3013bajtové saves mají stejná čtyři prodejní množství8, pět zpráv/read příznaků a383 raw koeficientů. Všechny ostatní bajty souhlasí po výslovném vynulování pouze pěti čtyřbajtových prodejních hodin/cutoff; žádné soubory se tím nemění. Skutečné časy jednotlivých prodejů se liší a nejsou vydávané za synchronní frame porovnání. Návod28 má28600 porovnaných pixelů,550 odlišných a maximální rozdíl kanálu2; pixelová shoda se netvrdí.

Readonly audit `python tools/validate_tomato_batch_state.py` ověřuje manifesty, snímky, úplné properties i binární strukturu. Metadata jsou `analysis/tomato-batch-state-validation.json`. Načtené vlastní DLL jsou ověřené cestou a hashem. Další zakázka, přirozeně koupené nářadí, další plodiny, budovy/výroba a ostatní podmínky celého projektu pokračují.

`tools/validate_game_seed_unlock.py` navazuje na nativní save skutečně splněné první zakázky. Originál i zdroj nezávisle sklidí další čtyři kusy trávy; kapacita nákladu3 vyžaduje dva návraty do stodoly. Skutečný druhý trh a prodej zvýší Money62→70. Herní událost v čase135 čeká na peněžní podmínku; osivo rajčat se přirozeně odemkne v čase140. Tutoriál19 vysvětlí rozšíření pole, ale fcu zůstává0. Nová zpráva vyvolá krok39: driver skutečně otevře deník a přečte všechny tři zprávy. Žádné profilové pole, čas, příznak ani herní instrukci test nepřepisuje.

Následují skutečné kroky13/14/15/16 a nevyřízený20. Původní krok13 dočasně poskytne cenu osiva20, kliknutí vybere rajčata a platné položení sáčku20 odečte. Konečné Money70, jeden zaplacený sáček kind1/count4/status0, PCarry6 a awQCollect9 souhlasí. Originál i zdroj přirozeně vytvoří i původní `tip4294967295=true`: RepeatedAction dokončí tip dvakrát. Tato zvláštnost zůstává zachovaná, nejde o testovací přepis.

Úspěšný pár `.local/seed-unlock/20261029T115458353557Z` má794 shodných názvů a typů vlastností. Liší se pouze růst/voda čtyř obdělávaných polí, GameTime/RunTime a letTime0; konkrétní hodnoty jsou v reportu. awQTask je v obou0 po dalším přirozeném časovém průchodu; přesný okamžik a izolovaná větev expirace kontraktu zatím nejsou tímto během prokázané. Binární save má2523 bajtů: jeden sáček, čtyři prodeje amount2, tři přečtené UTF16 zprávy a383 bitově shodných koeficientů. Při porovnání nezávislých běhů se vyjímají pouze bajty25..27 (původní neurčený padding),32..39/44..47/52..55/60..63 (cutoff a čtyři prodejní float32 hodiny). Všechny ostatní bajty souhlasí.

Osm tutoriálových oblastí obsahuje28600 RGB pixelů každá. Kroky39/14/15/16 jsou přesné (114400 pixelů); u11/19/13/20 je130/1976/167/53 odlišných pixelů s maximem1/2/1/1 kanálové jednotky. Animované pozadí není časově synchronizované; tyto oblasti se nevydávají za přesné pixelové porovnání. Původní neúspěšný průzkum s neobslouženým krokem39 zůstává v lokálním adresáři a není zahrnut jako úspěch.

## Obousměrné přenosy přirozeného sáčku

`tools/validate_game_seed_interop.py` provedl čtyři celé další procesy: native0→source2→native4→source5 a source1→native3. Každý skutečně obnoví menu/farmu, dostane fyzický Alt+Pause a normálně uloží při ukončení. Všech794 profilových klíčů/typů i diskrétních hodnot souhlasí. Ve všech čtyřech2523bajtových přenosech jsou bitově zachované všechny prodejní hodiny, cutoff, zprávy/read flags i383 koeficientů. Jedinými normalizacemi jsou tři původní padding bajty a samostatně ověřená původní změna X o+1 při každé obnově (121→122→123→124). Y280, kind1/count4/status0/unpaid0 se zachovají. Tento jev odpovídá původním0040fbb0/00412850 a parent origin-1; produkce jej zachovává.

Úspěšný přenosový běh je `.local/seed-interop/20261029T120912495565Z`. Předchozí čtyři procesy rovněž doběhly, ale závěrečný zapisovač reportu zastínil globální data proměnnou bytearray; úplný běh byl proto po opravě zopakován. Za doložený úspěch se považuje pouze nový kompletní report. Obě kontroly ověřily17 nezměněných originálů a skutečné cesty vlastních/originálních DLL. Produkční binárky se nezměnily; jde o porovnání zapojených systémů v celém EXE.

Metadata: `analysis/seed-unlock-state-validation.json`, `analysis/seed-interop-validation.json`. `tools/game_save_state.py` čte jen diagnostické kopie save, kontroluje hranice, počty i EOF a koeficienty porovnává jako raw uint32 bity.

```powershell
python tools/validate_game_seed_unlock.py
python tools/validate_seed_unlock_state.py
python tools/validate_game_seed_interop.py
```

Tyto opt-in běhy potřebují volný aktivní Windows desktop a existující přirozené baseline reporty. Následují skutečné rytí, setí, růst a sklizeň zakoupené plodiny, další osiva/rozšíření/výroba, dlouhé saves, všechny kontrakty, neověřené větve enginu a úplný Squall97. Celý projekt nadále není dokončený.


## Skutečné rytí, setí a zalévání

`tools/validate_game_seed_sowing.py` navazuje na stejný nativní zaplacený sáček, nezávisle pro originál i zdroj. Skutečný krok20 převezme lopatu,21 zahájí rytí dosud suchého aktivního pole34,22 převezme sáček a23 zaseje jedno rajče. GrowingAction přirozeně otevře24 kvůli nedostatku vody; skutečná studna naplní480 a postava zalije pole. Vstupní masky lopaty i pole jsou v reportu. Žádný worker, čas, profil nebo výsledek práce se do procesu nevkládá.

Běh `.local/seed-sowing/20261029T121007185167Z` dokončily oba celé procesy. Všech pět tutoriálových oblastí20–24 má přesně shodných143000 RGB pixelů. V794 profilových položkách souhlasí klíče, typy a diskrétní hodnoty: Money70, PCarry1, FFC34=1, FFS34=0 a všech pět tipů=true. Voda i úrodnost34 přesahují118; rozdíly růstu/vody/úrodnosti a hodin obou dopisů jsou jednotlivě zaznamenané. Konečný2523bajtový save má jeden zaplacený sáček rajčat se třemi zbývajícími semeny na X122/Y280. Všechny bajty obou saves souhlasí kromě tří původních neurčených padding bajtů25..27. Prodejní hodiny, cutoff, zprávy a383 koeficientů nejsou normalizované a jsou bitově shodné.

Nezávislý readonly audit `tools/validate_seed_sowing_state.py` kontroluje hashe vstupního kódu, všech snímků a17 originálů; metadata jsou `analysis/seed-sowing-state-validation.json`. Přenos nově osetého stavu mezi programy a skutečný celý růst/sklizeň se ověřují dále. Tento výsledek neuzavírá celé pěstování ani projekt.


## Přirozený celý cyklus rajčat a tržní graf

`tools/validate_game_tomato_harvest.py` čeká na skutečný konec původního100sekundového pěstování. Bez zrychlení času nebo profilových změn převezme sekačku, sklidí pole34, odnese rajče do stodoly, přejde na třetí skutečný trh a potvrdí původní Arrow tutoriál12 s cenovým grafem. Přesune rajče do nabídky, nechá je skutečně prodat za2 a vrátí se na farmu. V obou celých EXE je konečné Money72, awQCollect10, prázdný náklad/sklad/nabídka rajčat a další růst stejné rostliny. Kód `SalesHistory` zachoval původní vynechávání následníka po erase0040c337: po novém prodeji zůstávají ze čtyř starých transakcí dvě, ač jsou před cutoff. Tato vlastnost není opravovaná na jiné chování.

Dokončený pár `.local/tomato-harvest/20261029T121557573557Z` má794 shodných profilových klíčů/typů a diskrétních hodnot; čas, růst/voda/úrodnost a dopisové odpočty jsou zaznamenané zvlášť. Save má2515 bajtů, tři prodeje amount2, tři přečtené zprávy a383 shodných raw koeficientů. Mimo výslovně evidované X sáčku, padding a dva float32 nejnovějšího prodeje/cutoff jsou bajty přesné. Tutorial12 má227 odlišných pixelů z28600, maximum2 kanálové jednotky; oblast není označená jako přesně pixelově ověřená. První průzkum skončil před prodejem na dosud neobslouženém tutoriálu12 a je lokálně zachovaný jako neúspěšný.

`tools/validate_game_seed_transition.py` dále provedl tři původní a dva zdrojové průchody farma→trh→farma ze stejného přirozeného osetého profilu. Read-only snímky skutečného depth stromu zaznamenaly světové X sáčku před/po. Originál sám ze stejného vstupního X122 skončil na122.00028991699219,121.99986267089844 a121.99896240234375; zdroj na122.00005340576172 a121.99872589111328. Save v každém případě přesně přičte původní parent origin+1. Původní004f0110 přičítá float-spilled delta ke každému potomku a StateView004c56f0 závisí na jednotlivých intervalech snímků; to vysvětluje časovou závislost jako podloženou interpretaci, nikoli důkaz identity celého světa za libovolného časování. Dřívější řízené porovnání StateView zahrnuje22242 stavů v PC53/PC24. Přesné přehrání stejných intervalů v celém světě a synchronizované animované pozadí zůstává výslovně neověřené. Není použitá tolerance vydávaná za bitovou shodu.

## Osetý save a původní změny staré historie při obnově

Čtyři skutečné procesy `tools/validate_game_crop_interop.py` z `.local/sown-seed-interop/20261029T122633938575Z` ověřily osetý stav ve směrech native→source→native→source a source→native. Celých2523 bajtů souhlasí mimo samostatně kontrolované X+1 a tři padding bajty. Ve všech794 profilových klíčích/typu/discrétních hodnotách je jedinou výslovnou změnou nová původní událost155s: MEffPrice0 z8 na2. Driver před pauzou nechá oba skutečné programy dospět nejméně do156s a vykonat tuto událost. Starší neúspěšný průzkum ji rozdělil mezi155.056 a154.991s a správně odhalil různé ceny; nebyla to odchylka implementace.

Starší volba `--suite tomato-interop` v tomto přísném nezměnitelném-save experimentu naopak **neprojde** u přirozeně zestárlé historie: restore skutečně přehrává `record` pro jednotlivé prodeje. Nový `tools/validate_game_crop_history_interop.py` proto porovnává obě skutečné DLL/EXE implementace nad totožnými vstupy, včetně kontrolních původních načtení. Osm procesů `.local/tomato-history-interop/20261029T123204319988Z` prokázalo čtyři páry stejných výstupů:2515→2507,2515→2507,2507→2499 a2499→2499 bajtů. Počet prodejů se původním způsobem mění3→2→1→1. Každý pár se porovnává celý s jedinou normalizací padding25..27; X i všechny prodejní hodiny, cutoff, zprávy a koeficienty souhlasí bitově. Tato kontrola nevydává přirozeně mutující historii za bezeztrátový byte-copy. Další tvary a rozsáhlé historie čekají.

## První zaplacené rozšíření farmy

`tools/validate_game_farm_expansion.py` skutečně koupí dostupné rozšíření za70. Obě EXE v `.local/farm-expansion/20261029T123106289145Z` přejdou z Money72/fcu0 na Money2/fcu1. Původní XML má6 aktivních polí pro level0 a dalších14 pro level1: skutečný depth strom změní54 viditelných zakrytých polí na40 a počet aktivních obrázků rostlin z6 na20. Všech54/40 zakrytých polí má před/po v obou programech stejné pořadí, geometrii, deep, texturu, všech19 sprite slov a flags. Dříve opravená alokace skrytých obrázků tak souhlasí i po nákupu. Celé animované aktivní pole tím není označené za pixelově shodné.

Konečných794 profilových klíčů/typů a diskrétních hodnot souhlasí; liší se pouze evidované časové/růstové float hodnoty. Obě hry obnovily stejnou přirozenou historii na2507 bajtů a všechny save bajty kromě původního paddingu jsou přesné, včetně X. Metadata: `analysis/farm-expansion-state-validation.json`.

Nové reprodukční kroky po předchozích baseline bězích:

```powershell
python tools/validate_game_seed_sowing.py
python tools/validate_seed_sowing_state.py
python tools/validate_game_tomato_harvest.py
python tools/validate_tomato_harvest_state.py
python tools/validate_game_seed_transition.py
python tools/validate_seed_transition_state.py
python tools/validate_game_crop_interop.py
python tools/validate_game_crop_history_interop.py
python tools/validate_game_farm_expansion.py
python tools/validate_farm_expansion_state.py
```

Metadata nových průchodů jsou `analysis/tomato-harvest-state-validation.json`, `analysis/seed-transition-state-validation.json`, `analysis/sown-seed-interop-validation.json` a `analysis/tomato-history-interop-validation.json`. Před/po byly kontrolované hashe všech17 originálů i skutečné cesty DLL; produkční implementace nevolá originál. Vyčerpání sáčku, další plodiny, budovy/výroba, další rozšíření, dlouhé saves, celý engine a Squall97 pokračují.


## Skutečné vyčerpání sáčku a další časové události

`tools/validate_game_seed_exhaustion.py` pokračuje z přirozeně zaplaceného rozšíření. Skutečná postava převezme lopatu, vyryje nově aktivní14/15/16, převezme sáček se třemi semeny a jednotlivě je zaseje. Při třetím setí skutečný deferred discard odstraní sáček a PCarry přejde10→0. Následuje studna480 a tři skutečná zalití; v kbelíku zůstane120. Všech třináct dokončených pracovních kroků má readonly scalar kontrolu a snímek. Původní pole34 mezitím dále roste.

Oba celé procesy `.local/seed-exhaustion/20261029T123421800622Z` mají794 stejných profilových klíčů/typů a diskrétních hodnot. Zůstává Money2/fcu1; nová pole mají FFC1/FFS0 a vodu i úrodnost nad118. Přirozená událost300s změní ceny trávy/rajčat na−1/8; události312s a314s postupně zpřístupní fua2 a3. Starý dopis vyprší a přibude čtvrtá skutečná zpráva. Nejsou vložené žádné příznaky ani časové změny.

Oba úplné2650bajtové saves jsou **přesně stejné bez normalizace**: nula sáčků, jeden zbývající prodej, čtyři zprávy/read flagy a383 raw koeficientů. Metadata: `analysis/seed-exhaustion-state-validation.json`; nezávislý audit je `tools/validate_seed_exhaustion_state.py`. Obousměrné načtení tohoto konkrétního prázdného-sáčku stavu a všechny další systémy pokračují.


Prázdný-sáčku stav následně prošel také čtyřmi celými procesy `tools/validate_game_empty_seed_interop.py`: `.local/empty-seed-interop/20261029T123856899273Z`, native0→source2→native4→source5 a source1→native3. Každý přenos má přesně shodných2650 bajtů bez normalizace a794 shodných profilových klíčů/typů/diskrétních hodnot; mění se pouze zaznamenané průběžné růstové/vodní/úrodnostní floaty a hodiny. Metadata: `analysis/empty-seed-interop-validation.json`. Hlavní herní/DLL binárky zůstávají stejné jako v předchozím ověřeném sestavení; nový samostatný cílový test Squall nemění jejich implementaci.


Stav osmi osázených polí následně prošel čtyřmi celými procesy `tools/validate_game_cucumber_interop.py`: native0→source2→native4→source5 a source1→native3. Každý přenos zachoval3557 save bajtů bez normalizace a795 stejných profilových klíčů/typů/diskrétních hodnot. Nová vlastní Squall s opravenou normalizací vzdáleností je ověřená skutečně načtenou cestou i hashem. Metadata: `analysis/cucumber-interop-validation.json`, pracovní složka `.local/cucumber-interop/20261029T204828799793Z`.
