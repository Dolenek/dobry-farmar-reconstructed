# Rekonstrukce Squall

Původní `Squall.dll` má 97 pojmenovaných C exportů. Ghidra export obsahuje 916 funkcí: 911 dekompilovaných, čtyři přesměrování a jednu chybu v matematické x87 rutině `1003cf82`. Celou DLL zatím nelze sestavit.

## Zdrojový podklad

Pro rozbor je lokálně uložen zveřejněný [zdroj Squall 1.48](https://github.com/xtreme3d/squall/tree/20988426d2dde0a4b7aa6e18578f59c55d3af3a8), v `.local/squall-source`. Původní projekt a jednotlivé zdroje jsou čitelné a upravitelné; snapshot obsahuje vlastní MIT licenci. Jeho původ a hashe všech 107 souborů jsou v `analysis/squall-source.lock.json`.

Snapshot lze znovu získat jako [archiv připnutého commitu](https://codeload.github.com/xtreme3d/squall/tar.gz/20988426d2dde0a4b7aa6e18578f59c55d3af3a8), rozbalit bez nejvyšší složky do `.local/squall-source` a ověřit:

```powershell
python tools/inspect_squall.py
```

Hlavička obsahuje deklarace všech 97 původních názvů exportů a dvě další: `SQUALL_Channel_SetEndWorker`, `SQUALL_Sample_LoadFromMemory`. Mapa původních adres a referenčních deklarací je v `analysis/squall-source-match.json`. Deklarace popisují zveřejněnou verzi; ověření typů a ABI proti našemu EXE ještě není dokončené.

## Doložený rozdíl verzí

Porovnání inicializace s instrukcemi původní DLL prokázalo odlišnou hlasitostní křivku. V originálu vytváří `SQUALL_Init` (`10003280`) tabulku:

- index 0: `-10000`;
- indexy 1–99: zkrácení směrem k nule výrazu `10000 * log10(0.009F * index + 0.1F)`.

Konstanty jsou na adresách `10048188` (double 10000), `10048190` (float 0.1), `10048194` (float 0.009). Instrukce `fldlg2` a `fyl2x` realizují logaritmus o základu 10; následná `__ftol` zkrátí výsledek. Referenční zdroj používá `floor(5000 * log10(index / 99.0F))`. Například původní index 99 má hodnotu `-39`, kdežto referenční křivka vrací 0. Záměna celé knihovny za tuto verzi by proto změnila chování.

Původní tabulka je obnovena v `src/squall/VolumeTable.cpp` a sestavuje se jako `recovered_squall_utilities`. Test ověřuje všech 100 položek proti hodnotám nezávisle vypočteným s 70místnou desetinnou přesností z přesných konstant PE. Toto ověření dokládá převod algoritmu; přehrávání a porovnání běhu celé DLL dosud nebyly provedené.

## Zbývající práce

Zveřejněné zdroje pomáhají pojmenovat a pochopit původní rutiny. Každá použitá implementace se musí porovnat s inventarizovanou DLL; už první kontrola našla skutečnou odchylku. Zbývá obnovit kontext a kanály, DirectSound backend, WAV/ADPCM/MP3/OGG a další přítomné dekodéry, callbacky, 3D zvuk/EAX/ZoomFX, chybové stavy a celý exportní ABI. Zveřejněný projekt rovněž odkazuje na nepřibalené statické Ogg/Vorbis knihovny. Jejich původní verze zatím nejsou určeny.
