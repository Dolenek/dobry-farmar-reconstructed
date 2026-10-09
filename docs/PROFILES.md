# Profily

`ProfileHeaders` obnovuje hlavičkovou část cProfile. Původní 88bajtový objekt má singleton/vptr +0, NowPlaying +4, uložené CurrentUser WString +8, právě vybrané jméno WString +24, další ID +44, names map +48/head +4c/size +50 a flags +54/+55. Konstruktor `004ecb20` přiřadí vstupní jméno do **+24**, zatímco `loadHeaders` `004ea000` přiřadí XML CurrentUser do **+8**. Vyšší `004ec310` zachová volbu volajícího: instrukce `004ec44a` přiřadí +24 do +8, pokud se jména liší. Dřívější interpretace opačné přednosti byla opravena podle instrukcí a ověřena celým loaderem.

Hlavičkový loader přečte NextID, CurrentUser a NowPlaying, poté všechny indexované XML potomky bez filtru jejich tagu. Chybějící Name vytvoří prázdné jméno; chybějící ID ponechá lokální nulu. Names map se nemaže. Unique insert `004e93d0` zachová první ID duplicitního jména i při dalších loadHeaders. Na konci vynuluje dirty +54. NextID/NowPlaying při prázdném atributu ponechají dřívější hodnotu. Moderní stream konverze jsou prozatím evidovaná hranice.

`004e8900` přidává názvy v unsigned UTF-16 pořadí na konec vstupního seznamu a nemaže starší obsah. Lookup `004dfd90` neprovádí vložení. `getProfileID` `004e4110` hledá vybrané jméno, při chybě vyhazuje řetězec ProfileID error. Formátovací rutina `004e17e0` podle instrukcí používá decimal stream, fill '0' a minimální width 8. Rekonstruovaný formatter používá moderní stream; původní CRT formatter zatím nebyl přímo porovnaný.

```powershell
cmake --build build/windows --config Release
python tools/validate_profile_headers.py
```

Native oracle spouští původní konstruktor s prázdným vstupním jménem (load v této větvi nepřistupuje k souborům), původní header loader, map insert/lookup, getNames a getProfileID lookup. 200 konstruktorů, 16 000 header loads, 160 000 lookupů a 800 append-list kontrol souhlasí ve dvou přesnostech x87. Používá 1 280 parsovaných UTF-16 fixtures s duplicitami, chybějícími atributy, různými tagy, českými/japonskými/dlouhými názvy a XML entitami. Reference je soukromě mapovaná na jiné adrese, bez entry pointu; všechny originály zůstávají nezměněné. Report: analysis/profile-headers-validation.json.

Testové hranice: XmlDocument a typed XML parsování používají obnovené/moderní adaptéry; WString compare/assignment a heap používají již obnovené ABI a diagnostický CRT. getProfileID formatter je explicitně nahrazen obnoveným formatterem, takže shoda jeho lookup není prohlášená za nezávislé ověření formátování. Cleanup provede původní names-tree erase a přímý fixture úklid. Plný cProfile destructor se nespouští: také zapisuje soubory a čistí třináct globálních typed-property stores.

Metody `004e1010`, `004ec310`, `004e3a70`, `004e6cf0`, `004ea5d0` a `004ead60` nyní obnovuje `Profile`; jejich samostatné ověření popisuje další část. Header oracle nadále pokrývá pouze hlavičkové operace. Přepínání profilů a navigační akce menu SelectProfile, původní parser/CRT a obousměrné herní save soubory zbývají.

## Třináct typovaných vlastností

`ProfileProperties` obnovuje samostatné globální mapy Float (0065ff38), Double (00660464), Int (0065ff20), UInt (0065ff44), Long (00660470), ULong (0066047c), LongLong (00660488), DXColor (00660458), Bool (0065ff2c), UInt16 (00660494), Vector2 (00660434), String (00660440) a WString (0066044c). Long/ULong sdílejí pomocný kód 004e9b40, ale používají různé mapy; totéž platí pro Int/UInt a 00410770. Lookup vkládá nulovou/prázdnou výchozí hodnotu. Dispatcher 004ebd10 rozlišuje tagy přesně a ignoruje neznámé typy. Clear 004e9d80 maže všechny mapy, resetuje jejich sentinely a počty. XML property loop je z 004ec310; celý vyšší loader je nyní porovnaný v lifecycle oracle s níže uvedenými hranicemi.

Oracle `python tools/validate_profile_properties.py` porovnal 62 400 přístupů, 67 200 typových volání, 128 492 kompletních map a 284 clear operací ve dvou přesnostech x87. Pokrývá oddělené stejnojmenné klíče ve všech typech, prázdné/dlouhé/high-byte/embedded-NUL klíče, unsigned-byte pořadí, defaulty, přepis a opakované vložení po clear. Původní instrukce map, setterů, přepínače a mazání skutečně běží. Při diagnostice byly podle instrukcí opraveny testové offsety: Double/LongLong mají key +10, value +30 a nil +39 (osmibajtové zarovnání), ostatní key +c/value +28; Bool/UInt16 mají padding do color +2c/nil +2d. Report analysis/profile-properties-validation.json obsahuje adresy a hranice.

