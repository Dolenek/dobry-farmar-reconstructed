# Samostatné produkční sestavení

Potřebné jsou Windows, Python3, CMake3.20+, Visual Studio2022 s C++ Win32 nástroji a Windows SDK. Ověřeno: MSVC19.43.34810, SDK10.0.26100.0. Před konfigurací připravte zdrojové závislosti ověřované připnutými hashi:

```powershell
python tools/fetch_libmng.py
python tools/fetch_codecs.py
python tools/fetch_expat.py
python tools/fetch_minizip.py
python tools/fetch_direct3d_headers.py
python tools/fetch_squall_codecs.py
python tools/fetch_nsis.py
python tools/extract_game_icon.py
cmake -S . -B build/production -G "Visual Studio 17 2022" -A Win32 -DBUILD_TESTING=OFF -DBUILD_SQUALL_CANDIDATE=ON -DBUILD_SQUALL_REFERENCE_LAYOUT=OFF
cmake --build build/production --config Release --target DobryFarmar JNGLoad SquallCandidate FarmarUninstall --parallel 4
```

Výstup `build/production/Release` obsahuje `Dobrý farmář.exe`, `JNGLoad.dll`, `Squall.dll` a `Uninstall.exe`. EXE potřebuje obě DLL ve stejném adresáři. Hru lze spustit s `--data "cesta/k/původním/Data" --common-dir "cesta/k/vlastnímu/profilu"`. Data a ikona jsou původní média; původní EXE/DLL nejsou implementační závislostí. `extract_game_icon.py` jednorázově čte původní ikonu a lze jej nahradit dodáním již vytaženého `.local/game-artwork/game.ico`. NSIS kompilátor, runtime i pluginy se sestavují ze zdrojů. Originální odinstalátor nespouštějte v instalované hře pro diagnostické účely.

`analysis/production-source-only-build.json` dokládá skutečné sestavení všech čtyř PE32 cílů z odděleného stromu `work/production-source-20261107-v3`, bez zpřístupněného originálního herního adresáře a bez referenčního stromu Squall1.48. Testy byly vypnuté, zdrojové závislosti zůstaly připnuté. Čistá konfigurace před opravou selhala kvůli generování herního RC uvnitř `BUILD_TESTING`; RC se nyní připravuje i pro samotnou produkci.

Stejné samostatné sestavení skutečně vytvořilo profil, spustilo farmu, otevřelo deník, nabídku a nápovědu, uložilo hru a obnovilo ji v dalším procesu. Oba procesy běžně skončily0. Deník se kontroluje před nápovědou na nové farmě: po obnově může po šesti sekundách vstup blokovat první tutoriál. Původní chybný test i neúspěšný pokus s Alt+Pause jsou zachované v reportu; Alt+Pause zastaví také kroky UI, a proto se pro tento test nepoužívá. Deník na snímku byl také vizuálně zkontrolovaný. Kompletní tutoriál má vlastní párové native/source ověření.

Odinstalátor má přesně stejný SHA256 jako již skutečně ověřený zdrojový odinstalátor v izolovaných runtime/UI/bootstrap testech. Přenos tohoto důkazu je výslovně založený na totožnosti všech bajtů; nové opakování jeho účinků se zde netvrdí. Samotný produkční build nedokládá úplnost herních systémů; souhrnná evidence zůstává v `analysis/verification.json`.
