# Rekonstrukce Squall

## EAX/ZOOMFX na skutečném nepodporovaném zařízení

Všech sedm Listener/Channel EAX a Channel ZOOMFX exportů prošlo v `tools/squall_extensions_probe` s oběma skutečnými DLL. Každé ze dvou nezávislých sestavení má v PC53/PC24 celkem6692 shodných návratů a6360 celých guarded snapshotů výstupní paměti. Pět fází zahrnuje pre-Init, inicializovaný prázdný kontext, skutečný připravený2D i3D kanál a post-Free. Osm verzí včetně0/1/2/3,32 presetů, pět ID a šest writable/null/code1/read-only/cross-page/noaccess pointerů ověřuje pořadí chyb i nedotčenou paměť. Velikosti listener112/channel72/ZOOMFX52 odpovídají původním instrukcím.

Skutečný host hlásí EAX0. Listener vrací−17 před kontrolou neplatných parametrů; kanálové EAX mají rozdílné pořadí−4/−18 a ZOOMFX−4/−21, včetně read-only getterů. Úspěšné hardwarové vlastnosti/presety a podporované verze tímto během ověřené nejsou. Jejich navazující audit přes řízenou diagnostickou COM hranici zůstává otevřený. Všech97 exportů má nyní určité částečné porovnání, nikoli úplné ABI/chování nebo dokončený Squall.

Reprodukce: CMake `-S tools/squall_extensions_probe -B build/squall-extensions-probe-vs -G "Visual Studio 17 2022" -A Win32 -T "v143,version=14.43.34808"`, build Release, `py -3 tools/validate_squall_extensions.py`, totéž s nezávislou `--candidate` a `--output .local/squall-extensions-fresh-validation.json`, potom `py -3 tools/record_squall_extensions_verification.py`. Přenositelné důkazy: `analysis/squall-extensions-{validation,fresh-validation,regression}.json` a oba native instruction soubory. Produkční kód zůstal beze změny; původní17 souborů také.

## Skutečné kanálové callbacky

`tools/squall_channel_worker_probe` porovnal `SQUALL_Channel_SetWorker` ve dvou skutečných DLL a dvou nezávislých source sestaveních. Každé sestavení má v PC53/PC24 dohromady456 API výsledků,432 pointer/period případů,52 skutečných2D/3D kanálů a48 uložených callback pozorování. Callback v připraveném, hrajícím i pozastaveném kanálu dostal správné ChannelID i userdata, běžel ve skutečném zvukovém threadu a reentrantní veřejný Channel_Status getter vrátil správný stav bez deadlocku. Po odmítnutí code1 i nulové periody pokračoval původní callback s původními daty; po synchronním clear se během35ms už neozval.

Návrat0 z callbacku připraveného2D i3D kanálu skutečně odstranil buffer právě po jednom volání. Kanálový timer zachovává původní znaménkové porovnání: periody DWORDffffffff/fffffffe/80000000 spouštějí callbacky během35ms,7fffffff nikoli. Odlišné unsigned listener chování se tím nemění. Počty a časování callbacků jsou uložené zvlášť pro každý proces a nejsou vydávané za stejné. Souběh, samovolná změna callbacku a timer wraparound zůstávají neověřené.

Reprodukce: CMake `-S tools/squall_channel_worker_probe -B build/squall-channel-worker-probe-vs -G "Visual Studio 17 2022" -A Win32 -T "v143,version=14.43.34808"`, build Release, `py -3 tools/validate_squall_channel_worker.py`, totéž s nezávislou `--candidate` a `--output .local/squall-channel-worker-fresh-validation.json`, potom `py -3 tools/record_squall_channel_worker_verification.py`. Přenositelné reporty `analysis/squall-channel-worker-{validation,fresh-validation,regression}.json` a native instrukce10005b10 doplňují dosavadní listener důkazy. Produkce se neměnila; úplný Squall a celý původní cíl pokračují.

## Skutečné sample/group přehrávání, pauza a stop

`tools/squall_sample_play_probe` porovnává všech osm variant Sample/SampleGroup Play/PlayEx/Play3D/Play3DEx přes skutečné DLL a DirectSound. Dvě nezávislá sestavení mají každé v PC53/PC24 celkem774 skutečných vytvoření kanálu,12240 stejných API návratů a836 celých guarded snapshotů. Singleton skupiny17/−7/0 používají vlastní sekundové mono PCM16 WAV; všechny16 kombinace syrových Loop/Start0/1/−1/2 mají vlastní vytvoření/úklid.2D i skutečné3D buffery ověřují výchozí/explicitní parametry, skalární gettery a přesné cached position/velocity/min/max včetně canaries. Spuštěný buffer se hned pozastaví, aby nezávislé dohrání neovlivnilo audit parametrů.

Další11 pause kontrol na kontext současně používá dva2D/3D kanály téhož SampleID a další SampleID sdílející tentýž zvukový soubor. Pause0/1/−1/2 na připravených kanálech nezpustí playback; u spuštěných kanálů zachovává původní boolean pravidla. Sample_Stop odstraní právě odpovídající ID. Druhý sample se stejným Readerem zůstává hrát. Ukončení workeru se čeká přes původní přechody−24/0 do skutečného0; rozdílné počty pollů jsou zaznamenané a nepovažují se za stejný časový průběh. Před Init a po Free proběhlo dohromady112 empty-context volání na sestavení.

Reprodukce: CMake `-S tools/squall_sample_play_probe -B build/squall-sample-play-probe-vs -G "Visual Studio 17 2022" -A Win32 -T "v143,version=14.43.34808"`, build Release, `py -3 tools/validate_squall_sample_play.py`, totéž s nezávislou `--candidate` a `--output .local/squall-sample-play-fresh-validation.json`, potom `py -3 tools/record_squall_sample_play_verification.py`. Důkazy jsou `analysis/squall-sample-play-{validation,fresh-validation,regression}.json`. Produkce se neměnila. Výběr mezi více členy skupiny, limit skupiny, chybné3D pointery, preempce, selhání zařízení a souběžný teardown zůstávají otevřené; tento běh neporovnává slyšitelný výstup ani nedokončuje celý Squall.

