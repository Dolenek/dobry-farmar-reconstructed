# Zápis resource logu

`ResourceManager::logResources` obnovuje `0053fdf0`; původní iterator jmenné mapy je `0053f2d0`. Metoda vypíše všechny aktuální položky cache v pořadí názvů, včetně prostředků s nulovým reference count. Typ bere z virtuální metody resource `+4` a formátuje jako signed integer.

Formát je UTF-16 bez BOM, deklarace končí LF, každý `<Tag Path="…" Type="…" />` končí LF, závěrečné `</ResourceLog>` nemá nový řádek. Názvy se neescapují. Nejprve se sestaví celý text; až poté FileIndex vytvoří soubor a odstraní jeho případnou položku z resource cache. Odstraněný objekt tím není zničen.

`python tools/validate_resource_log.py` porovnává raw bajty proti původním instrukcím v PC53 i PC24. Zahrnuje prázdnou cache, skutečné prostředky obou shipped logů v dávkách, clearUseless, všech pět typů, přepis cachovaného XML a ampersand v platném Windows názvu souboru. Type 3 používá zdrojový shader na skutečném D3D8 device s již ověřenou diagnostickou VS1.1 fixture; oba původní resource logy nemají shader položku.

Původní cache je v diagnostice nezávisle osazený strom 48byte uzlů, nikoli původní manager allocation/factory. Old-CRT stream a file manager tvoří explicitní servisní hranice; skutečné soubory vznikají přes rekonstruované FileIndex/cRealFile ve dvou izolovaných adresářích. Tato kontrola nepotvrzuje původní stream locale pro ne-ASCII názvy, exception wrapping ani celý manager ABI. Původní programové soubory a datové balíky se nemění.

Výsledek: 62 shodných souborů, 8 952 položek a 501 782 UTF-16 code units; `analysis/resource-log-validation.json`. Navazující Start musí ještě vytvořit skutečný herní svět; zápis logu tento požadavek nenahrazuje.
