<div align="center">

[Česky](README.md) · **English**

# Dobrý farmář

<img src="docs/media/game-icon.png" alt="Original Dobrý farmář application icon" width="48" height="48">

**A Czech farming game for Windows that can be built from source again.**

On a small farm, you tend your fields, harvest crops, sell produce at the market, and fill orders. You spend your earnings on better tools and production buildings as you expand your farm. The original tutorial guides you through your first steps.

[Farm expansion](docs/FARM_SCENE.md) · [Achievements](docs/AWARDS.md) · [Market](docs/MARKET.md) · [Production](docs/MANUFACTURE.md)

## How the game was reconstructed

### ≈ 90 h 30 min · 226 commits

**GPT-6.1 Sol** with **High** reasoning was the only model used for reverse engineering and source reconstruction. The agent analyzed the machine code in **Ghidra**. [Analysis workflow](docs/ANALYSIS.md)

I set the acceptance criteria, including saved-game compatibility in both directions. I asked the agent to compare each part against the original and commit its changes to Git after each completed piece of work. [Commit history](https://github.com/Dolenek/dobry-farmar-reconstructed/commits/main/)

The result is editable C++ code. The game uses the original graphics, sounds, and other assets from your copy. The agent compared the rules, controls, visuals, and loading of saved farms directly with the original.

[Working main menu](https://github.com/Dolenek/dobry-farmar-reconstructed/commit/8c6f98e1e6d6450d5bedf8531633165a691e2a45) · [Saved-game transfers](https://github.com/Dolenek/dobry-farmar-reconstructed/commit/4928fab169d28c065f876ddaffd508f57220bf85) · [Final audit](https://github.com/Dolenek/dobry-farmar-reconstructed/commit/eb0345bfb19ef7f14974e84dd4020cfadfa659a1)

## How we checked it against the original

| Result | What we compared |
| :---: | :---: |
| **48 tutorial steps** | Every step reachable in normal play ran in both the original and the reconstruction. [Tutorial results](analysis/current-tutorial-runtime-coverage.json) |
| **416 images** | Every decoded pixel matched for each JNG image in the original game data. [Graphics verification](docs/JNGLOAD.md) |
| **97 audio library functions** | Every public Squall function has a reconstructed implementation and a verified interface. [Audio verification](docs/SQUALL.md) |
| **4 save transfers** | After completing 40 orders, we transferred the save between versions in both directions. All 17,369 bytes matched on every restore. [Save transfer verification](docs/CONTRACT_STREAK.md) |

The game, the image loader, the audio library, and the uninstaller all build from source. All four components use the 32-bit Windows interface. We verified the uninstaller's effects in an isolated environment. [Verification summary](docs/COMPLETION_AUDIT.md) · [Known differences and verification limits](docs/VERIFICATION_LIMITS.md)

## How to play

**1. [Download Dobrý farmář for Windows.](https://github.com/Dolenek/dobry-farmar-reconstructed/releases/latest/download/Dobry-farmar-Windows.zip)** The ZIP includes the compiled reconstruction. You need your own Czech copy of the original game to play.

**2. Extract the entire ZIP** to a folder you can write to, such as Documents.

**3. Copy the `Data` folder from the original game** next to `Hrat.cmd`. It must contain `Data.pack` and `cz.pack`.

**4. Double-click `Hrat.cmd`.** The game creates a `Saves` folder for profiles and settings.

[All releases](https://github.com/Dolenek/dobry-farmar-reconstructed/releases) · [Build from source](docs/SOURCE_BUILD.md)

## Explore the source code

[Game code overview](docs/GAME.md) · [Technical development notes](README.agent.md)

</div>
