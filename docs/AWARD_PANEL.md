# Herní panel ocenění — cAwardPanel

`AwardPanel` obnovuje původní 596bajtový Picture controller. Vlastní OK TextAnimationButton a dva MultilineText prvky; pouze si půjčuje AwardEffect, jehož PanelText/Congratulation kopíruje. Není singleton ani YesNo dialog. Moderní zdrojová třída nepředstírá původní C++ objektové ABI.

| Adresa | Obnovená operace |
| --- | --- |
| 004761c0 | Root Picture, effect a callback argumenty, tři owned controls, UI/AwardPanel.xml a owned XML deletion |
| 00475300 / 00475ae0 | Release posledního root Sound, potom Picture/owned destrukce |
| 00475b00 | AwardPicPosition, OkButton, Text, CongratulationText, effect labely, Sound; root Picture XML až nakonec |
| 00475700 | Picture onShown, nenulový root Sound přes SoundManager click |
| 004783d0 | Awards vytvoří tento panel, hide/showTransparently, přiřadí borrowed pendingEffect; druhý callback argument nepoužije |
| 00476ec0 | Awards vynutí owned=1 na konkrétním tlačítku bez zápisu profilové vlastnosti |

AwardPicPosition je skutečně pouze uložený Point2 na +244/+248. XML load ani onShown nepřesouvá effect. Slot +24c je borrowed effect, +250 poslední Sound. Root XML se načítá po textech a button XML. Nenulový Sound přepisuje starý pointer bez release; prázdný atribut jej zachová. Dtor uvolní jen aktuální referenci. Původní step je zděděný Picture 004f1810, nikoli nová pohybová či dialogová logika.

Dodané XML má sekci **Text0**, metoda však hledá **Text**. Všechna dodaná ocenění mají prázdný PanelText a nenulový Congratulation. Zdroj zachová tuto zvláštnost a neodvozuje chybějící popisy. Oracle nejprve porovná GPU snímky původního XML; potom explicitní variantou Text0→Text a přidáním skutečného congratulation textu do borrowed panel labelu prověří chybějící větev. Tato varianta se nepočítá jako původní grafika.

`tools/validate_award_panel.py`: PC53/PC24, 882 stavů, 42 panelů, 84 hromadných XML reloadů, 45 588 písmen a 168 GPU snímků / 80 640 000 přesně shodných RGB pixelů a draw volání. Skutečný hover/stisk/uvolnění OK vyvolá 42 shodných input callbacků. Původní kód běží pouze v oracle s vypnutým entry pointem. Resource/XML/font/mask a Button audio hranice zůstávají explicitními servisními adaptéry; root Sound používá reálnou obnovenou cache/cSound. SoundManager je disabled, takže tento test neověřuje audible playback.

`validate_awards.py` už používá skutečný Awards panel factory pro všech 21 indexů, callback potvrzení a skutečnou source discard queue/deletion. Původní UI factory běží v oracle, nepřesměrovává se na zdrojový panel. Porovnává show transition, pending effect, condition key commit, phase 4, disable, uvolnění owned panelu a markOwned helper. Původní mapová discard služba má diagnostický záznam; celý ViewRegistry deletion je ověřen samostatně.

Gameplay award manager a detekce podmínek ve světě, MainMenu navigace, zvuková reprodukce, fullscreen/device reset, jiné screen rozměry/RTL, malformed XML a exception ABI zbývají. Panel i propojení jsou sestavitelné; skutečný herní svět ani hlavní herní EXE tím ještě dokončené nejsou.
