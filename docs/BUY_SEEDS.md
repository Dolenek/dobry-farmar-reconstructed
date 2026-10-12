# Nákupní panel osiva

`src/game/ui/BuySeeds.cpp` obnovuje skutečný panel `cBuySeeds`, native 1 332 bajtů, singleton `0065ff64`. Vychází z již obnoveného MovableUnion a vytváří cancel TextAnimationButton, sedm AnimationButton se seed id, sedm TextMoney/coins a sedm MultilineText popisků. Všech osm 68bajtových definic obsahuje WString a vlastní AnimableAppearance; index 0 se při konfiguraci a nabídce přeskakuje.

| Adresy | Chování |
| --- | --- |
| `00420cb0`, `004200f0` | Konstrukce, callbacky, resource references, input unbind a vlastnictví |
| `004201f0`, `0041d9a0`, `0041d9f0`, `0041da60` | Skutečné XML, umístění, definice name/State a sedm buněk |
| `00420f50` | SeedAvail, ceny, dostupnost, texty, input binding a příjezd panelu |
| `0041d580`, `0041d5e0` | Odjezd, zrušení a obnovení HUD |
| `0041d610` | HUD enable, zavření, zvuk a skutečné Farm chooseSeed volání |
| `0041d520` | Původní tutorial 14 a 15 přes skutečný TutorialManager |
| `0041e050`, `00417910`, `0041e4c0` | Cursor test a callback stisku mimo panel |
| `00403740` | Ceny ze skutečných SeedInfo cGame/configuration |

`SeedAvail` jsou uints na `0065ff44`, nikoli ints. Cena se zobrazuje jako unsigned decimal, ale pro dostupnost se porovnává s Money jako signed int32. Dostupné druhy 1..7 se postupně přiřazují slotům 0..6 a zapisují do jejich seed id. Při prázdné nabídce původní hide smyčka přeskakuje slot 0, takže může ponechat jeho předchozí obsah. Source tuto zvláštnost zachovává.

ItemPlace definuje sedm absolutních pozic. ItemProto se aplikuje na všechny buňky, poté se přičítá umístění tlačítka a popisku a celý panel se přesune na HiddenPlace. Opakovaný BuySound v XML přepisuje pointer bez uvolnění předchozí reference; destructor uvolní pouze současný zvuk. Osmý ItemPlace by v native layoutu přepsal začátek první definice; source neověřený inconsistent vstup explicitně odmítá.

Otevření samo nepauzuje HUD; dělá to volající herní controller. Close pouze unbinduje mouse press `0x208`, vypne panel a zahájí odjezd. Cancel obnoví normální a absolute HUD před Close. Choose zachytí id, obnoví HUD, zavře panel, přehraje BuySound a volá Farm `0042c960`. Neodečítá zde peníze ani nevytváří odhadnutý crop: skutečná Farm/seed mover implementace musí být dodána rozhraním `SeedPurchaseWorld`.

Po zastavení panelu se nejprve nabídne disabled-anchor tutorial 14; po jeho zpracování repeated-action 15 kotvený k prvnímu buy buttonu. Manager a jeho skutečná okna jsou v diagnostice plně propojené. Výběr prvního osiva završuje 15 včetně původního druhého dokončení kroku −1.

Původní `0041e050` má doložené neurčené čtení paměti: `0041e140` iteruje přes indexy 1..7, přestože existuje jen sedm slotů 0..6. Poslední čtení vezme BuySound pointer na `+530`, považuje jej za Button a čte `+238` za koncem ověřeného 40bajtového cSound. Source zachovává konkrétní původní čtení, nevytváří náhradní slot ani neodhaduje hover výsledek. Tato větev je závislá na okolním heapu; její bezpečnost ani ekvivalence mezi samostatnými procesy nejsou prokázané. Cursor oracle sdílí stejný skutečný zvukový objekt pro source/native, což je výslovně uvedeno v metadata. Výsledek těchto sond není důkazem celého herního chování této chyby.

`python tools/validate_buy_seeds.py` porovnává PC53/PC24: 1 696 stavů, 264 otevření, 176 pozorovaných předání osiva Farm a 39 276 porovnaných písmen. Zahrnuje všech 128 kombinací dostupnosti sedmi druhů, skutečné config ceny, signed Money gates, owned kontroly/popisky a profil. 160 cursor sond porovnává přesné float spill pořadí, původní skipped-first-slot chování a uvedený shared-resource memory path.

248 GPU snímků / 119 040 000 shodných RGB pixelů zahrnuje skutečné hover/press/release na první a poslední osivo i cancel a oba propojené návody 14/15. Před těmito dvěma návody má kontrolovaný profil zpracované starší HUD návody; nejde o tvrzení, že je ověřen celý tutorial sequence. Farm `0042c960` je v diagnostice pozorovaná hranice a nastavuje kontrolovaný seed-move signál; tato hranice není produkční náhradou Farm.

Zbývá skutečný seed mover, jeho umístění, půda, peníze a cGame, celý průchod při hraní, obousměrné saves, konkrétní heap chování uvedené chyby v oddělených programech, XML reload/error/extreme cost matice, fyzický vstup a slyšitelný celý herní zvuk. Hlavní herní EXE, úplný Squall a izolovaný odinstalátor nejsou dokončené.

Návazné porovnání BuyBuilding doložilo chybu původní source interpretace zápisu na +1b0: jde o dirty flag cControl, nikoli deepProcessing (ten je +26a). Open nyní správně volá markTransformDirty a zachovává XML nastavení deepProcessing. Oracle přímo kontroluje oba příznaky; starší dílčí report tuto záměnu nekontroloval a jeho úspěch nebyl důkazem správnosti tohoto zápisu.
