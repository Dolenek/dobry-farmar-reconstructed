# PriceText a TextMoney

Zdroj `src/game/ui/PriceText.*` obnovuje PriceText (700 nativních bajtů) a TextMoney (704 bajtů) ze skutečných instrukcí. Nejde o dokončenou hru; propojení s dialogy a herními obrazovkami pokračuje.

| Původní adresa | Funkce |
| --- | --- |
| 0048bd30 / 0048bd60 | PriceText konstrukce / destrukce |
| 0048bd80 | pouze ASCII číslice 0–9 |
| 0048be00 | TextLine XML, teprve potom DegreeOffset |
| 0048c210 | rozestupy písmen podle běhu číslic |
| 00415b70 / 00415bf0 | TextMoney, vlastněný Picture/Coin |
| 00415d10 | povinný Coin XML, posun o aktuální pozici textu |
| 004161a0 / 00416580 | enable/disable: Coin snímek 0/1 |

PriceText má defaultní DegreeOffset 4. Volá TextLine::makeLetters, potom prochází seznam písmen opačně, zatímco index surového UTF-16 textu roste dopředu. Pro číslice přidává `(run % 3) * degreeOffset * 0.3333333432674408f`; ostatní znaky běh resetují. Nevkládá mezery a nepřepočítává šířku/výšku. Zachovaná je x87 posloupnost bez mezilehlého float spill.

XML nejprve provede base load včetně virtuálního makeLetters s dosavadním DegreeOffset. Nová hodnota se nastavuje až potom a sama nenastavuje dirty. Coin se při loadXML posune o ownerovu pozici, ale další setPosition ownera ho nepřesune. Disable volá TextLine::onDisabled před nastavením Coin snímku 1; enable používá View::onEnabled a snímek 0.

## Porovnání

`tools/validate_price_text.py` / CTest `original-price-money-text`: oba typy nezávisle v PC53 a PC24, všech 14 dodaných money definic a dva fixtures. 33 152 stavů, 208 584 písmen, 64 vlastněných objektů a XML reloadů, 256 GPU snímků / 122 880 000 RGB pixelů. Každý objekt projde 256 sekvencí textu, offsetu, alignmentu, multiplieru, enable/disable a přesunů. Porovnávají se celé sprity, draw pořadí/API i resource release.

Diagnostika mapuje originál bez spuštění entry pointu; XML, staré CRT a resource rozhraní mají explicitní adaptéry. Produkční zdroje originál nevolají. GPU kontexty jsou oddělené po typech kvůli monotónní diagnostické haldě pro původní adresové řazení.

## Zbývá

Skutečné dialogy, Farm/Market/Manufacture a celá hra. Delší lokalizovaný text než raw string může v originálu způsobit čtení mimo raw storage; tato oblast není ověřená. Neověřené jsou nativní exception/allocation/locale cesty a obousměrné herní savy.
