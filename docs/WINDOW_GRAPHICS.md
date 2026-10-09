# Okno, prostředky a Direct3D textury

Tato část ještě není hlavní herní EXE ani celý cGraphics. Zdrojový kód propojuje skutečné Windows okno s obnoveným nastavením a skutečné Direct3D8 textury s FileIndex, cResource/cTexture a rekonstruovanou JNGLoad.dll.

## Okno a zprávy

`GameWindow.cpp`: vytvoření `00509ab0`, procedura `00508c50`, smyčka zpráv `005096a0`. Načítá `game.window_title`, `graphics.width/height/ShowSysCursor`, používá CS_CLASSDC, ikonu 101, černý brush, šipkový kurzor, styl 0x00ca0000 a zadané vnější rozměry bez AdjustWindowRect. Při prázdné třídě převezme titulek. RegisterClassEx/CreateWindow/DestroyIcon zachovávají původní pořadí; pomocník okno nezobrazuje.

WM_ACTIVATE porovnává celý WPARAM (včetně horního slova), WM_ACTIVATEAPP přijímá jako aktivní právě hodnotu 1. Volání input → main pause → sound pause mají původní pořadí. WM_CHAR předává znak vstupu, WM_MOUSEMOVE a SC_KEYMENU vracejí 0. SC_RESTORE/MINIMIZE/MAXIMIZE používají přesné hodnoty bez maskování. WM_DESTROY pošle WM_QUIT s nulou; neobsazená smyčka volá hlavní process bez čekání. `WindowTargets` představuje obnovené původní call sites; napojení skutečného vstupu, hry a zvuku ještě čeká.

Původní ikona je samostatné výtvarné médium extrahované z RT_GROUP_ICON/RT_ICON, nikoli kód. `tools/extract_game_icon.py` ověřuje originální hash a ukládá pouze ICO do ignorovaného `.local/game-artwork`. Manifest `analysis/game-icon.json` obsahuje původ a hash. RC dostává lokální ASCII jméno, aby nedošlo k chybě jeho zpracování českých cest.

Ověřeno 18 nativních kontrol: existující skryté okno, český titulek/třída, rozměry, styl a resources, zprávy a pořadí callbacků, přesné WPARAM/SC porovnání, konec smyčky. Přímé porovnání původního okna ještě neproběhlo. Chybějící šířka/výška v originálu ponechává nedefinovaný lokální výstup; rekonstrukce inicializuje nulu. Originální exception ABI a neplatná konfigurace nejsou označeny za shodné.

## Textury a reference

`JngTexture.cpp`, `0050f580`: rozměry z readMNG zaokrouhlí od 1 zdvojnásobováním, CreateTexture má jednu mip úroveň, usage 0, A8R8G8B8 (0x15), MANAGED (1). LockRect má flags 0; kopíruje pouze původní šířku/počet řádků s respektem k pitch, padding nevyplňuje. Výsledky API ignoruje stejně jako původní rutina, potom UnlockRect/freeData.

`cResource`: 36 bajtů, reference +4 začíná na 1, String +8; ctor `0050ae80`, dtor `0050ada0`, addReference `0050ae20`, releaseReference `0050ae30`, name `0050ae50`. Release jen sníží 32bitové slovo a vrátí signed count <1; sám nemaže. Vtable zachovává pořadí dtor, typ, preReset, restore, addref, release, name. Typ 0 pro texturu je ověřený na `005585b0` i ve správci `00544260`; sound typ 4 dokládá `00542fd0`. Prázdné výchozí reset metody jsou skutečná původní společná rutina `004bd820`, nikoli náhradní stub.

`cTexture`: 72 bajtů, původní ctor `00559150`, načtení `005580c0`, dtor `005585c0`. Handle +24, UV měřítka +28/+2c, půltexely +30/+34, alokované rozměry +38/+3c, rozměry obrazu +40/+44. `005589b0` uvolní a vynuluje handle, `00558d80` znovu načte původní jméno. Cache, refcount a její propojení s FileIndex a Settings obnovuje [RESOURCE_MANAGER.md](RESOURCE_MANAGER.md); další image formáty a ostatní třídy prostředků čekají.

Na AMD Radeon (TM) Graphics prošlo 416 textur, 78 710 388 pixelů přesně odpovídá původní DLL, maximum alokovaných rozměrů je 2048×2048. Kontrola používá skutečný HAL device a původní SDK argument 120. Srovnává obrazovou oblast, formát, pool, mip úroveň, UV/půltexely a znovunačtení po preReset. Originál je testovací pixelový oracle; runtime implementace používá pouze naši JNGLoad.dll a systémovou Direct3D8. Parametry testovacího zařízení jsou explicitní fixture a nejsou vydávané za obnovenou inicializaci cGraphics.

