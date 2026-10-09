# Správce nastavení

`src/game/settings/Settings.cpp` obnovuje datovou část `cSettings`. Konstruktor `0053ee00` načítá `sys/settings.xml`; `00538610` přebírá atributy prvních potomků XML jako položky sekcí. Názvy jsou citlivé na velikost písmen, opakované sekce se sloučí a pozdější atribut přepíše předchozí. Načtení nepřepisuje název souboru pro save a nemaže existující mapu. XmlLoader umožňuje načtení přes obnovený ResourceManager s původním přímým zničením XML po přečtení; propojení a jeho omezení popisuje `RESOURCE_MANAGER.md`. Singleton/vtable ABI cSettings ještě chybí.

| Adresa | Obnovená operace |
| --- | --- |
| `00539420`, `00539850` | Čtení a zápis WString. |
| `00538bb0`, `00539010` | Čtení a zápis String přes původní engine konverze. |
| `0053bcd0`, `0053c1d0` | Čtení a zápis signed int přes široký stringstream. |
| `0053c690`, `0053cb90` | Čtení a zápis unsigned int přes široký stringstream. |
| `00539c30`, `004d93b0` | Čtení hex ARGB barvy do čtyř floatů; prázdná hodnota nemění výstup. |
| `0053b300`, `0053b800` | Čtení a zápis bool přes široký stringstream bez boolalpha. |
| `00537240` | Seřazené kompaktní XML, bez escapování hodnot. |
| `0050ce30`, `0050ca60` | Zápis BOM FEFF a UTF-16 jednotek přes iFile. |
| `0040da20`, `0040daf0` | Sdílené konverzní buffery, CRT `%S`; narrow fallback CP_ACP/WC_NO_BEST_FIT_CHARS. |

Getter vrací existenci položky, nikoli úspěch převodu. Při prvním neúspěšném hledání vytvoří prázdnou položku a nezmění výstup. Další hledání již vrací true. Číselné čtení kopíruje po první NUL a ignoruje stav streamu. Zápis čísel používá výchozí formát streamu. Save přidává mezeru za názvem sekce i za každým atributem, nedoplňuje řádkování a zapisuje dvěma voláními iFile::write. Všechny tyto operace byly ověřeny v dekompilaci a původních instrukcích.

```powershell
cmake --build build/windows --config Release
python tools/validate_settings.py
```

Poslední kontrola: 14 nativních scénářů propojeného FileIndex → XML → Settings, bajtově přesný UTF-16 fixture save, jeho nezávislé rozparsování a znovunačtení; všech 24 položek z původního nastavení souhlasí s nezávislým XML modelem. Test píše pouze do dočasné složky v `.local/settings-validation-work`, původní profily nepoužívá. Report `.local/settings-validation.json` výslovně rozlišuje data originálu od přímého porovnání původních rutin.

Neověřené: běh původního cSettings; přetečení, neplatné číselné vstupy a změny locale mezi starým a novým CRT; ne-ASCII narrow konverze; další typy vlastností (vektory, barvy, plovoucí hodnoty); singleton/vtable a engine výjimky. Původní sprintf může přepsat 256bajtový buffer. Rekonstrukce tuto hranici kontroluje a chybu neprohlašuje za shodný původní exception ABI. Jde o zdokumentovanou odchylku mimo zatím ověřené vstupy, která ještě vyžaduje rozhodnutí podle reference.

Načítání 24 nastavení a jejich zápis samy neznamenají funkční herní okno, profily nebo kompatibilní herní save.

Původně popsané adresy int byly opraveny po rozboru CRT vstupu: `004d6610` čte long a kontroluje rozsah signed int; `004d67c0` používá unsigned extrakci. Běžné kladné hodnoty byly ověřené již dříve, záporné a přetečení vyžadují oracle starého CRT. Barvy používají manipulaci flags streamu dec→hex. Neplatný neprázdný barevný text má v originálu neurčený lokální packed DWORD; rekonstrukce inicializuje tento lokál na 0, což je evidovaná odchylka mimo platné vstupy.

`Color::fromARGB` (`00417f80`) přesně používá původní float konstantu `3b808081`. `Color::toARGB` (`004455e0`) omezí kanál na 0–1, násobí 255, přičte 0.5 a provede truncation. 60 769 přímých porovnání s původními čistými rutinami zahrnuje náhodné DWORD barvy, libovolné konečné kanály i sousední floaty kolem všech půlicích prahů. Report je v `analysis/color-validation.json`. Oracle pouze přemapuje 20 x87 operandů konstant v soukromé paměťové kopii reference; soubor EXE se nemění a jeho entry point ani importy neběží. Tato operace není součástí rekonstruované implementace.
