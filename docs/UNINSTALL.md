# Obnovení původního odinstalátoru

Původní Uninstall.exe je NSIS 2.28 ANSI s výchozím pořadím instrukcí a solid-LZMA kompresí. Hlavička je na offsetu `0xd800`, rozbalená hlavička má 7848 bajtů; celý solid blok 37747 bajtů. Obsahuje 137 instrukcí, 120 řetězců, jeden odinstalační oddíl a českou jazykovou tabulku (1029).

```powershell
python tools/nsis_extract.py
```

Úplný dekódovaný program se šestiparametrovými instrukcemi a interpretovanými řetězci je v `analysis/uninstall-program.json`. Escape kódy NSIS jsou dekódované včetně českého `ý`; nelze je číst jako prosté nulou oddělené CP1250 řetězce.

Struktury, pořadí instrukcí, expanze řetězců a význam mazacích flagů byly porovnány s původními zdroji NSIS 2.28: [fileform.h](https://github.com/kichik/nsis/blob/v228/Source/exehead/fileform.h), [util.c](https://github.com/kichik/nsis/blob/v228/Source/exehead/util.c), [exec.c](https://github.com/kichik/nsis/blob/v228/Source/exehead/exec.c).

## Co původní program dělá

Oddíl začíná instrukcí 47. Rekonstruovaná aplikační logika je v `src/uninstall/original_removal.nsh`:

1. Rekurzivně odstraní instalační adresář `$INSTDIR`.
2. Z registru HKCU načte jméno složky nabídky Start; při prázdné hodnotě použije „Dobrý farmář“.
3. Odstraní zástupce této složky a zástupce hry, Špidly, her a pohádek na ploše.
4. Odstraňuje prázdné nadřazené složky nabídky Start až k `$SMPROGRAMS`, který samotný nemaže.
5. Pokud jsou nalezeny údaje PlayerName a jeho nabídky Start, odstraní i příslušný hráčský zástupce v podadresáři Games.
6. Odstraní klíč `HKCU\Software\Alawar\VirtualFarm` pouze pokud je prázdný; odstraní odinstalační registraci hry v HKLM.

Další instrukce obsahují nastavení UI, inicializaci plug-in adresáře a jazykového výběru. Řetězce obsahují také nabídku TrialPay. Samotná přítomnost těchto řetězců neprokazuje, že se tato nabídka skutečně zobrazí nebo že program otevírá síťové spojení.

## Stav

Obnovené mazací jádro není připojené k sestavovanému programu a nebylo spuštěno. Původní názvy a klíče jsou zachované pro rozbor; před vytvořením odinstalátoru nové distribuce se musí použít její vlastní instalační identifikátor a cesta. Rekonstrukce ostatních callbacků, prostředků UI a izolované porovnání účinků jsou ještě potřeba. Výstup zatím není úplné sestavení Uninstall.exe.
