# Rozšířená tlačítka a jejich nápovědy

`ExtendedTextAnimationButton` obnovuje cExtendedTextButton<cAnimationButton> (původních 868 bajtů). Používá obnovený TextAnimationButton, vlastněný Picture, TextLine/MultilineText a čtyři samostatné AnimableAppearance. Vyžaduje skutečné napojení na stav tutoriálu. Neobsahuje výchozí náhradu tutorial controlleru.

| Adresa | Chování |
| --- | --- |
| 00428370 / 004284c0 | Konstrukce/destrukce, čtyři appearances a vlastněný Picture. |
| 0042a1b0 | Tip: Offset, TipsAlphaChangeTime, WaitTime, Image, čtyři směrové vzhledy, Texts/Pics/MultiTexts. |
| 00428a90 | Rozdělení obrazovky pevně v x=400/y=300. Znaménko offsetu a index vzhledu, posun tooltipů podle uložených pozic. |
| 00428590 | Základní deferred input, tutorial guard, odpočet čekání, původní x87 fade a show/hide. Alpha není omezena na 0..1. |
| 00429bb0 / 004297c0 | Input registrace a stav nápověd při enable/disable. Enable na kurzoru se obejde bez tutorial guardu a tipsDisabled testu. |
| 00428ff0 / 004293e0 | Hover/unhover, čekání a blokace nápověd během tutoriálu. |
| 00499ca0 / 00499cc0 | Press/release a přechod nápovědy do útlumu. |
| 0049a340 | Skryje tlačítko i všechny nápovědy. |
| 004d2160 | Přenese appearance do controlu: reference textury, atlas/length/loop. Zachová playing/time a dirty flag. |
| 004cdee0 | Tutorial active: původní pole +bfc != -1. |

XML reload nepřepisuje seznamy: appenduje Image i nové texty a obrázky; následně znovu appenduje původní pozice všech současných nápověd. Positioning dál používá první uložené položky. Source tento neobvyklý postup zachovává, včetně opakovaných pointers na jediný vlastněný Picture. View vlastní děti; vector není druhý vlastník. Dřívější button sound reference originál rovněž drží, source zachovává již ověřené Button chování.

`python tools/validate_extended_buttons.py` porovnává skutečné původní konstruktory, XML, callbacky, positioning, x87 fade i destruktory v PC53 a D3D8 PC24. Pokrývá 16 původních tlačítek z osmi XML (Dig/Mow/WareHouse/WaterSource, ContractStatus, Farm, GameMenu, Manufacture) a dva řízené vstupy (Texts/Pics a chybějící Tip). Celkem 77 724 stavů, 77 328 kroků, 108 reloadů a 3 900 796 letter porovnání. Zahrnuty jsou hranice čtvrtin, záporné a mimoobrazové pozice, tipsDisabled, tutorial guard, libovolné směry, nulové délky a NaN.

144 kompletních D3D8 snímků má 69 120 000 shodných RGB pixelů a stejné pořadí sprite/draw volání. 128 snímků používá původní definice, 16 řízené fixtures. Frame clock a cursor jsou diagnosticky řízené. Přímé callback scénáře neověřují fyzickou myš/klávesnici. XML/CRT typed služby a sound služby jsou explicitní adaptéry; tutorial guard běží proti původnímu +bfc poli fixture, nikoli dosud celému tutoriálu.

Původní konstruktor waitRemaining neinicializuje. Zeroed diagnostic heap jej reprezentuje nulou; source jej inicializuje nulou, aniž by tvrdil shodu původního běžného alokátoru. Jiná extended-button specializace, action areas, budovy, celé GameMenu/cGame, původní výjimky/locale, 3D a herní save zůstávají rozpracované. Původní programové binárky se používají pouze v porovnávací diagnostice, nejsou součástí implementace.
