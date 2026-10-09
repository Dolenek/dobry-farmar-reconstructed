# Obnovené vykreslování spritů

`SpriteGeometry` obnovuje vrcholy a indexy z `00517f40`; `SpriteBatches` obnovuje sprite část této metody, změny dávek `00517ae0`, lock/list část `0051d2d0`, unlock/final batch část `00517010` a sprite draw větve `00515830`. Host stále řídí BeginScene/EndScene a prezentaci. Kompletní cGraphics, mesh příkazy, font overlay, reset a herní frame ještě čekají.

Sprite má 76 bajtů: čtyři rohy TL/TR/BL/BR s XY a ARGB, subdivision DWORD +30, shader resource +34, UV +38..44 a pointer na konstanty shaderu +48. Pole +34 není textura/maska a +48 není float. Vlastnictví konstruktorů/copy a původní container ABI konstant ještě čekají. Geometrie zachovává původní x87 spill místa, barevnou bilineární interpolaci, pořadí trojúhelníků a absolutní WORD indexy. Ve windowed odečítá 0.5 od XY; fullscreen používá měřítko/posun bez tohoto odečtu.

Dávka se mění při změně texture handle, DWORD flags, vertex shader handle nebo identity seznamu konstant. Záznam má texture/startIndex/triangles/vertices/flags/baseVertex/shader/constants a v originálu deváté slovo pro mesh. Obnovená sprite třída mesh příkazy nepřijímá. Bez změny sdílí sprites jednu dávku; nové startIndex/baseVertex navazují na předchozí počty. Buffer capacity a neplatné subdivision vstupy nejsou zatím ověřené.

| Flags | Původní sprite chování |
| --- | --- |
| 1 nebo 0x40 | SRCBLEND DESTCOLOR, DESTBLEND SRCALPHA; jinak SRCALPHA/INVSRCALPHA. |
| 2 | Vypnutí alpha blend a alpha test. |
| 4 | Stencil zápis ALWAYS/ref 1, KEEP při z-fail/fail, REPLACE při pass; ALPHAREF 136, ZERO/ONE blend; pak ALPHAREF 0 a stencil off před barevným průchodem. |
| 8 | Stencil EQUAL/ref 1/KEEP pro barevný průchod. |
| 0x20 | MAGFILTER i MINFILTER LINEAR; má přednost před 0x80. |
| 0x80 | MAGFILTER i MINFILTER ANISOTROPIC; výchozí jsou POINT. |

Fullscreen poté mění jen MAGFILTER: flags 1 → POINT, jinak ANISOTROPIC. Adresování U/V nastavuje `00514920` na CLAMP. Číselné stavy byly ověřené proti skutečné hlavičce D3D8 a nativním voláním: render state 24 je ALPHAREF, texture states 16/17 jsou MAGFILTER/MINFILTER. Dřívější interpretační chyby byly opraveny před úspěšným porovnáním. `GraphicsRenderState` zachovává také BLENDOP ADD, alpha funkci GREATER, material white/Power 7, ortografickou projekci a transponovaný view-projection. Matice mají 70 002 bitových porovnání s původními rutinami včetně aliasování. DesktopBorders obsahuje dvanáct černých vrcholů podle `00513130`.

`cShader` je 44bajtový prostředek typu 3: handle +24, vlastní 8bajtový bytecode buffer +28. Ctor `00557c30`, preReset `00557490`, restore `00557860`, dtor `00557400`; manager `00545a50/00543da0`. Vytvoření `0050d240` má deklaraci stream 0, v0 FLOAT3, v5 D3DCOLOR, v7 FLOAT2 a flags 0. Size/HRESULT se ignorují podle originálu. Původní destruktor nevolá DeleteVertexShader, pouze zničí bytecode a base resource; delete shaderu provádí preReset. Toto chování je zachováno. Runtime oracle samotné resource třídy a exception ABI ještě chybějí.

```powershell
python tools/build_sprite_oracle_fixups.py
python tools/build_sprite_oracle_fixups.py --draw
cmake --build build/windows --config Release
python tools/validate_sprite_geometry.py
python tools/validate_sprite_draw.py
```

Geometrie: 10 000 scénářů, 20 000 shodných výstupů vertex/index, subdivision 1..8, různá XY/barva/UV/base index, windowed i fullscreen matematika. Draw: 96 scénářů na nativním HAL SW_VERTEX_PROCESSING, dvě původní menu JNG textury přes obnovenou cache, všechny uvedené flags a kombinace, texture suppression, 32 scénářů s vlastním vs.1.1 fixture a konstantami c4/c5. Všechna API volání a triangle metric souhlasí; 46 080 000 RGB pixelů je přesně shodných. Backbuffer X8R8G8B8 nemá alfa kanál: jeho nepoužitý horní bajt se maskuje, RGB nemá toleranci. A8R8G8B8 readback by porovnával všechny čtyři kanály. Shader cache/refcount i preReset/restore byly skutečně provedené.

Oracle používá privátní mapping původního EXE bez entry pointu/import inicializace, testové fixups absolutních datových operandů a vlastní 680bajtovou graphics fixture s buffer/list/device pointery. Původní soubory se nemění a rekonstruovaný runtime nevstupuje do tohoto kódu. Moderní batching předává originálnímu draw ekvivalentní fixture seznam: tím není přímo ověřena původní alokace/přechod seznamu. Nebyla vytvořena fullscreen device ani měněn monitor mode. Nejde ještě o porovnání celých herních obrazovek, ostatních driverů, vstupu/zvuku/save systémů ani dokončenou hru.

Reporty: `analysis/matrix-validation.json`, `analysis/sprite-geometry-validation.json`, `analysis/sprite-draw-validation.json`; test-only operand manifesty neobsahují kopie programových instrukcí.
