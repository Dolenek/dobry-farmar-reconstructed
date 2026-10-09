# Ovládací prvky a hierarchie UI

Současná implementace propojuje geometrii, animaci, cView a 2D cControl/cPicture s resource cache a spritovými dávkami. Obnoveny jsou také níže uvedené tlačítkové a vstupní části. Další ovládací prvky, celé menu a hlavní herní EXE dosud nejsou obnovené.

## Sprity a animace

`updateControlSprite` obnovuje `004ef3b0`, včetně třetího stack argumentu opacity, který dekompilace vynechala. Zachovává řádkové/sloupcové UV, invertování, x87 sin/cos a spill body, RGB a alfa součin rootTransparency × controlAlpha × opacity. Original oracle porovnává všech 76 bajtů ve 200 000 scénářích (defaultní a device x87 přesnost).

`normalizeControlAnimationTime` (`004ed510`) zachovává hranice: přesný kladný konec se nesmyčkuje, záporný násobek délky smyčky se převede na délku. Výběr snímku (`00413a10`) pro time >= length vrací poslední snímek. `resolveControlAnimation` obnovuje 2D větev `004f0b70`, callback ended a prolínání dvou sousedních snímků. Oracle ověřuje 40 000 celých animovaných stavů a dvojic spritů; navazující Control oracle ověřuje timer a callback list. 3D mesh větev dosud není obnovená. Porovnání proběhlo v obou x87 precision kontextech po opravách mezivýpočtů; viz UI_FONTS.md.

## cControl a cPicture

`Control` obnovuje konstruktor/destruktor `004edc80`/`004f1560`, relativní posun potomků `004f0110`, změnu hloubky a registrace `004f1080`, inclusive hit test `00413a50`, setFrame `004eebb0` a další doložené settery. Hloubkový registr odpovídá původní množině `006604fc`, včetně pointer pořadí při shodné hloubce. `00506a10` vykresluje viditelné prvky v tomto pořadí a odstraňuje skryté až po průchodu.

`Picture` (`004f16f0`) navazuje shown/hidden callbacky na frontu `004f1680`. Update `004f1810` nejprve volá update callback, přičítá timer +8 při PlayAnimation a pak prochází frontu. `004ed670` zachovává uložený následník před callbackem a maže položku jen při true. `sprite()` aktualizuje dirty transformace; submit spojuje sprity s resource texturou, flags a SpriteBatches (`0052fe50`).

Přímý oracle s původními konstruktory, destruktory, STL množinami, callback listy a funkcemi porovnal 40 000 operací, 400 020 stavů, 24 546 callback událostí, 1 822 dvojic spritů, 1 788 celých hloubkových průchodů a zánik dvaceti objektů ve dvou x87 precision kontextech. Report je `analysis/control-validation.json`. Moderní třídy nejsou původní 556/568bajtové ABI. XML atributy a textura jsou zapojené do zdrojů, přímé porovnání původního loadXML zbývá. Mesh/Animation resource factory a 3D větev vyvolávají explicitní chybu do doby rekonstrukce; nejsou nahrazené odhadnutým chováním.

## cView

`View` používá moderní kontejnery, nikoli původní 44bajtové ABI. Vlastní potomky v pointer pořadí a maže je destruktorem (`004d55b0`, `004d58a0`). Změna rodiče `004d2e50` propaguje pouze inherited visibility; inherited enable/absolute se při změně rodiče samostatně neaktualizují. Constructor naproti tomu nastaví všechny tři inherited flags podle rodiče.

Obnovené show/hide (`004d37d0`, `004d3720`), enable (`004d3bb0`), absoluteEnable (`004d3fb0`) a jejich inlined disable větve zachovávají pořadí a opakování callbacků. Propagace `004d2cf0`–`004d2e20`, onShown/onHidden (`004d2ec0`, `004d2f50`) a process (`004d2fc0`) pracují i s neviditelnými potomky podle konkrétní větve původní rutiny. Base enabledStep/absoluteStep jsou prázdné původní sloty `004bd820`; implementace odvozených prvků zbývají.