## Skutečné paměťové callbacky a chyby alokace

`tools/squall_memory_probe` a `tools/validate_squall_memory.py` načítají skutečnou původní a vlastní DLL. Obě nezávislá sestavení mají každé1568 shodných návratových hodnot,228 guarded sample snapshotů a672 kontrol callback událostí v PC53/PC24. Každý kontext zaznamenává147 skutečných alokací/uvolnění/chyb s přesným pořadím, velikostí, generací callbacku a pořadovým identifikátorem alokace; syrové adresy jednotlivých heapů se za shodné nepovažují.

Ve třech stavech před Init / se skutečným DirectSound / po Free se porovnává38 dvojic code pointerů (A/B/null/1/−1/readable/noaccess). Původní10003bd0 vrací1 nebo−4, avšak10003bfc vždy volá100085e0 a uloží dodané callbacky i při chybě. Test po každé takové kontrole ihned obnoví platnou dvojici. Volání neplatného uloženého callbacku zatím ověřené není.

Vlastní mono PCM16 WAV má64 snímků,8000Hz a8ms. MemFlag0/1/−1/2, dva různé soubory a opakované načtení téhož názvu ověřily sdílení Readeru. Přepnutí A→B zachovává správný původní free callback každého živého Readeru;16bajtové canaries před/za každou alokací zůstaly celé. Skutečný UnloadAll uvolní všechny názvy i datové buffery právě jednou a opakované prázdné volání je bezpečné. Každé sestavení má18 záměrně neúspěšných načtení kvůli alokaci názvu nebo dat, úplný úklid a následné úspěšné opakování;102 dalších platných načtení na DLL.

Reprodukce: CMake `-S tools/squall_memory_probe -B build/squall-memory-probe-vs -G "Visual Studio 17 2022" -A Win32 -T "v143,version=14.43.34808"`, build Release, `py -3 tools/validate_squall_memory.py`, totéž s `--candidate build/squall-source-repro-vs20261010/squall-candidate/Release/Squall.dll --output .local/squall-memory-fresh-validation.json`, potom `py -3 tools/record_squall_memory_verification.py`. Přenositelné důkazy jsou `analysis/squall-memory-{validation,fresh-validation,regression}.json` a původní instrukce `analysis/squall-memory-native-instructions.txt`. Produkce se neměnila a odpovídá dosavadní102/102 regresi. Tyto dva dříve neporovnané exporty mají nyní částečné pokrytí; úplné97exportové ABI/chování, souběh a ostatní původní cíle pokračují.

## Skutečné 3D kanály: původní C3 větve a syrové kopie

Nové porovnání skutečných exportů prokázalo dvě odchylky zveřejněného podkladu. `SSample::Set3DParameters` na `10001a70` porovnává uloženou složku s novou přes x87 FCOMP/FNSTSW a testuje pouze C3 (`TEST AH,40h`). Stejné i neuspořádané hodnoty nechává beze změny. Tři vektory i oba testy vzdáleností nyní zachovávají tuto větev a původní pořadí. `Get3DParameters` na `10001c10` kopíruje DWORD přes MOV; původní C++ při kopii float měnilo signaling NaN na quiet NaN. Vektory se nyní kopírují dopředu po jednotlivých32bitových slovech a skalární getter také zachovává syrové bity. Ověřené starší listener implementace zůstaly beze změny.

`tools/squall_channel_vectors_probe` sestavuje samostatný `squall-channel-vectors-reference-test`. `validate_squall_channel_vectors.py` používá skutečné DLL a skutečné připravené mono PCM kanály se skutečným DirectSound3D, v přímém i odloženém listener režimu. Každé ze dvou nezávislých sestavení má v PC53/PC24 dohromady26 550 porovnaných návratů a5 272 celých guarded snapshotů:108 vektorových trojic na kontext, opakované signed quiet/signaling NaN payloady v každé složce,361 dvojic vzdáleností,512 trojic celočíselných hranic kužele a chráněné vstupy/výstupy. Cone setter původně požaduje zapisovatelný vstup, ostatní vektory jen čitelný; tato zvláštnost se zachovává.

Samostatné byte fixtures bez audio Init porovnávají tytéž dvě původní metody a vlastní metody na stejné adrese objektu. Na každé sestavení a obě přesnosti připadá4 096 setter a32 768 getter případů, včetně vybraných překryvů výstupů s uloženými parametry. Diagnostická COM hranice zaznamenává přesně předaný buffer, Deferred a úspěch/chybu. Jde o doplněk skutečných kanálů, nikoli náhradu zvukového backendu v implementaci. Zachované vlastní DLL před opravou znovu prokázaly rozdíl cone source−13/native1 i změnu `7f800001`→`7fc00001` v obou přesnostech.

Obě sestavení znovu prošla všech45 herních zvuků /122 527 752 přesných PCM bajtů, osm fyzických DirectSound bufferů /564 480 bajtů, herní Loading/Options/playback/pause i PersonSounds. Úplný aktuální Win32 CTest prošel102/102 za1019,85s. Čtyři další celé obousměrné obnovy sedmi budov zachovaly4089 save bajtů; dvě source obnovy jsou bez AV. Dvě aktuální DLL mají shodných všech400 384 bajtů kromě přesně dvou linkerových DWORD časových razítek; soubory se neupravují. Souhrn, source/binary hashe a všechny původní instrukce jsou v `analysis/squall-channel-vectors-regression.json` a navazujících reportech.

Algoritmus1 zde vytvoří reálné3D buffery; algoritmus3 v počátečním pokusu obě DLL odmítly s−11, přesný backend HRESULT není zachycený. Samostatný původní PCM oracle prošel i při změně počtu zařízení ze tří na dvě; zastaralý checker299 byl opraven na293 pevných porovnání a dvě kontroly názvu/capabilities na každé skutečné zařízení. Nejde o opravu produkčního chování. Následná kontrola nástrojů1/1 také prošla. Prostorový waveform, všechny ostatní algoritmy/hardware, running3D timing, EAX/ZoomFX, obecné aliasy/FP flags a chybové OutsideVolume úspěšného cone getteru zůstávají neověřené. Celý97exportový audit ani projekt nejsou dokončené.

