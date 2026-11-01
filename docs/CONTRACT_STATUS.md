# ContractStatus

`src/game/ui/ContractStatus.cpp` reconstructs the actual 136-byte native
controller using modern owned View children. It creates the substrate, goal
button, two product icons, two amount texts, two checkboxes, time text and
progress bar, then loads `ui/gameui/ContractStatus.xml`. GoalButton uses the
now recovered **ExtendedAnimationButton**, with its own borrowed status owner
and the original click callback. No original program code enters production.

## Methods and data

| Original | Reconstructed behavior |
| --- | --- |
| `004bb830` / `004b92c0` | constructor/XML ownership and sound-first destructor |
| `004ba730` | View XML, ten named children, LosSound/WinSound retained reloads |
| `004bd2f0` | new/restored assignment, initial stocks, deadline/progress/requirements |
| `004b98e0` | tutorial step 18 availability and goal-button anchor |
| `004bcc20` | absolute timer, expiry branches, readiness and profile updates |
| `004ba0b0` | compact product requirements, icons, signed amount text/ticks |
| `004bbee0` | fail, profile reset, money/stock requests, award count, sound/note |
| `004bc460` / `004bc840` | bool `taskPassCancel{id}` with default insertion |
| `004b9350` | goal click: tutorial guard then ready/requirements dialog |
| `004b93e0` | unsigned double keys `req{id}_{slot}` |
| `00467fa0` / `004036e0` | stock guard and borrowed product lookup |
| `004b1080` | padded unsigned time text, optional hours |

Native fields: id +2c, substrate +30, goal +34, icons +38/+3c, texts
+40/+44, checks +48/+4c, time +50, progress +54, accumulator +58,
remaining +5c/+60/+64, duration +68, event +6c, active/dirty +70/+71,
initial stocks +74/+78, sounds +7c/+80, expiry +84. Vtables:
`005f9f80` (14 slots), goal `005fa158` (89 slots), appearance subobject
`005fa14c` (two slots). Native goal allocation 868 bytes includes status +360.

Profile stores are verified from instructions, rather than ambiguous Ghidra
types: task/req/Money are **int**, WGoodCount/awQTask **uint**, taskTime **float**,
taskPassCancel **bool**. High-bit ids use unsigned decimal formatting.

## Preserved native details

- Assignment records stocks only for reqType 0, leaves unused slots unchanged,
  and resets passCancel only for a new task. Restored time reads taskTime.
- Deadline arithmetic wraps uint32 before x87 unsigned-to-float conversion.
  The accumulator comparison uses the unspilled sum. It subtracts at most one
  second per frame, including elapsed values greater than one second.
- Hour borrowing sets minutes to 59 and leaves seconds at zero. Minute
  borrowing sets seconds to 59. MM:SS omits hours even for long deadlines.
- After fail/completion, the original method continues the same frame's
  accumulator/readiness portion despite having cleared active.
- Dirty readiness always writes passCancel=true, including insufficient stocks.
  Ready only determines whether the ready-dialog request is made.
- Fail uses event **Bonus +44**, **WinNote +68**, **WinSound +80**. Expiry with
  passCancel uses **LosNote +84**, **LosSound +7c**. These selections are
  intentionally preserved despite the names suggesting the opposite.
- Requirement amounts use signed decimal text after unsigned min/subtraction.
  Missing requirements hide the current compact pair. The final hide loop
  selects by the fixed compact count; with no product requirements it hides
  pair 0 twice, leaving pair 1's prior visibility unchanged.
- Root sounds and tooltip/ProgressBar children retain the original reload
  behavior. Destruction releases the current sounds before owned children.

## Verification

`python tools/validate_contract_status.py` runs native methods and recovered
source independently in PC53 and real Direct3D8 PC24 precision. The current
report is `analysis/contract-status-validation.json` after recording the full
regression; local runs are `.local/contract-status-validation.json`.

