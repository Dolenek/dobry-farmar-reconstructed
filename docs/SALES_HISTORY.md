# cGame historie prodeje

`SalesHistory` obnovuje skutečné cGame list members a metody `0040c2b0`/`00410f10`. Native storage: allocator `+9f4`, 16-byte sentinel pointer `+9f8`, uint count `+9fc`, unsigned total `+a00`, float cutoff `+a04`. Constructor `00413210` použije `00522f00` a nastaví count/total/cutoff na nulu. Node obsahuje next, previous, float GameTime a uint price. Zdroj má upravitelnou soukromou `std::list<SaleRecord>`; její paměťový layout není vydáván za původní save formát.

`prune(time)` uloží float(time−60). Porovnání je přísné: timestamp menší než cutoff se odstraňuje; equality a unordered/NaN zastaví průchod. Po odečtení uint amount a smazání node původní `0040c337` načte **next pointer z jeho successor**, takže další node přeskakuje. Je-li successor sentinel, pokračuje jeho prvním node. Například ze dvou starých záznamů může zůstat druhý; u lichého seznamu může průchod oběhnout a odstranit vše. Tento původní postup je zachován, není nahrazen intuitivním odstraněním všech expired záznamů.

`record(amount,time)` nejprve prune, pak append `(time,amount)`, zvýší count a wrap32 total; unsigned total>999 nastaví profilové `awEarn=true`. Flag zůstává nastavený, i když pozdější prune sníží total. Price z Marketu je původní **uint/integer word**, nikoli dekompilací mylně popsaný float.

PC53/PC24 prošlo 5 738 stavů, 2 752 explicitních insertions a 1 400 explicitních prunes. Nadto skutečné Market prodeje volají přímo obnovený SalesHistory a původní cGame recordSale v referenci. Zachováno je 1 584 Market stavů a 168 GPU snímků / 80 640 000 shodných pixelů z propojené Warehouse/Manufacture/HUD/WareDetail/Gameplay/TutorialManager scény. Native count, sentinel a obousměrné node links, pořadí všech timestamp/amount words, unsigned total, cutoff a awEarn souhlasí po každé operaci.

Přímé případy zahrnují přesnou šedesátisekundovou hranici a nextafter, out-of-order times, 2 000 deterministických random actions na přesnost, uint overflow amounts, oba zeros, subnormals, extreme finite values, infinities a quiet/signaling NaN representations. Report je `analysis/sales-history-validation.json`; reprodukce `python tools/validate_sales_history.py`, CTest `original-sales-history`.

Diagnostický cGame buffer má skutečné native list fields, ale netvrdí úplný cGame constructor. Původní list constructor/insert/count/prune/record instrukce se vykonávají; history methods nejsou v tomto režimu přesměrované na observer nebo zdrojovou implementaci. Typed profile/string/allocator a řízený clock/input zůstávají explicitní hranice. Reference list se po porovnání explicitně uvolní; zdroj je owned RAII data.

Zbývá úplné cGame, floating product expense/income visual, všechny Money effects, původní save sales block a obousměrná kompatibilita celých her. Výjimky při length/allocator failure, běžné allocation identities a další x87 rounding modes/locales nejsou ověřené. Celek není dokončený projekt.

Kompletní navazující Win32 Release regrese prošla všech 71 CTest kontrol za 294,12 s. Canonical i independent physical game-audio audity prošly po 378 stavech / 36 playback / 16 pause kontrolách. Strict record ověřuje současné source hashe a všech 17 originálů.
