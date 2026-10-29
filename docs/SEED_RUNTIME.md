# Přirozený nákup osiva v celých EXE

`tools/validate_game_seed_unlock.py` navazuje na nativní save skutečně splněné první zakázky. Originál i zdroj nezávisle sklidí další čtyři kusy trávy; kapacita nákladu3 vyžaduje dva návraty do stodoly. Skutečný druhý trh a prodej zvýší Money62→70. Herní událost v čase135 čeká na peněžní podmínku; osivo rajčat se přirozeně odemkne v čase140. Tutoriál19 vysvětlí rozšíření pole, ale fcu zůstává0. Nová zpráva vyvolá krok39: driver skutečně otevře deník a přečte všechny tři zprávy. Žádné profilové pole, čas, příznak ani herní instrukci test nepřepisuje.

Následují skutečné kroky13/14/15/16 a nevyřízený20. Původní krok13 dočasně poskytne cenu osiva20, kliknutí vybere rajčata a platné položení sáčku20 odečte. Konečné Money70, jeden zaplacený sáček kind1/count4/status0, PCarry6 a awQCollect9 souhlasí. Originál i zdroj přirozeně vytvoří i původní `tip4294967295=true`: RepeatedAction dokončí tip dvakrát. Tato zvláštnost zůstává zachovaná, nejde o testovací přepis.

Úspěšný pár `.local/seed-unlock/20261029T115458353557Z` má794 shodných názvů a typů vlastností. Liší se pouze růst/voda čtyř obdělávaných polí, GameTime/RunTime a letTime0; konkrétní hodnoty jsou v reportu. awQTask je v obou0 po dalším přirozeném časovém průchodu; přesný okamžik a izolovaná větev expirace kontraktu zatím nejsou tímto během prokázané. Binární save má2523 bajtů: jeden sáček, čtyři prodeje amount2, tři přečtené UTF16 zprávy a383 bitově shodných koeficientů. Při porovnání nezávislých běhů se vyjímají pouze bajty25..27 (původní neurčený padding),32..39/44..47/52..55/60..63 (cutoff a čtyři prodejní float32 hodiny). Všechny ostatní bajty souhlasí.

Osm tutoriálových oblastí obsahuje28600 RGB pixelů každá. Kroky39/14/15/16 jsou přesné (114400 pixelů); u11/19/13/20 je130/1976/167/53 odlišných pixelů s maximem1/2/1/1 kanálové jednotky. Animované pozadí není časově synchronizované; tyto oblasti se nevydávají za přesné pixelové porovnání. Původní neúspěšný průzkum s neobslouženým krokem39 zůstává v lokálním adresáři a není zahrnut jako úspěch.

## Obousměrné přenosy přirozeného sáčku

`tools/validate_game_seed_interop.py` provedl čtyři celé další procesy: native0→source2→native4→source5 a source1→native3. Každý skutečně obnoví menu/farmu, dostane fyzický Alt+Pause a normálně uloží při ukončení. Všech794 profilových klíčů/typů i diskrétních hodnot souhlasí. Ve všech čtyřech2523bajtových přenosech jsou bitově zachované všechny prodejní hodiny, cutoff, zprávy/read flags i383 koeficientů. Jedinými normalizacemi jsou tři původní padding bajty a samostatně ověřená původní změna X o+1 při každé obnově (121→122→123→124). Y280, kind1/count4/status0/unpaid0 se zachovají. Tento jev odpovídá původním0040fbb0/00412850 a parent origin-1; produkce jej zachovává.

Úspěšný přenosový běh je `.local/seed-interop/20261029T120912495565Z`. Předchozí čtyři procesy rovněž doběhly, ale závěrečný zapisovač reportu zastínil globální data proměnnou bytearray; úplný běh byl proto po opravě zopakován. Za doložený úspěch se považuje pouze nový kompletní report. Obě kontroly ověřily17 nezměněných originálů a skutečné cesty vlastních/originálních DLL. Produkční binárky se nezměnily; jde o porovnání zapojených systémů v celém EXE.

Metadata: `analysis/seed-unlock-state-validation.json`, `analysis/seed-interop-validation.json`. `tools/game_save_state.py` čte jen diagnostické kopie save, kontroluje hranice, počty i EOF a koeficienty porovnává jako raw uint32 bity.

```powershell
python tools/validate_game_seed_unlock.py
python tools/validate_seed_unlock_state.py
python tools/validate_game_seed_interop.py
```

Tyto opt-in běhy potřebují volný aktivní Windows desktop a existující přirozené baseline reporty. Následují skutečné rytí, setí, růst a sklizeň zakoupené plodiny, další osiva/rozšíření/výroba, dlouhé saves, všechny kontrakty, neověřené větve enginu a úplný Squall97. Celý projekt nadále není dokončený.
