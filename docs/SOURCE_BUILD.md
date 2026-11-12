# Samostatné produkční sestavení

Nejnovější zdrojový strom po opravě JNGLoad je `work/production-source-20261107-v4` a jeho sestavení `build/production-source-standalone-v4`. `analysis/production-source-only-build-v4.json` dokládá všechny4 produkční PE32 cíle bez testů a bez původních programových binárek, oba původní DLL exportní kontrakty,48 dalších loader procesů/96 cyklů a skutečný nový profil i restart farmy s normálními výstupy0. Odinstalátor je znovu bajtově totožný s izolovaně ověřeným zdrojovým odinstalátorem.

Opakovaný v4 test deníku narazil na tutoriál1 již v první relaci: deník se ve skutečnosti neotevřel, přesto animace splnila starou slabou podmínku rozdílných pixelů. Neúspěšný report a oba snímky zůstávají zachované. Úspěšná v4 kontrola je proto označená `startup-only` a neprohlašuje nové ověření deníku; předchozí v3 skutečný panel/menu/help důkaz má přesně stejné herní zdroje, což recorder ověřuje. Tutoriál a celá herní integrace mají další samostatné skutečné native/source scénáře. Tento rozdíl rozsahu se nezakrývá změnou produkce ani úpravou profilových tip flagů.

Potřebné jsou Windows, Python 3, CMake 3.20+, Visual Studio 2022 s C++ Win32 nástroji a Windows SDK. Ověřený build používal MSVC 19.43.34810 a SDK 10.0.26100.0. Příkazy spusťte z kořene repozitáře. Skript pro vytažení ikony očekává původní soubor na relativní cestě `../farmář/Dobrý farmář/Dobrý farmář.exe`. Pokud už máte vytaženou ikonu na `.local/game-artwork/game.ico`, příkaz `python tools/extract_game_icon.py` přeskočte. Cestu k původním datům zvolíte při spuštění hry.

Před konfigurací stáhněte zdrojové závislosti. Skripty je ověří pomocí uložených kontrolních součtů.

```powershell
python tools/fetch_libmng.py
python tools/fetch_codecs.py
python tools/fetch_expat.py
python tools/fetch_minizip.py
python tools/fetch_direct3d_headers.py
python tools/fetch_squall_codecs.py
python tools/fetch_nsis.py
python tools/extract_game_icon.py
cmake -S . -B build/production -G "Visual Studio 17 2022" -A Win32 -T "v143,version=14.43.34808" -DCMAKE_SYSTEM_VERSION=10.0.26100.0 -DBUILD_TESTING=OFF -DBUILD_SQUALL_CANDIDATE=ON -DBUILD_SQUALL_REFERENCE_LAYOUT=OFF
cmake --build build/production --config Release --target DobryFarmar JNGLoad SquallCandidate FarmarUninstall --parallel 4
```

Výstup `build/production/Release` obsahuje `Dobrý farmář.exe`, `JNGLoad.dll`, `Squall.dll` a `Uninstall.exe`. EXE potřebuje obě DLL ve stejném adresáři. Hru lze spustit s `--data "cesta/k/původním/Data" --common-dir "cesta/k/vlastnímu/profilu"`. Data a ikona jsou původní média; původní EXE/DLL nejsou implementační závislostí. `extract_game_icon.py` jednorázově čte původní ikonu a lze jej nahradit dodáním již vytaženého `.local/game-artwork/game.ico`. NSIS kompilátor, runtime i pluginy se sestavují ze zdrojů. Originální odinstalátor nespouštějte v instalované hře pro diagnostické účely.

Toolset verze v příkazu označuje instalační adresář MSVC14.43.34808; samotný ověřený překladač hlásí19.43.34810. Skutečné v4 generované projekty používají `WindowsTargetPlatformVersion=10.0.26100.0`. Je vhodné tyto verze při reprodukci zadat výslovně, aby novější nainstalovaný toolset/SDK nepřevzal konfiguraci. Binární shoda různých verzí překladače se tím netvrdí.

`analysis/production-source-only-build.json` dokládá skutečné sestavení všech čtyř PE32 cílů z odděleného stromu `work/production-source-20261107-v3`, bez zpřístupněného originálního herního adresáře a bez referenčního stromu Squall1.48. Testy byly vypnuté, zdrojové závislosti zůstaly připnuté. Čistá konfigurace před opravou selhala kvůli generování herního RC uvnitř `BUILD_TESTING`; RC se nyní připravuje i pro samotnou produkci.

Stejné samostatné sestavení skutečně vytvořilo profil, spustilo farmu, otevřelo deník, nabídku a nápovědu, uložilo hru a obnovilo ji v dalším procesu. Oba procesy běžně skončily0. Deník se kontroluje před nápovědou na nové farmě: po obnově může po šesti sekundách vstup blokovat první tutoriál. Původní chybný test i neúspěšný pokus s Alt+Pause jsou zachované v reportu; Alt+Pause zastaví také kroky UI, a proto se pro tento test nepoužívá. Deník na snímku byl také vizuálně zkontrolovaný. Kompletní tutoriál má vlastní párové native/source ověření.

Odinstalátor má přesně stejný SHA256 jako již skutečně ověřený zdrojový odinstalátor v izolovaných runtime/UI/bootstrap testech. Přenos tohoto důkazu je výslovně založený na totožnosti všech bajtů; nové opakování jeho účinků se zde netvrdí. Samotný produkční build nedokládá úplnost herních systémů; souhrnná evidence zůstává v `analysis/verification.json`.
