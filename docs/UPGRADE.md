# Společná logika vylepšení

`src/game/engine/Upgrade.*` obnovuje abstraktní cUpgrade. Původní vtable 005ef5a0 má v pozicích +4/+8 skutečné `_purecall` (0058100d). Vzhled a načtení jednotlivé úrovně proto dodají obnovované odvozené herní objekty. Moderní C++ rozložení není vydáváno za původní objektové ABI.

| Adresa | Obnovené chování |
| --- | --- |
| 004169c0 / 00416970 | Konstruktor/destruktor: typ, prázdný String, property ID, počet úrovní 0. Původní objekt má 44 bajtů. |
| 00416a30 | Upgrades/Up0..Up3 podle min(počet sekcí,4); chybějící název přeskočí. Po odvozeném loaderu inkrementuje současný počet. Reload jej nenuluje. |
| 00417050 | Čte/inicializuje signed vlastnost Upgrade{id}. |
| 00417430 | Volitelné porovnání dostupnosti a peněz vůči další úrovni; obě porovnání jsou signed, i když UpAvail a tabulka cen obsahují uint32. Nemá kontrolu počtu úrovní. |
| 004174b0 | Omezuje pouze horní mez na levelCount-1, žádnou dolní mez. Při změně ukládá vlastnost a volitelně přehrává zvuk. Vždy obnoví odvozený vzhled. |
| 0040ba00 | Cena: nejprve kontroluje level≤3, pak item≤14, poté vrací tabulku. |
| 00404170 | Sdílený upgrade channel: odstraní předchozí, playSound(upgradeSound,false,true). |

Aritmetika další úrovně a horní meze zachovává wrap32, včetně INT_MIN/INT_MAX. Numerická část klíčů je unsigned: 00406570 používá facet +18 (00408550), který pomocí 00408150 sestaví formát z `lu` na 005ee678. Původní old CRT stream v diagnostice nahrazují explicitní adaptéry; samotný nativní formatter dosud není přímo porovnaný.

`python tools/validate_upgrade.py` spustí skutečné původní metody v diagnostice v PC53 i původní D3D8 PC24. Porovnává 131 296 stavů, 65 536 dotazů a setterů, 245 760 cen, 192 reloadů, 416 odvozených načtení a 44 816 požadavků na zvuk. XML scénáře zahrnují sparse/missing sekce, reload a změnu počtu uvnitř callbacku. Náhodné bitové ceny/dostupnost odhalují signed porovnání. Všechny profilové mapy, callbacky a jejich pořadí se porovnávají samostatně.

Zvuk prochází skutečným obnoveným SoundManagerem ve vypnutém režimu: potvrzuje resource a flagy, nevytváří channel. Nenulový channel, jeho opakované rušení a kompletní pořadí cGame teardown zatím ověřené nejsou. Odvozené callbacky jsou diagnostické trace fixtures, nikoli implementace herního světa. Test nespouští původní entry point a rekonstruovaný kód původní instrukce nevolá.

Další práce: skutečné odvozené budovy/farma, UI nákupu, event dispatch, plný cGame a uložení. Původní invalid-level/item výjimky jsou zkontrolované v instrukcích; jejich old C++ ABI, jiné locale a celá hra tím ověřené nejsou. Přenositelné výsledky ukládá `tools/record_verification.py` do `analysis/upgrade-validation.json`.
