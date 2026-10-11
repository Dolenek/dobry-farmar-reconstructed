# Nápověda ocenění — cAwardTip

`TooltipFrame` obnovuje skutečný 744bajtový cAwardTip. Moderní rozložení objektu není původní ABI. Vlastní MultilineText a čtyři nezávislé cAnimable definice podkladu; nejedná se o čtyři další vykreslované objekty. Definice drží reference textur a při výběru se kopíruje texture/atlas/frames/length/loop do Picture. Čas, PlayAnimation a DoubleAnimRender se při přiřazení zachovávají.

| Původní adresa | Obnovená funkce |
| --- | --- |
| 004c8d10 | Konstruktor, nulový offset/fade, vlastněný MultilineText a čtyři cAnimable |
| 004c8e00 / 004c8de0 | Destrukce definic před Picture/View a vlastněným textem |
| 004c8e80 | Text + Podlogka/Offset/čtyři obrázky, na konci alpha=0 |
| 004c95c0 | Výběr podle polovin obrazovky a umístění vedle bodu |
| 004c9a80 | Korekce mimo obrazovku, unscaled timer, fade a skrývání |
| 004d2080 / 004d2130 / 004d2160 / 004d22a0 | cAnimable defaulty, reference, přiřazení a načítání 2D definic |

Pořadí obrázků je UpLeft, UpRight, DownLeft, DownRight. Index je `(point.x <= width/2) + 2*(point.y <= height/2)` (každá podmínka tvoří samostatný booleovský člen). Souřadnice v levé/horní polovině vzniká jako halfSize + offset + point, opačná jako point − halfSize − offset. Původní funkce neprovádí obecné clampování. Při opuštění obrazovky step opět volá umístění s aktuální pozicí + offset; toto původní chování se zachovává.

Fade používá timer virtual +c, tedy unscaledElapsed, následně `elapsed * fadeDirection * 3.3f + alpha`. První porovnání s nulou používá x87 akumulátor před float spill, druhé porovnání s jedničkou jeho float hodnotu. Záporná hodnota skryje objekt a nastaví nulu; shora se omezuje na jedna. Alpha se nastaví podkladu i MultilineText. Step nevolá Picture step. Sloty +40/+44 jsou skutečně Control alpha (pointer +58), nikoli View transparency; ověřeno instrukcemi 004138f0/00413b50.

XML kořen se nenačítá jako Picture. Text se načítá nejdříve, Picture až ze sekce Podlogka, pak Offset a čtyři definice. Chybějící sekce ponechá dosavadní data. Konec XML resetuje pouze alpha podkladu, nikoli textu, fadeDirection nebo času. `placeBeside` mění resource reference jen při odlišném pointeru. Destruktor uvolní čtyři definice dříve než základní Control svou vybranou texturu a View vlastněný text.

`tools/validate_tooltip.py` porovnává původní funkce testovacím mapováním originálu se zakázaným entry pointem. Produkční kód originál nevolá. Původní XML UI/AwardTip.xml a všech 21 popisů z UI/Awards.xml, explicitní alternativní atlas/XML a prázdné sekce prošly v PC53/PC24: 7 320 stavů, 2 880 umístění, 720 XML reloadů, 177 116 písmen, 24 destrukcí a 96 celých GPU snímků / 46 080 000 přesně shodných RGB pixelů a draw volání. Reference textur se porovnávají při změnách, reloadech i destrukci. Závislosti XML/font/CRT používají dosavadní výslovné diagnostické adaptéry. Clock, cursor/input a rozměry obrazovky 800×600 jsou řízené; časovač je skutečná obnovená implementace včetně jiného multiplier.

Samostatný animovaný AwardEffect, AwardButton, Awards controller a navigace menu následují. Mesh/Animation 3D větve cAnimable jsou výslovně odmítnuté, dokud nebudou obnoveny jejich skutečné resource factories; nápověda ve všech původních datech používá 2D obrázky. Neověřené zůstávají malformed/nonfinite geometrie, obří unsigned rozměry obrazovky, parse exceptions a původní objektové/exception ABI. Tato kontrola nedokládá funkční herní EXE ani save kompatibilitu.
