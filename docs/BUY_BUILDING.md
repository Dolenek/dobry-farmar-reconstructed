# Nákup staveb

`src/game/ui/BuyBuilding.cpp` obnovuje cBuyBuilding, native 1 700 bajtů, singleton `00660010`. Panel je skutečný MovableUnion a obsahuje jedenáct buněk: PictureButton s id stavby, ExtendedButton masku s nápovědou, TextMoney a minci, TextLine popisek. Jedenáct definic má po 68 bajtech a obsahuje WString a AnimableAppearance. Původní buňka má krok 20 bajtů; její páté slovo se v konstruktoru neinicializuje a v doložených metodách nepoužívá.

| Adresy | Obnovené chování |
| --- | --- |
| `00491690`, `0048edf0` | Konstrukce, singleton, callbacks, input unbind a vlastnictví |
| `0048eef0`, `0048df00`, `0048df50`, `0048e030` | XML, všech jedenáct definic, místa a ItemProto |
| `004928e0` | UpAvail, ceny, tutorial sleva, ikony nápovědy a příjezd panelu |
| `0048e130`, `0048e550`, `0048e9e0` | Odjezd, cursor rectangle a stisk mimo panel |
| `0048e980` | HUD enable/absolute, Manufacture chooseBuilding, odjezd a zvuk |
| `0048dea0` | Návody 35 a 36 přes skutečný TutorialManager |
| `005070d0`, `005071b0`, `00507660`, `00507af0` | PictureButton s barevným překryvem |
| `0048fa00`, `00491900`, `0048fc60`, `00490160` | ExtendedButton s vlastněnou nápovědou |

Otevření prochází typy 0..10. `uints UpAvailN != 0xffffffff` znamená dostupnou stavbu; i nula je dostupná. Typy se zhušťují do viditelných buněk. Cena pochází z GameConfiguration `0040ba00(type,0)`. Pokud jsou návody povolené a 35 nebo 36 nejsou zpracované, zobrazí se nula. Dotaz na 36 se provádí pouze po úspěšném isProcessed(35). Money se čte dvakrát: první výsledek se zahodí, druhý porovnává cenu jako signed int32. Zobrazená cena je unsigned decimal. Podle dostupnosti se vypíná tlačítko a cenový text; maska zůstává dostupná pro nápovědu.

Původní tabulka `0065d298` přiřazuje dvě produktové ikony v nápovědě každému typu. Ostatní tooltip picture obsahuje šipku převodu. Sekce `Arrow` a `ArrowOffset` v dodaném Tip XML nejsou v původním ExtendedButton loaderu použité, což rekonstrukce zachovává. Loader stejně jako ostatní tooltip varianty při reloadu přidává další tips/layouts. Změna BuySound přepisuje pointer bez uvolnění předchozí reference; destructor uvolní současný zvuk.

PictureButton je skutečný Button s jednotlivými state textures. Překryv je vlastněný Picture, má rozměry a pozici tlačítka, hloubku button + 0.001f a render flags 9. Ve stavech Hovered/Pressed se ukáže a button dostane flag 4 pro zápis stencil masky; v Normal/Disabled se překryv schová a flag se odebere. Překryv se při transformaci přesune na současnou pozici tlačítka.

Choose nejprve obnoví HUD, pak předá typ a slot −1 do Manufacture `00494390`, zavře panel a přehraje BuySound. Proti BuySeeds se tedy liší pořadí předání do světa. Samotná konstrukce budovy a její herní účinky sem nepatří: rozhraní BuildingPurchaseWorld vyžaduje skutečný world controller, který se dále rekonstruuje.

Porovnání odhalilo záměnu v předchozím source BuySeeds: byte `+1b0` je dirty flag cControl, zatímco deepProcessing je `+26a`. Oba panely nyní při kopírování definice správně volají markTransformDirty. Oracle přímo kontroluje obě hodnoty; samotné dřívější průchody osiva tuto vlastnost nekontrolovaly.

`python tools/validate_buy_building.py` prošel v PC53 a PC24: 20 892 stavů, 4 120 otevření, 2 736 pozorovaných předání do Manufacture a všech 2 048 kombinací dostupnosti v každé přesnosti. Kontroluje osm kombinací povolení/zpracování návodů, profile maps, cenu, tooltip produktové ikony, vlastněné kontroly a resource lifetime. 160 cursor sond porovnává původní float spill pořadí a hranice panelu.

182 GPU snímků / 87 360 000 přesně shodných RGB pixelů používá skutečný D24S8 stencil buffer, odpovídající sprite pořadí a všechna původní draw API volání. Zahrnuje myš na první a poslední stavbě, stisk mimo panel, nápovědy, skutečná okna návodů 35/36, OK a nákup během repeated-action včetně druhého dokončení kroku −1. Oba render průchody mají shodný výchozí stencil/alpha stav a před každým z nich vyčištěný stencil buffer.

Diagnostika používá 512MiB monotonic heap kvůli původnímu pořadí stejně hlubokých prvků podle adres. Celá matice spolu s GPU readbacks tento limit překročila; validator proto vyžaduje všech šestnáct samostatných 128maskových procesů pro každou přesnost. GUI scénář je pouze v shardu 0. Toto rozdělení se týká diagnostiky a nemění produkční chování.

Zbývá skutečný Manufacture se všemi jedenácti factories, umístění, výroba, upgrade a odstranění, source cGame a celé scénové propojení. Manufacture boundary v diagnostice pouze zaznamenává volání a poskytuje kontrolovaný signál pro původní predikát 36; není produkční náhradou budov. Starší HUD návody jsou v izolovaném scénáři zpracované předem. Celý průchod tutoriálem, fyzický vstup a celý slyšitelný zvuk, jiné stencil formáty a extrémní/reload/error konfigurace ještě nejsou ověřené. Herní EXE, celé saves, úplný Squall a izolovaný odinstalátor zůstávají nedokončené.

Regrese odkryla ještě starý rozměr pomocného tutorial objektu v contract-status oracle: 3 072 bajtů nestačilo pro flagy +c08/+c09 a zápis zasahoval sousední testovací globals. Fixture nyní používá doložených 3 104 bajtů. Předchozí průchody izolované tip diagnostiky proto samy neprokazovaly korektní izolaci těchto dvou polí; opravený test znovu porovnává jejich skutečnou hodnotu.