```powershell
cmake -S tools/squall_channel_vectors_probe -B build/squall-channel-vectors-probe -G "Visual Studio 17 2022" -A Win32
cmake --build build/squall-channel-vectors-probe --config Release
py -3 tools/validate_squall_channel_vectors.py
# Samotné metody bez audio Init; diagnostická COM hranice je výslovná:
py -3 tools/validate_squall_channel_vectors.py --fixture-only --output .local/squall-channel-vectors-fixture-validation.json
```

## Globální konfigurace skutečných DLL

Jedenáct exportů Get/Set3DAlgorithm, Get/SetBufferSize, Get/SetDevice, Get/SetHardwareAcceleration, GetNumDevice a Get/SetSpeakerMode nově prošlo skutečným načtením obou DLL. `tools/squall_config_probe` sestavuje `squall-config-reference-test`; `python tools/validate_squall_config.py` ověřuje PC53/PC24 před Init, s reálným DirectSound a po Free. Každé ze dvou sestavení má4634 API výsledků a1320 celých guarded output snapshotů. Zkouší signed hranice bufferu1..5000, opakované změny support tabulky algoritmů, speaker modes,36 dvojic raw hardware flags a přestavbu zařízení bez kanálů; čtyři scalar gettery po každé změně a null/bad/alias/cross-page hardware výstupy. V této etapě se neprokázala odchylka a produkční kód se neměnil. Původní zvláštnost Set3DAlgorithm10003a70, který zapisuje do support tabulky místo změny aktuálního algoritmu, zachovává už existující zdroj.

Metadata jsou `analysis/squall-config-validation.json`, `analysis/squall-config-fresh-validation.json` a `analysis/squall-config-regression.json`. Aktuální výrobní/game zdroje stále odpovídají úplnému102/102 běhu z `analysis/live-production-regression.json`; nový nezávislý celý běh se nevydává za provedený. Připravené/hrající kanály přes změnu zařízení, device loss, failure rollback, všechny fyzické endpointy/algoritmy a host bez zvukové karty nejsou tímto scénářem ověřené. Celé chování97 exportů zůstává nedokončené.

## Skutečný listener worker callback

`SQUALL_Listener_SetWorker`10004cf0 má další přímé porovnání bez produkční změny. Původní TEST/JA po10004d4b přijímá unsigned nenulový interval; signaturu veřejného zdroje tvoří `unsigned int`, takže jeho `<=0` už toto chování zachovává. VzoryFFFFFFFF/FFFFFFFE/80000000 se přijímají jako velké DWORD periody. Nulový handler naopak vynuluje handler/userdata/periodu bez kontroly předaného intervalu.

Standalone projekt `tools/squall_worker_probe` a driver `python tools/validate_squall_listener_worker.py` ověřily v každém ze dvou sestavení152 návratů v PC53/PC24: pre-Init, skutečný DirectSound worker thread a post-Free; null/code1,0/1/10/100/INT_MAX a tři vysokobitové periody. Skutečné callbacky obou DLL mají původní userdata a pokračují s nimi i po odmítnutí bad-code a zero-period setteru. Po synchronním clear nepřibude callback během35ms pozorování. V každém sestavení proběhlo alespoň osm skutečných callbacků; jejich počty ani plánování nejsou normalizované jako shodný časový trace. Dvanáct35ms pozorování vysokých unsigned period neprokazuje dlouhodobý wraparound timer.

Metadata: `analysis/squall-listener-worker-validation.json` a `analysis/squall-listener-worker-fresh-validation.json`. Nejsou ověřené všechny callback reentrancy/race/teardown větve, neobvyklé code stránky ani ztráta zařízení. API tím získává částečné pokrytí, nikoli tvrzení o úplném ověření97 exportů.

## Původní normalizace výchozích vzdáleností vzorku

`InitSoundDefault` na10007f20 používají SetDefault i explicitní defaulty při Sample_LoadFile. Původní FCOMP s nulou na10007f78/10007f8f testuje AH,41h: MinDist≤0 **nebo unordered** nahrazuje1 a MaxDist≤0 **nebo unordered** nahrazuje1000000000 (raw4e6e6b28). Veřejný1.48 zdroj s `<=0` NaN ponechával. Dvě produkční podmínky nyní vyjadřují negaci původní ordered-positive přijímací podmínky; žádný další rozsah není odhadnutý.

Nový standalone projekt `tools/squall_sample_probe` sestaví `squall-sample-defaults-reference-test`. `python tools/validate_squall_sample_defaults.py` používá skutečné normálně načtené DLL a skutečnou původní menu Ogg stopu vyjmutou do diagnostické složky. PC53/PC24 společně ověřují4260 normalizovaných28bajtových záznamů,100 chráněných pointer případů a44 explicitních LoadFile defaultů; celkem8710 návratů. Všech sedm polí zahrnuje signed integer hranice a frekvenci0; pro obě vzdálenosti1040 float vzorů zahrnuje1024 signed quiet/signaling NaN payloadů. Input/output guardy i všechny uložené bity se porovnávají. Setter původně vyžaduje zapisovatelný input, přestože jej pouze čte; test zahrnuje readonly i cross-page ochrany. Sample je načtený před Init, takže tato samotná etapa nepotvrzuje přehrávání ani celý sample lifecycle.

Stejný aktuální harness proti zachované vlastní DLL commitu8599644 prokazuje4096 NaN odchylek a proti opravě nulové odchylky. Původní první nález a celé počty jsou v `analysis/squall-sample-defaults-before-validation.json`, aktuální výsledky v `analysis/squall-sample-defaults-validation.json` a druhém `-fresh` reportu. Původní knihovnu nový runtime nepoužívá; zůstává pouze nezávislým porovnávacím programem. FP status/masky, libovolné překryvy a všechny ostatní sample/playback/codecs větve zůstávají výslovně neověřené. Aktuální hlavní DLL má SHA99f7b608031aea175c7b80cfbc3a87123310f060f15fa830464a3b4607ee0fb8, druhá47b62d5d3a9d10b99f79b0fee81aadf5d78837e328d938511b6fdfbf1d7b3b4c. Shodují se všechny bajty kromě dvou doložených linker timestampů. Obě prošly aktuální PCM/buffer/game-audio/PersonSounds regresí a hlavní hra čtyřmi3557bajtovými save přenosy. `analysis/squall-sample-regression.json` odděluje tento scope od předchozího úplného102/102 běhu commitu8599644.

