# Obrazovka nastavení

Options obnovuje cOptions (108 native bajtů): constructor `00489770`, XML `00487ae0`, destructor `00487a30`/`00487ac0`, return callback `00488f10`, fullscreen `004892f0`, sound/music value callbacks `00488770`/`00488b40`, return-view setter `00487aa0`. Vlastní tři Picture (Back, Logo, Podlogka), Ok TextAnimationButton, FullScreen CheckBox, dva Slider a čtyři TextLine. Načítá skutečné `UI/Options.xml`; shipped XML neobsahuje tři staré Picture sekce, ale původní constructor je stále alokuje.

XML po UI čte graphics/fullscreen a jen změní CheckBox hodnotu, ponechá Tick update odložený na step. SoundManager disabled=true vypne oba slidery. Gettery se volají music a potom sound, hodnota se převede původním signed-int/x87 plus 2^32 postupem a přepočítá thumb. Callback hlasitosti používá truncation směrem k nule a předává dolních 32 bitů. Fullscreen callback nejdříve zapisuje skutečné Settings, potom vyžaduje graphics reset(true). OK nejdříve target.showTransparently/enable, potom vlastní discardTransparently. Návrat bez bound target má explicitní source exception místo původní neplatné pointer dereference.

OptionsServices nemají default ani prázdný backend; všech šest sound/graphics funkcí je nutné dodat. Jejich skutečné propojení s kompletním SoundManager a D3D reset dosud chybí. Diagnostické služby existují pouze v testu a zaznamenávají stav/pořadí požadavků. Nejsou vydávané za přehrávání ani změnu režimu monitoru.

Obě původní přesnostní prostředí prošla: 24 source/native stromů, 1 560 stavů, 53 040 textových písmen/spritů, 564 audio service událostí, 244 reset requests a 96 GPU snímků/46 080 000 přesně shodných RGB pixelů. Zahrnuje RTL, fullscreen false/true, disabled sound, různé hlasitosti, záporné a zlomkové finite callback values, oba slidery, fullscreen click, OK a fade návrat i singleton/resource/input cleanup. Native service hranice používají nezávislé fixtures, source Settings a XML jsou skutečné obnovené implementace. Target je obecný MenuView, nikoli dokončený MainMenu controller.

Win32 Release sestavení a všech 35 CTest kontrol prošlo (84,47 s); SHA-256 všech 17 původních souborů zůstalo shodné. Aktuální portable reporty obsahují Options v obou přesnostech. Předchozí překážka schvalovací kontroly byla odstraněna a odložené příkazy skutečně proběhly.

Zbývá skutečné audio, physical input, D3D fullscreen/windowed reset, MainMenu navigace, missing fullscreen property, opakované XML ref leaks, neplatné/NaN/infinite hlasitosti, object/exception ABI a celý herní runtime. Původní programový kód používá pouze testovací oracle.
