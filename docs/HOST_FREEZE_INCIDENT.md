# Zamrznutí obrazu při herním testování

Současný stav: uživatel následně výslovně schválil pokračování na tomto počítači s průběžným sledováním. Přesná odpověď je v `analysis/host-runtime-monitoring-authorization.json`. Níže popsané původní zastavení a návrh izolovaného hosta byly nahrazené výhradně hlídanými krátkými pokusy; incident se nadále nepovažuje za vyřešený. Odstavec „Nezávislý dohled po souhlasu uživatele“ zachycuje aktuální postup.

Uživatel7.11.2026 oznámil zamrznutí celých Windows vyžadující restart; následně
upřesnil, že mohl zamrznout obraz, zatímco session pokračovala. Další interaktivní
testy na běžném desktopu jsou zastavené. Při kontrole po restartu nezůstaly nalezené
vlastní herní testovací procesy ani jejich Python drivery.

Systém hlásí poslední boot ve22:27:40.5 SEČ a Kernel-Power41 ve22:27:51.566.
Tento záznam dokládá nečistý restart, nikoli jeho příčinu. Ve čteném intervalu
22:10–22:29 nebyl nalezený záznam Display/Dxg/WHEA/BugCheck vysvětlující zamrznutí.
Absence záznamu nevylučuje problém grafického ovladače. Host má Windows build26200
a AMD Radeon Graphics, ovladač31.0.21924.7001.

Poslední přerušený běh je `.local/tutorial-warning-runtime/20261107T211631362554Z`.
Obsahuje snímky začátku nativní smíšené sklizně, ale žádný konečný `report.json`.
Je proto přerušený, nikoli úspěšný. Předchozí uložené úspěchy40/42 a samostatně
dokončené nativní41 tím nejsou přepsané ani vydávané za dokončení posledního běhu.

Původní `TutorialMemory.visible_scene` používal `SuspendThread` na herním
vykreslovacím/UI vlákně při čtení měnícího se stromu. Pozastavení v libovolném místě
je rizikové, protože vlákno může držet sdílený zámek.
[Microsoft výslovně uvádí riziko deadlocku a že tato funkce není určena pro synchronizaci](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-suspendthread).
Tento mechanismus byl odstraněný. Souvislost s konkrétním zamrznutím není prokázaná.
Naměřené přibližně20s prodlevy SendInput zůstávají samostatnou otevřenou stopou.
Odstraněný je také fallback `AttachThreadInput`, kterým test spojoval svou vstupní
frontu s jiným foreground vláknem, aby vynutil fokus. Při odmítnutém foreground
požadavku nyní test skončí před odesláním vstupu.

Nové čtení kontroluje topologii bez zastavování vláken, má časový a velikostní
limit a odmítne pozorovanou souběžnou změnu. Nezaručuje atomický snímek všech
vykreslovaných dat. Dosud se ověřuje pouze pomocí CPU testů nad paměťovými fixtures;
nové skutečné GPU ověření se na hostu nespouštělo.