ViewRegistry obsahuje původní normal/absolute množiny, pending add/remove a společný dispatch flag. `00506f90`, `00506fe0`, `00507030`, `00507080` při dispatch pouze změní pending množiny; flush `00506b90`, `00506d90` přidá a pak odebere pointery a vyprázdní příslušnou dvojici. Smíšené sekvence se porovnávají s původními STL mutacemi, nikoli s jejich testovou náhradou.

Transparent show/hide/discard (`004d4590`, `004d4980`, `004d4d80`) nastavují příslušný režim. `004d59c0` mění pouze root, nepřicvakne kladný overshoot k 1 a při dokončení hide vynuluje průhlednost. Volá process i po skončení přechodu. Dokončený discard vytváří záznam pro následné mazání; zdrojová implementace queue/mazání odpovídá `0047d0b0` a `004fb180`, runtime oracle této dokončené větve zatím chybí. `004fb180` nejprve odpojí všechny queued prvky od rodičů a následně je maže.

XML část `004d43a0` čte Visible, Enable, původní překlep `TranparencySpeed` a AbsoluteEnable. Poslední false neodpojí dříve absolute aktivovaný prvek. Typed extraction a původní XML/exception ABI nejsou runtime porovnané.

```powershell
cmake --build build/windows --config Release
python tools/validate_control_sprites.py
python tools/validate_views.py
```

View oracle používá soukromou PE mapu bez entry pointu a vlastněné 44bajtové objekty. Původní konstruktor/destruktor, propagace i STL registrace jsou spuštěné; testové hranice jsou CRT allocator/free, řízený timer a virtual callback log následovaný původním handlerem. Souhlasí 32 000 operací, 384 024 stavů, 35 826 callback událostí, 32 000 snímků registrů a pořadí mazání 24 vlastněných views. Dokončení transparent discard, loadXML, copy constructor a celé frame/input/render pořadí zůstávají evidovaně neověřené.


## Tlačítka, popisky a masky

`ButtonPrototype` obnovuje `004f53b0`, `004f5480`, state setter `004f30e0`, mouse/control handlery `004f2620`, `004f2690`, `004f26b0`, event pořadí a deep-selection globály `006604dc/006604e0`. Registruje levé tlačítko 520/512 a mouse movement. Hover/play guard se vyhodnotí znovu po callbacku; click zvuk je před změnou stavu. `AnimationButton` (`004f1c30`, `004f1ce0`, `004f20f0`) mění snímek. `Button` (`00507ec0`, `00507fd0`, `00508560`) mění resource reference; chybějící stavová textura zachová current texture. Destruktor `00508120` uvolňuje normal/pressed/hover/disabled v tomto pořadí, poté základ deaktivuje prvek a uvolní current texture.

`TextButton`/`TextAnimationButton` obnovují `0051fe90`/`0041e8d0` a vlastněný `ButtonText`. Opakované setState znovu nastavuje moving i při stejné hodnotě. Virtuální setAlpha `0051f570` vždy propíše hodnotu do textu i jeho písmen. XML `0051f9b0`/`0041f120` čte Text, UseTextPosition a nastaví text depth na parent+1. Motion `0051e040`/`0041fa40` střídá vertikální Offset a rychlost Speed, při překročení cíle jej přicvakne; moving zhasne až další průchod. Source arithmetic zachovává obě GPU x87 přesnosti.

Button oracle: 480 024 stavů, 40 000 operací, 16 738 callback/sound událostí, 480 000 mask hit testů a 24 destruktorů (`analysis/button-validation.json`). ButtonText motion: 18 000 scénářů, 170 262 letter/sprite porovnání (`analysis/button-text-validation.json`). Propojený parent/text oracle přímo spouští původní konstruktory a XML loadery: 2 160 scénářů, 9 786 letter/sprite porovnání a 18 vlastněných stromů (`analysis/text-buttons-validation.json`). Šest hlavních animovaných tlačítek se načítá z MainMenu.xml. Tři texture-switching varianty používají popisky podmenu a samostatnou explicitní base XML fixture ze shipped Options/SoundVolume/Slider. XML parser a typed stream konverze jsou testové hranice, zvukový factory/manager je evidovaná reference fixture. Toto není úplné menu ani slyšitelný runtime zvuku.

