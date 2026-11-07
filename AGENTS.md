# Runtime incident on the user's host

On 2026-11-07 the user reported a Windows/display freeze during game tests,
requiring a restart. The host has an active local incident hold in
`.local/desktop-runtime-hold.json`.

- Do not launch graphics, DirectInput, physical-input automation, or game audio
  runtime tests on the user's ordinary desktop while this incident is unresolved.
- Do not run the full CTest suite on the host: it includes graphical runtime tests.
- Do not clear or bypass the incident hold. Runtime investigation belongs in an
  isolated guest; a further host experiment requires the user's explicit agreement
  to that concrete experiment after its safeguards have been reviewed.
- Continue reconstruction, native instruction analysis, builds, report auditing,
  and CPU-only checks that do not open graphics devices or inject input.
- The interrupted run `.local/tutorial-warning-runtime/20261107T211631362554Z`
  has no completed report. Do not restart it automatically or count it as a pass.
- `windows_game_memory.py` must not suspend a running rendering/UI thread to
  obtain a scene snapshot. Optimistic read validation is not an atomic snapshot.

Preserve the original full reconstruction goal, original files and Ghidra project.
Make progress commits; do not publish anything to GitHub.
