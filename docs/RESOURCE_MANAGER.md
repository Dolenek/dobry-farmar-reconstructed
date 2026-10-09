# Cache a životnost prostředků

Obnovená datová část cResourceManager je v `src/game/resources/ResourceManager.cpp`. Konstruktor `00548100` vytváří singleton a tři uspořádané kontejnery: jméno → prostředek, prostředek → jméno a množinu prostředků podle číselné adresy. Použité moderní kontejnery zatím neobnovují 40bajtové ABI. Původní konstruktor při druhém singletonu vyhazuje `const char* "Object already exist."`.

| Původní adresa | Chování |
| --- | --- |
| `00545e80`, `00544260` | Texture: lowercase názvu, opětovné použití typu 0 s addReference, jinak nový objekt a zápis do všech cache. |
| `00545a50`, `00543da0` | Shader: lowercase názvu, typ 3, cache/refcount nebo nový 44bajtový cShader. |
| `00546770`, `00544720` | Font: lowercase názvu, typ 2, cache/refcount; veřejná metoda na cache hit neopakuje vložení do pointer set. |
| `005473d0` | XML: obdobně, typ 1; originál převádí iXML pointer na cResource pointer +4. |
| `005478c0` | Odstranění podle pointeru z obou map a množiny, bez decrement nebo delete. Neznámý pointer přes operator[] vloží prázdné jméno a vzápětí ho odstraní. |
| `00547ce0` | Odstranění podle přesného názvu, bez lowercase a bez zničení objektu. |
| `00540b90` | Jméno z reverzní mapy, jinak výjimka `const char* "Unknown resource."`. |
| `005403b0`, `005407a0` | preReset/restore všech prostředků v pořadí adres pointerů. |
| `00540f90` | V pořadí názvů preReset a restore pouze pro typ 0. |
| `005426b0`, `00542b60` | Snapshot a delete pouze prostředků se signed reference count <1. Ani metoda clear nemaže kladné reference. |
| `00542fd0` | Delete typu 4 bez ohledu na reference count, v pořadí pointerů. Skutečné zvukové prostředky ještě chybějí. |
| `005455c0` | Destruktor maže první položku hlavní mapy, dokud není prázdná, bez ohledu na reference count. cResource destruktor odstraní její cache; singleton se vypne až na konci. |

`XmlResource` obnovuje vlastnictví parsovaného stromu a resourceType `00561160` (1). Původní XML přepisuje releaseReference adjustor `00561190` → `00415340`: vrátí true a neprovede decrement. Původní cSettings po čtení volá přímo deleting destructor iXML. `Settings::XmlLoader` a `ResourceManager::loadSettingsXML` propojují tento tok s existující XML vrstvou; konec scope zničí XML a odstraní ho z cache. Celé 96bajtové rozložení cXML s oběma vtable a kompletními typed accessors ještě není obnovené.

FileIndex::create volá odstranění normalizovaného jména ze správce. Dříve vrácený objekt zůstává použitelný; následné načtení vytvoří jiný objekt z přepsaného souboru. Jeho případné pozdější zničení nesmí vyřadit novější položku téhož názvu.

```powershell
cmake --build build/windows --config Release
python tools/validate_resource_manager.py
python tools/validate_textures.py
```

Ověřeno 15 propojených scénářů archive → XML → cache → common overwrite → Settings, výhradně v dočasném adresáři. GPU test načítá všech 416 původních textur přes ResourceManager, ověřuje sdílený objekt a reference, provede reset/restore a po ověření pixelů clearUseless. Všech 78 710 388 obrazových pixelů souhlasí s původní JNG DLL. Po čtení skutečných nastavení z cache nezůstane XML. Jde o porovnání pixelů původního decoderu a kontrolu rekonstruované integrace, nikoli přímé spuštění původního správce.

Aktuální GPU test rozšířil pokrytí na 425 JNG a deset DDS/TGA/BMP; detaily a omezení v [NATIVE_IMAGES.md](NATIVE_IMAGES.md). Shader resource a reset vlastního shader fixture popisuje [SPRITE_RENDERING.md](SPRITE_RENDERING.md); font factory a lookup oracle [UI_FONTS.md](UI_FONTS.md). Zbývá přímý runtime oracle správce a constructor/lifetime shader/font; mesh/animation/sound factories a jejich třídy, resourcesToLog, původní manager/XML ABI, engine exception obaly a úplné zapojení do herního procesu. DeleteAllSound zatím nemá ověření skutečným sound objektem. Množina prostředků podle adres je rozlišená od mapy podle názvu; tyto různé pořadí se nesmí sjednotit.
