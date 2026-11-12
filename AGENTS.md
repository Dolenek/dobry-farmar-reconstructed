# Runtime testing

When the local `.local/desktop-runtime-hold.json` gate is active:

- Run graphics, input, and audio experiments in an independently supervised
  Job created by `host-test-watchdog`. Keep health and progress logs, use hard
  time limits, and start with short probes.
- Use focused CPU-only checks in an ordinary test session. The full CTest
  suite includes graphical runtime tests.
- Keep the gate active. A monitored lease must verify the live watchdog
  executable and actual Job membership.
- Count only completed reports as passes. Take scene snapshots without
  suspending a rendering or UI thread. Optimistic read validation only checks
  consistency of the observed topology.

See [runtime testing](docs/RUNTIME_TESTING.md). Preserve the original files
and Ghidra project. Make progress commits and keep them local unless asked
to publish.
