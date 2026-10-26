# Vytvoření prvního profilu

`src/game/ui/NewUser.{h,cpp}` obnovuje první obrazovku před hlavním menu. Jde o původní 76bajtový objekt `00506780`, singleton `006604f4`, odlišný od pozdějšího SelectProfile. `MainMenuContext.ensureFixedBack()` nyní zpřístupňuje skutečnou společnou továrnu `00481990`, která se volá už před prvním profilem.

Původní MenuView základ začíná na +4; OK je raw+3c, Edit +40, nepřítomný backing +44 a completion +48. Konstrukce vytváří tlačítko, Edit a načítá `UI/NewUser.xml`; loader `00505bf0` přidává Texts mezi vlastněné dekorace. XML reload staré popisky ponechá a přidá další. Destructor `00506920` odpojí Enter, zničí MenuView a teprve pak uvolní singleton. Moderní lifetime základ zachovává toto pořadí.

`005053a0` řídí enabled stav OK podle prázdného jména, `005066f0` jeho viditelnost. Filtr `00504fc0` vždy povolí Backspace, jinak kontroluje skutečný font přes Edit. Dodaný font odmítá i mezery; požadavek „First Farm“ se proto shodně stane „FirstFarm“. Akceptace `005062d0` otevře profil, požádá o průhledné odstranění obrazovky a spustí completion. Test používá skutečný nativní `0048a680` a obnovené MainMenu, včetně opětovného použití menu.

## Porovnání

`python tools/validate_new_user.py` spouští diagnostiku v PC53 i PC24. Celkem porovnává čtyři životní cykly, 88 stavů, 28 vstupních akcí a 56 celých GPU snímků, tedy 26 880 000 shodných RGB pixelů. Zahrnuje odmítnutí prázdného Enter, psaní/mazání, Unicode a filtr, změny tlačítka i caret, XML reload se šesti popisky, Enter i skutečný mouse click, oba profily, přechody, frontu odstranění a úklid singletonů/vstupu. Je součástí CTest jako `original-new-user`.

Obě strany při opakovaném XML načtení ponechávají staré click/hover zvuky s jednou referencí. Diagnostika ověří všech šestnáct ponechaných referencí a teprve poté je uklidí. Produkční Button loader zůstává shodný s ověřeným původním chováním.

## Hranice

Nativní Profile konstrukci zde nahrazuje již evidovaný explicitní diagnostický adaptér, zatímco zdrojová strana používá skutečnou izolovanou ProfileSession a její persistenci. Nativní XML/font/texture/CRT a zvukové hranice používají existující testovací služby. Zvukové požadavky jsou porovnané, fyzický playback má samostatné audity. Původní EXE běží pouze jako mapovaná reference bez entry pointu; implementace jej nevolá.

Celé původní spouštění s logy, kompletní engine a hlavní interaktivní EXE zatím chybějí. Tento test nedokládá celý nativní cGame ani obousměrné save soubory. Neověřená je také nativní exception ABI. Další práce pokračuje přes tři původní JPEG loga ke skutečné aplikaci, nikoli přes vytvoření náhradního profilu.

Úplné Win32 sestavení a96/96 CTest prošly za 492.92 s. Oba game-audio a oba PersonSounds buildy prošly; striktní zápis ověřil zdrojové/binární hashe a17 zachovaných originálů. Regrese: `analysis/new-user-regression.json`.

První fyzický PersonSounds běh v samostatném sestavení selhal na nepohybujícím se kurzoru původní Squall.dll v PC53 (limit2000ms). Opakování stejných binárek i podmínek prošlo všemi40 kontrolami postupu a8 pauzami. První selhání i opakování jsou výslovně evidované v regresním reportu; nejde o důkaz odstranění obecné nedeterminističnosti fyzického backendu.