Numeric wide-stream a ACP převody jsou explicitní testové adaptéry. Platné testované hodnoty ověřují routing a uložení, nikoli nezávislou shodu starého CRT. Prázdné/neplatné numerické vstupy některých původních setterů mohou ponechat neinicializované lokální bity; ty nejsou prohlášené za kompatibilní.

## Zápis a životní cyklus

`dumpXML` obnovuje třináct původních writerů `004e4c70`, `004e4e90`, `004e50c0`, `004e52e0`, `004e5500`, `004e5720`, `004e5940`, `004e3840`, `004e5b60`, `004e5d90`, `004e30f0`, `004e3320` a `004e35a0`. Typy zapisuje v tomto pořadí, klíče v unsigned-byte pořadí a text bez XML escapování. 240 přímých porovnání celých serializací prošlo ve dvou x87 kontextech; původní iterace a skládání tagů běží, starý CRT stream a konverze používají evidované adaptéry. Zachovány jsou i původní zvláštnosti: DXColor se zapisuje desetinně, ale čte šestnáctkově; Vector2 končí `>` místo `/>`. Obecný bezchybný XML roundtrip proto není prohlášený za ověřený.

`Profile` načítá hlavičku a vlastnosti z FileIndex/XmlDocument, vytváří nové ID a ukládá UTF-16 soubory s BOM. Opakované create spotřebuje další ID i při již existujícím jménu. SaveHeaders resetuje dirty před zápisem; prázdná names map invaliduje a maže profiles.xml. Destructor nastaví NowPlaying=false, zapíše hlavičku a vlastnosti, vyčistí všech třináct map a singleton. Soubory jsou ve složce game/profiles z Settings.

Oracle `python tools/validate_profile_lifecycle.py` spouští celý původní konstruktor, loader, create, oba writers a destructor. Porovnává 160 konstruktorů/destruktorů, 544 skutečných UTF-16 zápisů, stav, mapy, pořadí invalidací a vlastnictví XML. Testuje chybějící/existující profil, rozdílné CurrentUser a požadované jméno, opakovaný load/create a prázdný výběr. Dvě nezávislé izolované FileIndex složky dostávají source a oracle soubory; původní instalace je pouze čtená. Report analysis/profile-lifecycle-validation.json uvádí hranice CRT/XML/file služeb. Oracle nepouští entry point hry a implementace nepoužívá původní kód.

## Výchozí herní hodnoty a menu

`initializeGameProfile` obnovuje `cGame::onInitProfile` `00411390`: 790 položek (291 Float, 62 Int, 306 UInt, 131 Bool). Pokrývá pole, výrobu, sklady, trh, úkoly, dopisy, ocenění a tutorial flags. Typ každého klíče je určen podle adresy mapy v instrukcích; sdílený getter Int/UInt neznamená společnou mapu. Oracle provede 220 skutečných volání původního inicializátoru a porovná všechny mapy. Původní getter `0040ab50` vrací 21 award klíčů; číslování záměrně nemá Award18. Indexované prefix/tip klíče mají explicitní moderní formatter boundary. Relokace oracle zahrnuje i absolutní ADD operand `0040aee1`, nejen MOV/PUSH/LEA.

Source diagnostika menu nyní vytváří a znovu otevírá skutečné profily přes `Profile`, původní herní defaulty a ResourceManager XML cache v izolované složce. UserName dostává skutečně zvolené jméno a 96 snímků nadále souhlasí pixelově. Native menu používá řízenou WString službu; plný native lifecycle je ověřen samostatně. Navigační akce SelectProfile, herní saveLevelState/loadLevelState a spuštění rekonstruované celé hry zůstávají neobnovené.

## Mazání a volba při spuštění

`Profile::remove` obnovuje `004e8d20` a tree erase `004e4520`. Chybějící jméno nic nemění. Po odstranění vždy přiřadí první zbývající jméno (nebo prázdné) do uloženého i vybraného názvu, zapíše hlavičku a nastaví suppressSave +55. Potom enumeruje složku s prefixem osmimístného ID, invaliduje a maže všechny odpovídající common soubory. Neomezuje mazání na `.xml`; prefix může odpovídat i delšímu ID. Vlastnosti zůstávají až do destruktoru, který je vyčistí, ale již nic nezapisuje.

160 lifecycle scénářů obsahuje původních 80 bez mazání a dalších 80 s mazáním. Ve dvou x87 kontextech prošlo 168 volání (chybějící, zvolený, jiný a poslední profil, pořadí vzestupně i sestupně) a 358 odstranění souborů. Porovnány jsou pořadí invalidací, hlavičky, mapy, fyzická absence odstraněných souborů a zachování pack souborů/adresářů. File enumeration má adaptér FileIndex a diagnostický String list s původním layoutem; původní profile erase/deletion instrukce běží. Case-sensitive isCommonExist test používá lowercase index klíč podle původní implementace.

`headersExist` (`004e14e0`) a `storedName` (`004e15e0`) obnovují bootstrap výběru: chybějící profiles.xml vrátí prázdné jméno, jinak přečtou CurrentUser a uvolní XML owner. Původní obě rutiny jsou porovnané před všemi 160 konstrukcemi; nepoužívají již existující singleton. Celek je přenositelně zaznamenaný v analysis/profile-lifecycle-validation.json. Přepnutí obrazovky a potvrzovací dialog mazání zůstávají prací na SelectProfile/EditBox/ListBox.
