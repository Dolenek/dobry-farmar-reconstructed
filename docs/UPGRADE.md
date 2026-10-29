# Společná logika vylepšení

## Původní alias zaměřené výrobní budovy

Skutečný výrobní tutoriál odhalil rozdíl v cGame `0040be10`: oproti nástrojům s výslovným posunem+508 předává výrobní větev přímo Manufacture.focused `+f8` v ECX do `00417050` a `004174b0`, bez převodu na Upgrade základ. Getter tak čte View `shown/parentShown/enabled/parentEnabled` na+24 jako ID a `absolute/parentAbsolute` plus dva padding bajty na+28 jako levelCount. Původní key je proto při těchto všech true příznacích `Upgrade16843009`. Hardware breakpoint potvrdil return0040bed7 a parent00410e3c během bezplatného upgradu34; klíč vzniká před demolicí33. Vtable původního objektu005f8378 má+4→0050bd60 (MOV EAX,2/RET), tedy type, nikoli skutečné obnovení Upgrade vzhledu.

`refreshViewUpgradeAlias` obnovuje tuto větev nad odpovídajícími skutečnými stavovými bajty vlastního ViewState, včetně jeho vlastních dvou výplňových bajtů. Žádné pozorované číslo klíče ani BAAD hodnota nejsou do implementace vložené napevno. Typové čtení a signed clamp s wrap32 zůstávají původní; volá se View.type a pokračuje původní focus pořadí. Čtyři farmářské nástroje dál používají skutečný Upgrade základ. Ostatní event dispatch větve se tím nepřepisují bez samostatných instrukčních důkazů.

Rozšířený `validate_upgrade.py` přidává6144 případů PC53/PC24 se shodnými explicitními words: všech16 kombinací prvních čtyř bool, čtyři kombinace absolutních bool, osm vzorů paddingu včetně BAAD a signed hranic, šest výchozích stavů mapy včetně chybějícího klíče. Celé profilové mapy i virtuální type callback souhlasí s původními GetLevel/SetLevel. Stejný výsledek prošel ve druhé sestavovací cache. Dosavadních131296 abstraktních Upgrade stavů se těmito dodatečnými případy nepřejmenovává.

Skutečné nezávislé EXE nyní mají shodných795 klíčů/typů; jediná nestabilní integer hodnota přesně odpovídá zaznamenanému vlastnímu paddingu. Originál měl word BAAD0101 a uložil BAAD0100 (−1163067136), zdroj měl41520101 a uložil0. Tyto hodnoty nejsou normalizované jako totožné. Identická vstupní words mají výše doloženou přesnou sémantiku, nezávislé heap výplně ani celý44bajtový původní View ABI se tím nevydávají za shodné. Trace, výchozí odchylka a aktuální porovnání: `analysis/manufacture-focused-upgrade-trace.json`, `analysis/manufacture-tutorial-runtime-observation.json`, `analysis/manufacture-tutorial-state-validation.json`, `analysis/focused-view-upgrade-regression.json`.

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
