# Víceřádkový text a textové dekorace menu

`MultilineText` obnovuje cMultilineText (původních 636 bajtů): konstruktor `004f5e70`, XML `004f8580`, setString `004f8ce0`, step `004f8e10`, tvorbu řádků `004f7e10`, klon řádku `004f7250`, zalamování `004f7700`, alpha `004f6a80` a destruktor `004f6130`. Moderní objekt netvrdí shodu původního object/exception ABI. View vlastní skrytý TextLine vzor i vytvořené řádky; seznam řádků pouze eviduje jejich pořadí.

XML načítá View a TextLine vzor ze stejného uzlu. Vzor se skryje, vyprázdní a přepne na nelokalizovaný text. Vnitřní sekce Text, pokud existuje, má přednost před atributem String, včetně prázdné sekce. Literál `/n` se převádí na LF. Interlinear má konstruktorový default 40, Localized true, WrapText false a VerticalAlign 3; XML bez VerticalAlign znovu nastavuje 3. Color se odvozuje z packed barvy vzoru.

Změněný text se přestaví okamžitě nebo označí dirty; nezměněný text nepřestavuje. Přepnutí localized/wrap/alignment samo dirty nenastavuje. Přestavba odstraní všechny staré řádky, aktualizuje velikost vzoru, případně lokalizuje celý text a rozdělí ho na LF. Prázdné segmenty před LF vytvoří řádek, poslední prázdný segment za LF už ne. Každý řádek je skutečná TextLine kopie se současným Settings RTL, vlastními písmeny a referencemi, pozicí `y + index*Interlinear`, zděděnou alpha a barvou.

Zalamování hledá mezery od konce, zkracuje předchozí řádek a kontroluje skutečnou TextLine šířku. Přesunutá část ponechá úvodní mezeru. Slovo bez mezery může překročit šířku. Nalezení mezery na pozici nula vytváří další řádek z dosavadní přesouvané části a končí, i když předchozí řádek přesahuje. Tyto původní zvláštnosti se nenahrazují obvyklým word-wrap algoritmem. Vertikální režimy 0/1/2 vycentrují nebo ukotví blok pomocí počtu řádků a Interlinear; režim 3 neposouvá blok. Mezivýpočty a float spills odpovídají instrukcím obou GPU x87 přesností.

`MenuView::loadXML` nyní přímo vytváří MultilineText pro BackMultitext. BackText používá TextLine. Opakované načtení stále přidává další dekorace a View potomky, jak to dělá originál.

`analysis/text-buttons-validation.json` eviduje 32 vlastněných native/source stromů, 4 768 úplných stavů a 86 784 porovnání písmen/spritů. Pokrytí: shipped `UI/YesNo.xml` Text, obě přesnosti, RTL a lokalizace off/on, čtyři vertikální režimy, LF a `/n`, prázdné segmenty, dlouhá slova, opakované mezery, okamžitá/odložená aktualizace, alpha/pozice a reference deltas při destrukci. Dalších 36 porovnání dekorací ověřuje BackText/BackMultitext, opakovaná XML načtení, barvu a prioritu neprázdné Text sekce.

Native oracle používá původní instrukce konstruktorů, klonování, přestavby, zalamování i virtuálního getteru šířky `004db210`. String/CRT operace, XML tree/typed access a GPU/resource služby jsou explicitní testové hranice. Zvětšený oracle vyčerpal dosavadní testovou monotónní paměť 256 MiB; rezerva 512 MiB zachovává pořadí adres pro depth ties, aniž mění produkční alokaci. Přepočet adres getteru šířky byl doplněn pro referenční EXE načtené mimo původní base. Původní entry point se nespouští; produkční implementace nepoužívá původní PE.

Neověřené: neplatné/záporné rozměry a NaN/infinite spacing, plné původní ABI, skutečný YesNo/SelectProfile controller a navigace, fyzický vstup a slyšitelný zvuk. Celá hra stále není rekonstruovaná ani sestavitelná.

```powershell
cmake --build build/windows --config Release
python tools/validate_text_buttons.py
```
