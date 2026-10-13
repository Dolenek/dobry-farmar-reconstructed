# Částice hadice a postřikovače

`WaterParticles.h/.cpp` a `WaterEmitters.h/.cpp` obnovují skutečné HoseParticle, SprinkleParticle, HoseEffect a SprinkleEffect. Čtou původní Effects/HoseEffect.xml, Effects/SprinkleEffect.xml a WaterParticle.dds. Emitery skutečně vytvářejí a aktualizují vlastní částice, uchovávají původní pořadí, režimy emise, RNG, pohyb/škálování/alpha a odložený úklid. Nejde o pozorovací produkční provider. Skutečné Farm/Sprinkle/Person napojení se obnovuje dál.

`python tools/validate_water_effects.py` porovnává obě částice: PC53/PC24 celkem708 stavů, dvanáct konstrukcí a kopií, 288 GPU snímků / 138 240 000 totožných RGB pixelů. Direct steps používají osm elapsed hodnot, copy/reset i skutečný normal/absolute frame dispatch. Původní CRT rand běží v diagnostice skutečně; source GameRandom má stejnou posloupnost a celý stav.

Varianta `--emitters`: 1 076 emitter stavů, 512 přímých vytvoření částic, 8 238 navazujících particle/prototype stavů a 352 GPU snímků / 168 960 000 shodných pixelů. Obsahuje timed0.3, count4 a infinite režimy, stop, původní počitadla/akumulátory, skutečně vytvářené částice, D3D8 draw API, particle parent/list links a skutečné deferred delete/refcounts. Standalone particle GPU prefix se v tomto režimu neopakuje, aby Win32 diagnostická monotonic halda vystačila i pro emission frames.

Native HoseParticle604 / `0x25c`: Picture568, velocity+238/+23c, gravity+240, maximumScale+244, finished byte+248, originalSize+24c/+250, lifetime+254, elapsed+258. SprinkleParticle600 / `0x258`: velocity+238/+23c, finished+240, maximumScale+244, originalSize+248/+24c, lifetime+250, elapsed+254. Copy zachová velocity/scale/originalSize/lifetime, resetuje finished/elapsed; Hose copy navíc resetuje gravity0.

MaxScale XML se po načtení vždy sníží o1. Particle step přičte čas animace, změní rychlost a pozici, elapsed/lifetime ratio, velikost a alpha min(1,2−2×ratio), dokončení je strict elapsed>lifetime. X position a X size mají dodatečné float spills, Y zachovává mezivýpočty širší. Pro ratio se používá ještě neuložená suma elapsed. `OriginalArithmetic` a x87 instruktáž zachovávají tyto rozdíly v PC53 i PC24.

Shared gravity `0065d288/8c` má v původních PE datech {0,10}; actual Sprinkle XML nastaví {0,600}. Lifetime a emitter RNG používají přesný float coefficient `3.0518509447574615e−05`. Water vector rotation `00441810` používá degree factor `0.01745329238474369`, float cosine/negative sine, ale dosud neuložený sine v X komponentě. Win32 source provádí stejný x87 fcos/fsin a spills. Hose trajectory zachovává random distortion, speed/lifetime, normalization/inverse spills, rotation, X velocity a dopočítanou Y gravity. Direct creation porovnává všechny tyto výsledky.

| Adresa | Obnovená operace |
| --- | --- |
| `004416c0` / `00441790` | HoseParticle ctor/copy. |
| `004418f0` / `00441a60` / `00441e20` | Hose XML, actual Picture normal step a particle absolute step. |
| `00460a90` / `00460b20` | SprinkleParticle ctor/copy. |
| `00460bc0` / `00460a70` / `00460e30` | Sprinkle XML/lifetime/gravity, JMP do Picture normal step a absolute motion. |
| `00441770` | Společný scalar destructor pro obě particle specializace. |
| `00445420` / `00442370` / `00442820` / `004440c0` | Hose emitter ctor/XML/create/absolute dispatch. |
| `00464270` / `00461370` / `00461820` / `00462f10` | Sprinkle emitter ctor/XML/create/absolute dispatch. |
| `00442fa0` / `004433f0` / `00443830` | Hose count/infinite/time generation loops. |
| `00461df0` / `00462240` / `00462680` | Stejné původní režimy pro Sprinkle template. |
| `0042ccc0` | Start time stream: mode1, duration==0 infinite, generating1, accumulated=interval a timeGenerated0. |
| `004446c0` / `00463510` | Emitter destruction, inl56 deferred requests a owned Control/View cleanup. |
| `00445540` / `00464380` | Derived scalar destructors. |

Emitter rooty jsou cControl, konstruktor/XML skrývá a vypíná **prototype particle**, nikoli emiter. Jejich normal step `00445580` je skutečný JMP do původního RET `004bd820`; práce běží v absolute step. Parent/owned links jsou porovnané. Emission time loop i po dosažení time limit zpracuje zbytek akumulátoru v aktuálním while; count-mode retry na null tvorbě interval nespotřebuje. Hotové particles jsou odstraněné ze seznamu a queued se skutečným origin `.\Game\Farm\HoseEffect.cpp206` / `.\Game\Farm\SprinkleEffect.cpp190`; destructor používá `d:\work\farmer\src\farmer\game\ParticleSystemMy.inl56`.

Native HoseEmitter648 / `0x288`, SprinkleEmitter628 / `0x274`. Common fields: accumulator+22c, count limit+230, created+234, list head+23c/count+240, interval+244, timeLimit+248/generated+24c, generating/infinite+250/251, predicate+258, mode+25c a prototype+260. Hose angle+264 ctor neinicializuje, Speed+268/+26c, radius+270, start+278/+27c, target+280/+284. Sprinkle lifetime range+264/+268, randomAngle+270. Primary emitter vtables mají62 entries; další adresa již patří jinému objektu. Particle vtables mají71/69 entries; MI tables mají dvě položky, třetí nula se nevydává za další method.

Diagnostika originální program pouze mapuje; produkční kód jej nevolá. Native cGame discard se pozoruje, skutečná source ViewRegistry.deleteDiscarded i native scalar deletion se provádějí a porovnávají. Decimal/concat CRT adapters zachovávají původní origin strings bez potřeby spustit EXE entry point.

Zbývají actual Farm/Sprinkle/Person callers a celé source game EXE/playthrough/saves/audio. Missing Frequency čte v native starý stack word; source zatím hlásí tuto nepodporovanou větev explicitně. Další start/target/angle sequences, reload/owner interruption, malformed XML, zero/negative/NaN periods, allocation retry a exception ABI ještě nejsou ověřené. Slyšitelné audio ani celá hra nejsou těmito grafickými kontrolami prokázané.
