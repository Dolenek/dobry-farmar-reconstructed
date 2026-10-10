# Vstupní textové pole profilu

`EditBox` obnovuje cTextBox (původní 732bajtový objekt) nad TextLine. Konstruktor `005213c0` vypne lokalizaci, nastaví MaxLength=10, BlinkingPeriod=0.7, zapnutý stav blikání a nulový čas/kurzor/callbacky. Moderní objekt nemá původní binární layout. Zdroje nepoužívají originální kód.

`00521860` načte TextLine XML, float MaxLength a BlinkingPeriod a vytvoří vlastněný Letter pro podtržítko z fontu. Zachovává UV, zápornou výšku, hloubku parent+20, alpha, packed color a render flags. Advance kurzoru je šířka textury × glyph advance bez Multiplier. Kurzor není v poolu písmen; vlastní jej View. Kladné znaménko jeho výšky bylo opraveno podle instrukcí při native porovnání.

`00521500` zavolá confirm callback s měnitelným wchar_t ukazatelem. False skončí bez změny orientace a bez changed callbacku. Při přijetí resetuje rightInput; pro RightToLeftEnable a keyboard language 040d ponechá starou orientaci při Backspace a nastaví ji pro znaky >=00ff. Append zůstává na konci UTF-16 řetězce; orientace ovlivňuje kurzor a společný TextLine RTL display. Backspace ořízne poslední jednotku, Enter/LF nic nepřidají. Ostatní znaky vyžadují glyph, kromě ASCII mezery.

MaxLength omezuje šířku, nikoli počet znaků: cached text width + underscore advance + nový advance musí být **ostře menší** než limit. U glyphu nový advance neobsahuje Multiplier; mezera používá SpaceWidth × Multiplier. Cached width se při více input events před layoutem nepřepočítává. Changed callback se volá i pro Enter/LF, nedostupný glyph nebo dosažený limit. Neplatné/unordered numerické hodnoty se automaticky nenormalizují.

Enable `00521820` registruje character handler; disable `00521840` a destructor `00522c40` jej odstraní po TextLine onDisabled. `00521f00` přičte elapsed a porovná ještě nezaokrouhlený výsledek s periodou. Při překročení otočí stav, nastaví čas na nulu a kurzor ukáže/skryje; neponechá zbytek času. `00522320` po TextLine layoutu umístí kurzor podle alignmentu 0/1/2 a rightInput. Process `00522860` označí jeho transform jako dirty.

```powershell
cmake --build build/windows --config Release --target text-buttons-reference-test
python tools/validate_text_buttons.py
```

Oracle používá skutečný `UI/SelectUser.xml`/NameEdit. Ve dvou x87 kontextech porovnává 24 konstruktorů/destruktorů, 4 800 snímků a 43 016 úplných písmen/spritů. Vstup prochází rekonstruovanými binding sets. Pokrývá confirm reject/mutation, backspace, Enter/LF, Czech/Hebrew/NBSP/unavailable/NUL input, tři alignmenty, RTL off/on a český/hebrejský language fixture, saturaci šířky, disable/enable, callback pořadí, blink, caret geometry/UV/sprite, resource reference deltas a destructor keyboard cleanup. Původní instrukce constructor/load/input/blink/layout/process/destructor běží bez entry pointu.

Hranice: XML a numeric stream mají dříve evidované adaptéry, resource factories používají obnovené textury a původní font records, substring používá rekonstruovaný WString. GetKeyboardLayout je řízená IAT fixture; fyzická klávesnice/IME, původní výjimkové ABI a celý SelectProfile controller zatím nejsou ověřené. Hlavní menu nadále souhlasí v 96 snímcích / 46 080 000 RGB pixelech. Přenositelné výsledky: analysis/text-buttons-validation.json. ListBox, navigace a hlavní herní EXE zbývají.
