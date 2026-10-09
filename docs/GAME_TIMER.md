# Herní časovač

`GameTimer` obnovuje chování `cTimer`: konstruktor `00536ae0`, procesor `00536720` → `00536550`, inicializace `00536690`, ukončení `005366e0` a destruktor `00536430`. Výchozí backend používá skutečné WinMM timeGetTime/timeBeginPeriod(1)/timeEndPeriod(1) a Sleep. Sestavení linkuje winmm; implementace nepoužívá původní EXE. Moderní objekt nemá původní 64bajtové rozložení, vtable a tři interní ukazatele metod.

Konstruktor načte Time/MinFps a Time/MaxFps, výchozí hodnoty 15 a 200. Při chybějícím MaxFps původní kód zapisuje **MinFps=200**; toto chování je zachované. Float čtení `0053e3d0` a zápis `0053e8d0` v Settings používají moderní široké streamy. Původní CRT extrakce/formátování ještě nejsou přímo porovnané.

Rozdíl času používá uint32 přetečení. Původní podmínka čekání porovnává rozdíl v milisekundách s 1/MaxFps v sekundách; jednou zavolá Sleep(1) a znovu vzorkuje. Převod na sekundy používá float konstantu 0.001. Celkový čas a sdílený součet používají neuložený x87 výsledek, zatímco aktuální snímek se uloží, omezí pomocí MinFps a násobí multiplierem. Pořadí zaokrouhlení je zachované.

Sdílené akumulátory `00660608/0066060c` přežívají zničení i opětovné vytvoření časovače. Každých deset volání se průměr počítá **před** vzorkováním aktuálního snímku. FPS používá neuložený podíl; průměr se následně uloží, omezí a vynásobí. Getter elapsed `00536480` vrací kladný průměr, jinak aktuální snímek; `005364a0` dělí aktuální snímek multiplierem. Další gettery/settery jsou `005364b0` až `00536520`.

```powershell
cmake --build build/windows --config Release
python tools/validate_timer.py
```

Přímý oracle vykonává původní konstruktor, procesor, gettery, setter MinFps a destruktor v soukromě mapované referenci bez entry pointu. 200 konstruktorů a destruktorů, 120 000 snímků a 120 200 stavových porovnání souhlasí bitově v běžné i původní D3D x87 přesnosti. Zahrnuje uint32 přetečení, nulové/záporné/zlomkové limity, měnící se multiplier, sdílené průměrování a 30 358 volání Sleep. Clock/Sleep/WinMM jsou řízené testové hranice; Settings a přiřazení String mají doložené adaptéry. Přenositelný report: analysis/timer-validation.json.

Celá diagnostická scéna menu používá tento časovač s řízenými milisekundami a jeho vyhlazený elapsed pro source view/text/přechody. Původní frame dostává tutéž hodnotu přes getter, zatímco jeho timer advance zůstává testovou hranicí. Všech 72 snímků a 34 560 000 RGB pixelů souhlasí; hide/show se dokončí i při vyhlazování. Test neprokazuje fyzické časování originálního herního procesu.

Zbývá ověřit skutečný WinMM backend za běhu celé hry, původní exception ABI a duplicitní singleton větev, úklid při výjimce v konstruktoru a původní CRT konverze Settings. Časovač ani pixelově shodná scéna menu nejsou dokončeným herním EXE.
