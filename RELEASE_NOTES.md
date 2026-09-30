# YBC:KeyShifter 1.0.0

Initial Windows x64 release.

## Included

- VST3 plugin bundle: `YBC KeyShifter.vst3`
- Standalone application: `YBC KeyShifter.exe`
- Plugin display name: `YBC:KeyShifter`
- Vendor: `YoungBonesClub`

## Features

- Estimate BPM and musical key from dropped audio samples.
- Calculate pitch/varispeed shifts from source BPM or a target BPM.
- Display resulting key, tuning offset and speed ratio.
- Russian-first UI with an English option and host-state language persistence.
- Transparent audio pass-through.

## Installation

Copy `YBC KeyShifter.vst3` into the host's VST3 folder and rescan plugins. The standalone executable can be launched directly. Windows x64 is required.

## Source build

See `README.md` for build requirements and instructions. The Windows build uses MSVC Build Tools, CMake and Ninja; a full Visual Studio IDE is not required.
