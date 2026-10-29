# Skutečné procesy a obousměrné saves

Diagnostika už spouští celý původní EXE z jeho původního umístění, s jeho původními DLL. Produkční rekonstruovaná hra používá výhradně vlastní EXE/DLL a obnovený zdrojový kód. Původní soubory ani Ghidra projekt se nepřepisují.

## Izolace a reprodukce

Testovací Win32 `native-process-launcher` dočasně zastaví vlastní nově spuštěný referenční proces na vstupu PE, obnoví původní bajt i EIP a přesměruje jen jeho IAT položku systémové SHGetFolderPathA pro CSIDL0x8023 na unikátní testovací složku. Ostatní CSIDL předá skutečnému OS. Poté se debugger odpojí a běží původní vstupní bod, hra, grafika, vstup i zvuk. Nejsou nahrazené žádné původní herní funkce a originální EXE/DLL se nekopírují do implementace. Samostatný testovací CMake projekt není závislostí produkční hry.

Data.pack a cz.pack se zkopírují pouze jako datové prostředky do izolované pracovní složky reference. Test před i po cyklech ověřuje17 originálních hashů, kontroluje vlastní foreground před každým SendInput, skutečné cesty načtených DLL, normální ukončení i výstupní soubory. Při již běžícím okně originálu nový referenční běh odmítne.

```powershell
cmake --build build/windows --config Release --target DobryFarmar game-source-runtime-test --parallel 4
cmake -S tools/native_process -B build/native-process -G "Visual Studio 17 2022" -A Win32
cmake --build build/native-process --config Release
python tools/validate_application_runtime.py
python tools/validate_native_application_interop.py
python tools/validate_rich_save_interop.py
```

Tyto opt-in kontroly vyžadují volný Windows desktop a Pillow, nejsou součástí bezobslužného CTest. Poslední používá HUD snímek a report první source kontroly jako bránu do skutečné farmy.

## Úvodní farma

Pět celých procesů projde originál NativeAudit → zdroj → originál a zdrojový SourceOther → originál. Skutečný Start vytvoří/načte celý svět, pause/help/return/continue proběhne fyzickou myší a všechny procesy normálně skončí0. Tři předání mají beze změny všech1981 bajtů save a všech790 názvů/typů/hodnot profilových položek; vyvíjí se pouze GameTime/RunTime. Ověřeno je i rozhraní mezi plným původním Profile/FileManager a naším ProfileSession.

Dřívější scénář se snažil v každém cyklu otevřít Journal. V opakování jej po GameTime>6 převzala skutečná watering tutorial bublina; tyto lokální běhy zůstávají evidované jako neúspěšné. Interop test takový časový předpoklad nepoužívá. Journal má vlastní native UI porovnání a skutečný source runtime test. Metadata: `analysis/native-application-interop-validation.json`.

## Sáček, prodeje, zprávy a nenulové koeficienty

`game-source-runtime-test` vytvoří skutečný zdrojový svět se sáčkem kind2/count7/status0/unpaid, záznamem prodeje time5/amount17, cutoff-55, přečtenou vlastní UTF16 zprávou a383 koeficienty, z toho22 nenulových. Jde o explicitní diagnostický obsah vytvořený skutečnými source factory/save metodami; počet7 je testovací vstup, nikoli tvrzení o přirozeném nákupu ve hře.

Normální zdrojový hlavní proces nejprve načte tento profil a nechá vykonat skutečnou úvodní událost (Money20 a další zpráva). To odděluje regulérní herní změnu od serializace. Poté následují celé procesy originál → zdroj → originál nad stejným kompletním profilem/save.

Porovnávají se všechny skutečné2072 bajty každého dalšího save. Jediné normalizace jsou tři původně nespecifikované padding bajty v sáčku a samostatně ověřená původní změna X o+1 při obnově. Zachované jsou kind/count/status/unpaid, Y, prodej/cutoff, obě zprávy a read flagy a všechny383 float32 koeficienty včetně bitů. V profilu zůstává všech790 položek a hodnot kromě přirozeně rostoucích časů.

X ve zdrojovém zahřátém save126 přejde po nativním načtení/zápisu na127, ve zdroji128 a znovu v originálu129. Celý původní program tak potvrzuje dřívější instrukční stopu0040fbb0/00412850 a parent origin-1; produkční implementace tento původní jev zachovává. Metadata: `analysis/rich-save-interop-validation.json`.

## Přirozeně přijatý první kontrakt

Navazující [celé procesy zakázky](CONTRACT_RUNTIME.md) ověřily přijetí prvního přirozeného dopisu, Money22→2, aktivní task0=3/time120 a tutoriál17/18. Pět procesů porovnalo tři úplné2186bajtové přenosy native→source→native a source→native bez normalizace. Nezávislý počáteční pár má očekávaně různé prodejní hodiny; přesný rozsah a všech791 vlastností jsou v reportu. Neprokazuje zatím splnění zakázky nebo všechny rozehrané stavy.

## Další potřebná práce

Tato evidence dokládá konkrétní celé saves v obou směrech. Nepokrývá zatím všechny farmy, rozsáhlé historie/kontrakty/budovy, všechny počty a druhy sáčků, poškozené soubory ani libovolné dlouhé rozehrané stavy. Úplný tutorial, všechny herní systémy, native frame/reset/lifecycle, celé Squall97 a izolovaný odinstalátor dále pokračují. Projekt není dokončený.


## Přirozeně zakoupené osivo

[Skutečný nákup osiva](SEED_RUNTIME.md) přidal další čtyři celé přenosy2523bajtového save v obou směrech. Všech794 profilových klíčů/typů a diskrétních hodnot souhlasí; save má přesné prodejní hodiny, tři zprávy/read flags a383 koeficientů. Jen původní X+1 při obnově a tři neurčené padding bajty se vyhodnocují zvlášť. Rozsah nevydává celý herní svět za ověřený.


## Oseté pole a zestárlé prodeje

[Navazující přirozené průchody](SEED_RUNTIME.md) ověřily další čtyři2523bajtové přenosy osetého stavu. Osm dalších procesů porovnalo totožné vstupy v obou programech a původní mutaci historie3→2→1→1 transakcí (2515→2507→2499→2499 bajtů). Čtyři výsledné páry mají všechny smysluplné save bajty přesné, včetně souřadnice sáčku, hodin, zpráv a koeficientů; normalizují se jen tři původní padding bajty. Samostatně je zaznamenán původní časově závislý pohyb desetinné souřadnice při přechodech scény; nesynchronizované celé snímky tím nejsou označené za shodné.
