# Nápověda v pauzovacím menu

`HelpView.h/.cpp` obnovuje `cHelpView` (constructor `0047fa60`, destructor `0047dde0`, singleton `0065f778`) a jeho skutečné stránky `cHelpPage` (`0047dbe0/0047dc80`). Stránky vlastní původní obrázky a MultilineText; indexový vektor slouží pouze k navigaci. XML `0047f370` načítá tři tlačítka a stránky, jejich obsah obnovuje samostatná funkce `0047ed50` bez volání základního View XML loaderu.

Next `0047e250` a Previous `0047e690` skrývají starou stránku, ukazují a povolují novou a ovládají hraniční tlačítka. Close `0047de80` spouští hideTransparently nápovědy a showTransparently skutečného GamePauseMenu. Konstruktor na `0047fc5d` skrývá tlačítko Previous; dekompilace bez příjemce volání tuto důležitou vazbu neukazuje.

Nový režim diagnostiky `help-view[-device]` dodává skutečnou nápovědu do skutečného GamePauseContext. Otevření, opakované přepínání obou stránek, zavření, plné XML znovunačtení a další navigace probíhají běžným myším vstupem a celým UI frame. Porovnává se také zbytek existujícího pauzovacího toku: Options, Awards, YesNo, Loading a MainMenu.

PC53/PC24 prošly dohromady 30 kontrol stavu nápovědy, 24 jejích operací a 376 úplných D3D8 snímků / 180 480 000 přesně shodných RGB pixelů. V těchto snímcích jsou zahrnuté i ostatní pauzovací obrazovky. Ověřené jsou vlastněné grafy stránek, lokalizované řádky, singleton a uvolnění textur. Znovunačtení přepíše indexový vektor, ale původní stránky zůstanou mezi vlastněnými potomky, stejně jako v originálu.

Spustit lze `python tools/validate_help_view.py`. Záznam používá hashe všech herních zdrojů, porovnávacího EXE a 17 nezměněných původních souborů. Starší režim `game-pause` nadále používá původní výslovnou hranici Help; nový režim ji nahrazuje skutečnou implementací.

Celý cGame zůstává neobnovený; okolní game View/discard, řízené hodiny/kurzor a vypnutý UI zvuk jsou dosavadní diagnostické adaptéry. Samostatné audio audity ověřují fyzické backendy. Dalšími závislostmi hlavního objektu jsou Journal a herní award manager; celý herní EXE, pracovní cykly na actual Farm, celé save soubory, zbytek Squall a izolovaný odinstalátor dále pokračují.

Úplné Win32 sestavení a 92/92 CTest prošly za 413.78 s. Oba aktuální game-audio a oba PersonSounds buildy prošly; striktní zápis ověřil source/binary hashe a 17 nezměněných originálů. Celou regresi eviduje `analysis/help-view-regression.json`.
