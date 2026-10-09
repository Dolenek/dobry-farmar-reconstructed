# Bitmapové fonty a sprity písmen

`cFont` načítá původní fonty přes FileIndex a ResourceManager. `TextLine` a `Letter` nyní propojují layout, lokalizaci, vlastněné potomky, textury a globální hloubkové řazení se SpriteBatches. Původní XML typed extraction, ostatní ovládací prvky, vstup a celé herní menu ještě nejsou obnovené.

| Adresa | Obnovená část |
| --- | --- |
| `0050bd70` | Datová část konstruktoru fontu: magic DWORD `0x666f6e74` (bajty `tnof`), počet rozsahů, záznamy po šesti bajtech, počet a pole znaků po 20 bajtech. |
| `0050bd60` | Resource type 2. |
| `0050b3d0` | Dostupnost UTF-16 znaku: lower_bound podle konce rozsahu a kontrola obou hranic. |
| `0050b450` | Výběr pěti floatů: u0/v0/u1/v1/advance; nepodporovaný znak nechá výstup nedotčený. |
| `00546770`, `00544720` | Font cache: ASCII lowercase názvu, kontrola typu, reference. Znovu použitý font se na rozdíl od texture/shader veřejné metody nevkládá znovu do množiny pointerů. |
| `004dc800` | Rozměry, mezery, umístění, UV a hloubka jednotlivých písmen; následný posun podle zarovnání. |
| `0050c500` | cLetter geometrie a barva výsledného spritu. |
| `004ed4f0` | Výběr průhlednosti nejvyššího rodiče ovládacího prvku. |

V `cz.pack` jsou tři fonty (`UI/Font/font_tips.fnt`, `UI/honey_font.fnt`, `UI/menu_font.fnt`), každý s 15 rozsahy a 739 glyfy. Záznamy fontů používají moderní map/vector; původní 68bajtové objektové/container ABI není potvrzené. Konstruktor odmítá useknuté vstupy bezpečnou výjimkou; původní chybové/exception chování tím není doložené.

Textová geometrie zachovává původní float spill body. Mezery U+0020 a U+00A0 přidají SpaceWidth; neznámé znaky neposunou kurzor. Šířka atlasu je allocatedWidth × scaleU. Střed písmena se posouvá o polovinu advance před i po vytvoření; hloubka je textDeep + index × 0.01F. Align 0 centruje, 2 zarovnává doprava, ostatní hodnoty neposouvají. Výška není na začátku vynulovaná: uchová předchozí maximum.

UV předaná cLetter jsou u0, 1−v0, u1, 1−v1, zatímco velikost Y vychází z rozdílu (1−v1)−(1−v0). Záporná výška je zachovaná. cLetter převezme RGB ze svého packed Color, ale jeho původní alfa složku nahradí součinem rootAlpha a controlAlpha. Tato zvláštnost není nahrazena běžným násobením všech alfa faktorů.

```powershell
cmake --build build/windows --config Release
python tools/validate_fonts.py
python tools/validate_text_layout.py
```

Font test porovnává všech 65 536 UTF-16 jednotek ve třech fontech s původními read-only metodami: 393 216 shod, 2 217 dostupných glyfů, sedm cache/reference kontrol. Původní konstruktor nebyl spuštěn; struktura souborů se navíc porovnává s nezávisle čtenými binárními záznamy.

TextLine oracle spouští původní `004dc800` na vlastním stromu fontu a diagnostických letter objektech. Virtual metody zachycují výstup. Kopírování WString/free a resource/activation hranice mají explicitní testové adaptéry; tyto části UI nejsou výsledkem testu ověřené. Na 12 000 deterministických scénářích souhlasí šířka/výška a 88 583 úplných geometrií. Následné původní `0050c500` a skutečný průchod rodiči `004ed4f0` souhlasí se stejným počtem 76bajtových spritů včetně barev. Test nespouští původní entry point a mění jen svou soukromou mapu PE.

## Lokalizace

