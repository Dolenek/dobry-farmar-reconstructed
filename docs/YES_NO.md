# Potvrzovací dialog

`YesNoDialog` obnovuje cYesNo (592 původních bajtů): konstruktor `00501b60`, XML `00500df0`, enable/disable `005014d0`/`00501530`, step `00501590`, destruktor `00501970`, click/input callbacky `00500650`/`00500a20`, callback settery `00501430`/`00501480` a výchozí discard `00501a20`. Moderní objekt nepřebírá původní virtuální/object/exception ABI.

Argument konstruktoru je obrazovka, kterou dialog dočasně vypne. Není to View parent: dialog je root a vlastní dvě TextAnimationButton a MultilineText. Konstruktor připojí callbacky a načte skutečné `UI/YesNo.xml` přes ResourceManager. Loader načítá Background do samotného Picture a YesButton/NoButton/Text do potomků; kořenový YesNo XML uzel samostatně neaplikuje jako View.

Enable nejprve volá View hook, vypne zadanou obrazovku a naváže Enter `0x1c`, Space `0x39` na yes button a Escape `1` na no button. Disable obrazovku znovu zapne a zruší tyto tři bindings. Obnovení původního enabled stavu neuchovává; originál zapne i obrazovku, která byla před otevřením vypnutá. Step navazuje na Picture step a znovu vypne obrazovku, pokud je enabled i parentEnabled. Destruktor volá vlastní disable hook jen při těchto dvou aktivních příznacích a potom nechává Picture/View odstranit vlastněné controls.

Callbacky dostávají samotný dialog nalezený přes parent kliknutého tlačítka. Výchozí yes i no zařadí dialog do discard queue s původním diagnostickým původem `.\Ui\YesNo.cpp45`; vlastní potvrzovací akce mohou callbacky přepsat. Výchozí callback neprovádí okamžité delete.

Přímé srovnání používá shipped XML a dvě GPU x87 přesnosti. `analysis/text-buttons-validation.json` obsahuje 34 vlastněných source/native dialogů, 2 400 celých stavů, 41 584 letter/sprite porovnání a 1 940 callbacků. Testuje nullable target, původně zapnutý/vypnutý target, opakované enable/disable, step enforcement, klávesy, oba button callbacky, text a resource/input cleanup. Výchozí duplicate discard requests mají shodný původ; source queue provede odložené frame deletion a odstraní bindings.

Dalších 64 skutečných diagnostických GPU snímků porovnává celou dialogovou scénu: background, obě tlačítka, vlastní texty a víceřádkový obsah. Všech 30 720 000 RGB pixelů i draw API volání odpovídá native scéně. Source/native hierarchie samostatně zpracují hover/press/release/leave, Enter/Space/Escape, RTL off/on, textové změny, alpha a přesun. Source využívá obnovený GameTimer, GameCursor, InputBindings, ViewRegistry, ControlRegistry a SpriteBatches. To ještě není rekonstrukce herního EXE.

Oracle provádí původní konstruktory, XML loader, callbacky, bindings, modal hooks a step. Native resource/XML factory má nezávislý recovered strom ze shipped souboru a řízený virtual release; source constructor má skutečný ResourceManager load. Default callback má explicitní diagnostic String decimal/concat a discard-service boundaries. Původní nonempty discard queue se zde nespouští. Celá scéna sdílí dříve popsané timer/cursor/input/graphics služby a evidenci zvukových požadavků; zvuk není slyšitelně přehráván. Controller scénáře běží v samostatných procesech kvůli paměti monotonního testového alokátoru.

Zbývá propojení do SelectProfile a hlavního menu, původní nonempty discard queue, old CRT origin formatter/úplné ABI, fyzický vstup a skutečné audio. Originální PE je pouze reference testu; produkční knihovny jej nepoužívají.

```powershell
cmake --build build/windows --config Release
python tools/validate_text_buttons.py
```
