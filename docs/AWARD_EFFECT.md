# Animovaný efekt ocenění — cAwardEffect

`AwardEffect` obnovuje původní 704bajtový Picture controller. Zdrojové rozložení není původní objektové ABI. Používá skutečné Effect/Button sekce všech 21 ocenění v UI/Awards.xml, obrázky, popisy a uložené velikosti, pozice a rychlosti. Konstruktor načte a odstraní XML, nastaví render flag 0x20 a velikost 0×0; samotné Picture vlastnictví a registrace zůstávají původní.

| Adresa | Funkce |
| --- | --- |
| 004c6a40 | Konstruktor, defaulty, XML a nulová výsledná velikost |
| 004c61e0 | Awards[index], PanelText, Congratulation, Effect, Button size, root Sound |
| 004c60e0 / 004c61c0 | hide/disable, sound release, Strings a Picture destrukce |
| 004c5b70 | Přepnutí phase, reset času; phase 5 navíc scale=1 |
| 004c5bc0 | Picture tick a původní phase switch |
| 00421520 | Normalizace vektoru na délku s rozdílným X/Y spill |

Phase 0 zvětšuje obrázek a přesouvá jej od initialPosition k PositionPanel. Při dosažení scale 1 přejde na 2, zruší flag 0x20 a volá SoundManager click pro nenulový Sound. Phase 2 přidává elapsed k animaci, při dosažení length nastaví čas 0 a phase 1. Phase 4 provede pohyb k destination; dot product směru a zbývající vzdálenosti rozhodne o dosažení cíle a phase 5. Velikost interpoluje mezi původní Effect size a Button size podle zbývající vzdálenosti / vzdálenosti destination−PositionPanel. Instrukce opravují neplatně rekonstruované lokální proměnné v dekompilaci tohoto výpočtu.

Phase 6 nastaví loop, zvětšuje scale až na HoveringScale a přejde na 7; započaté hoverAnimation pokračuje v čase. Phase 7 přímo přidává elapsed. Phase 8 zmenšuje scale, zruší loop, dokončuje animaci a po zmenšení i dokončení přejde na 5, zruší flag 0x20 a resetuje čas. Hloubka při hover změnách je originalDeep + scale, velikost scale×Button size. Phase 1, 3, 5 a neznámé hodnoty nemají další větev, ale úvodní Picture tick stále proběhne. Setter pro 6/8 zachovává čas; ostatní jej resetují, 5 navíc scale. Veškeré tyto zvláštnosti jsou zachované.

Float spill hranice vycházejí z instrukcí: v normalizaci vektoru X ponechá ratio v x87 registru, Y použije jeho float spill; při phase 0 má X interpolace dodatečný spill před přičtením initialPosition, Y nemá. Pohybová interpolace velikosti obdobně uchovává X produkt, Y jej reloaduje z float. Přechody při zvětšování porovnají akumulátor před jeho float spill. Zdroj používá x87 aritmetické pomocníky a konkrétní nativní vector postup.

`tools/validate_award_effect.py`: PC53/PC24, 10 878 stavů, 10 080 step volání, 252 hromadných XML reloadů, 42 vlastněných objektů, 42 sound callbacků a 168 GPU snímků / 80 640 000 přesně shodných RGB pixelů a draw volání. Všechny původní obrázky mají GPU snímky. Původní kód běží pouze v diagnostice s vypnutým entry pointem. XML/resource/CRT rozhraní jsou explicitní dosavadní diagnostické adaptéry. Frame používá obnovený Timer s řízenými clock/input/cursor službami.

Původní Awards nemá root Sound; fixture jej doplní skutečným Sounds/menu_click.wav pro ověření service callbacku a release. SoundManager je v této diagnostice disabled, recorder zavolá jeho skutečnou disabled větev. Audible/physical playback tohoto volání není ověřený. Původní XML přepis Sound nepouští předchozí reference; source to zachovává, opakované neprázdné reloady však nejsou zahrnuté. Degenerate move denominator, NaN/inf, malformed XML a původní exception ABI zůstávají neověřené. Pokračuje AwardButton, Awards controller, navigace menu a herní award manager; nejde o dokončenou hru.
