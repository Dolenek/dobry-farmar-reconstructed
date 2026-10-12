# Obrazová větev mimo JNG

Dispatcher `00515380` rozhoduje podle prvního DWORD 0x474e4a8b, nikoli podle přípony. V původních balících je dalších devět JNG pod názvem `.dds` (osm v Data.pack, jedna font textura v cz.pack). Nový `ImageTexture` zachovává podpisové rozhodování a všechny tyto soubory posílá do stávající ověřené JNG větve.

`NativeImage` obnovuje storage čtečky používané deseti původními DDS/TGA/BMP daty: DDS `0059847a`, TGA `0059781c`, BMP/DIB `0059878d` → `00596f2d`. DDS zachovává format/pitch, level/face pořadí; TGA přímou barvu a grayscale, RLE, vertikální/horizontální orientaci; BMP neindexovanou BI_RGB větev, pitch zaokrouhlený na čtyři a obrácení kladné výšky. Původní BMP ignoruje bfOffBits a počítá adresu za DIB/paletou; zachováno. Ostatní formáty/větve se nevydávají za úspěšně dekódované obrazy.

Čtečky mají přímý oracle: vlastní 48bajtový image descriptor, privátní mapping EXE bez entry pointu/CRT a testové přesměrování původního operator new `0057edb1` do alokátoru diagnostiky. DDS nepotřebuje pro původní tři obrazy alokaci; TGA/BMP ji používají a vlastní adresář testu ji uvolní. Původní soubory se nemění. Všech deset obrazů, metadata i 345 368 source bajtů se shodují. Data zahrnují tři skutečné DDS (A8R8G8B8/A4R4G4B4), šest TGA včetně RLE a jeden X8R8G8B8 BMP.

`ImageTexture` má zatím obnovenou herní native image variantu D3DX: jeden mip, usage 0, MANAGED, původní source format při podpoře zařízení. Z `00592561` přebírá maximum rozměrů/aspect/SQUAREONLY/POW2. Pro FILTER_NONE zachovává kopírování přesahu a nulový Float4 mimo source z `00595901`. Není ještě plnou D3DX implementací: scoring format fallback, compressed texture upload, indexed/komprimované BMP, indexed TGA, JPEG/PNG/PPM a jiné větve čekají. Nepodporovaná větev má explicitní chybu místo náhradního obrázku.

Warehouse doplnil potřebu native A8R8G8B8 klikacích masek. Samostatný `click-mask-reference-test` nyní skutečně provádí původní D3DX `00592e55`, texture creation a surface filtering pro všech sedm shipped BGRA DDS/TGA. Celkem s JNG: PC53/PC24 po 432 masks / 81 099 735 shodných alfa bitech. Původní image probes i jejich indirect callbacks mají zdokumentované operand relocace, zkoušené při nuceném načtení reference mimo 00400000. Pouze CRT malloc/free a JNG GPU decode jsou diagnostické boundaries; native D3DX cesta nepoužívá zdrojový native loader jako referenci. Toto dokládá konkrétní BGRA/level-zero mask cestu, nikoli úplné D3DX, ostatní konverze ani všechny mip levels. Rozsah a reprodukce v [UI_CONTROLS.md](UI_CONTROLS.md).

Nativní GPU test nyní načítá přes ResourceManager všech 425 podpisových JNG a deset dalších obrazů: cache/refcount, formát/pool/mip, rozměry a UV, skutečný upload a preReset/restore, clearUseless. 81 014 421 JNG pixelů odpovídá původní JNGLoad.dll; dalších 86 346 source pixelů native obrazů odpovídá storage z výše porovnaného dekodéru. Cache a reload mají 435 kontrol. Použitá AMD Radeon TextureCaps=126021 nevyžadují POW2, takže native rozměry zůstaly původní. Přímé porovnání celého původního D3DX GPU loaderu, jeho fallbacku, paddingu po změně rozměrů a jiné grafické karty ještě neproběhlo.

```powershell
cmake --build build/windows --config Release
python tools/validate_native_images.py
python tools/validate_textures.py
```

Reporty jsou `analysis/native-image-validation.json` a `analysis/texture-validation.json`. Historická přímá JNGLoad validace 416 souborů s příponou `.jng` zůstává zachována; rozšířený GPU test navíc porovnal devět JNG s jinou příponou. Tři JPEG loga v cz.pack zatím nejsou podporována a nejsou zahrnuta do počtu 435.