## Původní C3 porovnávání vektorů listeneru

Další skutečný DLL test prokázal odchylku i ve změnových podmínkách `10003f80`/`100042d0`/`10004470`. Originál po každém FCOMP čte FNSTSW AX a pouze TEST AH,40h: C3=1 znamená shodnou i neuspořádanou hodnotu. Veřejné C++ `!=` při NaN žádalo další DirectSound update, kde originál ponechal vektor beze změny; konkrétní opakovaná quiet-NaN posloupnost vracela source−8/native1 v PC53 i PC24. Původní failed report a oba potvrzovací logy zachovává `analysis/squall-listener-vectors-before-validation.json`, kandidát je vlastní DLL commitu2745060.

Šest porovnání nyní používá přímé x87 FCOMP původních paměťových slov a test pouze C3. Zachované jsou short-circuit pořadí složek i pořadí Position/Velocity/Front/Top; všechny ostatní zápisy a DirectSound volání zůstávají původní. Win32 MSVC helper nepředává signaling NaN přes mezilehlé float hodnoty. Přenositelná větev vyjadřuje stejnou ordered nerovnost; aktuální DLL je ověřená na MSVC Win32.

`tools/squall_listener_vectors_reference_test.cpp` má samostatný reprodukovatelný projekt `tools/squall_vectors_probe`: `cmake -S tools/squall_vectors_probe -B build/squall-vectors-probe -G "Visual Studio 17 2022" -A Win32`, pak Release build a `python tools/validate_squall_listener_vectors.py`. Dvě produkční sestavení mají po4556 porovnáních v PC53/PC24:1776 celých guarded snapshotů,1680 setter volání a1008 getter pointer případů. Skutečně se zkouší pre-Init, immediate i deferred DirectSound listener a post-Free;11 float trojic včetně opakovaných/signed nul, subnormálů, infinity, quiet/signaling NaN a orientačních bází; všech16 přítomnostních masek čtyř vstupů, nečitelné/read-only/cross-page adresy a úplné i částečné výstupní aliasy. Porovnání ověřuje i cache po odmítnutém COM update; žádný COM mock se nepoužívá. Neověřuje prostorový waveform, všechny float vzory, vnitřní aliasy, FP status/masky ani device loss či závody workerů.

Aktuální DLL hashe jsou3dd8b00465a4314f6df330d4032102a63f5f92a46e8e206a25fcb1e47da8b1a9 a6be1ca09e4e42270fd82e223ba34599c6c8b2455ba2cf17d091895994864b223. Obě prošly znovu ordered factor, skutečným game-audio a PersonSounds porovnáním. Poslední úplný102/102 CTest je výslovně předchozí commit2745060; pro tuto opravu se neopakoval a nenahrazuje se tvrzením o novém úplném běhu. Aktuální scope a celkové neověřené oblasti jsou v export auditu a `analysis/squall-vectors-regression.json`.

## Ordered kontrola faktorů listeneru

Původní settery `10004600`, `10004750` a `100048a0` přijímají pouze uspořádané `0 < factor <= 10`. Veřejný zdroj1.48 používal `factor <= 0 || factor > 10`, což propouští NaN. Před inicializací tím vracel−24 místo původního−4; inicializovaný listener mohl být také změněn. Produkční podmínky nyní přesně negují původní přijímaný interval. Nejde o obecné přepsání rozsahu3D API.

`tools/validate_squall_listener_factors.py` porovnává skutečně načtené obě DLL před Init, s inicializovaným DirectSound listenerem a po Free. Pro každý setter a lifecycle zkouší1040 binary32 vzorků: signed nuly/subnormály, konečné hranice včetně sousedů10, infinity a1024 signed quiet/signaling NaN payloadů. PC53/PC24 společně mají24962 porovnání výsledků,18720 vstupů,18432 NaN a6240 úspěšných skutečných getterů s přesnými bity uloženého faktoru i výstupními guardy. Shoda byla zopakována ve druhém sestavení. Stejný aktuální harness proti zachované vlastní DLL z commitu38bedf7 znovu prokazuje6144 původních NaN odchylek; tato stará DLL slouží jen jako diagnostická reference. Prvotní nález má samostatný report `analysis/squall-listener-before-validation.json`.

Reprodukce po sestavení cíle `squall-listener-factors-reference-test`: `python tools/validate_squall_listener_factors.py`; druhému sestavení se předají `--candidate`, `--oracle`, `--output` jako u ostatních Squall driverů. Baseline `--before-candidate` musí být skutečná zachovaná vlastní DLL s hashem ověřeným reportem, nikoli originál. Metadata jsou `analysis/squall-listener-factors-validation.json` a `analysis/squall-listener-factors-fresh-validation.json`. Nejsou tím ověřené všechny2^32 float vzory, FP status/exception masky, libovolné výstupní adresy ani ostatní3D/callback/device větve. Úplný audit97 exportů zůstává rozpracovaný. Navazující regrese obou aktuálních DLL znovu ověřila PCM/buffery, skupiny, kanálové rozsahy, game-audio a PersonSounds; nový úplný Win32 CTest prošel102/102 za540.12s bez focused retry. Aktuální SHA jsou3b0cf8d76fb55255880d74806bd68e05e890b13a886b21ae81932c72e91bb366 a311a7d7db52b31db5e16e284fbfb3f847fe587601b5d99bf7569a62faef29537. Dvě DLL mají shodné všechny bajty kromě obou přesně identifikovaných linker timestampů; skutečné soubory se nenormalizují. Podrobnosti: analysis/squall-listener-regression.json a dva aktuální export-contract reporty.

