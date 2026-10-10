# ProgressBar a přednačítání

`ProgressBar` obnovuje původní 584bajtový prvek: konstruktor `004f91e0`, destruktor `004f9230`, transformaci `004f9290`, posun víčka `004f9b20` a XML `004f9f80`. Upravitelný kód je v `src/game/ui/ProgressBar.cpp` a `ProgressSprite.cpp`; moderní layout objektu není původní ABI.

Transformace ořezává atlas i geometrii podle current/max a Side. Původní Side 3 posune horní vrcholy při ořezu spodního UV; tuto zvláštnost kód zachovává. InvertX/InvertY a doubleRender se při výpočtu tohoto spritu neuplatní. Rozdílné x87 spill body kosinu, sinusu a přičtení pozice jsou zachované. Chybějící Side v XML přepíše konstruktorovou hodnotu 2 na 0. Opakované načtení Lid vytvoří další vlastněný Picture a starý prvek ponechá mezi potomky až do destrukce.

`python tools/validate_progress.py` porovnává v PC53 i původní device PC24 celých 76 bajtů spritu a current: 200 000 případů. Celý prvek používá skutečné `UI/Load.xml`, původní texturu a explicitní testovací XML s Lid: 24 vlastněných stromů, 7 368 stavů a 48 reloadů včetně reference/destruktoru. Přímý native/source frame na Direct3D8 dává 96 snímků, 46 080 000 přesně shodných RGB pixelů a shodná draw volání. Originální PE se mapuje pouze v diagnostice, jeho entry point neběží. XML/allocator/resource hranice jsou stejné dokumentované adaptéry jako u ověřování ostatních UI controls.

Výsledek je v `analysis/progress-validation.json`. Neověřuje úplný herní runtime, původní objektové/exception ABI ani obecné malformed XML/NaN životní cykly. Navazující `LoadingFromLog` a jeho resource log/music lifecycle jsou obnovené a ověřené v rozsahu [LOADING_FROM_LOG.md](LOADING_FROM_LOG.md). Nová hra stále vyžaduje skutečný `cGame` a herní systémy; ProgressBar sám tento cíl nesplňuje.
