# Původní JPEG loga a barvy

`NativeJpeg.cpp` rozšiřuje produkční NativeImage/ImageTexture o JPEG vstup. Všechna tři dodaná loga (`Logo.jpg`, `Logo/Logo2.jpg`, `Logo/Logo3.jpg`) jsou v původním `cz.pack`. Jejich absence v hlavním Data.pack není důvodem přeskočit úvod hry.

Původní reader `005975c7` používá IJG61 a424bajtovou jpeg_decompress_struct. Samostatně připnutý [archiv IJG6a](https://www.ijg.org/files/jpegsrc.v6a.tar.gz) má SHA256 `0e963fdd29d5049f61f3dc137bdc9437473a0ad505dbcfc8925f0fcad46c5929`; hashe všech132 souborů jsou v `analysis/codecs.lock.json`, načítá je `tools/fetch_codecs.py`. Konfigurace přebírá připnuté Microsoft jconfig.vc z6b (6a tento preset nemá), včetně byte boolean; ABI61 a velikost424 ověřují compile-time assertions. JNGLoad si ponechává oddělenou již ověřenou6b knihovnu.

Samotné IJG6a ani6b nevytvářejí shodné barvy: první logo se lišilo v39088 bajtech o1 úroveň. Originál používá vlastní MMX převod `005a7b11/005a79d4`: v8pixelových skupinách počítá floor s koeficienty R=359/256, B=454/256 a G=(-88Cb-183Cr)/256 po odečtení128 od chroma, bez půlbodového biasu. `JpegColor.cpp` obnovuje tuto aritmetiku i saturaci obyčejným celočíselným C++; CPUID leaf1 EDX bit23 odpovídá `005a1b47`. Scalar tail používá oddělené16bitové IJG koeficienty; tyto vstupy zatím nejsou porovnané. Dekodér předává nezměněné upsamplované YCbCr do obnoveného převodu. Výstupní úložiště je X8R8G8B8 s nulovým X bajtem, nikoli neprůhlednou alfou.

## Ověření

`python tools/validate_jpeg.py` porovnává vPC53 iPC24 každé z3 log:782080 shodných dekódovaných bajtů, rozměry, pitch, formát a195520 texelů celých alokovaných GPU textur vůči skutečnému `00592e55`. Porovnává také všech16777216 Y/Cb/Cr trojic přímo s původní MMX rutinou. D3DX a dekodér při testu zachovávají skutečné původní callbacky; implementace původní programový kód nepoužívá. Původní EXE je mapovaná reference bez entry pointu, vynuceně přemístěná mimo00400000. Opravené testovací relokace pokrývají DQT zigzag, main-buffer callbacky, indexované Huffmanovy tabulky a X8 konvertor; CRT allocation/free/memmove mají výslovný adaptér.

CTest `original-jpeg` ověřuje tuto cestu. Existující texture/resource-cache test nově zahrnuje i3 JPEG včetně cache a pre-reset/reload vedle425 JNG a10 ostatních původních textur.

## Co není ověřeno

Porovnání pokrývá původní tři JPEG, aktuální MMX CPU a oba device/FPU kontexty. Neprokazuje celou aplikaci ani všechny JPEG varianty. Pokus izolovat nativní non-MMX cestu skončil chybou diagnostiky před porovnáním, takže tuto větev neoznačujeme za ověřenou. Čekají grayscale, CMYK, progressive, malá šířka/scalar tails, poškozené a zkrácené vstupy, native exception ABI a obecné D3DX fallback/filtering. Zdrojová paměťová čtečka při opakovaném EOF ukončí dekódování chybou; původní callback znovu poskytuje původní buffer, jehož chování pro poškozené vstupy není prokázané.

Dále pokračuje skutečný StartupView a kompletní engine/main EXE s profily, Loading a cGame. Původní binárky slouží pouze jako testovací reference.

Úplné Win32 sestavení a97/97 CTest prošly za 466.60 s. Oba game-audio a PersonSounds buildy i aktuální Squall PCM/MDCT/runtime a oba DLL audity prošly; striktní zápis ověřil zdrojové/binární hashe a17 zachovaných originálů. Regrese: `analysis/jpeg-regression.json`.