31,264 complete state comparisons, 5,144 assignments, 12,480 timer steps,
12,480 direct clicks, 16,384 time formatting cases and 252,678 letter states.
Every one of the **131 shipped contracts** is assigned in four ids (0, 1, 2,
0x80000001). Additional states cover reqType 0/1/2, stock/count boundaries,
hour/minute borrowing, pause predicates, inactive/expired cases, both expiry
branches and unsigned award wrapping. Constructor XML ownership, stock
out-of-range guards, retained children and destructor sound release are checked.

64 complete GPU frames contain matching ordered sprite submissions/draw API
calls and **30,720,000 RGB pixels**. Eight normal press/release sequences
actually open dialogs through InputBindings. Eight corresponding releases
with another tutorial step active remain blocked. Cursor and clock are controlled.

Old CRT streams, native XML and profile lookup are explicit diagnostic
adapters; padding/controller/UI instructions execute. Product and event fixture
records come from previously independently verified definitions/configuration.
The money setter uses cdecl, established by caller stack cleanup at `004bbf8e`.
Diagnostic sound playback is disabled; these tests cover requests and lifetime.
Native accumulator/duration/event/initialStocks are unspecified before assignment;
the zeroed fixture does not establish original allocator behavior. Source chooses
defined initial values for these otherwise unused fields.

## Integration still required

`ContractStatusWorld` is mandatory and entirely abstract. It requires the
actual tutorial, elapsed/transition state, cGame money refresh, warehouse
stock change, GameMenu removal/notifications and three dialogs at `004b7fe0`,
`004a94e0`, `004b6b90`. The diagnostic records ordered requests; it does not
claim to implement those world effects. Real binding, full scenes, audible
playback, physical devices and bidirectional saves remain pending. Invalid
product lookup dereferences/old-CRT exception and locale paths are unverified.
This module is part of ongoing reconstruction, not a playable main EXE.

## Actual opposite-program restoration and natural expiry

The service-fixture boundaries above are historical. The current complete applications now restore the actually accepted rose contract37 from each other's ordinary saves. The source resumes the native task with230 seconds; native startup already consumes one second and reports229. The original15× shortcut then advances the real countdown without supplying the missing roses. Both actual tasks expire, clear slot0/time0, retain the original inactive passCancel=true flag, open ContractFail and close it through its actual OK button. Money stays4504 and stock stays roses2: the already paid30 deposit is lost, with no additional expiry charge or bonus.

Four work snapshots per EXE and all final discrete profile properties agree. Time1390 follows the actual natural expiry of later letters and precedes the next1410 event and RunTime1800 award. The two original1140/1335 event notes are added to the journal; all4563 save bytes, coefficients, seed records and sale records are exact. The failure dialog has29 equal actual textured sprites,26 equal glyph sprites and14850 exact caption RGB pixels. Its original Dogovor.jng uses PNG IDAT alpha, while ContractProvide's panel uses JPEG JDAA alpha. Both audited blue regions have alpha254. The failure backdrop has3612 differing pixels, maximum1; its independently frozen world phases are explicitly not claimed equal. The initial JPEG-only auditor assumption was corrected without changing production or original data.

Four further complete restores (native0→source2→native4→source5, plus source1→native3) compare every profile property and all4563 save bytes. The three paid sprinkler positions and20 actually watered fields remain active. Both source restores were observed by the read-only exception debugger, exit normally and have no access violations. No save/property bytes are normalized or written by the probe. The completed-task streak was already0 in this fixture, so natural resetting of a nonzero streak is still pending.

Reproduce with `tools/validate_game_contract_expiry.py`, `tools/validate_contract_expiry_state.py`, `tools/validate_game_contract_expiry_interop.py --crash-trace build/source-crash-trace-vs/Release/source-crash-trace.exe` and `tools/record_contract_expiry_verification.py`. Reports: `analysis/contract-expiry-state-validation.json`, `analysis/contract-expiry-interop-validation.json`, `analysis/contract-expiry-regression.json`. Production source hashes still match the complete102/102 regression; all17 original files remain unchanged. Remaining natural world events, awards, whole DLL behavior and other original-goal scopes continue.
