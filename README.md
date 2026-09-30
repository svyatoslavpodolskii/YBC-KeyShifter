# YBC:KeyShifter

A lightweight VST3 pitch and tempo calculator for varispeed sample work. Load a sample to estimate its BPM and key, then calculate the tempo and tuning produced by a pitch shift. Audio passes through unchanged; this is an analyzer and calculator, not a time-stretch effect.

**Vendor:** YoungBonesClub  
**Developed by:** Svyatoslav Podolskii
**Formats:** VST3 and standalone app  
**Current build target:** Windows x64

## Features

- Drag and drop WAV, MP3, AIFF and FLAC samples.
- Estimate BPM and musical key, with confidence readouts.
- Calculate resulting BPM from a pitch shift, or calculate the shift from a target BPM.
- Enter semitones directly, including signed fractional values such as `+7.50` and `-3.00`.
- View the resulting key, tuning offset and playback-speed ratio.
- Switch between Russian and English using the flag control. Russian is the default, and the selected language is saved in the host state.
- Use the plugin as a transparent stereo/mono pass-through utility.

## Build on Windows

### Requirements

- Windows x64
- CMake 3.22 or newer
- Git
- MSVC C++ Build Tools with the Windows SDK
- Ninja (the build script checks the detected Build Tools installation and then PATH)

The full Visual Studio IDE is not required. The first configure downloads JUCE 9.0.3 with CMake FetchContent, so internet access is needed for the initial build.

### Build

Open the project folder in VS Code and press `Ctrl+Shift+B`, or run this from a regular terminal:

```bat
build_windows.bat
```

The script configures a Release build with Ninja and builds both the VST3 and standalone targets. It also initializes the MSVC environment automatically.

### Output and installation

The VST3 bundle is created here:

```text
build\ResampleCalc_artefacts\Release\VST3\YBC KeyShifter.vst3
```

Copy the whole `.vst3` folder to your host's VST3 plugin directory, for example:

```text
C:\Program Files\Common Files\VST3\
```

Then rescan plugins in your DAW. The display name is `YBC:KeyShifter`; the bundle uses `YBC KeyShifter` because colons are not valid in Windows filenames.

The standalone build is at:

```text
build\ResampleCalc_artefacts\Release\Standalone\YBC KeyShifter.exe
```

## How it calculates

For a pitch shift of $s$ semitones:

```text
result BPM = source BPM * 2^(s / 12)
```

For a target BPM:

```text
semitones = 12 * log2(target BPM / source BPM)
```

Tempo and pitch are physically linked in varispeed playback, so a BPM target can imply a fractional semitone value.

## Detection notes

BPM estimation uses onset-envelope autocorrelation. Key estimation uses FFT chroma and major/minor tonal profiles. These are musical estimates, not forensic analysis; drum-only material, noisy intros, modal harmony and tempo drift can reduce accuracy. BPM and key can be corrected manually.

## Source and third-party licensing

JUCE is fetched from its upstream repository at build time; it is not vendored in this repository. Check the JUCE 9.0.3 licensing terms and include any notices required by your distribution model. No license has been assigned to this project yet. Choose and add a project license before accepting outside contributions or distributing the source under an open-source license.
