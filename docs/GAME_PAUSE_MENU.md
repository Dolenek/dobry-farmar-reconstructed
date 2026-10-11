# Menu pro pauzu hry

`src/game/ui/GamePauseMenu.*` obnovuje původní cMenu (singleton 0065ffd8, 80 bajtů, MenuView na +4). GameMenu jej vlastní na +0xa0. Jde o menu z `UI/Menu.xml`, nikoli journal. Kontext musí přežít menu i dokončení Loading; vyžaduje skutečnou operaci uložení a zrušení cGame, reset grafiky a Help View.

| Operace | Původní adresa |
| --- | --- |
| Konstruktor / destruktor | 00487330 / 00487980 |
| XML | 00485680 |
| Enabled / disabled hook | 00485d90 / 00486170 |
| Pokračovat, Escape a Enter | 00485560 |
| Options | 00485590 |
| Awards / návrat | 00486550 / 00485660 |
| Help | 00485630 |
| Potvrzení návratu | 00487210 |
| Yes / No | 00486d90 / 004870a0 |
| Loading completion → skutečný MainMenu | 00489e80 |
| Povinná operace save/discard cGame | 00486970 → 00432b20 |

Konstruktor vytvoří Continue, Awards, Options, MainMenu a Help v původním alokačním pořadí, načte XML a teprve potom přiřadí callbacks. Enabled/disabled hook registruje/odregistruje Escape a Enter s nulovým context pointerem. Pokračování obnoví **cGame** (0065f6ec) enable a absoluteEnable před hideTransparently menu. Kontraktové dialogy naopak pozastavují GameMenu 0065f740; oba objekty jsou odlišené.

Options se při existujícím singletonu znovu použije, nastaví return view a zobrazí. Awards při existujícím singletonu akci odmítne; nová obrazovka skryje pause i cGame, návrat obnoví jejich viditelnost a zachová pozastavenou hru. Help skryje pause a provede showTransparently/enable jeho povinného View.

Návrat vytváří skutečný YesNoDialog s původním ruským raw textem, který dále zpracovává StringManager. No zařadí dialog ke zrušení se source origin `Menu.cpp194` a obnoví pause. Yes zařadí dialog s `Menu.cpp181`, **skryje celý cGame**, požádá o jeho uložení/zrušení s `Menu.cpp185` a spustí skutečný LoadingFromLog nad `menureslog.xml`, batch 3, clear flag true. Dokončení vytvoří skutečný MainMenu a jeho FixedBack. Původní instrukce 00486ed1 upřesnila příjemce hide oproti neúplné dekompilaci; zdroj byl opraven před ověřením.

## Porovnání

`original-game-pause` / `tools/validate_game_pause.py`: PC53/PC24, 46 kontrolovaných stavů, 28 navigačních akcí, skutečné input releases a Escape/Enter. Options včetně opakovaného otevření a návratu, Awards včetně guardu a návratu, Help View, obě větve YesNo a skutečný Loading/MainMenu běží propojeně. Loading má 96 porovnaných kroků; jeho completion ani MainMenu constructor nejsou nahrazené adaptérem.

264 úplných D3D8 snímků, draw submissions/API a 126 720 000 přesně shodných RGB pixelů. Čtyři XML reloady ověřují chybějící child sekce i původní definici. Původní XML přepisuje zvukové pointery bez uvolnění: 40 ponechaných source/native referencí je ověřených a pak uvolněných pouze diagnostickým cleanupem. Destrukce porovnává texture reference a singleton.

## Zbývající vazby

Diagnostika mapuje originál bez entry pointu; implementace jeho kód nevolá. XML, staré CRT, resources a native profilové služby mají explicitní adaptéry. Zdrojový ProfileSession používá skutečné izolované soubory. Game a Help v tomto testu představují obnovené View. Operace 00486970 je trace hranice: skutečné uložení a zrušení celé scény tím ověřené není. Zvuky controls používají typed diagnostické resources; skutečný disabled SoundManager/MusicPlayback obsluhuje hudbu Loading. Slyšitelný výstup zůstává neověřený.

Pokračují plný GameMenu HUD, Help, Farm, tutorial, cGame, hlavní EXE, celá Squall a obousměrné saves. Neověřené jsou fyzická vstupní zařízení, exception/allocation a neplatné XML cesty, porušené profilové/globální invarianty a zánik kontextu před completion.
