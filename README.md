# PumpedUpKick

![PumpedUpKick](readme-assets/PUK.png)

[简体中文](README_zh-CN.md)

A free, lightweight sidechain-ducking plugin — KickStarter's instant workflow with LFOTool-style envelope control.

**AU / VST3 / Standalone** · macOS & Windows · built with [JUCE 8](https://juce.com) (AGPLv3)

![Screenshot of this VST Plugin](/readme-assets/PUK.png)

## Features

- **One-knob workflow** — pick a preset, set the rate, turn the Mix knob. Done.
- **BPM-synced ducking** — locks to the host timeline (1/1, 1/2, 1/4, 1/8, 1/16). Pauses (and the playhead hides) when the host transport stops; free-runs only when there is no transport at all (standalone).
- **Waveform display** — the last cycle's input (grey) and ducked output (orange) are drawn behind the envelope, so you see exactly what gets carved away.
- **Full envelope editing** (LFOTool-style):
  - drag points to move them (hold **Shift** to snap to the grid)
  - **double-click** empty space to add a point
  - **double-click / right-click / ⌥-click** a point to delete it
  - drag a segment's **midpoint handle** vertically to bend the curve
- **MIDI one-shot trigger** — switch Trigger to **MIDI** and the envelope only fires on incoming notes: perfect for non-4/4 patterns. Audio passes through untouched between notes.
- **Presets** — 5 factory shapes (Classic, Soft, Tight, Long Pump, Wobble) plus save-your-own user presets.
- **Smooth control** — click-free gain smoothing (0.1–20 ms).
- **Tiny footprint** — the envelope renders to a lock-free lookup table; the audio path is one table read and one multiply per sample. No allocations, no locks, no FFT.

## Installing (pre-built downloads)

Grab a zip from [Releases](../../releases). macOS builds are universal binaries (Apple Silicon + Intel).

**macOS — the quarantine step is required.** This is free software built without a paid Apple Developer ID, so the binaries are not notarized. macOS quarantines anything downloaded from the web and your DAW will silently refuse to load it. After copying the plugins into place, run:

```sh
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/Components/PumpedUpKick.component
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/PumpedUpKick.vst3
```

If you'd rather not run the command, build from source instead: locally compiled binaries carry no quarantine flag and just work.

**Windows**: copy `PumpedUpKick.vst3` to `C:\Program Files\Common Files\VST3\`.

## Building

Requirements: CMake ≥ 3.24, a C++17 compiler (Xcode command-line tools on macOS, MSVC on Windows). JUCE is fetched automatically.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j8
```

macOS builds are universal (arm64 + x86_64) and target macOS 10.13. Both settings are applied *before* `project()` in [CMakeLists.txt](CMakeLists.txt), which is load-bearing: `project()` creates those cache entries itself, so setting them afterwards is silently ignored and you get a host-native, host-OS-only binary that still works fine on the machine that built it. If you have a build directory from before this fix, delete it — a stale cache keeps the old values no matter what the CMakeLists says.

On macOS the AU and VST3 are copied automatically to:

- `~/Library/Audio/Plug-Ins/Components/PumpedUpKick.component` (Logic Pro)
- `~/Library/Audio/Plug-Ins/VST3/PumpedUpKick.vst3` (FL Studio, Ableton, …)

If Logic doesn't see the AU immediately, run `auval -a` or restart Logic (it rescans on launch).

On Windows, copy `build/PumpedUpKick_artefacts/Release/VST3/PumpedUpKick.vst3` to `C:\Program Files\Common Files\VST3\`.

### Using it in Logic (MIDI trigger mode)

Insert PumpedUpKick as a **MIDI-controlled effect** (instrument slot → *AU MIDI-controlled Effects* → PumpedUp Audio → select the audio track as sidechain… or simply insert it as a normal audio effect and use Sync mode). In FL Studio, route any MIDI channel to the plugin via *MIDI output* port matching, or just use Sync mode.

## Distribution notes

- **macOS**: for distribution to other machines, sign both bundles with a Developer ID Application certificate and notarize:
  ```sh
  codesign --force --deep --sign "Developer ID Application: …" PumpedUpKick.component
  xcrun notarytool submit … && xcrun stapler staple …
  ```
  Unsigned local builds work fine on your own machine (Gatekeeper only checks downloaded binaries).
- **Windows**: build with MSVC (`cmake -G "Visual Studio 17 2022"`); code signing optional.
- **License**: JUCE is used under AGPLv3, so this project is AGPLv3 — source must remain available. VST3 support is via Steinberg's VST3 SDK (GPLv3-compatible). "VST" is a trademark of Steinberg Media Technologies GmbH.

## Automated builds & releases

[.github/workflows/ci.yml](.github/workflows/ci.yml) runs on **every** push/PR to `main` and builds both platforms. It doesn't just check that the code compiles — it loads the plugin and exercises it:

- `lipo` asserts both `arm64` and `x86_64` slices are present, so a release can never silently drop Intel support
- `auval` (macOS) instantiates the AU, renders audio and tests MIDI handling
- [pluginval](https://github.com/Tracktion/pluginval) at strictness level 10 runs parameter fuzzing, bus-layout and thread-safety checks against the AU and VST3 on macOS, and the VST3 on Windows

[.github/workflows/release.yml](.github/workflows/release.yml) runs the same validation, then packages and stages a release when you push a version tag:

```sh
git tag v1.0.0
git push origin v1.0.0
```

Note that a plain `git push` never pushes tags, so normal commits can't trigger a release by accident. The release is created as a **draft** with auto-generated notes (the commits since the last tag) and the zips already attached — rewrite the notes into something user-facing, then hit publish.

To sanity-check a build without tagging, run the workflow manually from the Actions tab (`workflow_dispatch`). That produces downloadable artifacts but never touches releases.

Builds are unsigned (see Distribution notes above); signing/notarization would need certificate secrets added to the repo and isn't wired up.

## License

Copyright (C) 2026 CanisAlpha

PumpedUpKick is free software: you can redistribute it and/or modify it under the terms of the [GNU Affero General Public License](LICENSE) as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. See [LICENSE](LICENSE) for the full text.

The UI embeds the [Special Gothic](https://github.com/AlistairMcCready/Special-Gothic/) typeface, Copyright 2023 The Special Gothic Project Authors, licensed under the [SIL Open Font License 1.1](src/font/OFL.txt).

## User presets

Saved as XML at:

- macOS: `~/Library/Application Support/PumpedUpKick/Presets/*.pukpreset`
- Windows: `%APPDATA%\PumpedUpKick\Presets\*.pukpreset`

Preset files are portable — share them by copying the file.

## Architecture

| Piece | File | Notes |
|---|---|---|
| Curve model | `src/Curve.h` | control points + per-segment tension, power-curve interpolation, lock-free double-buffered LUT |
| DSP / phase engine | `src/PluginProcessor.cpp` | ppq-locked phase in Sync mode, sample-accurate note-on retrigger in MIDI mode, one-pole gain smoothing |
| Envelope editor | `src/EnvelopeEditor.h` | vector-drawn, 60 fps playhead animation with change-detection to skip redundant repaints |
| Presets | `src/Presets.h` | factory shapes |
| UI | `src/PluginEditor.cpp`, `src/LookAndFeel.h` | flat dark theme, resizable (fixed aspect) |
