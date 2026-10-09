# Profily

`ProfileHeaders` obnovuje hlavičkovou část cProfile. Původní 88bajtový objekt má singleton/vptr +0, NowPlaying +4, uložené CurrentUser WString +8, právě vybrané jméno WString +24, další ID +44, names map +48/head +4c/size +50 a flags +54/+55. Konstruktor `004ecb20` přiřadí vstupní jméno do **+24**, zatímco `loadHeaders` `004ea000` přiřadí XML CurrentUser do **+8**. Výběr aktuálního jména podle uložené hlavičky teprve provádí vyšší `004ec310`; není zaměněný za samotné načtení hlaviček.

Hlavičkový loader přečte NextID, CurrentUser a NowPlaying, poté všechny indexované XML potomky bez filtru jejich tagu. Chybějící Name vytvoří prázdné jméno; chybějící ID ponechá lokální nulu. Names map se nemaže. Unique insert `004e93d0` zachová první ID duplicitního jména i při dalších loadHeaders. Na konci vynuluje dirty +54. NextID/NowPlaying při prázdném atributu ponechají dřívější hodnotu. Moderní stream konverze jsou prozatím evidovaná hranice.

`004e8900` přidává názvy v unsigned UTF-16 pořadí na konec vstupního seznamu a nemaže starší obsah. Lookup `004dfd90` neprovádí vložení. `getProfileID` `004e4110` hledá vybrané jméno, při chybě vyhazuje řetězec ProfileID error. Formátovací rutina `004e17e0` podle instrukcí používá decimal stream, fill '0' a minimální width 8. Rekonstruovaný formatter používá moderní stream; původní CRT formatter zatím nebyl přímo porovnaný.

```powershell
cmake --build build/windows --config Release
python tools/validate_profile_headers.py
```

Native oracle spouští původní konstruktor s prázdným vstupním jménem (load v této větvi nepřistupuje k souborům), původní header loader, map insert/lookup, getNames a getProfileID lookup. 200 konstruktorů, 16 000 header loads, 160 000 lookupů a 800 append-list kontrol souhlasí ve dvou přesnostech x87. Používá 1 280 parsovaných UTF-16 fixtures s duplicitami, chybějícími atributy, různými tagy, českými/japonskými/dlouhými názvy a XML entitami. Reference je soukromě mapovaná na jiné adrese, bez entry pointu; všechny originály zůstávají nezměněné. Report: analysis/profile-headers-validation.json.

Testové hranice: XmlDocument a typed XML parsování používají obnovené/moderní adaptéry; WString compare/assignment a heap používají již obnovené ABI a diagnostický CRT. getProfileID formatter je explicitně nahrazen obnoveným formatterem, takže shoda jeho lookup není prohlášená za nezávislé ověření formátování. Cleanup provede původní names-tree erase a přímý fixture úklid. Plný cProfile destructor se nespouští: také zapisuje soubory a čistí třináct globálních typed-property stores.

Další dohledané metody: `004e1010` bere game/profiles z Settings a přidá trailing slash; `004ec310` načítá profiles.xml a vybraný .xml soubor vlastností; `004e3a70` zapisuje UTF-16 headers nebo maže prázdné; `004e6cf0` zapisuje typed Props; `004ea5d0` přidá nový profil a zvýší ID; `004ead60` zapisuje/čistí při zničení. Tyto části, přepínání a mazání profilů, menu SelectProfile, původní parser/CRT a obousměrné herní save soubory zbývají. Header testy samy neznamenají hotové profily ve skutečné hře.

## Třináct typovaných vlastností

`ProfileProperties` obnovuje samostatné globální mapy Float (0065ff38), Double (00660464), Int (0065ff20), UInt (0065ff44), Long (00660470), ULong (0066047c), LongLong (00660488), DXColor (00660458), Bool (0065ff2c), UInt16 (00660494), Vector2 (00660434), String (00660440) a WString (0066044c). Long/ULong sdílejí pomocný kód 004e9b40, ale používají různé mapy; totéž platí pro Int/UInt a 00410770. Lookup vkládá nulovou/prázdnou výchozí hodnotu. Dispatcher 004ebd10 rozlišuje tagy přesně a ignoruje neznámé typy. Clear 004e9d80 maže všechny mapy, resetuje jejich sentinely a počty. XML property loop je z 004ec310; celý vyšší loader ještě není ověřený.

Oracle `python tools/validate_profile_properties.py` porovnal 62 400 přístupů, 67 200 typových volání, 128 492 kompletních map a 284 clear operací ve dvou přesnostech x87. Pokrývá oddělené stejnojmenné klíče ve všech typech, prázdné/dlouhé/high-byte/embedded-NUL klíče, unsigned-byte pořadí, defaulty, přepis a opakované vložení po clear. Původní instrukce map, setterů, přepínače a mazání skutečně běží. Při diagnostice byly podle instrukcí opraveny testové offsety: Double/LongLong mají key +10, value +30 a nil +39 (osmibajtové zarovnání), ostatní key +c/value +28; Bool/UInt16 mají padding do color +2c/nil +2d. Report analysis/profile-properties-validation.json obsahuje adresy a hranice.

Numeric wide-stream a ACP převody jsou explicitní testové adaptéry. Platné testované hodnoty ověřují routing a uložení, nikoli nezávislou shodu starého CRT. Prázdné/neplatné numerické vstupy některých původních setterů mohou ponechat neinicializované lokální bity; ty nejsou prohlášené za kompatibilní. Formátování, skutečné soubory Props, lifecycle profilů, UI a herní save kompatibilita zbývají. Tento dílčí výsledek není dokončeným projektem.
