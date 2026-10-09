# Souřadnice myši a propojení menu

`GameCursor` obnovuje kurzorovou část cLogic: `00530290` a getter `00531a40`. Graphics window bounds `00512330` volají GetWindowInfo a vracejí rcClient v okně, rcWindow ve fullscreen. Transformace `0050d260` ve fullscreen nejprve kopíruje obě souřadnice, odečte offsety a poté dělí scale. Každý mezivýsledek se uloží do float. V okně se jen kopíruje. Souřadnice se neomezují na hranice obrazovky.

Aktualizace nejprve získá bounds do lokálních čtyř nul, potom GetCursorPos do sdíleného POINT. Při úspěchu odečte levý horní roh, uloží do float a zavolá transformaci s totožným vstupem/výstupem. Při neúspěšném GetCursorPos zachová dřívější herní pozici. Při neúspěšném GetWindowInfo bounds zůstanou nulové. Sdílená WINDOWINFO a POINT odpovídají `00660570` / `006605f4`. WINDOWINFO je původně v nulové .bss; rutina sama cbSize nenastavuje. Skutečné Win32 API na zdejším Windows přijalo i tuto nulovou inicializaci.

```powershell
cmake --build build/windows --config Release
python tools/validate_cursor.py
python tools/validate_text_buttons.py
```

Cursor oracle spouští původní čtyři metody v soukromé kopii reference na jiné adrese bez entry pointu. Ve dvou přesnostech x87 prošlo 40 000 frame aktualizací, 40 000 bounds a 80 000 samostatných/aliasovaných transformací. Kontroluje celé signed LONG vstupy, záporné/nulové/zlomkové scale, offsety, selhání obou API, sdílené struktury i přesné vstupy a pořadí API volání. Dále vyzkouší skutečný Win32 backend ve dvou vlastních skrytých oknech. Kurzor uživatele nepřesouvá a režim monitoru nemění. Report: analysis/cursor-validation.json.

Diagnostika celého menu nyní používá source GameCursor a původní `00530290` s původními bounds/transform metodami. GetWindowInfo pracuje nad skutečným diagnostickým oknem; obrazovkové vzorky kurzoru jsou řízené. Šest tlačítek se ovládá přes InputBindings (najetí, stisk, uvolnění, odjetí), bez ručního přiřazení ButtonState. Porovnává výsledné stavy a pořadí 24 sound požadavků. Včetně průhledného hide/show prošlo 96 snímků, 46 080 000 přesně shodných RGB pixelů a draw API volání.

Hranice menu testu: native input process používá řízené události a obnovený InputBindings; fyzický DirectInput ještě není součástí tohoto porovnání. Sound metody pouze zaznamenávají a porovnávají názvy zdrojů, argumenty a pořadí; zvuk nepřehrávají. MainMenu controller/action callbacky stále nejsou zapojené, takže release nezpůsobí přechod do další obrazovky. Timer advance a graphics begin/finish zůstávají popsanými testovými hranicemi. Úplné cLogic, shortcuty, objekty/exception ABI, skutečná navigace a celý herní běh zbývají.
