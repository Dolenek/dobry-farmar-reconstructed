# Rozšířená tlačítka a jejich nápovědy

`BasicExtendedTooltipButton` obnovuje dvě původní specializace: `ExtendedTextAnimationButton` (868 bajtů, TextAnimationButton) a `ExtendedAnimationButton` (864 bajtů, AnimationButton). Společná část používá vlastněný Picture, TextLine/MultilineText a čtyři samostatné AnimableAppearance. Vyžaduje skutečné napojení na stav tutoriálu. Neobsahuje výchozí náhradu tutorial controlleru.

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

`python tools/validate_extended_buttons.py` porovnává každou specializaci proti jejím vlastním původním metodám v PC53 a D3D8 PC24. Používá 16 definic z osmi XML (Dig/Mow/WareHouse/WaterSource, ContractStatus, Farm, GameMenu, Manufacture) a dva řízené vstupy (Texts/Pics a chybějící Tip). U každé definice explicitně konstruuje příslušnou původní třídu; tím netvrdí, že skutečný cGame používá obě specializace pro každé tlačítko. Celkem 155 448 stavů, 154 656 kroků, 216 reloadů a 7 749 776 letter porovnání. Zahrnuty jsou hranice čtvrtin, záporné a mimoobrazové pozice, tipsDisabled, tutorial guard, libovolné směry, nulové délky a NaN.

288 kompletních D3D8 snímků má 138 240 000 shodných RGB pixelů a stejné pořadí sprite/draw volání. 256 snímků používá původní definice, 32 řízené fixtures. Frame clock a cursor jsou diagnosticky řízené. Přímé callback scénáře neověřují fyzickou myš/klávesnici. XML/CRT typed služby a sound služby jsou explicitní adaptéry; tutorial guard běží proti původnímu +bfc poli fixture, nikoli dosud celému tutoriálu.

Původní konstruktor waitRemaining neinicializuje. Zeroed diagnostic heap jej reprezentuje nulou; source jej inicializuje nulou, aniž by tvrdil shodu původního běžného alokátoru. Vlastní specializace action area, budovy, celé GameMenu/cGame, původní výjimky/locale, 3D a herní save zůstávají rozpracované. Původní programové binárky se používají pouze v porovnávací diagnostice, nejsou součástí implementace.

AnimationButton varianta má samostatné nativní adresy: konstruktor 00423550, destruktor 00421600, XML 004236f0, step 004222b0, enable 004230b0, disable 00421be0, positioning 004227b0, hover 00422cc0, unhover 00421800, press 004215e0, release 0048fb30, hideTips 0048fb50. Její tooltip data začínají na +27c, u TextAnimationButton na +280. Shoda logiky je doložená čtyřmi samostatnými porovnávacími běhy, nikoli pouze podobným pseudokódem.
