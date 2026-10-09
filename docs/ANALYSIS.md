# Reprodukce analýzy

Spouštět příkazy z kořene tohoto repozitáře. Nástroj používá izolovaný projekt v `work/ghidra`. Ghidra nepovoluje v cestě svého projektu adresáře začínající tečkou, proto projekt není uložen v `.local`.

## Windows

První použití již existujícího projektu hlavní hry:

```powershell
python tools/analyze.py --ghidra-home "C:\ghidra_12.1.4_PUBLIC" --java-home "C:\Program Files\Eclipse Adoptium\jdk-21.0.7.6-hotspot" --binary game --seed-project "..\farmář - Ghidra\Dobrý Farmář.gpr" --process "Dobr*.exe" --repair-game-flow
```

Pokud nemáš původní Ghidra projekt, vynechat `--seed-project` a `--process`; nástroj vytvoří nový projekt a importuje hlavní EXE.

Import dalších binárních souborů do pracovního projektu:

```powershell
python tools/analyze.py --ghidra-home "C:\ghidra_12.1.4_PUBLIC" --binary jngload
python tools/analyze.py --ghidra-home "C:\ghidra_12.1.4_PUBLIC" --binary squall
python tools/analyze.py --ghidra-home "C:\ghidra_12.1.4_PUBLIC" --binary uninstall
```

Pracovní projekt nesmí být současně otevřen v GUI. Původní projekt může zůstat otevřený, pokud během vytváření kopie neprobíhá jeho ukládání. Kopie se při následných bězích znovu nepřepisuje. Vlastník se upraví pouze v lokální kopii pro aktuální účet.

## Linux / WSL

```bash
python3 tools/analyze.py \
  --ghidra-home /mnt/c/ghidra_12.1.4_PUBLIC \
  --java-home /tmp/dobry-farmar-tools/jdk-21.0.7+6 \
  --binary jngload
```

JDK v `/tmp` je dočasný lokální nástroj. Pokud už neexistuje, předat cestu ke svému Linuxovému JDK 21+. Windows `java.exe` není Linuxový JDK.

## Opakovaný export

Po prvním importu používat například `--binary jngload --process JNGLoad.dll --no-analysis`. Bez `--no-analysis` proběhne i analýza. Import existujícího jména automaticky nepřepisuje analyzovaný program.

## Oprava dodaného herního projektu

Projekt obsahoval chybné příznaky „bez návratu“ u `__chkstk` (`005809d0`) a CRT `free` (`005803af`, přesměrování `00580b56`, implementace `00580b5b`). To zkracovalo herní funkce na prology a zahazovalo větve za uvolněním paměti. Instrukce návratu jsou ověřené přímo v původním EXE: `005809e4`, `00580a0c`, `00580bcb`.

Přepínač `--repair-game-flow` spouští před analýzou `RepairGameFlow.java`. Oprava kontroluje konkrétní SHA-256 původního EXE a adresy, obnoví návraty a pokračování volání, opraví těla volajících funkcí a vypne opětovnou automatickou detekci nevracejících se funkcí. Známé ukončovací funkce zůstávají označené. Změny zapisuje jen do pracovní kopie, s auditním záznamem `analysis/raw/game/flow-repair.json`.

První oprava samotného `__chkstk` zvětšila všech 830 dotčených těl. Následná oprava volání CRT zvětšila dalších 1494 těl. Počty se mohou překrývat a jejich součet není počet jedinečných herních funkcí. U jiné edice hry se tyto adresy nesmějí použít bez nového ověření.

## Výstupy

Každý program má podadresář v `analysis/raw`:

- `summary.json`: stav exportu, hash vstupu, počty úspěšných a neúspěšných dekompilací.
- `functions.jsonl`: adresa, jméno, podpis, velikost těla a volané funkce; jeden záznam na řádek.
- `pseudocode.c`: analytický pseudokód oddělený původními adresami.
- `symbols.json`, `strings.json`, `types.json`: symboly, texty s odkazy a známé typy.
- `headless.log`, `script.log`: diagnostika analýzy a exportu.

Stav `export_complete` znamená dokončený průchod funkcemi známými Ghidře. Neprokazuje, že Ghidra našla každý blok kódu, správně odvodila všechny typy nebo že jde o sestavitelný program. Chybné a chybějící funkce se musí dále řešit.
