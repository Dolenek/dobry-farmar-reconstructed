# Runtime incident on the user's host

On 2026-11-07 the user reported a Windows/display freeze during game tests,
requiring a restart. The host has an active local incident hold in
`.local/desktop-runtime-hold.json`.

- The user subsequently explicitly authorized continued use of this machine
  with monitoring; see `analysis/host-runtime-monitoring-authorization.json`.
- Host graphics/input/audio experiments must run in the independently supervised
  Job created by `host-test-watchdog`, with progress/health logs and hard limits.
  Start with short probes before longer scenarios. No unmonitored host runtime.
- Do not run the full CTest suite on the host: it includes graphical runtime tests.
- Do not clear the incident hold or use an environment flag to bypass it. The
  monitored lease must verify a live watchdog executable and actual Job membership.
- Continue reconstruction, native instruction analysis, builds, report auditing,
  and CPU-only checks that do not open graphics devices or inject input.
- The interrupted run `.local/tutorial-warning-runtime/20261107T211631362554Z`
  has no completed report. Do not restart it automatically or count it as a pass.
- `windows_game_memory.py` must not suspend a running rendering/UI thread to
  obtain a scene snapshot. Optimistic read validation is not an atomic snapshot.

Preserve the original full reconstruction goal, original files and Ghidra project.
Make progress commits; do not publish anything to GitHub.
