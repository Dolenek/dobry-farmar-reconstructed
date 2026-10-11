# Obrazovka ocenění a tlačítka — cAwards / cAwardButton

`Awards` vlastní 21 tlačítek `AwardButton`, jejich popisy, CaptionText a ReturnButton. Každé tlačítko vlastní ověřený TooltipFrame s MultilineText a AwardEffect. Původní Awards alokace je 748 bajtů, MenuView začíná na +4; AwardButton má 648 bajtů. Moderní zdrojové objekty neslibují původní C++ objektové ABI.

| Původní adresa | Obnovená metoda |
| --- | --- |
| 004728c0 / 00472d50 / 00474da0 | AwardButton konstrukce/destrukce, vlastní tooltip a effect, poslední texture reference |
| 00474dc0 | Button XML, normal/disabled reference, AwardTip XML a popis |
| 00473150 / 00474910 | raw vlastnictví a změna appearance bez přepsání input state |
| 00474520 | skrytí dokončeného hover efektu před základním tickem |
| 00473d50 / 00474130 | enable/disable, appearance a tooltip fade |
| 00473520 / 00473940 | hover/unhover, tooltip a owned effect phase 6/8 |
| 004787e0 / 00478dc0 / 00478da0 | Awards singleton, konstrukce a owned destrukce |
| 004763a0 | MenuView/ReturnButton/CaptionText, indexované popisy před button XML |
| 004776b0 / 00477ab0 | refresh skutečných profilových booleans awardShow0..20 a onShown |
| 00476ae0 | discardTransparently před návratovým callbackem |
| 00477e80 | potvrzení ocenění: discard parent, condition boolean, phase 4, pending clear, disable |
| 0040ab50 | tabulka 21 původních názvů podmínek |

Setter vlastnictví ukládá původní byte (včetně nenulových hodnot jiných než 1) a volá appearance callback. Nemění input state. Disabled/hovered/pressed appearance ruší flag 0x20 a vybírá disabled texture; normal jej přidá a vybere normal texture. Nenulová stará textura je released a výsledná retained; nulový replacement zachová původní pointer stejně jako originál. Kliknutí/unclicked AwardButton mají původní prázdnou metodu 004bd820. Dědění input bindingů a hover metod zůstává skutečné.

XML reload původně nepouští dřívější normal/disabled texture reference ani přepsané button Sounds. Source zachovává tento postup. Diagnostika potvrdí shodné resource delta a retained sound reference; jejich následné explicitní uvolnění je jen úklid oracle. Source Effect používá skutečný disabled SoundManager, native Effect má diagnostický sound resource. Root UI/Awards.xml obsahuje **Sound="Sounds/Award.wav"**; dřívější popis jeho nepřítomnosti byl nesprávný a je opravený v AWARD_EFFECT.md.

Původní singleton se inicializuje před MenuView a ruší až po jeho owned children. Source použije stejnou posloupnost základních tříd. Callback returnEvent originál při konstrukci neinicializuje; skutečný MainMenu/award manager jej přiřazuje později. Source jej vyjadřuje bezpečně prázdnou std::function; původní neinitializovaný slot není vydáván za shodné ABI.

Potvrzení herního panelu potřebuje pouze button->parent, nikoli metodu zvláštního panelu. Nativní 00477290 ukládá View do obecné discard mapy; 00432b20 je mapové vyhledání/vložení, nikoli AwardPanel metoda. Callback zapíše odpovídající **condition key**, který není awardShow. Tabulka přeskočí označení Award18: index 18 má Award19MaxMarketAssortiment, 19 Award20Megaplanning, 20 Award21CollectAllAwards. Pending effect musí existovat, protože originál jej dereferencuje před pozdějším null guardem. Potom přejde na phase 4, pointer je nulový a Awards disabled.

`tools/validate_awards.py` porovnává PC53/PC24: 16 632 stavů, šest celých obrazovek, šest reloadů, 574 992 písmen, šest návratových callbacků, 126 potvrzení a 156 GPU snímků / 74 880 000 přesně shodných RGB pixelů i draw volání. Native profile getter/decimal formatting jsou explicitní servisní adaptéry, source používá skutečnou obnovenou typed profile mapu. Original PE entry point je vypnutý a žádný původní kód není součástí implementace. CRT atexit inicializace key table je zaznamenána, shutdown CRT není spuštěný.

AwardPanel a jeho Awards factory/markOwned helper jsou nyní obnovené a porovnané v AWARD_PANEL.md. Condition detection a award manager v herním světě ještě nejsou propojené. MainMenu navigation, audible audio, úplný DirectInput/device lifecycle, jiné screen rozměry/RTL, malformed XML, chybějící pending effect a C++ exception ABI zbývají. Výsledek ověřuje propojený UI strom; hlavní herní EXE a hra tím ještě dokončené nejsou.
