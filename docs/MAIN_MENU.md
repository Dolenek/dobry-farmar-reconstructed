# Controller hlavního menu

`MainMenu` obnovuje `00484da0` (96 native bytes, singleton a MenuView +4), `00483e90` a tabulku `005f60a8`. Vlastní ověřený `MainMenuContent` a společný `FixedBack`. Publikování singletonu před základní třídou a zrušení až po zničení vlastněných controls zajišťuje první base `MainMenuLifetime`.

`00481990` vytvoří FixedBack pouze jednou, načte `Ui/FixedBack.xml` a zavolá obyčejné `show`. Destruktor menu jej zničí a vynuluje. XML loader `00481e10` volá základní `MenuView::loadXML`; při zapojení odvozeného controlleru muselo být toto volání v MainMenuContent kvalifikované, aby nedošlo k virtuální rekurzi. Ověřené přiřazení a pořadí controls zůstalo zachované: content.awards je native +54 s Credits action, content.credits je +50 s Awards action.

Původní enabled hook `004836d0` skutečně volá základní **onDisabled** (odregistruje menu ze step množiny) a přidá Escape 1. Disabled hook `00483ab0` volá základní **onEnabled** (registruje menu) a Escape odstraní. Zdroj zachovává tuto zvláštnost; neobrací ji podle názvu metod. `00481770` doplní jméno aktuálního profilu jen při prázdném textu.

| Původní adresa | Obnovené chování |
| --- | --- |
| 004817d0 / 004816d0 | ExitDialog, hide/show transition; No discard dialog a znovu zobrazí/povolí menu. |
| 004817c0 / 005306c0 | Yes zavolá `PostQuitMessage(0)` přes původní logic virtual slot +14. |
| 004818f0 / 00481860 / 00485490 | Skutečný SelectProfile, accepted/cancelled návrat; vytvoření menu při chybějícím singletonu a aktualizace jména po přijetí. |
| 004826c0 | Options singleton guard, hide/show transition a skutečné menu jako return view. Audio služby jsou přímo obnovený SoundManager; graphics reset dodá engine. |
| 00482ef0 / 00482ae0 / 004817a0 | Skutečné Credits/Awards a návrat, singleton guard, původní pořadí hide/show. Credits nemá dodatečné hide před show. |
| 00484700 | Discard původu `.\Game\Ui\MainMenu.cpp153`, hide, game/LogResources, MenuResLog.xml a skutečný LoadingFromLog(GameResLog.xml, batch 30, game music). |

`MainMenuContext` je moderní vyjádření engine závislostí; není tvrzená shoda jeho ABI s originálem. Musí přežít menu i callback Loading. Vyžaduje skutečné graphics služby a nenulový game constructor callback odpovídající `00413210`; nemá náhradní ani prázdnou implementaci hry. **Tento cGame a hlavní herní EXE ještě nejsou obnovené.**

`python tools/validate_main_menu.py` spouští původní controller, screen constructors a přechody v izolovaném diagnostickém EXE. PC53/PC24 porovnaly 44 stavů, 2 894 letter spritů, 26 skutečných input click akcí a 234 úplných GPU snímků / 112 320 000 RGB pixelů včetně shodných draw calls. Source používá reálné profilové soubory. Native profile/old-CRT/resource služby tvoří uvedené diagnostické hranice. Ověřené jsou i Escape, No návrat, oba skutečné WM_QUIT callbacky, singleton guards, nový MainMenu po destrukci, FixedBack reuse a celý 539-entry Loading v 36 dávkách celkem.

Quit kontrola zpracuje vlastní Windows message queue: filtrovaný PeekMessage bez odstranění dalších čekajících zpráv nemusel bezprostředně zpřístupnit WM_QUIT. Původní callback se neměnil. Resource-log writer má samostatné raw UTF-16 porovnání; native MainMenu virtual logging boundary zaznamenává jeho přesné jméno. Sound je v této navigační diagnostice disabled; 52 shodných hover/click požadavků nejsou důkaz slyšitelného výstupu.

Callback konstrukce cGame v diagnostice **pouze počítá** dokončení a `00413210` se nespouští. Toto není nový herní runtime ani splnění cíle. Dále pokračují skutečný herní svět, spojený physical input/audio, complete reset a save/load kompatibilita. Neověřené větve a použité hranice jsou v `analysis/main-menu-validation.json`.
