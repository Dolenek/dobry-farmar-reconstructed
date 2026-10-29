# Herní řídicí objekt ocenění

Podmínka7 už má skutečné přirozené ověření v obou kompletních EXE: `tools/validate_game_tool_upgrades.py` navazuje na opravdu zaplacený sklad3 a koupí osm upgradů Dig3/Mow3/Water2 za3000. Nepoužije award suffix ani přímý zápis podmínky; původní manager sám otevře efekt/panel a vstup potvrdí OK/návrat. Obě hry mají všechna diskrétní profilová data i celých4089 save bajtů přesných. Nové návody46/47 mají57200 přesných RGB pixelů; návod29 má170 odlišných pixelů/max26 a není prohlášen za přesný. Metadata jsou `analysis/tool-upgrades-state-validation.json` a `analysis/tool-upgrades-regression.json`. Přirozené podmínky9/18, souběh ocenění při předčasném návratu a další úplné rozsahy pokračují.

`GameAwardManager.h/.cpp` obnovuje `cAwardsWindow` (`004c7d00`, 224 původních bajtů, singleton `0065f6e8`). Jde o řídicí objekt vytvářený cGame, který používá skutečné již obnovené `Awards`, `AwardEffect` a `AwardPanel`. Constructor přečte 21 podmínek profilu, načte základní View XML a nastaví `awardShow0..20` podle skutečného vlastnictví. `enabledStep` `004c8650` blokuje nové ocenění při pohybu scén nebo šesti původních modalitách, vyhodnocuje podmínky a spouští celý tok ocenění.

Vznik efektu pozastaví celý game View v normálním i absolutním kroku. Po animaci se zobrazí skutečná obrazovka a panel. Myší OK potvrzuje původní condition key a spouští let k ikoně. Po příchodu ve fázi 5 manager nastaví `awardShow`, obnoví vlastnictví ikony, povolí obrazovku a zruší efekt. Návrat `004c70e0` znovu povolí celý game. Native destructor `004c8630` čistí singleton před View; samostatný dosud nedokončený efekt sám nemaže, což zdroj zachovává.

Podmínky jsou rekonstruované z instrukcí. Zachovávají i zvláštnosti: farm upgrades vyžadují Warehouse 3, Water 2, Dig 3, Mow 3; `awQDestroy` vyžaduje již 20, přes název achievementu; „upgrade all manufacture“ ve skutečnosti vyžaduje **přesně sedm built příznaků** ve 20 slotech (`00493af0`). Market assortment zkoumá zásoby 0..18. Konečná podmínka čte původní condition keys 0..19, nikoli zobrazovací flagy.

## Přímé porovnání a hranice

`python tools/validate_game_award_manager.py` porovnává v PC53 a PC24 18 podmínek pod/přesně na hranici a 36 skutečných cyklů udělení ocenění. Dohromady prošlo 2144 stavů / 120 operací a 2012 celých D3D8 snímků / 965 760 000 přesně shodných RGB pixelů. Ocenění vzniká automatickým View krokem; efekt, obrazovka, přechod, panel, OK, uložení flagů, let, zrušení a návrat používají skutečné source/reference objekty a obyčejný vstup.

**Podmínky 7, 9 a 18 nejsou v tomto scénáři vyhodnocené.** Jejich flags jsou výslovný vstup „již vlastněno“. Implementace je připravená, ale ověření nad celým Warehouse/Water/Dig/Mow, Manufacture a Market světem čeká na cGame. Okolní game je View a dosavadní HUD/StateView diagnostika, nikoli hotová hra. Manager se mezi řízenými scénáři dočasně vypíná pro samostatné měření prahů; samotné cykly běží normálním frame dispatch.

Vstupy jsou rozdělené do šesti procesů na každou přesnost. Původní šestice ocenění vyčerpala po čtyřech dokončených cyklech monotónní testovací heap 805 306 368 bajtů. Limit se nezvyšuje; každý nový proces provádí tři ocenění. Referenční CRT statická inicializace, desetinné formátování/konkatenace a deferred discard používají dosavadní výslovné adaptéry. Původní těla callbacků manageru a panelu skutečně běží. UI audio je zde vypnuté; fyzické backendy mají samostatné audity.

Současně jsou opravené singleton lifetime báze HelpView a Journal pro chybu konstrukce. Osm source kontrol dočasně odebere XML položku z paměťového FileIndex, dvakrát zopakuje chybějící načtení a ověří nulový singleton, nezměněné registry View/vstupu/interakce/cache a texture reference. Index se obnoví přes scope cleanup; žádný původní ani datový soubor se nemění. **Native výjimečné CRT/unwind/throw ABI tím není prokázané.** Normální Help/Journal porovnání zůstává v úplné regresi.

Další práce je úplná konstrukce a vazby cGame, zbývající tři podmínky, hlavní EXE a skutečný běh s obousměrnými saves, celé pracovní cykly nad Farm, zbytek Squall a izolovaný odinstalátor. Projekt tím není dokončený.

Úplné Win32 sestavení a 94/94 CTest prošly za 451.46 s. Oba aktuální game-audio a oba PersonSounds buildy prošly; striktní zápis ověřil source/binary hashe a 17 nezměněných originálů. Celou regresi eviduje `analysis/game-award-manager-regression.json`.
