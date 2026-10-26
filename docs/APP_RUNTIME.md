# Hlavní zdrojový proces

Target `DobryFarmar` sestavuje `build/windows/Release/Dobrý farmář.exe` z `main.cpp`, `GameApplication` a obnovených knihoven. Produkční program nevolá originální herní kód. Importované JNGLoad.dll a Squall.dll jsou vlastní zdrojové buildy; systémová d3d8.dll se načítá ze systémového adresáře.

## Sestavení a spuštění

Použijte stávající závislosti a Win32 konfiguraci popsanou v README:

```powershell
cmake -S . -B build/windows -A Win32 -DBUILD_SQUALL_CANDIDATE=ON
cmake --build build/windows --config Release --target DobryFarmar --parallel 4
& 'build/windows/Release/Dobrý farmář.exe' --data 'CESTA/K/PŮVODNÍM/Data' --common-dir '.local/moje-testovaci-profily'
```

Bez přepínačů používá relativní Data a společnou složku odvozenou z původních sys/settings.xml. `--common-dir` dovoluje izolovat profily. Před ResourceManager/Settings proběhne část FileManager constructor0054d720: základní app_directory, fallback window_title, SHGetFolderPathA(null,0x8023,null,0,out), vytvoření a mount common overlay. Šest zdrojových fixtures ověřuje názvy, odmítnutí neúplných nastavení, pořadí overlay a OS boundary; původní plný constructor a skutečný default ProgramData průchod dosud nejsou přímo porovnané.

## Ověřený skutečný běh

Opt-in `python tools/validate_application_runtime.py` vyžaduje volný Windows desktop a Pillow. Spustí pouze vlastní kopie nově sestaveného EXE/DLL v unikátní lokální složce a původní datové balíky čte na místě. Před každým globálním SendInput ověřuje foreground vlastního okna. Příliš malou plochu odmítne; při okně sahajícím mimo plochu posune pouze testované okno. Profily i nastavení jsou izolované.

Dva normální procesové cykly skutečně vytvořily SourceAudit, přešly z menu do farmy, vytvořily1981bajtový save, skončily s kódem0 a tentýž profil/save znovu načetly. Pauzovací menu, nápověda, návrat, pokračování, deník a návrat prošly skutečnou myší; kontrolují se změny obrazovek, přesná oblast pause menu a opětovné HUD pixely. Záznam obsahuje source/EXE/DLL/screenshot hashe a cesty skutečně načtených vlastních DLL. Všech17 původních souborů zůstalo shodných. Toto je source smoke, nikoli srovnání úplných procesů s originálem.

Dřívější pokus na800×600 měl neúplně dosažitelnou klientskou oblast a jeho zaslané kliknutí Help OK nezavřelo nápovědu. Původní lokální záznam je opraven na neúspěšný vizuální audit; nic se nezapočítává jako ověření. Nový skript kontroluje skutečný návrat a zaznamenává rozměry desktopu.

## Složení a zbývající věrnost

Runtime propojuje skutečné FileIndex/ResourceManager/Settings, okno, D3D8, buffery/render/shadery/textury/fonty, DirectInput, časovač, kurzor, Squall SoundManager/hudbu, lokalizaci, ProfileSession, Startup, MainMenu a celý obnovený cGame s engine-lived GamePauseContext. Device vzniká s původním software vertex processing0x20 / PC24. Present je před vykreslením nové scény, Clear používá původní flags7/depth0; při minimalizaci proběhne discard collector. Původní Logic destructor nemaže všechny globální Views; zachována je tato dosavadní vlastnická hranice.

Neověřené a nadále vypracovávané: úplný native frame/reset a lifecycle, Logic shortcut/debug overlays, GameInput cheat větve, náhlé device loss/změny režimů, počáteční původně neinicializovaný SoundManager pause byte, výjimečné cesty, úplný tutoriál a všechny herní systémy v porovnání procesů, obousměrné celé saves, všechny Squall97 účinky a izolovaná odinstalace. Zdrojový smoke není důkaz dokončeného projektu. Metadata: `analysis/application-runtime-validation.json`, `analysis/file-system-bootstrap-validation.json`.

Úplná regrese tohoto celku:99/99 CTest za464,42 s bez opakování; oba game-audio a PersonSounds buildy, aktuální Squall PCM/MDCT/runtime a oba DLL audity prošly. Striktní zápis ověřil aktuální source/binary hashe včetně skutečného GUI běhu a všech17 zachovaných originálů. Viz `analysis/application-regression.json`.