## Reprodukce a další práce

```powershell
python tools/fetch_direct3d_headers.py
python tools/extract_game_icon.py
cmake -S . -B build/windows -A Win32
cmake --build build/windows --config Release
python tools/validate_window.py
python tools/validate_textures.py
python tools/validate_graphics_presentation.py
```

Windows SDK 10 neobsahuje Direct3D8 hlavičku. Pět zdrojových/licenčních souborů je připnuto v `analysis/direct3d8-headers.lock.json` (MinGW-w64 v12.0.0, původ Wine, LGPL-2.1-or-later). Hlavičky se nemění, malý MSVC adaptér přidává deklaraci WINBOOL a __MSABI_LONG. D3D8 DLL se nestahuje, diagnostika používá systémovou 32bitovou DLL.

Zbývá kompletní inicializace/rendering/reset zařízení, D3DX obrazová větev, úplný rendering modelů/fontů a jejich zapojení do hry, ostatní resource třídy, viditelné herní obrazovky, původní runtime oracle a exception ABI. Padding texelů není porovnaný. Úspěšný upload nedokládá shodu výsledné vykreslené obrazovky.

## Parametry prezentace a alokace bufferů

`GraphicsPresentation` obnovuje přípravu D3DPRESENT_PARAMETERS a okna `0051b9f0`; `PrimaryGraphicsSettings` čte primární položky z první části `0051a410`. Jeho světelná část zatím chybí a není nahrazena nefunkčním backendem. V konstrukční části `0051c5d0` se uloží styl, zjistí desktop mode a vybere stencil D24S8, při nepodpoře D24X4S4; při druhém neúspěchu je původní chybový řetězec. Výřez 4:3 se počítá před načtením konfigurace oproti pevným 800×600, s původními float konstantami 1.3333333730697632 a 0.75.

Windowed větev vrací uložený styl, používá AdjustWindowRect, centrování podle desktop rozměrů minus konfigurované client rozměry, NOTOPMOST a SWP_NOACTIVATE|SWP_SHOWWINDOW. Backbuffer odpovídá konfiguraci, formát aktuálnímu desktopu, swap DISCARD, auto depth/stencil, jeden buffer a multisample NONE. Fullscreen nastaví styl 0, TOPMOST, desktop rozměry, flags NOACTIVATE|NOREDRAW, swap hodnotu 4 (COPY_VSYNC), interval IMMEDIATE. Při pozdějším návratu do windowed se interval nemaže; je to zachované původní chování. Flags parametru zůstávají nula z původní inicializace. Formát a rozměry nejsou vybírány podle konfigurace colorbit/refrashrate, protože tato rutina tyto položky nepoužívá.

`SpriteBuffers::resize` obnovuje `005105e0`: uvolní staré tři handles, index count = sprites×6, vertex count = sprites×4, INDEX16 buffer count×2 a vertex buffer count×24 s FVF 0x142. Usage 0x208 (DYNAMIC|WRITEONLY), první pool DEFAULT, při nenulovém HRESULT pokus MANAGED. Konečný HRESULT originál nekontroluje. Struktura vertexu je XYZ, DWORD barva, UV (24 bajtů). Naplnění indexů/vertexů a draw obnovují samostatné metody popsané v [SPRITE_RENDERING.md](SPRITE_RENDERING.md).

58 nativních kontrol propojuje původní Settings přes ResourceManager s createGameWindow, GraphicsPresentation a skutečným HAL SW_VERTEX_PROCESSING zařízením. Windowed CreateDevice se podařilo s původními 800×600 parametry a správným depth/stencil povrchem. Alokace a přealokace bufferů pro kapacity 3000→17→3000 souhlasí v rozměrech, formátu, FVF a usage. Fullscreen byla ověřena jako příprava parametrů a Win32 operace; žádné fullscreen zařízení ani změna režimu monitoru nebyly provedeny. Windows po původním SetWindowLong(style=0)/SetWindowPos doplnil WS_CLIPSIBLINGS (0x04000000); diagnostika tuto skutečnost uchovává v reportu místo předpokladu nulového výsledného stylu. Přímé spuštění původního cGraphics ještě neproběhlo. Přenositelný report: `analysis/graphics-presentation-validation.json`.
