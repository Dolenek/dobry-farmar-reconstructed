# Původní cFlyText a cGame efekty

`FlyingText` obnovuje cFlyText (760 native bytes) a `GameEffects` skutečné product/money expense/income factories. Metody nečtou ani nevolají původní herní binárku. Root cGame View vlastní nově vytvořené objekty přes původní View ownership; jejich dokončení používá skutečný source deferred discard.

| Adresa | Obnovené chování |
| --- | --- |
| `0047cc50` | TextLine base, speed=50, total/current distance=0, position/alpha index=1, callback/argument=null, prázdné point vectors a icon=null. Jen nonempty XML path načte definici. |
| `0047c250` | TextLine XML → nový Picture child → Icon XML → Speed. PositionPoints resize na zero 20-byte records, cumulative distance a previous velocity podle původních x87 spillů; dvě points a nonzero distance jsou povinné. AlphaPoints resize na zero 12-byte records, previous alpha delta, dvě points povinné. Root first position/alpha a icon first alpha. |
| `0047d590` | TextLine step, další timer query, movement distance a original index walk, segment interpolation, independent alpha curve a dirty flags. Dokončení kontroluje unspilled distance. Duplicate alpha distances jsou přeskočeny index walkem. |
| `0047d4d0` | Last position a alpha, případný callback(argument) a následné vymazání pouze callback word, deferred discard s empty origin. Step pokračuje alpha částí i po finish. |
| `0047b8e0` | Skutečné copyAppearance do icon a explicitní dirty. |
| `0047ba00` / `0047baf0` | Alpha vector → position vector → TextLine/owned children; scalar destructor může uvolnit celý objekt. |
| `0040ef00` | Nový FlyProductExpence(parent cGame), text **1**, malé product appearance a dirty icon. |
| `0040cf20` / `0040d450` | Nový FlyTextExpence/FlyTextIncome(parent cGame), decimal unsigned amount bez odhadnutého minus/plus prefixu. |

Native primary vtable `005f5a50` má 72 entries a MI `005f5a44` dvě. Layout od +2b8: speed, total distance, distance, position index, alpha index, completion, completion argument; positions allocator+2d4/begin+2d8/end+2dc/cap+2e0, alpha allocator+2e4/begin+2e8/end+2ec/cap+2f0, icon+2f4. Modern source layout není původní object ABI. Reload zachovává aktuální distance, oba indices i callback a ponechá starý icon jako View-owned child. XML Lifespan=2 se původní třídou **nečte**; lifecycle určuje speed/path. Tyto native quirks jsou zachovány.

PC53/PC24 standalone oracle prošel 696 stavy, 12 konstrukcemi/destrukcemi, 12 XML reloady, šesti single-shot callbacks a 288 úplnými GPU snímky / 138 240 000 shodnými RGB pixely. Všechny tři skutečné definice UI/FlyProductExpence.xml, UI/FlyTextExpence.xml a UI/FlyTextIncome.xml používají original textures/fonts. Souhlasí všechny position/alpha record words, current indices/distance, glyphs, icon geometry/alpha, callback clearing, retained children, discard origin a reference deltas při skutečném source deleteDiscarded.

Propojený `flying-market` režim používá skutečné Market sales → GameEffects.productExpense → FlyingText a zároveň skutečnou SalesHistory/awEarn. V referenci mají cGame expense i history metody původní těla. Bylo vytvořeno 366 actual effects, ověřeno 1 752 skupinových stavů, actual icon copy, obě money visual factories s šesti uint amounts, 72 explicitních effect steps na přesnost a skutečné odložené odstranění. Zachováno je 1 584 Market stavů a jeho 168 snímků / 80 640 000 pixelů i propojené Warehouse/Manufacture/HUD/WareDetail/Gameplay/TutorialManager.

Hranice propojeného testu jsou výslovné: jeho diagnostický game View zůstává hidden kvůli dosavadním world fixtures; owned effects se krokovaly explicitně. Jejich visible rendering dokazuje standalone whole-frame oracle. Úplné cGame root/world sestavení a setMoney refresh všech objektů tím tvrzeno není. Native allocations se pouze pozorují pro identifikaci nových effect objects; factory, ctor, XML, movement a appearance metody se nenahrazují. Old CRT/resource/XML/timer/input/allocator hranice zůstávají popsané v metadata.

Reprodukce: `python tools/validate_flying_text.py` a `python tools/validate_flying_text.py --connected`; CTest `original-flying-text`/`original-flying-market`, reports `analysis/flying-text-validation.json`/`analysis/flying-market-validation.json`. Zbývá celé cGame/world, všechny Money refresh efekty a hlavní game loop, audible playback, full saves/bidirectional compatibility, malformed/missing/zero paths, NaN/inf movement a allocator/exception ABI. Projekt stále není dokončený.

Navazující kompletní Win32 Release regrese prošla všech 73 CTest kontrol za 301,20 s. Oba physical game-audio audity canonical/independent prošly po 378 stavech / 36 playback / 16 pause kontrolách. Strict record znovu ověřuje source hashe a všech 17 originálů.
