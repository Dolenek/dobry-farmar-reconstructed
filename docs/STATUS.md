# Stav rekonstrukce

| Část | Stav | Důkaz / další práce |
| --- | --- | --- |
| Oddělený zdrojový repozitář | Hotovo | README, Git a pravidla ignorování. |
| Inventář CD podkladů | Hotovo | `analysis/inventory.json`: 17 souborů, SHA-256, importy a exporty, ZIP adresáře. |
| Izolovaná kopie Ghidra projektu | Hotovo | Lokálně v ignorovaném `work/ghidra`; originál se nemění. |
| Opakovatelný export Ghidry | Hotovo pro zjištěné funkce | Čtyři programy; výsledky a chyby v `analysis/export-status.json`. Neprokazuje úplnost ani sestavitelnost pseudokódu. |
| Oprava chybné analýzy hry | Provedena v pracovní kopii | Ověřené návraty `__chkstk` a CRT `free`, obnovené pokračování volajících funkcí. |
| Hlavní hra | Částečně | String/WString/Color, soubory/balíky/XML/nastavení, okno/message pump, resource cache, textury/JNG a nativní DDS/TGA/BMP, Direct3D8 prezentace, sprite geometrie/dávky/draw, matice, shader, bitmapové fonty s TextLine/Letter geometrií, StringManager a cView, cControl/cPicture a vlastněné TextLine/Letter s control sprity/animací. Button/AnimationButton s vlastněnými textovými variantami, JNG masky a DirectInput binding/polling. WinMM časovač s původním vyhlazováním a pozice myši přes Win32/window/fullscreen transformace. Propojená skladba hlavního menu s přechody a pixelovým porovnáním celé diagnostické scény. Obnovené hlavičky a třináct map vlastností profilů, UTF-16 persistence a lifecycle s 790 původními herními defaulty; skutečné profily napojené na source diagnostiku menu. Herní EXE, kompletní engine a systémy ještě neobnovené. |
| JNGLoad | Sestavená a porovnaná pro herní JNG | MSVC Win32 DLL ze zdrojů; rozměry a všechny BGRA bajty 416/416 JNG shodné s originálem, názvy/ordinály exportů a architektura ověřené. Další vstupy a exception ABI čekají. |
| Squall | Částečně | Obnovena původní tabulka hlasitosti; 97 exportů mapováno na zveřejněný referenční zdroj. Doloženy rozdíly verzí; celá DLL ještě není sestavitelná. |
| Odinstalátor | Částečně | Rozbaleno a dekódováno 137 instrukcí; mazací oddíl obnoven do NSIS. UI a další callbacky dosud nejsou sestavené. |
| Funkční herní sestavení | Není k dispozici | Žádné původní EXE není vydáváno za obnovený program. |
| Původní menu a nová hra | Potvrzeno uživatelem | Uživatelská zpráva „Menu i nová hra fungují“. |
| Porovnání obnoveného běhu s originálem | Provedeno pro dílčí části na Windows | JNGLoad: 416 obrázků pixelově totožných. String: 60 000 přímých porovnání. Celá hra zatím nemá rekonstruované EXE. |
| Dosavadní ověření kódu | Prošlo na Windows x86 | 35 CTest kontrol; Color: 121 538 přímých porovnání; matice: 140 004; sprite geometrie: 40 000; draw: 108 scénářů/51 840 000 RGB pixelů; fonty: 393 216; TextLine: 12 000 scénářů/88 583 úplných letter spritů. Lokalizace: 10 464 přímých porovnání; control sprity: 200 000, animace: 40 000; cView: 384 024 stavů a 32 000 registrů. Control/Picture: 400 020 stavů; TextLine/Letter lifecycle: 18 000 scénářů/169 056 písmen a spritů, 4 800 RTL, 600 destruktorů. Aritmetika Color/matice/geometrie/control/text lifecycle je porovnaná v defaultní i původní device x87 přesnosti; celá hra tím ověřená není. Původní DDS/TGA/BMP readery: deset souborů/345 368 shodných bajtů. Direct3D8 přes resource cache: 425 JNG uploadů/reloadů, 81 014 421 pixelů shodných s původní DLL, dále deset nativních textur/86 346 pixelů. Hlavní menu: 96 diagnostických snímků včetně hide/show přechodů, 46 080 000 přesně shodných RGB pixelů a draw API volání. Časovač: 120 000 původních snímků, 120 200 stavových porovnání a 200 konstruktorů/destruktorů. Menu používá obnovený časovač s řízeným clock backendem. Test používá řízené frame služby a neověřuje skutečnou navigaci/audio. Profily: 160 nativních lifecycle scénářů, 544 UTF-16 zápisů, 220 inicializací herních defaultů; celý herní save/load čeká. Úplný původní D3DX GPU loader a plný herní runtime čekají. |

## Prostředí

`CheckBox` pro FullScreen v Options je obnovený včetně odložené Tick viditelnosti a původního pořadí release/click/changed: 24 stromů, 2 968 stavů, 1 184 callbacků a 64 GPU snímků/30 720 000 shodných RGB pixelů. `CHECK_BOX.md` eviduje adresy a hranice; skutečný fullscreen/device reset zbývá.

