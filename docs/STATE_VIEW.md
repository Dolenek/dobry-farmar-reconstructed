# Přechody herních obrazovek

`StateView` obnovuje společný základ pohyblivých obrazovek. Původní objekt má 100 bajtů a tabulku 15 virtuálních metod na `005fa9f8`; upravitelná implementace používá již obnovené `MenuView`, `Picture` a `Control`. Součást implementace nepoužívá původní programový kód.

| Původní adresa | Obnovená operace |
| --- | --- |
| `004c48e0` | Konstruktor a dva vlastněné grafické uzly |
| `0042c7d0` | Destruktor, přímý skok do destruktoru MenuView |
| `004c4a80` | XML, rychlost pohybu a změna rodiče dekorací |
| `004c4eb0` / `004c5270` | Zobrazení / skrytí vnitřního uzlu |
| `004c5630` | Zahájení pohybu |
| `004c56f0` | Časový krok a dokončení pohybu |

Konstruktor vytvoří Picture na pozici `(-1,0)` o velikosti `(1,1)` a uvnitř Control na `(0,0)` stejné velikosti. Oba uzly zapne a zobrazí. XML nejprve načte skutečné dekorace MenuView, potom je připojí pod pohyblivý Picture. Výchozí rychlost je 2400. Původní konstruktor nevyplňuje zbývající čas na `+5c`; zdroj jej inicializuje nulou a diagnostika používá nulovou alokaci. Po prvním skutečném pohybu už jsou všechny porovnávané hodnoty definované původním kódem.

Stav nula pouze přepíše identifikátor pohybu. Ostatní stavy spočítají délku, dobu a rychlost podle původních x87 instrukcí: čas se uloží do floatu, ale převrácená hodnota používá dosud nezaokrouhlenou dobu. Rychlost násobí samostatně uložené floatové rozdíly souřadnic. Krok vždy přečte škálovaný časovač, i bez aktivního pohybu. Podmínka používá nezaokrouhlený zbývající čas; původní x87 větev při NaN pokračuje v pohybu. Při dokončení stav jedna skryje obrazovku, ostatní nenulové stavy zavolají skutečný virtuální hook, zobrazí vnitřní uzel a obnoví enable před vynulováním stavu. Základní hook je v originálu skutečný prázdný návrat `004bd820`.

`validate_state_view.py` porovnává původní instrukce v PC53 i původní D3D8 PC24: celkem 22 242 stavů, 2508 zahájení a 14 592 kroků. Obsahuje náhodné souřadnice/rychlosti/čas, přepínání viditelnosti a enable rodiče, nulový pohyb, záporný čas, nulovou délku a nulovou nebo zápornou rychlost. Porovnává všechny pohybové hodnoty, oba vlastněné uzly, rodiče dekorací a uvolnění textur.

Dvanáct načtení skutečných `Farm.xml`, `Manufacture.xml` a `WareDetail.xml` ověřuje jejich společný základ. Skutečný rekonstruovaný časovač, cursor a frame dispatcher posouvají podklady při příchodu a odchodu; 108 D3D8 snímků má shodná draw volání i všech 51 840 000 RGB pixelů. Nativní implementace je načtena pouze v diagnostice bez spuštění vstupního bodu; adaptéry XML/CRT/resource rozhraní jsou výslovně oddělené.

Toto ověření nezahrnuje odvozené herní controllery Farm, Warehouse, Manufacture ani jejich vstupní hooky, samotné přepínací handlery GameMenu, tutoriál nebo celý herní svět. Zbývá extrémní přetečení/subnormály, libovolné NaN payloady, chybné XML a allocation/exception větve. Hlavní herní EXE, kompletní Squall, obousměrné save a izolovaný odinstalátor zůstávají podmínkami dokončení celého projektu.
