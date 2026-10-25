# Herní deník a zprávy HUD

`Journal.h/.cpp` obnovuje `cJournal` (156 původních bajtů, singleton `0065f764`). Constructor `00446aa0` vytvoří skutečná tlačítka Up/Down/OK a MultilineText prototyp; XML `00445880` čte původní obdélník, vzdálenost a barvy. Destructor `00446d10` uvolní singleton před View vlastnictvím potomků. enabledStep je původní skok `00445580 → 004bd820`, proto třída používá skutečný zděděný prázdný View krok.

Otevření `00446d50` pozastaví normální i absolutní HUD a sestaví zprávy od poslední dozadu (`00446740`). Posun Up/Down (`00446a60/00446a80`) používá první index ±1 a dopředné rozvržení `00446420`. Oba směry zachovávají clamp indexů, přijetí první zprávy i při přeplnění, přesné x87 spill body a různé pořadí součtů při umisťování. Zavření `004457e0` znovu povolí HUD a `GameMenu::setMessagesRead` (`00445780`) označí všechny zprávy za přečtené a vrátí tlačítku deníku normalColor.

Nutné metody `MultilineText` jsou obnovené současně: copy constructor `004f6000` má vlastní kopii prototypu, ale nový prázdný text a seznam řádků; `measuredSize` `004f5db0` měří původní maximální šířku a výšku prvního řádku plus interlinear; `setPackedColor` `004f5cb0` mění pouze uloženou barvu. Deník ji stejně jako originál nastavuje až po okamžitém vytvoření řádků. Toto pořadí se nepřepisuje na nové vizuální chování.

## Porovnání

`python tools/validate_journal.py` v PC53/PC24 porovnává 430 stavů / 102 operací a 362 úplných D3D8 snímků / 173 760 000 přesně shodných RGB pixelů. Skutečné tlačítko HUD otevře skutečný Journal, myš ovládá posun i zavření. Pět sad obsahuje 1, 2, 6, 12 a 28 skutečných HUD zpráv, různé délky, zalomení a read příznaky. Oba směry jsou porovnané také pro záporné, prostřední, poslední a příliš velké indexy. Plný XML reload doplňuje další otevření/zavření myší.

Kontrolují se prototypy a čerstvé kopie textu, rozměry, řádky, profilové mapy, pořadí HUD služeb a texture reference při rušení. Test nyní vykonává skutečné původní `TextLine::textHeight` přes virtuální volání. Jeho dříve nenavštívený odkaz na cookie na `004db603` vyžadoval diagnostickou relokaci; bezpečnostní kontrola zůstala zapnutá. Původní kód běží pouze jako reference v testu, nikdy jako produkční implementace.

Pro skutečnou konstrukci cGame se také závislost `GamePauseContext.help` změnila na povinný odložený provider: původní `00413210` vytváří pauzovací menu dříve než HelpView. Provider při použití vrací skutečnou později vytvořenou nápovědu. Není vložený žádný náhradní View. Regrese zahrnuje starší Pause i skutečnou Help navigaci.

Celý cGame stále chybí. Ostatní HUD scene/stock/gameplay/tutorial služby, řízený timer/kurzor, profilové adaptéry, minimální okolní StateViews a vypnutý UI zvuk mají dosavadní výslovný diagnostický rozsah. Undefined otevření prázdného seznamu a výjimečné ABI nejsou tímto validním scénářem ověřené. Pokračují herní award manager, úplná factory a world vazby cGame, hlavní EXE, celé pracovní cykly na Farm, obousměrné saves, zbývající Squall a izolovaný odinstalátor.

Úplné Win32 sestavení a 93/93 CTest prošly za 418.62 s. Oba aktuální game-audio a oba PersonSounds buildy prošly; striktní zápis ověřil source/binary hashe a 17 nezměněných originálů. Celou regresi eviduje `analysis/journal-regression.json`.
