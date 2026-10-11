# Nabidka zakazky ContractProvide

`src/game/ui/ContractProvide.*` obnovuje controller nabidky podle puvodnich instrukci a propojuje skutecne ContractDialogBase, TextMoney, pozadovane produkty, MultilineText, tlacitka a format casu. Produkcni rozhrani ContractOfferWorld a ContractOfferLetter vyzaduji skutecne svetove implementace; nemaji vychozi nahradni chovani.

| Operace | Puvodni adresa |
| --- | --- |
| Konstruktor, native 220 bajtu | 004ad4b0 |
| Destruktor | 004ada90 |
| XML | 004abef0 |
| Enabled step | 004abea0 |
| Start | 004ac570 |
| Prijeti | 004accc0 |
| Pockat | 004ad0d0 |
| Aktivace ve skutecnem native GameMenu | 004c02c0 |

Start precte event.note (+14), nastavi oba produktove pozadavky bez dotazu na sklad, signed bonus/penalty a cas bez hodin. Nejprve vzdy dotazuje penize, pote zakaze Yes pri poctu zakazek presne 3 nebo nedostatecne zaloze. GameMenu (0065f740) pozastavi absoluteDisable/disable a nabidku otevre showTransparently. Disabled vlastni nebo inherited stav Yes dotazuje tutorial step 0x2a; visibility nema vliv.

Prijeti skryje nabidku, obnovi GameMenu enable/absoluteEnable, spotrebuje dopis, znovu precte jeho event a teprve potom penize a penalty. Zachovava unsigned wrap odectu a signed vysledek pred money setterem. Aktivace prideli prvni neaktivni slot. Prijeti i cekani ponechavaji borrowed dopis pointer. Native constructor jej neinicializuje; zdroj ma nullptr.

## Overeni

`original-contract-provide` / `tools/validate_contract_provide.py`: PC53/PC24, vsech 131 dodanych eventu, stavy poctu 0/1/2/3 a 128 numerickych edge eventu vcetne negative/4 count pri volnem slotu. 2 738 stavu, 1 308 startu, 1 304 primych akci, 430 798 pismen, 12 XML reloadu. Tutorial porovnava vlastni/parent enabled a anchor. Swap eventu pri spotrebovani doklada skutecne znovu precteni event pointeru.

Originalni 004c02c0 neni v diagnostice presmerovana: vykona aktivaci do tri skutecnych native ContractStatus a porovnaji se jejich controls, casy, flags, stocks, nezavisle profily a svetove trace poradi. Zdrojovy vyber slotu je v tomto testu explicitni diagnosticka implementace; plny zdrojovy GameMenu controller zatim chybi.

96 uplnych D3D8 snimku, ordered submissions/API a 46 080 000 presne shodnych RGB pixelu; ctyri skutecne input release uzavrou nabidku a pri prijeti aktivuji zakazku. Destrukce porovnava resource reference a singleton. Vsech 17 referencnich souboru zustava stejnych.

## Hranice a navaznost

Originalni EXE je mapovane jen v diagnostice bez entry pointu. Produkce nevola originalni kod. XML/CRT/resources maji explicitni diagnosticke hranice. GameMenu fixture dodava obnoveny View a tri puvodni sloty/count. Farm spotrebovani dopisu, money refresh a tutorial zustavaji trace adaptery; nejde o dokoncenou farmu. SoundManager je skutecny, backend disabled, slysiteiny vystup neoveren.

Dalsi prace: plny GameMenu/Journal, Farm letter actor a jeho consume ucinky, warehouse, tutorial, cGame, hlavni EXE a obousmerne saves. Neoverene: invalid produkt, vsechny obsazene sloty pri count !=3, exception/allocation/locale cesty a neurceny native constructor pointer mimo nulovanou diagnostickou haldu.
