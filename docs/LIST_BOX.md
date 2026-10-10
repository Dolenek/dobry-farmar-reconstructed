# Seznam profilů a kopírování 2D ovládacích prvků

`ListBox` obnovuje konstruktor `00521100`, XML `005206e0`, append `005212a0`, update `0051e6d0`, clear `00521080`, callbacky posouvání `00521020`/`00521060`, výběr `0051ec50` a vlastněný lifecycle `0051d920`. Původní objekt má 628 bajtů; moderní C++ objekt původní ABI netvrdí. Vlastní Up/Down Button, skrytý TextButton vzor a vytvořené řádky. Index řádku je původní control pole `+1b4`, selection callback dostává index a ListBox.

Počet viditelných řádků je truncace `(BottomRight.y-TopLeft.y)/Interlinear`; načtené `MaxRows` update nepoužívá. Originál dělí ještě nerozlitý x87 rozdíl, pro umístění řádků si výšku zvlášť uloží jako float. Souřadnice X vychází z pozice ListBoxu, nikoli z TopLeft. Řádky se posunují po jednom; předcházející a následující řádky se pouze skryjí. Visible řádky se show/enable i při opakovaném update.

Append klonuje skutečný vzor `0051f100`, nesestavuje odhadnutou náhradu z XML. Obnovený copy chain: View `004d56b0`, Control `004ed720` a 2D cAnimable `004d20b0`, Picture `004f1730`, ButtonPrototype `004f5b60`, Button `00507f00`, TextLine `004dd750`, ButtonText `0051d8a0`. View kopie dostává stejný parent, prázdnou množinu potomků, nulový přechod/rychlost/průhlednost a registraci bez dynamického onEnabled. Control kopíruje vlastnosti/reference textur a userData, ale má nové sprity a prázdný callback seznam. TextLine má nová písmena a načte současné Settings RTL. ButtonText nekopíruje motion flags. Vlastněný text klonu se přepojí z původního parentu pod nové tlačítko. Mesh/Animation resource copy a TextAnimationButton copy zůstávají neověřené.

XML `Strings` čte Count a atributy `str0`, `str1` atd. Instrukce `00520b45` zapisuje přímo do rezervovaného vector storage bez změny end. Existující aktivní řádky nahradí, dřívější objekty však zůstávají View potomky; nové položky za logickým end se také pouze stanou potomky. Source zachovává tuto skladbu bez C++ zápisu mimo vector size. Count nad capacity vyvolá explicitní chybu namísto emulace poškození paměti. Shipped UserList tuto větev nepoužívá.

Původní clear vynuluje vector, ale nezmění firstRow. Je-li seznam posunutý, unsigned `count-firstRow` podteče a update dereferencuje uvolněnou/nulovou tabulku. Samostatný native diagnostický proces potvrdil access violation `0xc0000005`; report ukládá přesné fault RVA. Source ponechá shodné firstRow a prázdný vector, ale bounds check vyvolá `std::out_of_range`. Jde o výslovnou odchylku pro bezpečnost paměti, nikoli tvrzení o opravené původní mechanice. Clear na prvním řádku a následné append byly porovnány přímo.

`analysis/text-buttons-validation.json` zaznamenává obě device x87 přesnosti: 120 kopií TextButton, 24 seznamů z `UI/SelectUser.xml`, 732 appendů, 2 796 úplných stavů seznamu, 1 920 výběrových callbacků a čtyři Strings XML fixtures (prázdný/populovaný vector). Porovnávají se přesné float bity, hierarchie, text/písmena/sprity, page/indexy, bounds a reference deltas při vytváření i mazání. Native růst vectoru používá diagnostic CRT memmove; Strings stream key formatting moderní stream boundary. Originální herní entry point se nespouští a produkční knihovny původní PE nikdy nenačítají.

Zbývá skutečný controller SelectProfile, propojení s menu/novou hrou, fyzický input a slyšitelné audio. Neplatná/zero interlinear, záporné rozměry a XML Count nad rezervou nejsou běžnými testy pokryté. Samotný seznam není kompletní herní runtime.

```powershell
cmake --build build/windows --config Release
python tools/validate_text_buttons.py
```