`loadClickMask` obnovuje JNG větev `0051c040`: X sloupce, Y bity a alpha > 0.001. Původní pixel stride je Width, nikoli LockRect Pitch. Native GPU/bit-array oracle souhlasí pro 425 vstupů v každém precision kontextu: 850 loadů, 300 058 sloupců, 162 028 842 alpha bitů (`analysis/click-mask-validation.json`). Nativní resize/bit-array kód a původní threshold běží přímo; JNG texture loader boundary je předtím pixelově ověřený rekonstruovaný loader. Non-JNG D3DX mask loader není obnovený a explicitně vyvolává chybu.

MainMenu.xml odkazuje u podmenu States na C_big/L_big DDS, které v inventarizovaných balících chybí. Instrukce loaderu `00481e10` však tyto sekce vůbec nečtou. Načítá StartButton, Options, ExitButton, ChangeUserButton, AwardsButton, CreditsButton, UserName a UserText; constructor `00484da0` vytváří sedm TextAnimationButton a dva TextLine, jeden tlačítkový objekt ponechá bez XML konfigurace. Počet sedm opravuje dřívější chybný odhad osmi objektů. Neexistující data podmenu nejsou dosavadní překážkou běhu hlavního menu.

`MenuView` obnovuje decoration-bearing základ `004fb040` a `004fa8c0`: View XML nejprve, potom Background/BackText/BackMultitext v pořadí indexů section. Při opakovaném loadXML přidává nové potomky a decorations; původní objekty nemaže. Destruktor `004fafc0` nejprve uvolní pouze pomocnou množinu a View následně maže potomky. Background větev je přímo porovnaná pro MainMenu, FixedBack a Options, dvě načtení každého XML v obou GPU precision kontextech: 30 spritů/dekorací a šest celých menu-view destruktorů s přesnými reference deltas. BackText/BackMultitext nejsou tímto porovnané; druhý factory zatím musí být explicitně navázaný a při chybějící implementaci vyvolá chybu. Není použitá náhrada za Multitext.

XML cache vlastní deleting iXML view: opakovaný `loadSettingsXML` na stejný cached objekt nesmí v diagnostice vytvořit dva nezávislé `shared_ptr` deletery. Native menu test nyní sdílí existující ownership control block; původní resource delete pravidlo zůstalo zachované.

## DirectInput a registrace událostí

`InputBindings` obnovuje 532 control setů (531 = AllControls), samostatné mouse/character sety a pointer pořadí function/view. UnbindAll `00524a30` čistí pouze control sety; UnbindKeyboard `00524e60` character set. Mouse/character registrace přijímají NULL, control registrace jej ignoruje. Control dispatch zachytí všechny události a AllControls před callbacky; mouse callbacky dostávají X/Y/Z pro každý handler. Characters mají live traversal.

`InputDevices` obnovuje fyzickou device setup část: DirectInput8/version 0x800, keyboard/mouse2 formáty, coop 6 a buffer 1024; všech 267 object format records je přesně shodných s původními PE daty. Process `00527310` nejprve zpracuje 1024 jednotek UTF-16 (callback může doplnit frontu během průchodu), potom buffered keyboard a mouse controls a relativní mouse state. Success HRESULT 0/1; DIERR_NOTACQUIRED (0x8007000c) získá zařízení znovu a přeskočí data. DIERR_INPUTLOST (0x8007001e) vyhazuje chybu; původní engine exception ABI chybí. Lazy key state cache `00525620`/`00525240` se invaliduje na konci snímku. Acquire/Unacquire volají keyboard i mouse; exclusive mouse přepne coop 5/6 po Unacquire. GameWindow targets se dají navázat na rekonstruovaný input.

Native oracle: 5 340 000 porovnání všech 534 handler setů, 500 snímků a 630 566 dispatch/COM/callback událostí, 5 000 cached key dotazů (`analysis/input-bindings-validation.json`). Native balanced STL/dispatch code běží přímo, device COM je řízená fixture, nikoli skutečná fyzická klávesnice/myš. Plný cInput constructor/key-name map, destructor a původní object ABI, fyzické získání zařízení a plný herní runtime zbývají. Propojení řízených input events s GameCursor a celou scénou menu nyní ověřuje GAME_CURSOR.md.


## Průchod UI snímkem

