# cLogic zkratky a odložený grafický reset

`LogicShortcuts` obnovuje binding/callback část původního cLogic005315b0/00531a70. Constructor registruje Alt-down0x138/0x1b8, Alt-up0x38/0xb8 a PrintScreen release0xb7; při Alt se přidá šest release akcí. Uvolnění kteréhokoliv Alt je opět odstraní. Snapshot pravidla původního Input platí i při několika událostech v jednom snímku. Destructor neodstraňuje šest dočasných bindings, pokud Alt zůstal stisknutý; běžné ukončení runtime zastaví dispatch před destrukcí.

| Klávesy | Původní tělo | Účinek |
| --- | --- | --- |
| Alt+Enter |00530700| Obrátí graphics.fullscreen a nastaví požadavek resetu; reset se neprovádí uvnitř callbacku. |
| Alt+T |00531010| Přepíná wireframe a samostatný příznak potlačení textur. Není to show_triangles pro debug text. |
| Alt+Home |0052fa00| Nezávislý procesový přepínač násobku času0.1/1. |
| Alt+Pause |0052f9a0| Procesový přepínač Logic pause; nenahrazuje samostatné OS sound pause. |
| Alt+Insert |0052fa40| Další nezávislý přepínač násobku15/1. |
| Alt+F5 |0052fa80| Skutečný ResourceManager.refresh, pouze texture resources typu0. |
| PrintScreen |00530c50 →00512c40| Cache GetBackBuffer(0,MONO), příprava nepoužitého data/time.dds názvu; úplné původní tělo nezapisuje obrázek. |

Čtyři procesové bytes0065d368..0065d36b začínají1 a přežívají životnosti cLogic. Porovnání vykonává skutečné původní constructor/destructor/callbacky mimo preferovanou adresu PE: šest životních cyklů,3661 stavů,3654 dispatch snímků,12930 událostí a2477 přesně stejných service událostí. Input/Settings/Timer/Graphics/Resource jsou explicitní diagnostické hranice; nativní pause setter se vykonává. Zdrojový screenshot cache navíc projde4 backend calls a2 release včetně opakování neúspěšného získání. Metadata: `analysis/logic-shortcuts-validation.json`.

## Hlavní runtime

Hlavní EXE skutečně používá tyto služby. Reset request00519ed0 se zpracuje v renderovacím průchodu. Obnovené pořadí0051ce50 uvolní border/screenshot/vertex/index handles před pre-reset resource callbacky; příprava a Device.Reset se při chybě opakují se Sleep100. Potom následuje render setup, restore resources, buffery a ShowWindow(SW_RESTORE)/UpdateWindow/SetActiveWindow. První scéna nemá předchozí Present, další snímky jej mají před novým vykreslením.

Přímý procesový test `python tools/validate_runtime_shortcuts.py` spouští celý originál a vlastní EXE se stejnými daty/profilovým jménem. Fyzicky provede Alt+T, fullscreen a návrat, další přepínače, texture refresh a obyčejný fullscreen se zapnutými texturami; oba procesy normálně končí0. Fullscreen1920×1080 a následný windowed800×600 mají dohromady2553600 přesně shodných RGB pixelů. Tato konkrétní dvojice zachovává původní zvláštnost: Device.Reset vrátí solid fill, ale texture suppression zůstane, takže obraz je do dalšího Alt+T bílými neotexturovanými plochami. Původně chybějící předání tohoto příznaku do už pixelově ověřeného drawSpriteBatches bylo opravené.

Ostatní živé snímky mají nezávislý čas animací a nejsou vydávané za pixelově shodné. Reálné běhy používají skutečné původní DLL pouze v referenci; source proces používá vlastní DLL a test ověřuje cesty i hashe. Metadata: `analysis/runtime-shortcut-gui-validation.json`.

## DPI a hranice

Windows při starém D3D8 fullscreen může změnit DPI awareness procesu; kompatibilitní stav původního EXE byl v opakování jiný než u nového zdrojového aliasu. Při125% škálování proto nezávislý unaware Python hlásil640×480 pro fyzických800×600. GameGui před souřadnicovými operacemi převezme aktuální DPI kontext vlastního testovaného okna. Rozměry i awareness se zaznamenávají; produkční hra tím nemění svůj DPI režim.

Celá Logic process větev/debug overlays, GameInput cheats, device-loss/failure/reset varianty, všechny OS lifetime sekvence, úplné herní systémy/rozvinuté saves, zbytek Squall97 a izolovaná odinstalace dále pokračují. Scalar service test není náhradou úplného runtime; zvlášť se evidují nedokončené oblasti.

## Regrese této etapy

Úplné Win32 Release sestavení prošlo;100/100 CTest za474.15s. Obě aktuální audio sestavení, oba PersonSounds audity, Squall runtime a obě loaded-DLL kontroly prošly. Celý zdrojový proces znovu vytvořil profil, obnovil farmu, pauzu, Help a Journal; následovaly úvodní i bohatší obousměrné saves mezi skutečnými procesy. Striktní kontrola aktuálních source/binary/report hashů a všech17 originálů prošla. Souhrn: `analysis/logic-shortcuts-regression.json`.

Fyzický test nepovažuje samotnou změnu stylu okna za hotový Present: uchovává všechny přechodové snímky, u bílé resetové dvojice vyžaduje tři po sobě jdoucí přesné shody, u běžného režimu skutečně otexturovaný obraz v omezeném čase. Dřívější pevná prodleva1s zachytila černý snímek originálu a při dalším běhu výřez předchozího fullscreen snímku zdrojové hry. Oba neúspěšné běhy zůstaly lokálně zachované; finální ověření neměnilo produkční kód ani toleranci pixelového porovnání. To nenahrazuje řízené časování ani vyčerpávající device-loss/lifecycle audit.