Původní `Squall.dll` má 97 pojmenovaných C exportů. Ghidra export obsahuje 916 funkcí: 911 dekompilovaných, čtyři přesměrování a jednu chybu v matematické x87 rutině `1003cf82`. Úplný zdrojový kandidát DLL nyní lze sestavit; má přesně původních 97 exportních názvů a ordinálů bez forwardingů. Shoda celé implementace ještě není prokázaná.

## Zdrojový podklad

Pro rozbor je lokálně uložen zveřejněný [zdroj Squall 1.48](https://github.com/xtreme3d/squall/tree/20988426d2dde0a4b7aa6e18578f59c55d3af3a8), v `.local/squall-source`. Původní projekt a jednotlivé zdroje jsou čitelné a upravitelné; snapshot obsahuje vlastní MIT licenci. Jeho původ a hashe všech 107 souborů jsou v `analysis/squall-source.lock.json`.

Snapshot lze znovu získat jako [archiv připnutého commitu](https://codeload.github.com/xtreme3d/squall/tar.gz/20988426d2dde0a4b7aa6e18578f59c55d3af3a8), rozbalit bez nejvyšší složky do `.local/squall-source` a ověřit:

```powershell
python tools/inspect_squall.py
```

Hlavička obsahuje deklarace všech 97 původních názvů exportů a dvě další: `SQUALL_Channel_SetEndWorker`, `SQUALL_Sample_LoadFromMemory`. Mapa původních adres a referenčních deklarací je v `analysis/squall-source-match.json`. Deklarace popisují zveřejněnou verzi; ověření typů a ABI proti našemu EXE ještě není dokončené.

## Doložený rozdíl verzí

Porovnání inicializace s instrukcemi původní DLL prokázalo odlišnou hlasitostní křivku. V originálu vytváří `SQUALL_Init` (`10003280`) tabulku:

- index 0: `-10000`;
- indexy 1–99: zkrácení směrem k nule výrazu `10000 * log10(0.009F * index + 0.1F)`.

Konstanty jsou na adresách `10048188` (double 10000), `10048190` (float 0.1), `10048194` (float 0.009). Instrukce `fldlg2` a `fyl2x` realizují logaritmus o základu 10; následná `__ftol` zkrátí výsledek. Referenční zdroj používá `floor(5000 * log10(index / 99.0F))`. Například původní index 99 má hodnotu `-39`, kdežto referenční křivka vrací 0. Záměna celé knihovny za tuto verzi by proto změnila chování.

Původní tabulka je obnovena v `src/squall/VolumeTable.cpp` a sestavuje se jako `recovered_squall_utilities`. Test ověřuje všech 100 položek proti hodnotám nezávisle vypočteným s 70místnou desetinnou přesností z přesných konstant PE. Toto ověření dokládá převod algoritmu; přehrávání a porovnání běhu celé DLL dosud nebyly provedené.

## Parametry a rozložení kanálů

`src/squall/Parameters.cpp` obnovuje celou rutinu `100030a0..1000321d`. Její 52bajtový typ sdílí herní `SoundSetup`; herní konstruktor dál dosazuje přesně původních třináct hodnot. Nečitelný vstup se nuluje před validací. Rate, bits, channels, buffer, algoritmus, listener a HW flags mají původní hranice; platnost HWND kontroluje skutečné `IsWindow`. Počet zařízení poskytne vlastní audio backend. Dotaz se provede jen pro kladný Device.

Všechny tři float faktory se omezují na 10 a nekladné nebo NaN hodnoty přecházejí na 1. Instrukční testy `fnstsw/test ah,41` na `10003170`, `10003198`, `100031c0` dokládají větev pro unordered vstup. Referenční zdroj naopak nemá horní omezení DistanceFactor a jeho `<=0` testy ponechávají NaN. Přímé nativní porovnání obou přesností prošlo pro 201 154 úplných 52bajtových výstupů: seedované hodnoty, jednotlivé hranice, infinities/NaNs/subnormály, skutečné HWND, nečitelný a přes hranici stránky zasahující vstup, překryv při forward word copy a short-circuit dotazů na zařízení. Report: `analysis/squall-parameters-validation.json`. Počet zařízení je výslovný testovací vstup, žádné audio zařízení se tu neinicializuje. FP exception/status flags a neplatný výstupní ukazatel nejsou ověřené.

Originální `SQUALL_Init` přiděluje a nuluje **300 bajtů na kanál** (`100033d2..100033e5`, `100033fe..10003418`). Připnutý veřejný zdroj má `sizeof(SSample)==308`: navíc obsahuje ChannelEndWorker na offsetu 300 a ChannelEndUserData na 304. Pole původního průběžného callbacku zůstávají na 292/296; export koncového callbacku v originálu chybí. Reference proto vyžaduje i úpravu interní struktury, nikoli pouhé omezení exportního seznamu. Win32 probe měří reference: context 1344, parameters 52, sample defaults 28. Shodnost všech ostatních polí tím není dokázaná. Report: `analysis/squall-layout-validation.json`.

Opakování porovnání a probe (nejprve připnutý zdroj podle postupu výše):

```powershell
cmake -S . -B build/windows -G 'Visual Studio 17 2022' -A Win32 -DBUILD_SQUALL_REFERENCE_LAYOUT=ON
cmake --build build/windows --config Release --target squall-reference-layout squall-parameters-reference-test
python tools/validate_squall_layout.py
python tools/validate_squall_parameters.py
```

`squall-reference-layout` je diagnostický cíl mimo běžné sestavení; neobsahuje implementaci zvuku. `read_function.py --binary squall --asm --bytes N` nyní korektně čte starší metadata bez body bounds. V takovém případě explicitně hlásí omezený rozsah; součet body_bytes se nepoužívá jako domnělá souvislá hranice funkce.

## Zbývající práce

Zveřejněné zdroje pomáhají pojmenovat a pochopit původní rutiny. Každá použitá implementace se musí porovnat s inventarizovanou DLL; už první kontrola našla skutečnou odchylku. Zbývá obnovit kontext a kanály, DirectSound backend, WAV/ADPCM/MP3/OGG a další přítomné dekodéry, callbacky, 3D zvuk/EAX/ZoomFX, chybové stavy a celý exportní ABI. Zveřejněný projekt rovněž odkazuje na nepřibalené statické Ogg/Vorbis knihovny. Jejich původní verze zatím nejsou určeny.

## Sestavitelný kandidát a přesné PCM

Upravitelná implementace je v `src/squall/runtime`, s původem 84 převzatých zdrojů/hlaviček a licencí v `analysis/squall-runtime-provenance.json`. Původní 107souborový snapshot zůstává nezměněný. Žádná původní programová binárka ani zveřejněná statická knihovna není použita jako implementace. Candidate sestavuje skutečný DirectSound backend, kontext, kanály, callbacky, WAV/PCM/ADPCM/MPEG/Vorbis/WMA a tracker code. Přítomnost kódu není důkaz shodného chování.

Původní kanál má 300 bajtů, callbacky 292/296 a kontextová pole kanálů 252/256. `Abi.cpp` tyto hodnoty kontroluje při Win32 sestavení. Původní dekodér Vorbis přiděluje `OggVorbis_File` o 720 bajtech (`100138f0`); stejně velká candidate struktura sama neurčuje verzi kodeku. WMA extension branch a tracker fallback jsou podle `1002d8c0` opět zapojené. Opravený GetNumDevice odpovídá původnímu tail jumpu `10003c70 -> 10007e40`. Výchozí public zdroj zde dereferencoval neinicializovaný CDXOutput ukazatel.

Tabulka má podle `1000329a` 256 položek; první stovka je vypočtená původní křivkou a dalších 156 zůstává nulových. Inverzní převod `Channel_GetVolume` na `100056b0..100056d7` používá `trunc((pow(10,attenuation*0.0001)-double(0.1F))*100*(1/double(0.9F)))`. Nativní test prošel pro všech 10 001 DirectSound hodnot −10000..0 v obou přesnostech a všech 256 slov tabulky. Source reference inverse je odlišná. Testovací COM getter dodává attenuation, nativní lookup/aritmetika a lock/unlock jsou vykonané; skutečné přehrávání tím není ověřené.

Ogg 1.1 a Vorbis 1.0.1/1.1.0 jsou kandidáti připnutí podle archivu a každého source SHA v `analysis/squall-codecs.lock.json`. Zdroje se získávají z oficiálních archivů Xiph. [Oficiální historie Vorbisu](https://github.com/xiph/vorbis/blob/main/CHANGES) a [archivní hashe](https://github.com/xiph/vorbis/blob/main/releases.sha2) dokládají původ podkladů; původní release tag v této DLL není prokázaný. Obě kandidátní verze vykazovaly stejné počáteční PCM odchylky.

Instrukce `10037330`, `10037840`, `10037d40` a `10037dd0` určily přesné binary32 spill hranice MDCT a přenos nezapsaných registrů mezi 32/16/8point stadii. Upravený source v `src/squall/codecs/vorbis` zachovává BSD licenci Xiph a realizuje tyto hranice explicitně. Vnitřní double výpočty zachovávají šířku PC53/PC24; konkrétní float zápisy mají skutečný binary32 spill. Pouhé přepnutí MSVC floating-point režimu shodu neopravovalo. Nyní oba režimy prošly: 40 800 lookup hodnot, 800 000 butterfly hodnot a 4 177 920 hodnot celých MDCT transformací velikosti 64..8192, bez tolerance.

Všech **45 herních zvukových souborů včetně přípony WAV je ve skutečnosti Vorbis** (native decoder vtable RVA `482a0`). Z obnoveného FileIndex/SoundFiles se načítají v memory režimu 1. Obě přesnosti prošly pro všechny sample IDs, délky, frekvence a 28bajtové defaulty; 540 náhodně přístupných PCM oken a všech **122 527 752 bajtů celého mono/stereo PCM**. Průběžné čtení používá 4096frame chunky a sentinel `0xffffffff`, buffer guards zůstávají shodné. Native reference se zde normálně načítá jako samostatný test oracle včetně CRT entrypointu. Source strana používá vlastní staticky sestavený runtime; exportní contract sestavené DLL se ověřuje zvlášť. Celé veřejné DLL runtime tím ještě není ověřené.

Reprodukování:

```powershell
python tools/fetch_squall_codecs.py
cmake -S . -B build/windows -G 'Visual Studio 17 2022' -A Win32 '-DBUILD_SQUALL_CANDIDATE=ON' '-DSQUALL_VORBIS_CANDIDATE=1.1.0' '-DSQUALL_VORBIS_FP=fast'
cmake --build build/windows --config Release --target SquallCandidate squall-runtime-reference-test squall-vorbis-reference-test squall-dll-reference-test --parallel
python tools/validate_squall_runtime.py
python tools/validate_squall_dll.py
cmake --build build/windows --config Release --target game-audio-reference-test --parallel
python tools/validate_game_audio.py
```

Výstup: `build/windows/squall-candidate/Release/Squall.dll`. Candidate cíle jsou explicitně mimo běžné sestavení, dokud celý nativní audit není dokončen. Přenositelné důkazy: `analysis/squall-runtime-validation.json`, `analysis/squall-candidate-abi.json` a stav jednotlivých exportů `analysis/squall-export-audit.json`. Výchozí DirectSound zařízení zde mají shodný počet 3; jiné hosty mohou mít jiný počet. Všechny potvrzené i neověřené oblasti jsou v reportu.

Další práce: úplné veřejné API, fyzický DirectSound playback, kanály/workers/synchronizace, 3D/EAX/ZoomFX, mode 0 se seekovatelnými callbacky, EOF/loop/error/poškozené vstupy, další dekodéry a celý herní audio lifecycle. PCM shoda na herních souborech neprokazuje obecnou shodu dekodéru ani historický release tag.

## Skutečně načtená DLL a fyzické buffery

`squall-dll-reference-test` nepřilinkovává statický Squall runtime. Normálně načítá obě samostatné DLL, rozpozná všech 97 pojmenovaných exportů a vykonává vybrané veřejné API. Přes dekodéry uvnitř skutečně načtené zdrojové DLL souhlasí všech 122 527 752 mono/stereo PCM bajtů všech 45 herních souborů, včetně sequential sentinel a guard bajtů. Vlastní neveřejné symboly test vyhledává v její vlastní linker mapě; mapa musí mít timestamp načteného PE. Nativní neveřejné adresy slouží pouze tomuto oracle. DLL nepřidává exporty ani forwarding.

Na tomto hostu obě DLL skutečně inicializují DirectSound s původními herními parametry. Obě přesnosti prošly pro 299 API výsledků: enumerace tří zařízení, kompletní názvy/capabilities, vzorky/defaulty, initialized getters, připravený ambientní hudební kanál, hranice volume/pan/frequency a délka/loop/priority. Osm celých fyzických bufferů čtených skutečným DirectSound `Lock` obsahuje přesně **564 480 shodných PCM bajtů**. Zahrnují frame pozice 0, 13 230 a 100 framů před koncem při loop=1. Nejde o falešný COM backend. `start=0` znamená přípravu bez přehrávání; slyšitelný výstup ani časově korelované přehrávání zatím nejsou ověřené.

Stop má asynchronní mezistav: `100054b0` vrací při STAGE=STOP −24, po odstranění workerem 0. Test každému workeru dovoluje pouze tyto stavy a nejvýše dvě sekundy; teprve po ukončení obou porovnává terminální stav. Nesrovnává časování dvou sekvenčních volání jako současný snímek.

Při tomto ověření byla opravena chyba sestavovacího cíle: Visual Studio s jediným vstupem `.def` a bez object file mohlo přeskočit link DLL po změně statické runtime knihovny. Cíl nyní kompiluje existující `Abi.cpp` s kontrolami rozložení a skutečně přelinkovává DLL. Předchozí statické PCM porovnání tím nebylo zneplatněno; samotná předchozí DLL tím nebyla dokladem těchto runtime oprav. Nynější porovnání prošlo také nad samostatným čistým sestavením `build/squall-source-repro-vs20261010`, bez předchozí DLL v jeho výstupu. Nová cache musí explicitně volit Visual Studio; výchozí Ninja nepřijímá `-A Win32`.

Reporty `analysis/squall-dll-validation.json` a `analysis/squall-dll-fresh-validation.json` obsahují hash zdrojů, kandidáta, testovacího EXE a mapy, jednotlivé přesnosti, rozsah a neověřené oblasti. Audit exportů rozlišuje vybrané porovnané vstupy od úplně ověřeného podpisu/chování všech 97 API. Žádný export zde není automaticky označen za úplně ověřený.

Navazující `SquallSoundApi` skutečně propojuje obnovené herní audio objekty s vlastní DLL. V PC53/PC24 prošel null-HWND herní Init a 378 stavů včetně skutečných MainMenu/Options controls, pohybu playback cursoru, pause/resume a úklidu; podrobnosti v `SOUND_MANAGER.md`. Samostatný prepared-buffer report výše nadále neobsahuje playback. Playback důkaz je v oddělených `analysis/game-audio-validation.json` a `analysis/game-audio-fresh-validation.json`; neprokazuje audible výstup ani celý herní runtime.


## Skupiny skutečných kanálů přes veřejné DLL API

Nový `squall-groups-reference-test` načítá obě skutečné DLL s původním entrypointem a volá jen veřejné exporty, skutečné DirectSound buffery/workery a obnovené read-only souborové callbacky. PC53/PC24 prošly840 veřejnými výsledky; stejný audit prošel nad druhým samostatným sestavením. Hlavní DLL zůstáváa6241f37479d69716e15e07787529a68cd3359cc33d3d7b9c81bd1e63d5071ab; druhé sestavení máa403ddc6c0f49d6c33e186297eafd000f4ceee30852b6e6285f1678c2b5a2b64. Ve všech400384 bajtech se liší jen dvě linkerová časová pole: PE COFF288..291 a debug directory283716..283719; po jejich oddělené klasifikaci je zbytek souborů přesný. Reálné binárky nejsou upravené. Metadata: `analysis/squall-groups-build-comparison.json`.

V každé přesnosti288 prázdných volání prověří čtyři group exporty před Init, po skutečném Init, po ukončení kanálů a po Free. Živá část má dva ambientní kanály ve skupině17 a třetí ve skupině−7. Skupinové volume indexy0/1/43/99/100/101/255 se porovnávají se všemi třemi skutečnými gettery; jiná skupina se nemění. Neplatné indexy mimo256slovnou tabulku jsou zkoušené pouze bez odpovídajících živých kanálů, kdy originál tabulku vůbec nečte. Frekvence−1/0/99/100/22050/44100/88200/100000/100001/INT_MAX souhlasí v návratech i skutečných getterech.

Připravený buffer má veřejný status3 a skupinová pauza jej nespouští. Skutečné Channel_Start a Channel_Pause nejprve vytvoří paused status2; potom se porovnává skupinové resume/pause s0/1/−1/2 a status1/2. Původní libovolný nenulový Pause zapíná pauzu. Stop připouští jen původní přechod−24→0 do dvou sekund a kontroluje nedotčenou jinou skupinu. Scheduler okamžiky nejsou porovnávané jako současný snímek. Instrukční podklad tvoří10007100/10007190/10007220/100072c0 a vnitřní10002260/10001f40/10001f80/10001fe0.

První test chybně očekával prepared status2; obě DLL měly shodně3 a test skončil před živou pauzou. Opravený scénář používá skutečný Start a ověřuje i prepared no-op. Produkční zvukový kód se neměnil. Reporty `analysis/squall-groups-validation.json` a `analysis/squall-groups-fresh-validation.json` a aktualizovaný `analysis/squall-export-audit.json` zachovávají stav částečného porovnání; žádný úplný podpis/chování97 exportů se automaticky neoznačuje za hotový. Zbývají jiné druhy kanálů, souběh worker/callback/teardown, další zařízení, slyšitelný a přesně časově korelovaný výstup, neplatné live volume indexy a ostatní API.

```powershell
cmake --build build/windows --config Release --target squall-groups-reference-test --parallel 4
python tools/validate_squall_groups.py
cmake --build build/squall-source-repro-vs20261010 --config Release --target squall-groups-reference-test --parallel 4
python tools/validate_squall_groups.py --candidate build/squall-source-repro-vs20261010/squall-candidate/Release/Squall.dll --oracle build/squall-source-repro-vs20261010/Release/squall-groups-reference-test.exe --output .local/squall-groups-fresh-validation.json
python tools/record_squall_group_verification.py
```


## Původní adresně závislá pozice a další veřejné kanálové operace

Nový audit našel další konkrétní rozdíl oproti veřejné1.48: v `SSample::GetPlayPosition` má převzatý zdroj neinicializované DWORD, ale skutečný10002180 rezervuje lokální slot instrukcí `push ecx`. PLAY jej přepíše skutečným DirectSound kurzorem a PAUSE použije PausePtr; v ostatních stavech zůstávají bity adresy kanálu. Produkční `Channels.cpp` nyní tento původní jev vyjadřuje explicitně jako32bitovou adresu `this`. Nevrací odhadnutou nulu a nespoléhá na náhodnou optimalizaci moderního kompilátoru. Nezávisle alokované kanály proto přirozeně nemají totožnou raw připravenou pozici.

`squall-channel-ranges-reference-test` porovnává metody obou skutečně načtených DLL nad kopií skutečného300bajtového kanálu **na stejné adrese**. Používá skutečný zapůjčený DirectSound buffer a timestamp-ověřenou linker mapu zdroje; originální privátní adresa je pouze diagnostický oracle. V PC53/PC24 prošlo480 případů pro stavy0..7, double buffering0/1, tři PausePtr a pět logických kurzorů. Návrat i výsledné32bitové slovo se porovnávají přesně; nejde o normalizaci dvou různých heap adres ani o náhradu COM implementace.

Další veřejná část skutečně zkouší frame/ms fragmenty, prohozené meze, clamping, neplatné a stejné hranice, úplné pole návratů a výstupů. Ve dvou přesnostech prošlo1276 pointer případů: samostatné i aliasované výstupy, null, neplatná adresa, read-only stránka a přechod do nečitelné stránky; celé guard pole zůstává shodné. Před přesným seek porovnáním se skutečné buffery spustí a pozastaví, kurzor se obnoví přes původní setter. Smyčky se ověřují pro−1/0/1/2/3/255/256/INT_MAX a priority i pro65536/INT_MAX a záporné chyby; původní priorita nemá horní limit65535 uváděný v komentáři veřejného zdroje. Čtyři neplatné ChannelID mají vlastní návraty a výstupní guardy.

Celkem každý ze dvou samostatných buildů prošel2542 porovnáními výsledků v PC53/PC24:1582 výsledků veřejného API a960 návratů/hodnot přímého same-address replay. Počty zahrnují48 fragment setter případů a28 frame/ms seek případů. Původní první probe porovnával připravené GetPlayPosition přes různé adresy a ukázal rozdíl; nynější ověření odděluje původní adresní sémantiku od skutečných pozastavených pozic.

Hraniční seeky jsou rozdělené: po přesně zaznamenané předchozí posloupnosti se frame pozice za koncem a záporná milisekundová pozice v osmi samostatných pokusech nevrátily do čtyř sekund, shodně source/native v PC53/PC24. Každý přímý testovací proces ohraničí a ukončí `subprocess` timeout. Toto není důkaz nevracení pro každý vstup či každou historii: samostatný první seek za konec bez stejné předchozí posloupnosti se v průzkumu vrátil. Produkční chování se nepřepisuje vlastním odmítáním nebo clampem; širší malformed/overflow/liveness rozsah zůstává neověřený.

Metadata jsou `analysis/squall-channel-ranges-validation.json` a `analysis/squall-channel-ranges-fresh-validation.json`; jejich pracovní logy mají hashe i označení source/native setteru. Opravený hlavní kandidát má SHAb28e7e071a51fece6dd726314ea1a0bde4a3e5bee0e7e16fc61a4298034e9fd7. Druhý build má SHAe121f93c309f9f8c747f99919c198cbb6e00d7fc17408679df34a95b71d7567a. Regrese PCM, skutečných bufferů, skupin, Loading/menu/Options a PersonSounds už prošly v obou sestaveních; úplné Win32 sestavení také. Navazující celé herní EXE znovu prošly čtyřmi přesnými2650bajtovými save přenosy. Úplná regrese skončila101/102 za521.94s: izolovaný recovered-game-source-world očekával tutorial20 po jediném snímku. Následný diagnostický běh obou přesností prošel; přesný spouštěč původního selhání nebyl zachycen. Driver nyní před tímto očekáváním používá existující skutečnou settle bránu, protože native frame dispatcher vynechává step průchody během UI přechodu; při dalším selhání vypíše stav sáčku/farmy/registry/tutoriálu. Každý běh také atomicky rezervuje novou složku, aby PID reuse nemohl znovu otevřít starý profil. Takto upravený samotný test prošel třikrát po sobě (PC53/PC24) za26.87s. Nejde o jediný nový102/102 běh; původní failed JUnit a tři focused výsledky eviduje `analysis/squall-position-regression.json`. Produkční herní logika se kvůli tomuto testu neměnila; celý projekt zůstává rozpracovaný.

```powershell
cmake --build build/windows --config Release --target squall-channel-ranges-reference-test --parallel 4
python tools/validate_squall_channel_ranges.py
cmake --build build/squall-source-repro-vs20261010 --config Release --target squall-channel-ranges-reference-test --parallel 4
python tools/validate_squall_channel_ranges.py --candidate build/squall-source-repro-vs20261010/squall-candidate/Release/Squall.dll --oracle build/squall-source-repro-vs20261010/Release/squall-channel-ranges-reference-test.exe --output .local/squall-channel-ranges-fresh-validation.json
```

Zbývají prostorové/listener operace, callbacks a souběh, ostatní dekodéry/zařízení, úplné podpisy/chování97 exportů i další herní systémy. Tento audit není úplnou náhradou veřejné verze ani dokončením rekonstrukce.