`ViewRegistry::dispatchSteps` obnovuje absolute → normal část `00531b20`. Oba průchody kontrolují normální pending removals `00660538`; není to omylem zaměněná absolute množina. Každý průchod zapne dispatch flag a teprve na konci aplikují `00506d90`/`00506b90` čekající registrace. Callback v absolute průchodu tedy může zabránit i zbývajícímu absolute/normal callbacku přes normální unregister. `00660528` je počet prvků v množině přechodů `00660520`, nikoli samostatný pause flag. Při neprázdné množině originál vynechá oba step průchody, ale aktualizuje přechody; dokončené prvky odstraní až po celém průchodu. Dřívější frame fixture přepisovala tento STL size word syntetickým příznakem. Tato nesprávná hranice testu je odstraněná a zdroje nyní podmiňují dispatch skutečnou neprázdnou množinou.

## Propojený obsah a vykreslení hlavního menu

`MainMenuContent` obnovuje vlastněnou skladbu `00484da0` a XML vazby `00481e10`, včetně nepoužitého sedmého tlačítka, dvou textových řádků a tří dekorací. Zachovává pořadí alokací. Loader používá View subobject na +4; AwardsButton načítá do raw pole +50, CreditsButton do +54, zatímco constructor instaluje callbacky `00482ae0` na +50 a `00482ef0` na +54. Test potvrdil tyto vazby bez sémantického přehazování. `00481770` doplní aktuální jméno profilu pouze do prázdného UserName; již vyplněný název samovolně nepřepisuje. Porovnání zahrnuje 54 kontrol obsahu/refresh operací v obou precision kontextech.

Native scene oracle nyní spouští celé `00531b20` nad skutečnou vlastněnou hierarchií menu, včetně hide/show přechodů. Dále běží původní depth průchod `00506a10`, submit `0052fe50` a draw `00515830`. Porovnáno 96 snímků v obou device x87 přesnostech, 46 080 000 RGB pixelů a shodná draw API volání (`analysis/text-buttons-validation.json`). Dvě nezávislé hierarchie vytvářejí nezávislé GPU buffery; ordered sprites a všechny RGB bajty se porovnávají přesně. Testový monotonic allocator sjednocuje pořadí adres při shodném depth, neboť původní comparator řadí tyto prvky podle pointerů. Před jeho použitím se lišilo pořadí překrývajících se dekorací a několik pixelů; nejsou zavedené tolerance ani změněný produkční comparator.

Hranice tohoto ověření: rekonstruované SpriteBatches jsou shared submission boundary; plná nativní batch construction není touto scénou spuštěná. Native cursor update již spouští původní 00530290/00512330/0050d260 s fyzickými bounds diagnostického okna a řízenými screen vzorky. Input process zpracovává řízené události přes obnovené InputBindings. Šest tlačítek projde najetím/stiskem/uvolněním/odjetím; shoduje se 24 sound požadavků. Timer advance a graphics begin/finish zůstávají řízené prázdné služby, source používá obnovený GameTimer. Mesh/extra-drawable/discard registry jsou prázdné; nativní render-state setup a úplný graphics singleton ABI čekají. Readback ignoruje pouze nedefinovaný X bajt X8R8G8B8. MainMenu singleton, enable/disable hooks, navigační callbacky, skutečné profily, fyzický input a audio dosud nejsou plný herní runtime. Samostatný herní EXE dosud není sestavitelný.

View transparency nyní používá původní x87 operace v `004d59c0`, nikoli moderní SSE double mezivýpočty. Dva kontexty přímo porovnaly 384 024 běžných stavů a dalších 20 000 precision scénářů transparent show/hide. Native main-frame oracle spouští celé `00531b20` s explicitními testovými hranicemi: 1 024 snímků, callback mutace registrací, shoda všech dvanácti view stavů a registračních množin (`analysis/ui-frame-validation.json`). Cursor update/timer advance/input processing/graphics begin-finish jsou v tomto oracle řízené prázdné služby; sound/mesh/drawable registry jsou prázdné. Neprokazuje plný herní frame, skutečné vykreslení/input/audio ani nonempty discard queue. Dokončený mode 3 se v frame fixture převádí na hide, jeho diagnostic String/mazání dosud není porovnané.
