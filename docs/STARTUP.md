# Úvodní loga a přechod do profilu/menu

`StartupView` obnovuje 60bajtový původní objekt: constructor0048b4a0, destructor0048b920, enabledStep0048b030, nextLogo00489ef0 a input0048ab10. Vlastní Picture je na400,300, depth10000; jeho show/enable předchází registraci kořene. Původní procesový byte0065d290 je1, vstup jej pokaždé překlápí. Zachováno je i rozdílné pořadí getTexture/setTexture pro jednotlivá loga, čtyřsekundové hranice, alfa a odložený discard.

Skutečný LoadingFromLog načítá menureslog.xml po třech položkách s hudbou. Completion0048aa90 vytvoří skutečný NewUser a následně menu;00489e80 načte uložený profil a vytvoří skutečný MainMenu. Engine drží StartupContext déle než zaniklý StartupView, protože jej používají completion callbacky.

`python tools/validate_startup.py` vykonává původní těla pouze jako diagnostickou referenci. V PC53 i PC24 projdou automatická tři loga i přeskočení skutečným key-down/up dispatch, nový profil StartupFarm a jeho další načtení. Celkem794 stavů,12 vstupních akcí,786 celých snímků a377280000 přesně shodných RGB pixelů. Čtyři životní cykly mají shodný discard a úklid.

Nativní Profile konstrukce/header/name má explicitní existující diagnostický adaptér; zdrojový ProfileSession používá skutečné izolované soubory. Clock/input a resource továrny používají dokumentované hranice ostatních UI testů; fyzický zvuk je zde vypnutý. Celé původní spuštění tím není prokázané. Screenshot buffery se znovu používají, aby nezaplňovaly monotónní diagnostickou haldu; pořadí UI alokací zůstává zachováno.

Metadata: `analysis/startup-validation.json`. Skutečný hlavní proces: [APP_RUNTIME.md](APP_RUNTIME.md).