`GameGui` před začátkem interaktivního scénáře kontroluje místní
`.local/desktop-runtime-hold.json`. Aktivní nebo nečitelná pojistka scénář odmítne
před spuštěním hry a odesláním vstupu. Nemá přepínač prostředí pro obejití.
Další grafické/input ověřování patří do izolovaného hosta. Místní systém je
Windows11 Home; [tato edice Windows Sandbox nepodporuje](https://learn.microsoft.com/en-us/windows/security/application-security/application-isolation/windows-sandbox/).
Sandbox EXE ani běžné VM nástroje nebyly při prvním průzkumu nalezené. Dotaz na
volitelnou systémovou funkci vyžadoval administrátorský token; tento dotaz není
dokladem jejího stavu. Nic se neinstalovalo, nezapínalo a nebyl vyžádaný restart.

Původní cíl zůstává aktivní. Analýza instrukcí, zdroje, sestavení a kontroly bez
grafického zařízení pokračují. Celý CTest se na hostu neopakuje, protože zahrnuje
grafické testy. Pozdější souhlas uživatele již umožňuje další hlídané experimenty;
není třeba jej znovu vyžadovat pro každý takový pokus. Potřeba skutečné ruční
aktivace okna při odmítnutém fokusu je praktická součinnost, nikoli nové povolení.

## Nezávislý dohled po souhlasu uživatele

`host-test-watchdog` vytváří vlastní pojmenovanou procesovou skupinu Job s automatickým ukončením jejích procesů při zániku hlídače. Hlídač neotevírá grafické zařízení a neodesílá vstup. Sleduje změny souboru postupu, absolutní čas, zátěž CPU, volnou RAM a vlastní procesy. Překročení limitů ukončí pouze tuto skupinu. CPU zkoušky ověřily normální dokončení, zastavený postup, absolutní limit i úklid dvou skutečných dětí po násilném ukončení hlídače. Viz `analysis/host-watchdog-cpu-validation.json`.

Místní pojistka zůstává aktivní. Samotná proměnná prostředí není povolení: ověřuje se PID testu, platnost lease, živý správný hlídač a skutečné členství testu v jeho Job. Vstupní API mají záznam před voláním a dobu návratu; pomalé volání zastaví další interakci po dokončení párového uvolnění. Při skutečně nevracejícím se volání zasahuje nezávislý hlídač.

Živý heartbeat driveru nyní není jedinou kontrolou postupu. Sdílený probe nejvýše jednou za0,5s kontroluje přesně své herní okno pomocí250ms WM_NULL a zapisuje `owned-window-health.jsonl`. Před dotazem ověřuje vlastnický PID; při změně vlastníka neodesílá zprávu. Jednu startup prodlevu toleruje, úspěch počítadlo resetuje a tři následná selhání zastaví scénář. Sedm CPU kontrol s nahrazenými API ověřilo tuto politiku i dosavadní vstup/fokus pojistky: `analysis/owned-window-health-cpu-validation.json`. Nejde ještě o ověření doplněné kontroly na živé hře. Watchdog nadále nezávisle zasahuje při zablokovaném driveru.

Osmisekundový source startup bez vstupu skončil0 pod20s limitem. Má24 pozorování okna: první dotaz překročil250ms, dalších23 odpovědělo. Všechny časové značky DWM vertical blank rostly; CPU bylo4,76–19,48% a volná fyzická paměť nejméně5736873984 bajtů. Čítače samy nejsou důkazem fyzického obrazu. První kratší pokus nesprávně kontroloval per-process čítače DWM, které se při každém spuštění čtecí sondy resetují; je zachovaný jako selhání testu.

Dva navazující pokusy o obnovu skončily před vstupem kvůli fokusu. Poslední přesně zaznamenal SetForegroundWindow=0, žádné SendInput se neprovedlo, oba herní procesy skončily0 a nezůstal živý vlastní proces. Asynchronní aktivace se nově potvrzuje omezeným WM_NULL; odmítnutí Windows se neobchází. Připravený je nanejvýš20s interval pro skutečnou aktivaci okna uživatelem, pouze na výslovnou volbu konkrétního hlídaného pokusu. Viz `analysis/monitored-probe-initial-observations.json`.

Uživatel následně uvedl, že běžné testy fungují a sekání začalo při testování achievementů. Příčina zamrznutí zůstává otevřená.

Dvě další sondy úspěšně obnovily farmu z původního profilu. Oba procesy hry i hlídač skončily s kódem 0. První obnovu doprovázelo odmítnuté čtení měnícího se stromu grafických prvků. Druhá sonda použila běžnou herní pauzu a přečetla 446 prvků včetně všech 60 aktivních polí. Časové značky DWM při obou obnovách postupovaly. Podrobnosti i dřívější selhání na odchylce kurzoru jsou v `analysis/monitored-source-restore-validation.json`.

Čtyřsekundové záznamy výstupu Windows pro konkrétní herní proces obsahovaly 279 950 nenulových vzorků u originálu a 345 868 u rekonstrukce. Běžely z různých herních okamžiků. Porovnání dekódovaných vzorků z totožných zvukových dat je doložené v samostatných testech Squallu. Oba výstupy procesu váže k jejich skutečným běhům `analysis/current-game-loopback-audio-validation.json`.
