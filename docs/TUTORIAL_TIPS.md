# Okna tutoriálu

`src/game/ui/TutorialTips.cpp` obnovuje pět původních typů oken. Používají skutečné XML `Tutorial/CommonHelpTip.xml`, `ArrowHelpTip.xml` a `ActionTip.xml`, MultilineText, Picture a TextAnimationButton. Zdroje nevolají původní programový kód.

| Adresy | Typ / operace |
| --- | --- |
| `004cdde0`, `004cd400`, `004cc780`, `004cc7c0`, `004cda00` | CommonTip: vlastnictví, XML, umístění, start a potvrzení OK |
| `004cc150`, `004cb8f0`, `004cb4d0`, `004cb7e0`, `004cbd60` | ArrowTip: kotva, orientace šipky, rychlost a fáze |
| `004cc700`, `004cc290`, `004cc320` | Tip s dočasně zakázanou kotvou a obnovením její dostupnosti |
| `004ca6e0`, `004ca7d0`, `004ca690` | ActionTip: zastavení gameplay, podmínka a cancellation |
| `004cac90`, `004caf70`, `004cac10` | RepeatedActionTip: původní dvojí vyhodnocení podmínky a dokončení |
| `004ccc40`, `004cd020`, `004f6630`, `004f6e80` | Multiplier/interlinear, hodnota −1 vrací XML default |
| `00413bf0`, `00413c10` | Převrácení X/Y skutečné geometrie controlu |

ArrowTip nejprve zkonstruuje CommonTip a poté vytvoří druhou sadu ovládacích prvků. První image/text/button zůstanou vlastněné ve stromu View. Zdroj zachovává toto původní chování a sedm controlů. Kotvy napravo od x=400 obracejí X, kotvy pod y=300 obracejí směr offsetu Y. Umístění se počítá podle přesného pořadí původních float spillů; při přítomné kotvě se souřadnice šipky převádějí přes původní signed 64bit truncation a spodních 32 bitů. Fáze se při startu neresetuje.

ActionTip volá predicate před kontrolou cancellation. Pokud predicate uspěje, cancel flag se nevyčistí. RepeatedActionTip volá základní krok a poté predicate ještě jednou; může tedy dokončení oznámit dvakrát. Tuto zvláštnost nelze odstranit jako zdánlivou chybu, protože mění následný stav skutečného TutorialManager/profilu.

`python tools/validate_tutorial_tips.py` porovnává PC53 i PC24: 3 474 stavů, 165 720 písmen, 500 startů a 2 600 kroků. Každý typ používá všech 50 skutečných textů z `Tutorial/Tutorial.xml`. Porovnávají se kompletní controly, texty, letter sprity, pozice, orientace, arrow origin/velocity/phase, vlastnictví, callbacky a skutečný pause flag GameMenu. 64 GPU snímků a 30 720 000 RGB pixelů zahrnuje viditelná okna a hover/press/release kliknutí na OK.

Diagnostika vykonává původní instrukce pouze v testovacím programu. XML, staré CRT a resource služby mají explicitní adaptéry. `TutorialTipHost` je povinné rozhraní skutečného správce; test eviduje finish notification a rušení kroků. Nejde o implementaci světové logiky.

Pokračuje TutorialManager s profilem, 29 instalovanými predicates a skutečnými herními objekty. Zbývají již zakázané/parent-disabled kotvy, override defaultů a XML reload matrix, null-predicate výjimky, extrémní a NaN souřadnice i skutečné důsledky dvojího dokončení v profilu. Hlavní herní EXE, obousměrné saves, kompletní Squall a izolovaný odinstalátor ještě nejsou dokončené.
