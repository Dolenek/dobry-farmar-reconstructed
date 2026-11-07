# Zamrznutí obrazu při herním testování

Uživatel7.11.2026 oznámil zamrznutí celých Windows vyžadující restart; následně
upřesnil, že mohl zamrznout obraz, zatímco session pokračovala. Další interaktivní
testy na běžném desktopu jsou zastavené. Při kontrole po restartu nezůstaly nalezené
vlastní herní testovací procesy ani jejich Python drivery.

Systém hlásí poslední boot ve22:27:40.5 SEČ a Kernel-Power41 ve22:27:51.566.
Tento záznam dokládá nečistý restart, nikoli jeho příčinu. Ve čteném intervalu
22:10–22:29 nebyl nalezený záznam Display/Dxg/WHEA/BugCheck vysvětlující zamrznutí.
Absence záznamu nevylučuje problém grafického ovladače. Host má Windows build26200
a AMD Radeon Graphics, ovladač31.0.21924.7001.

Poslední přerušený běh je `.local/tutorial-warning-runtime/20261107T211631362554Z`.
Obsahuje snímky začátku nativní smíšené sklizně, ale žádný konečný `report.json`.
Je proto přerušený, nikoli úspěšný. Předchozí uložené úspěchy40/42 a samostatně
dokončené nativní41 tím nejsou přepsané ani vydávané za dokončení posledního běhu.

Původní `TutorialMemory.visible_scene` používal `SuspendThread` na herním
vykreslovacím/UI vlákně při čtení měnícího se stromu. Pozastavení v libovolném místě
je rizikové, protože vlákno může držet sdílený zámek.
[Microsoft výslovně uvádí riziko deadlocku a že tato funkce není určena pro synchronizaci](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-suspendthread).
Tento mechanismus byl odstraněný. Souvislost s konkrétním zamrznutím není prokázaná.
Naměřené přibližně20s prodlevy SendInput zůstávají samostatnou otevřenou stopou.
Odstraněný je také fallback `AttachThreadInput`, kterým test spojoval svou vstupní
frontu s jiným foreground vláknem, aby vynutil fokus. Při odmítnutém foreground
požadavku nyní test skončí před odesláním vstupu.

Nové čtení kontroluje topologii bez zastavování vláken, má časový a velikostní
limit a odmítne pozorovanou souběžnou změnu. Nezaručuje atomický snímek všech
vykreslovaných dat. Dosud se ověřuje pouze pomocí CPU testů nad paměťovými fixtures;
nové skutečné GPU ověření se na hostu nespouštělo.

`GameGui` před začátkem interaktivního scénáře kontroluje místní
`.local/desktop-runtime-hold.json`. Aktivní nebo nečitelná pojistka scénář odmítne
před spuštěním hry a odesláním vstupu. Nemá přepínač prostředí pro obejití.
Další grafické/input ověřování patří do izolovaného hosta. Místní systém je
Windows11 Home; [tato edice Windows Sandbox nepodporuje](https://learn.microsoft.com/en-us/windows/security/application-security/application-isolation/windows-sandbox/).
Sandbox EXE ani běžné VM nástroje nebyly při prvním průzkumu nalezené. Dotaz na
volitelnou systémovou funkci vyžadoval administrátorský token; tento dotaz není
dokladem jejího stavu. Nic se neinstalovalo, nezapínalo a nebyl vyžádaný restart.

Původní cíl zůstává aktivní. Analýza instrukcí, zdroje, sestavení a kontroly bez
grafického zařízení pokračují. Celý CTest se na hostu neopakuje, protože zahrnuje
grafické testy. Případný další experiment na běžném desktopu vyžaduje výslovný
souhlas uživatele s konkrétním předem popsaným experimentem.
