# Průběžný skutečný zásobovací cyklus

Přípravný originální průchod obnovil native výsledek100-sklizňového testu, prodal skutečné mléko, zaplatil kravín a všechny tři úrovně za1300 a skutečným drag sliderem průběžně doplňoval trávu. Následovalo36 sklizní,102 dalších výrob a94 prodejů mléka po2. Finální čítače jsou136/108, Money1392, zásoba trávy0, náklad0, loaded3, mléko12 a market milk1.119 dvojitě stabilních readonly snapshotů samostatně prokázalo konzervaci trávy, mléka a peněz, náklad nejvýše16 a aktuální zásobu kravína nejvýše12. Během držení slideru může skutečná výroba uvolnit další místo; součet transferů se proto nezaměňuje za statickou kapacitu12.

Ve skutečném originálu přirozeně vzniklo ocenění1 za100 výrob, jeho panel obsahoval39 glyphů. Výroba po potvrzení dál normálně pokračovala; finálních108 není vydáváno za změnu prahu100. Průchod také skutečně překročil první14400→11190 clock loop. Native proces skončil0 a watchdog se neaktivoval. Source herní porovnání se tímto pilotem netvrdí. Důkaz je `analysis/counter-supply-pilot-observations.json` a archivovaný původní driver/helper v jeho runtime adresáři.

Dva předchozí diagnostické pokusy zůstávají zachované: float místo celočíselných souřadnic myši a kód Pause0xC5 převzatý z pause-interop šablony místo Insert0xD2. Druhý zastavil clock, takže slider nemohl enabled-stepovat; nešlo o doloženou herní chybu. Opravený pilot již používá skutečné Alt+Insert15×.

`tools/validate_game_counter_supply_cycle.py` a `tools/game_counter_cycle.py` nyní provádějí delší nezávislé průchody obou celých EXE k nejméně1000 sklizním a1000 výrobám. Používají běžné harvest/warehouse/market/build/upgrade/slider akce; žádné cheats ani přepisování profilových čítačů. Každý přesun má vlastní bilanční kontrolu a nové modální ocenění se skutečně obsluhuje. Explicitní1800sekundový native watchdog a1700sekundová diagnostická mez zabraňují nekonečnému běhu.

Dokud neprojdou oba procesy, samostatný audit a obousměrné saves, delší rozsah se neoznačuje za ověřený. Produkční implementace ani17 originálních souborů se neměnily. Celý původní cíl pokračuje.
