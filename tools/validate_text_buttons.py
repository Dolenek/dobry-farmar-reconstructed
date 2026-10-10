#!/usr/bin/env python3
"""Compare owned text buttons and native menu XML methods with explicit service boundaries."""
import argparse
import datetime
import hashlib
import json
import subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--oracle',type=Path,default=ROOT/'build/windows/Release/text-buttons-reference-test.exe')
    parser.add_argument('--output',type=Path,default=ROOT/'.local/text-buttons-validation.json')
    args=parser.parse_args()
    original=ROOT.parent/'farmář/Dobrý farmář/Dobrý farmář.exe'
    inventory=json.loads((ROOT/'analysis/inventory.json').read_text(encoding='utf-8'))
    expected=next(row['sha256'] for row in inventory['files'] if row['path']==original.name)
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    executable=args.oracle.resolve(strict=True)
    contexts=[]
    for precision in ('preserved','original-device-0x20'):
        command=[str(executable),str(original),str(original.parent/'Data')]
        if precision=='original-device-0x20':command.append('device-precision')
        result=subprocess.run(command,capture_output=True,text=True,timeout=90)
        if result.returncode:raise RuntimeError(f'Owned text button oracle failed ({result.returncode}, {precision}): {result.stderr}')
        context=json.loads(result.stdout)
        assert context['scenarios']==1080 and context['lines_destroyed']==9 and context['letter_comparisons']>4000
        assert context['menu_views_destroyed']==3 and context['decoration_comparisons']==15
        assert context['menu_content_comparisons']==27
        assert context['menu_frames']==48 and context['menu_pixels']==23040000
        assert context['menu_input_frames']==24 and context['menu_sound_requests']==12
        assert context['edit_frames']==2400 and context['edit_letters']>20000
        assert context['copies']==60 and context['copy_letters']==690
        assert context['list_states']==1398 and context['list_letters']>315000
        assert context['list_boxes']==12 and context['list_selections']==960 and context['list_xml']==2
        assert context['multi_states']==2384 and context['multi_letters']==43392 and context['multi_objects']==16
        assert context['menu_text_branches']==18
        context['precision']=precision;contexts.append(context)
    report={key:sum(context[key] for context in contexts) for key in ('scenarios','letter_comparisons','lines_destroyed','decoration_comparisons','menu_views_destroyed','menu_content_comparisons','menu_frames','menu_pixels','menu_input_frames','menu_sound_requests','edit_frames','edit_letters','copies','copy_letters','list_states','list_letters','list_boxes','list_selections','list_xml','multi_states','multi_letters','multi_objects','menu_text_branches')}
    fault=subprocess.run([str(executable),str(original),str(original.parent/'Data'),'clear-fault'],capture_output=True,text=True,timeout=90)
    if fault.returncode!=77:raise RuntimeError(f'Original scrolled ListBox clear did not match documented fault: {fault.returncode}, {fault.stderr}')
    report['list_clear_fault']=json.loads(fault.stdout)
    assert report['list_clear_fault']['native_clear_fault'] and report['list_clear_fault']['source_clear_exception']
    assert report['list_clear_fault']['exception_code']==0xc0000005
    report['precision_contexts']=contexts
    controller_contexts=[]
    for precision,mode in (('preserved','controllers'),('original-device-0x20','controllers-device')):
        result=subprocess.run([str(executable),str(original),str(original.parent/'Data'),mode],capture_output=True,text=True,timeout=90)
        if result.returncode:raise RuntimeError(f'YesNo controller oracle failed ({result.returncode}, {precision}): {result.stderr}')
        context=json.loads(result.stdout)
        assert context['dialog_states']==1200 and context['dialog_letters']==20792 and context['dialog_objects']==17
        assert context['dialog_callbacks']==970 and context['dialog_discard_checks']==1
        assert context['dialog_frames']==32 and context['dialog_pixels']==15360000
        context['precision']=precision;controller_contexts.append(context)
    for key in ('dialog_states','dialog_letters','dialog_objects','dialog_callbacks','dialog_discard_checks','dialog_frames','dialog_pixels'):
        report[key]=sum(context[key] for context in controller_contexts)
    report['controller_precision_contexts']=controller_contexts
    assert hashlib.sha256(original.read_bytes()).hexdigest()==expected
    report.update(status='passed',recorded_at_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),reference_sha256=expected,
        oracle_sha256=hashlib.sha256(executable.read_bytes()).hexdigest(),original_entry_point_executed=False,reference_files_modified=False,
        original_functions=['0051fe90','0041e8d0','0051f5a0','0041ed10','0051f570','0051f9b0','0041f120',
            '0051dbf0','0041f5f0','004dd980','004d43a0','004eded0','004d22a0','004f1860','004f4b80','004f20f0','00508560',
            '005202d0','004fe470','0051f540','0046bea0','00508120','004f5480','004da840','004d58a0',
            '004fb040','004fa8c0','004fafc0','00481e10','00481770','00531b20','00506a10','0052fe50','00515830',
            '005213c0','00521500','00521820','00521840','00521860','00521f00','00522320','00522860','00522c40',
            '004d56b0','004d20b0','004ed720','004f1730','004f5b60','00507f00','004dd750','0051d8a0','0051f100',
            '00521100','005206e0','005212a0','00521020','00521060','00521080','0051e6d0','0051ec50','0051d920'],
        test_adapters=['Allocation/free and String/WString operations use diagnostic CRT/recovered ABI strings',
            'XML virtual attribute/section calls wrap recovered trees parsed from original packages',
            'Typed XML scalar/vector/color conversion uses controlled modern stream adapters; old CRT extraction is unverified',
            'Native font lookups execute original function over original-layout fixtures populated from shipped font records',
            'Resource factories return recovered GPU textures and diagnostic sound references; audible playback is unverified',
            'Native click-mask factory boundary uses recovered mask loading, separately compared over all shipped JNGs',
            'Screen cursor samples are controlled; original cursor frame/window transform execute; native input registration uses recovered binding sets',
            'Three texture-switching parents use a separate base XML pass from the shipped Options/SoundVolume/Slider definition',
            'Main-menu object construction follows 00484da0 control allocations without executing its singleton/action wiring',
            'Existing fixed-background singleton exercises the guard of 00481990; generic MenuView hooks replace MainMenu hooks in this fixture',
            'Source menu names come from recovered Profile file persistence and actual game defaults through the resource/XML pipeline; native menu name service retains an original-layout WString fixture, with lifecycle compared independently'],
        coverage=['Six shipped MainMenu animated buttons: constructor, native XML, localized owned letters, parent state/alpha/position/text changes, sprites and owned destruction',
            'Three texture-switching owned text buttons: native parent/text construction and XML, nonnull texture switches, per-texture reference deltas and child destruction',
            'MenuView MainMenu/FixedBack/Options decorations: repeated XML loads, native sprites, owned destruction and per-texture reference deltas',
            'Whole main-menu content: seven owned text buttons, two text lines, three backgrounds, native 00481e10 XML bindings and 20 profile-name refresh sequences per context',
            '48 whole-scene menu RGB comparisons per context: independent source/native hierarchies and GPU buffers; native 00531b20 step/transition frame, 00506a10 traversal, 0052fe50 submission and 00515830 draw; transparent hide/show through completion',
            'Six buttons driven through cursor hover, press, release and leave via recovered input dispatch; states and sound request order match native methods',
            'Profile NameEdit: 12 native/source constructors and destructors per context, actual shipped XML, character input through binding sets, mutable/rejecting callbacks, width limits, caret geometry/sprites/blink and three alignments',
            'Profile NameEdit: RTL off/on, Czech/Hebrew keyboard-language fixtures, whitespace, backspace, Enter/LF, unavailable/Czech/Hebrew/NBSP characters and disabled binding cleanup',
            'Both original GPU x87 precision contexts'],
        pending=['Original XML parser and old CRT typed conversion ABI','Full object/exception ABI',
            'Original MainMenu loader 00481e10 ignores legacy submenu sections',
            'MainMenu singleton, navigation callbacks and unusual enable/disable hooks','Whole menu physical input/frame service integration and actual navigation','Actual sound loading/playback','Complete main game EXE'])
    report['test_adapters'] += ['Test-only monotonic allocation preserves matching address ranks: original equal-depth UI comparator orders by object pointer; production comparators remain unchanged',
        'Native graphics virtual submission boundary uses recovered SpriteBatches; native batching routines themselves are not executed by this scene test',
        'Source scene processes reconstructed GameTimer over controlled milliseconds and uses its smoothed elapsed value in UI motion/transitions; timer is separately compared over 120000 native frames',
        'Native 00530290 cursor update calls original 00512330 bounds and 0050d260 transform over physical Win32 window bounds with controlled screen samples; source uses recovered GameCursor',
        'Native input process boundary dispatches controlled events using recovered InputBindings; physical DirectInput polling is separate work',
        'Native timer advance and graphics begin/finish are controlled empty test services; native elapsed getter freezes the same recovered timer value after the source advance; mesh/extra-drawable/discard registries are empty',
        'Sound virtual methods record and compare resource names, hover arguments and request order; actual playback remains unverified',
        'Native draw fixture uses sprite-only linked batches and previously checked recovered render-state setup; device creation, reset, mesh and complete graphics singleton ABI remain separate work',
        'GPU readback ignores only the undefined X byte of X8R8G8B8; compares all RGB bytes exactly',
        'Draw API trace normalizes only distinct source/native vertex/index buffer identities; texture identities and draw parameters remain exact']
    report['test_adapters'] += ['EditBox keyboard-language query uses a controlled GetKeyboardLayout IAT fixture; physical keyboard/IME is unverified',
        'EditBox native substring copies use recovered WString adapter; original character routing, glyph/width comparisons, callbacks, blink, caret and lifetime instructions execute']
    report['test_adapters'] += ['ListBox native vector growth uses diagnostic CRT memmove; original clone/append/vector bookkeeping execute',
        'ListBox Strings decimal stream key formatting uses controlled modern stream construction/insertion/extraction/destruction; native XML clone/replacement and unchanged logical end execute',
        'Original scrolled clear is checked in a separate process terminated by its access-violation observer; source bounds check raises out_of_range instead of dereferencing released storage']
    report['coverage'] += ['120 native/source TextButton copies: copied resources/state, fresh owned text pool and reset motion, position/string mutation and destruction reference deltas',
        '24 ListBox constructors, shipped UserList XML, 732 appended profiles, 2796 whole list states, 1920 indexed selection callbacks, scrolling/repositioning, clear/repopulate and owned destruction in both device precisions',
        'Four indexed Strings XML loads: empty reserved and populated vectors, logical count unchanged, replaced/orphan clones retained by View hierarchy and all text/resource lifetime compared',
        'Original scrolled ListBox clear access violation confirmed; safe source exception documented as an intentional memory-safety difference']
    report['pending'] += ['Control copy with Mesh/Animation resources; TextAnimationButton copy and full original object ABI',
        'ListBox invalid/zero spacing, negative bounds and Strings exceeding reserved capacity; full profile selection controller integration']
    report['original_functions'] += ['004f5e70','004f8580','004f8ce0','004f8e10','004f7e10','004f7250','004f7700','004f6a80','004f6130','004db210']
    report['coverage'] += ['MultilineText: 32 native/source owned trees, 4768 full states and 86784 letter/sprite comparisons; /n and LF splitting, empty/trailing rows, space-based wrapping and over-width words, all four vertical alignments, RTL off/on, localization on/off, immediate/deferred updates, position/alpha propagation and destruction',
        'MenuView BackText/BackMultitext: 36 accumulated decoration comparisons, repeated XML loading and owned resource deletion in both precision contexts; attribute text and overriding nonempty Text section, color, wrapping and RTL']
    report['test_adapters'] += ['MultilineText and MenuView XML section text wraps recovered XmlNode text; string length/find/compare/substring operations use explicit recovered/modern CRT boundaries',
        'TextLine width getter 004db210 executes original dirty-layout path through its vtable; relocation operands include this indirect dependency',
        'Test-only monotonic heap reserves 512 MiB after the enlarged comparison exhausted its previous 256 MiB; allocation order and production code are unchanged']
    report['pending'] += ['MultilineText invalid/negative dimensions and infinite/NaN spacing; complete SelectProfile controller navigation']
    report['original_functions'] += ['00501b60','00500df0','005014d0','00501530','00501590','00501970','00500650','00500a20','00501a20','00501430','00501480']
    report['coverage'] += ['YesNo controller: 34 source/native owned dialog trees using shipped UI/YesNo.xml, 2400 full states, 41584 letter/sprite comparisons, 1940 keyboard/button callbacks, nullable/enabled/initially disabled suspended view, disable/enable/step enforcement and destructor restoration',
        'YesNo scene: 64 native/source GPU frames with actual owned background/buttons/multiline text, 30720000 exactly matching RGB pixels and draw API calls; cursor hover/press/release/leave, Enter/Space/Escape dispatch, text updates, alpha and movement, RTL off/on',
        'YesNo default callback: matching duplicate discard requests/origin in both precisions; source queue owns deferred frame deletion and removes input bindings']
    report['test_adapters'] += ['YesNo native XML virtual factory/release uses an independent recovered tree parsed from shipped UI/YesNo.xml; source constructor uses actual ResourceManager loadSettingsXML',
        'YesNo native callbacks, key bindings, suspended-view lifecycle and step execute original instructions; default callback decimal formatting/concatenation use recovered diagnostic String boundaries and discard service records the request instead of executing the native queue',
        'YesNo controller cases run in separate processes from earlier UI scenarios to bound monotonic diagnostic heap consumption; scene uses the same explicit cursor/timer/input/graphics/sound boundaries as main-menu rendering']
    report['pending'] += ['Native nonempty discard-queue traversal/deletion and old CRT diagnostic origin formatting; physical dialog keyboard/mouse and actual audio']
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(f"Owned text buttons: {report['scenarios']} scenarios, {report['letter_comparisons']} letters/sprites, {report['lines_destroyed']} owned trees destroyed")
if __name__=='__main__':main()
