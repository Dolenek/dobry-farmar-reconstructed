# Dialogy zakázek

`src/game/ui/ContractDialogs.*` obnovuje společný ContractDialogBase a původní ContractAim, ContractFail a ContractPassConfirm. Jsou propojené s obnoveným ContractStatus, texty, ikonami peněz, tlačítky, zdroji a UI přechody. Celý cGame, Farm, warehouse a tutoriál ještě chybí.

| Typ / operace | Původní adresy |
| --- | --- |
| ContractDialogBase, 196 bajtů | ctor 004aadb0, dtor 004a8b30, XML 004ab380 |
| Řádek požadavku, 20 bajtů | XML 004aa690, nastavení 004aa890 |
| ContractAim, 252 bajtů | ctor 004aa040, dtor 004aa660, XML 004a8d50, start 004a94e0, OK 004a9c60 |
| ContractFail, 216 bajtů | ctor 004b73f0, dtor 004b7980, XML 004b65b0, start 004b6b90, OK 004b7010 |
| ContractPassConfirm, 216 bajtů | ctor 004b8ce0, dtor 004b9290, XML 004b7a00, start 004b7fe0, OK 004b8510, Cancel 004b8900 |

## Zachované chování

Base vlastní MultilineText `Note`, dva texty bonus/penalty, dva TextMoney, dva požadavkové řádky (Picture/ReqText/NumberText) a samostatné ToMarketState/ToWareState appearance resources. XML nejprve volá MenuView a child sekce, pak nejvýše dvě AimPlaces. Každý řádek volitelně načte AimProto a k dosavadním pozicím přičte Place; chybějící proto/poloha ani reload nepředstavují reset.

Požadavkový helper skrývá pouze reqType 1; ostatní hodnoty používají vzhled produktu. Zobrazuje unsigned minimum skladu a požadavku, nebo plný požadavek podle argumentu. Dva předané navigation-state argumenty se v instrukcích nepoužívají. ContractAim má další dva řádky AimProtoAdd s nezáporným chybějícím množstvím.

Start dialogu pozastaví GameMenu View (singleton 0065f740) nejprve absoluteDisable, potom disable a zahájí showTransparently. Aim/Confirm OK obnovuje enable a absoluteEnable v tomto pořadí; Fail OK nejprve zruší vypůjčený status pointer a obnoví absoluteEnable/enable. Aim po OK pointer zachová. Confirm OK vykoná skutečný ContractStatus::fail a potom pointer zruší; Cancel ho pouze zruší. Jsou zachované již prokázané původní bonus, zvuk a note volby funkce fail. Fail zobrazuje LosNote v samostatném `Text` MultilineText.

## Oveření

`original-contract-dialogs` / `tools/validate_contract_dialogs.py`: PC53/PC24, všech 131 původních událostí a po 128 numerických hraničních událostech. 3 362 stavů, 1 570 otevření, 1 562 přímých potvrzení/zrušení, 660 990 písmen, 36 XML reloadů, šest controller lifetime porovnání. Porovnávají se vlastněné controls, resource referenční změny, singletony, profily a pořadí světových účinků.

Skutečný ContractStatus::goalClicked, readiness i expiry otevírají skutečné dialogy. Originální start metody ani jejich přechody nejsou v tomto testu přesměrované. 192 úplných D3D8 snímků a 92 160 000 RGB pixelů, draw submissions/API a osm skutečných input release porovnání zahrnují Aim OK, Fail OK, Confirm Cancel i Confirm OK.

## Hranice

Diagnostika mapuje reference EXE bez entry pointu; produkce originální kód nevolá. XML, staré CRT a resource interfaces mají explicitní diagnostické adaptéry. Profily jsou skutečné nezávislé ProfileProperties. Kontext game zastupuje obnovený GameMenu View; plný cGame a jeho světová scéna nejsou tímto testem dodané. Warehouse/money refresh/removal/notification a tutoriál jsou evidované trace hranice; disabled SoundManager ověřuje requests a lifetime, nikoli slyšitelný zvuk.

Pokračuje ContractProvide, původní Farm letter objekty a GameMenu, warehouse, tutoriál a kompletní hra/savy. Neověřené jsou invalid product, native exception/allocation/locale cesty a neurčený constructor pointer stav mimo nulovanou diagnostickou haldu.
