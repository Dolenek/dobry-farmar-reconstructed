# Přenosné vydání pro Windows

Veřejný ZIP obsahuje zdrojově sestavený `DobryFarmar.exe`, rekonstruované `JNGLoad.dll` a `Squall.dll`, x86 knihovny Microsoft Visual C++, spouštěč `Hrat.cmd` a licenční oznámení. Hráč dodá `Data/Data.pack` a `Data/cz.pack` z vlastní české kopie hry. Spouštěč nastaví pracovní složku na adresář vydání a směruje profily a nastavení do `Saves`. Přenosná kopie se instaluje rozbalením a odstraňuje smazáním její složky.

## Vytvoření balíčků

Skript ve výchozím nastavení použije přesné soubory z doloženého sestavení `build/production-source-standalone-v4`. Pro nové [samostatné sestavení](SOURCE_BUILD.md) předejte jeho odpovídající ověřovací report přes `--build-report`. Připravte také tři x86 DLL z Microsoft Visual C++ Redistributable odpovídající [připnutým kontrolním součtům](../analysis/windows-release-crt.lock.json). Pro první vydání byly soubory vytaženy z podepsaného instalačního balíčku Microsoftu ve verzi 14.51.36247.0.

```powershell
python tools/package_windows_release.py --crt-dir "cesta/k/x86-crt"
python tools/package_windows_release.py --crt-dir "cesta/k/x86-crt" --local-original-dir "cesta/k/puvodni-hre"
```

První příkaz vytváří `dist/v1.0.0/Dobry-farmar-Windows.zip` pro GitHub Releases. Druhý vytváří `dist/local-complete/Dobry-farmar-Windows-komplet.zip` s původními daty pro místní použití. Oba přidají soubor `.sha256`. Složka `dist` je mimo verzované soubory.

Skript ověřuje kontrolní součty produkčních zdrojů a sestavených binárek, kontroluje architekturu Win32 a porovnává importy s exporty přibalených DLL. V úplném místním ZIP ověřuje také oba datové balíky. [Záznam ověření vydání](../analysis/windows-portable-release.json)