StringManager (`0052f230`, `0052ed20`, `0052e670`) načítá `sys/strings.xml` přes ResourceManager, používá přesné UTF-16 klíče a přepíše duplicitní překlady. Český balík obsahuje 461 překladů. Prázdný vstup vrací vstupní referenci; chybějící překlad používá společný statický WString. Přímý oracle getString s vypnutým DumpStrings souhlasí v 10 464 případech včetně reference na společný fallback. Kopírování WString je testový adaptér, původní getString/map lookup se spouští na vlastních datech.

Zapnuté `[game]DumpStrings` předřadí `!!!!!!!` a eviduje chybějící klíče. Destruktor zapisuje původní název `strdubp.xml`, UTF-16 bez BOM a bez XML escapování, ve vzestupném pořadí klíčů. Čtyři izolované fixture kontroly ověřují vlastní implementaci; původní dump větev a celý konstruktor/destruktor nebyly runtime porovnané. Moderní kontejnery nejsou původní 32bajtové ABI.

## GPU propojení

Sprite draw oracle nyní obsahuje 108 scénářů a 51 840 000 přesně shodných RGB pixelů. Dvanáct přidaných snímků načítá skutečné MainMenu XML, šest textových položek a české překlady, tři fonty a jejich původní atlasy; prochází textovou geometrií, letter sprity a rekonstruovanými dávkami. Porovnává draw volání a GPU výstup s původní `00515830`. Umístění těchto diagnostických snímků je fixture; není tím doložená celá původní menu obrazovka, její layout, viditelnost nebo vstup.

## TextLine a Letter jako ovládací prvky

`TextLine` obnovuje konstruktor/destruktor `004daca0`/`004da840`, reusable pool písmen `004dd030`, shown/disabled/update větve `004da2f0`/`004da2d0`/`004dae40` a alpha/deep propagaci. Při zkrácení textu se mažou první položky poolu, nikoli poslední. Písmena jsou zároveň vlastněnými View potomky. TextLine se sama nevkládá mezi drawable controls; jednotlivé Letter ano. Nastavení `[game]RightToLeftEnable` řídí `004dc1c0`: obrací UTF-16 jednotky, ale zachová souvislé ASCII alfanumerické bloky. Localized změna sama nenastaví dirty, stejně jako původní inline setter.

Native oracle používá skutečné původní TextLine/Letter konstruktory, destruktory, child sety, callback listy, pool a makeLetters. Ověřil 18 000 scénářů, 169 056 vlastněných písmen včetně spritů, 4 800 porovnání změny směru a 600 zániků TextLine. Settings/localization a String allocation/assign/replace jsou výslovné adaptéry na obnovené služby; textura je skutečný obnovený cTexture na Direct3D8. Původní font lookup běží na původním node layoutu naplněném shipped fontem. Original loadXML je pro tento oracle nahrazen inicializací doložených UserText polí; zdrojový loadXML používá opravdový XmlNode a resource cache.

## Kontext x87 po vytvoření zařízení

Původní `0051c5d0` volá CreateDevice s flags `0x20`. Direct3D8 tím mění precision control x87 na 24 bitů. Test s původní aritmetikou po zařízení odhalil rozdíl jednoho float ULP v poloze písmen a rozdíl v packed barvě. `OriginalArithmetic.h`, TextLineLayout a Color nyní respektují aktuální x87 přesnost a původní pořadí operací. TextControl oracle běží jak s FPU_PRESERVE diagnostikou (`0x22`), tak s původními flags `0x20`; obě skupiny jsou přesně shodné. Report obsahuje control word před/po. Následný audit Matrix, ControlSprite/Animation a SpriteGeometry odhalil další mezivýpočty citlivé na přesnost. Po zachování původních x87 operací prošlo porovnání v obou kontextech: Matrix 140 004, ControlSprite 200 000, animace 40 000, SpriteGeometry 40 000 a Color 121 538. View transparency a další budoucí herní výpočty tento audit zatím nepokrývá.

Zbývá font constructor/lifetime oracle, původní container/exception ABI, další přechody a settery TextLine, XML loading porovnání, ostatní ovládací prvky a skutečné menu se vstupem.
