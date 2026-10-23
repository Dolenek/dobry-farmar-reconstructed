# Panel informací o poli

Navazuje na ověřený commit `f2ffdac`. `FieldTip` je nyní rekonstruovaný ze zdrojů a přímo porovnaný v PC53/PC24: 4 998 vlastních stavů, čtyři XML reloady, spolu s poli 140 GPU snímků / 67 200 000 přesně shodných RGB pixelů. Pole volá skutečné singleton select/hideFor, původní pozorované tip hranice byly odstraněné. První běh prošel konstrukcí/XML a skončil na diagnostickém návratu názvu kultury: původní0040c750 používá add eax,0065fa08 (instrukce0040cb9e), jehož adresu je při mapování mimo00400000 také nutné upravit. Generátor diagnostických operandů byl rozšířen o tento konkrétní selector; nejde o změnu produkční logiky.

| Původní adresa | Význam |
| --- | --- |
| `00441360` | Constructor, native1012 bytes /0x3f4, singleton0065f774 |
| `004415e0` / `004415c0` | Destructor / scalar wrapper |
| `004408d0` | XML, skutečné UI/gameUI/FarmFieldTip.xml |
| `004405e0` | AbsoluteStep po skutečném MovableUnion |
| `00440380` | Refresh ukazatelů, názvu a ikony |
| `004402a0` / `004402d0` | Select / hideFor pouze pro matching owner |
| `0040c750` | Původní selector19 ruských product name literals; Field používá prvních8 |

Primary005f2168 má65 slotů. Adresy+104/+108 už patří sousední HoseParticle MI005f226c a nejsou dalšími FieldTip metodami. MI005f215c má dva sloty, singleton interface005f2158 jeden. Native data: water280/fertil284/ready288, title28c/icon290; label vector294allocator/298begin/29cend/2a0cap; selected2a4, hideTimer2a8=0, pending2ac=false, showTimer2b0 unspecified; osm appearance records2b4 stride40. Constructor vytváří title, icon, tři progress controls a načte XML, které připojí backgrounds a text labels. Reload zachovává staré vlastněné děti a připojuje další labels.

Select(nonnull) vždy resetuje showTimer na0.5 a pending=true; select(null) pouze přepíše pointer. HideFor musí mít stejný pointer, pak nuluje selected/pending a nastaví hideTimer0.3. Hidden/parentHidden panel odečítá showTimer; při <=0 refreshuje a move2(current,originalPosition). Visible panel se selected refreshuje; jinak odečítá hideTimer a při<=0 move1(current,hiddenPlace). Porovnání časů má původní mezilehlé float stores a unspilled comparison.

Refresh water/fertil používá hodnotu/capacity×100. Ready state1 původně nejprve nastaví100 a následná samostatná state3 větev/else hodnotu přepíše poměrem (creation-progress)/creation×100; tato zvláštnost je zachovaná. State3 skrývá název a ikonu, ostatní stavy používají původní ruský literal přes skutečnou lokalizaci TextLine a odpovídající appearance. XML nemění pozici na hiddenPlace po load; root začíná hidden na90,285, hiddenPlace je-200,280.

PC53/PC24 porovnání, viditelné GPU snímky všech osmi kultur, partial reload/owned labels a reference delta prošly. Úplná navazující regrese prošla všemi80 CTest kontrolami za721,22 s, včetně kontroly masek bez opakování. Oba game-audio buildy prošly378 stavy/36 playback/16 pause kontrolami. `tools/record_verification.py` ověřil současné source/binary hashe i všech17 originálů; výsledky jsou v `analysis/farm-field-validation.json` a `analysis/verification.json`. Celý Farm, SeedPackage, Person, herní EXE a původní celý cíl dále zbývají.

Poslední ruční PC24 běh trval podle nástroje938,24 s, PC53 zhruba8,56 s. Oba vrátily0 a shodné counters; hra používá v diagnostice řízený timer. Příčina neobvyklé délky druhého běhu není prokázaná a není vydávána za herní výkon. Výstupy jsou `.local/fieldtip-PC53-current.json` a `.local/fieldtip-PC24-current.json`.