Obrazovka `Credits` je obnovená ze skutečného UI/Credits.xml s ReturnButton a návratovým přechodem: 16 stromů, 412 stavů, 25 628 dekorací a 40 GPU snímků/19 200 000 přesně shodných RGB pixelů. Detaily a hranice MainMenu cíle jsou v `CREDITS.md`; opakované sound XML zachovává potvrzené původní zadržení referencí.

`ExitDialog` obnovuje YesNoExit s původním cursor/time pohybem tlačítka a Bee hover obrázky: 24 stromů, 14 512 stavů, 14 400 pohybových kroků a 64 GPU snímků/30 720 000 přesně shodných RGB pixelů. `EXIT_DIALOG.md` uvádí adresy a hranice; actual MainMenu exit/Win32 zavření stále zbývají.

Další obnovené části: `ListBox` a 2D copy chain pro profily, `MultilineText` a BackText/BackMultitext dekorace menu. YesNo controller: 34 vlastněných dialogů, 2 400 stavů, 1 940 callbacků a 64 celých GPU snímků/30 720 000 shodných RGB pixelů v obou přesnostech. SelectProfile nově propojuje skutečné profily, jejich přepínání/vytvoření/mazání a YesNo: 32 selectorů, 1 650 stavů a dalších 64 GPU snímků/30 720 000 přesně shodných RGB pixelů. Source zapisuje skutečné hlavičky/vlastnosti; nativní UI má explicitní profilovou service fixture a plný lifecycle se porovnává samostatně. Podrobnosti a neověřené oblasti jsou v `LIST_BOX.md`, `MULTILINE_TEXT.md`, `YES_NO.md` a `SELECT_PROFILE.md`. Propojení navigace hlavního menu, skutečné audio a funkční herní EXE stále zbývají.

- Původní Ghidra: `C:\ghidra_12.1.4_PUBLIC`, minimální JDK podle instalace: 21.
- Nativní JDK 21.0.7+6: `C:\Program Files\Eclipse Adoptium\jdk-21.0.7.6-hotspot`. Výchozí `java` na PATH je Java 8; nástroji analýzy předávat `--java-home`. Linuxový dočasný JDK se zde nepoužívá.
- Se souhlasem uživatele byly do VS 2022 Community doplněny MSVC x86/x64 a Windows SDK. Instalátor skončil kódem 0, bez restartu.
- Ověřené sestavení: MSVC 19.43.34810, Windows SDK 10.0.26100.0, CMake 4.4.1, Python 3.12.10; generátor Visual Studio 17 2022, Win32, Release. Nová cache je v `build/windows`.
- Původní hra nebyla z tohoto prostředí spuštěna; menu a zahájení nové hry ověřil uživatel u sebe na Windows.
- Linuxové výsledky jsou zachované v `.local/jng-validation-original-codecs.json`. Windows pixelové výsledky jsou v `.local/jng-validation-windows.json`; přenositelný souhrn v `analysis/jngload-windows-validation.json`. Exporty jsou v `analysis/jngload-windows-abi.json`, přímé porovnání String v `.local/string-validation.json` a `analysis/string-reconstruction.json`.
- Integrované načítání: `.local/resource-validation.json`, přenositelný záznam `analysis/resource-loading-validation.json`, postup a omezení v `docs/RESOURCE_LOADING.md`. XML parser kandidát Expat 1.95.8 nemá ověřenou původní verzi; neúspěšný pokus o původní parser oracle je evidovaný a není součástí normálního sestavení.
- Další propojení popisují `docs/SETTINGS.md` a `docs/WINDOW_GRAPHICS.md`; přenositelné reporty jsou `analysis/settings-validation.json`, `analysis/window-validation.json` a `analysis/texture-validation.json`. Direct3D8 testovací zařízení není vydáváno za obnovenou inicializaci cGraphics a neověřuje viditelné herní obrazovky.

## Pokrytí exportu Ghidry

| Program | Funkce zjištěné Ghidrou | Úspěšný pseudokód | Přesměrování | Chyby |
| --- | ---: | ---: | ---: | ---: |
| Hra po opravách toku | 28867 | 28819 | 48 | 0 |
| JNGLoad | 781 | 776 | 4 | 1 |
| Squall | 916 | 911 | 4 | 1 |
| Uninstall | 79 | 73 | 6 | 0 |

Herní počet zahrnuje 21448 kompilátorových `Unwind` a 3431 `Catch` rutin. Počet exportovaných funkcí proto nelze vydávat za počet rekonstruovaných herních metod. U obou DLL je zbývající chyba v x87 matematické rutině; adresy a diagnostika jsou v `analysis/export-status.json`. Pseudokód má stále neověřené typy a místy chybně rozpoznané podpisy.

## Podmínky dokončení

Celá hra a obě DLL se sestaví ze zdrojů bez kopírování původního programového kódu. Odinstalátor se sestaví z obnovené aplikační logiky. Běh s původními daty pokryje všechny herní systémy a zachová kompatibilitu uložených her. Každá zjištěná odchylka má zaznamenaný scénář a výsledek opravy.
