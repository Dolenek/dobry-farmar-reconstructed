<div align="center">

**Česky** · [English](README.en.md)

# Dobrý farmář

<img src="docs/media/game-icon.png" alt="Původní ikona hry Dobrý farmář" width="48" height="48">

**Česká farmářská hra pro Windows, znovu sestavitelná ze zdrojového kódu.**

Na malé farmě se staráte o pole, sklízíte úrodu, prodáváte na trhu a plníte zakázky. Z výdělku pořizujete lepší nástroje a výrobny a postupně rozšiřujete hospodářství. Prvními kroky vás provede původní tutoriál.

[Rozšíření farmy](docs/FARM_SCENE.md) · [Ocenění](docs/AWARDS.md) · [Trh](docs/MARKET.md) · [Výroba](docs/MANUFACTURE.md)

## Jak vznikla rekonstrukce

### ≈ 90 h 30 min · 226 commitů

Jediným modelem pro reverzní inženýrství a rekonstrukci zdrojů byl **GPT-6.1 Sol** s úrovní uvažování **High**. Agent analyzoval strojový kód v **Ghidře**. [Postup analýzy](docs/ANALYSIS.md)

Zadal jsem kritéria dokončení včetně obousměrné kompatibility uložených her. Agent měl jednotlivé části porovnávat s originálem a po dokončení každého celku zapsat změny do Gitu. [Historie commitů](https://github.com/Dolenek/dobry-farmar-reconstructed/commits/main/)

Výsledkem je upravitelný kód v C++. Hra používá původní grafiku, zvuky a další data z vaší kopie hry. Pravidla, ovládání, vzhled a načítání uložených farem agent porovnal přímo s originálem.

[Funkční hlavní menu](https://github.com/Dolenek/dobry-farmar-reconstructed/commit/8c6f98e1e6d6450d5bedf8531633165a691e2a45) · [Přenos uložených her](https://github.com/Dolenek/dobry-farmar-reconstructed/commit/4928fab169d28c065f876ddaffd508f57220bf85) · [Závěrečný audit](https://github.com/Dolenek/dobry-farmar-reconstructed/commit/eb0345bfb19ef7f14974e84dd4020cfadfa659a1)

## Jak jsme ověřovali shodu

| Výsledek | Co jsme porovnali |
| :---: | :---: |
| **48 kroků tutoriálu** | Všechny kroky dostupné při běžném hraní proběhly v originálu i v rekonstrukci. [Výsledky tutoriálu](analysis/current-tutorial-runtime-coverage.json) |
| **416 obrázků** | U každého obrázku JNG z původních herních dat se shodoval každý dekódovaný pixel. [Ověření grafiky](docs/JNGLOAD.md) |
| **97 funkcí zvukové knihovny** | Všechny veřejné funkce Squallu mají obnovený kód a ověřené rozhraní. [Ověření zvuku](docs/SQUALL.md) |
| **4 přenosy uložené hry** | Po dokončení 40 zakázek jsme přenesli uloženou hru mezi verzemi v obou směrech. Při každé obnově se shodovalo všech 17 369 bajtů. [Ověření přenosu](docs/CONTRACT_STREAK.md) |

Ze zdrojů se sestavuje hra, knihovna pro čtení obrázků, zvuková knihovna a odinstalátor. Všechny čtyři součásti používají 32bitové rozhraní Windows. Účinky odinstalátoru jsme ověřili v izolovaném prostředí. [Souhrn ověření](docs/COMPLETION_AUDIT.md) · [Známé odchylky a hranice ověření](docs/VERIFICATION_LIMITS.md)

## Jak si zahrát

<sub>Ke spuštění potřebujete vlastní, legálně získanou kopii hry Dobrý farmář. Veřejné vydání záměrně počítá s tím, že si původní herní data dodáte sami. Zkopírujte Data.pack a cz.pack do složky Data vedle Hrat.cmd. Spuštění vyžaduje oba soubory.</sub>

**1. [Stáhněte Dobrého farmáře pro Windows.](https://github.com/Dolenek/dobry-farmar-reconstructed/releases/latest/download/Dobry-farmar-Windows.zip)** ZIP obsahuje sestavenou rekonstrukci. Ke hraní potřebujete vlastní českou kopii původní hry.

**2. Rozbalte celý ZIP** do složky, kam můžete zapisovat, například do Dokumentů.

**3. Zkopírujte složku `Data` z původní hry** vedle `Hrat.cmd`. Musí obsahovat soubory `Data.pack` a `cz.pack`.

**4. Dvakrát klikněte na `Hrat.cmd`.** Hra vytvoří složku `Saves` pro profily a nastavení.

[Všechna vydání](https://github.com/Dolenek/dobry-farmar-reconstructed/releases) · [Sestavení ze zdrojů](docs/SOURCE_BUILD.md)

## Pro zájemce o zdrojový kód

[Přehled kódu hry](docs/GAME.md) · [Technické poznámky z vývoje](README.agent.md)

</div>
