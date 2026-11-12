# Testování běžící hry

Herní scénáře se spouštějí s vlastní kopií dat a testovacími profily. Skutečný vstup potvrzuje očekávaná změna scény nebo profilu. Sonda čte scénu bez pozastavování vláken a kontroluje konzistenci topologie. Při souběžné změně čtení odmítne. Herní pauza dovoluje pozorovat stabilní stav a ověřit, že se neposunul herní čas.

## Dohled nad interaktivními testy

`host-test-watchdog` vytváří skupinu Windows Job pro procesy jednoho scénáře. Sleduje postup, čas, zátěž CPU a volnou paměť. Po překročení limitu ukončí pouze procesy z této skupiny. Samostatná sonda v omezených intervalech měří odezvu okna ověřeného herního procesu. [Kontroly hlídače bez hry](../analysis/host-watchdog-cpu-validation.json)

Pokud je aktivní místní `.local/desktop-runtime-hold.json`, interaktivní nástroje ověřují aktuální lease, živý hlídač a skutečné členství testovacího procesu v jeho Job. Kompletní CTest obsahuje také grafické scénáře. Kontroly, které grafiku a vstup nevyžadují, lze vybírat jednotlivě.

Obnova farmy pod dohledem skončila běžným ukončením a při herní pauze bylo možné přečíst 446 prvků včetně 60 aktivních polí. Záznamy odlišují dokončené běhy od pokusů ukončených před potvrzením výsledku. [Ověření obnovy farmy](../analysis/monitored-source-restore-validation.json)
