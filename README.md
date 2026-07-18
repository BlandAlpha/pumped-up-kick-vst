# PumpedUpKick

![PumpedUpKick](readme-assets/PUK.png)

[简体中文](README_zh-CN.md)

A free, lightweight sidechain-ducking plugin — KickStarter's instant workflow with LFOTool-style envelope control.

**AU / VST3 / Standalone** · macOS, Windows & Linux · built with [JUCE 8](https://juce.com) (AGPLv3)

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

**Linux**: copy `PumpedUpKick.vst3` to `~/.vst3/`. Works in REAPER, Bitwig, Ardour, Qtractor and other VST3 hosts.

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

## Using the MIDI one-shot trigger

> Note: This part is written by Claude. Currently I cant test every DAW myself, the following instructions is **REFERNCE ONLY!!** Do not trust blindly!

Sync mode needs no routing at all — insert the plugin as a normal audio effect and it locks to the beat. The **MIDI trigger** (one-shot) mode is for everything that isn't four-on-the-floor: switch **TRIGGER** to **MIDI**, and the envelope then runs exactly once per incoming Note-On (any pitch, any velocity — both are ignored) with **RATE** setting how long one shot lasts (1/4 = one beat). Between notes, audio passes through untouched. The catch: your DAW has to route MIDI notes *into an audio effect*, and every host does that differently.

**Logic Pro** — audio effects can't receive MIDI in Logic, so load it as a MIDI-controlled effect:
1. On the track you want ducked, set **Output** to a bus (e.g. Bus 1), and mute the Aux that Logic auto-creates (or set the Aux's output to *No Output*) so you don't hear the dry copy.
2. Create a **Software Instrument** track and pick **AU MIDI-controlled Effects → PumpedUp Audio → PumpedUpKick** in the instrument slot.
3. In the plugin window header, set the **Side Chain** menu (top right) to Bus 1.
4. Play or program notes on the instrument track — each note fires the envelope.

**FL Studio**
1. Load PumpedUpKick on the mixer insert of the track to duck.
2. In the plugin wrapper's gear menu → *Settings*, set **MIDI input port** to a number (e.g. 1).
3. In the Channel Rack, add a **MIDI Out** channel and set its **Port** to the same number. Notes played on that channel now trigger the envelope.

**Ableton Live**
1. Put PumpedUpKick on the audio track.
2. Create a MIDI track; set **MIDI To** to the audio track, and in the chooser below it pick **PumpedUpKick**.
3. Arm the MIDI track and play — clips or live input both work.

**REAPER** — simplest of all: track FX receive the track's own MIDI, so either put MIDI items directly on the audio track, or send MIDI from another track (create a send, set audio channels to *None* and MIDI to *All*).

**Bitwig Studio** — create a note track and set its output chooser to the PumpedUpKick device sitting on your audio track.

**Cubase** — create a MIDI track and pick the PumpedUpKick instance in its output routing dropdown (plugins that accept MIDI are listed there).

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

[.github/workflows/ci.yml](.github/workflows/ci.yml) runs on **every** push/PR to `main` and builds all three platforms. It doesn't just check that the code compiles — it loads the plugin and exercises it:

- `lipo` asserts both `arm64` and `x86_64` slices are present, so a release can never silently drop Intel support
- `auval` (macOS) instantiates the AU, renders audio and tests MIDI handling
- [pluginval](https://github.com/Tracktion/pluginval) at strictness level 10 runs parameter fuzzing, bus-layout and thread-safety checks against the AU and VST3 on macOS, the VST3 on Windows, and the VST3 on Linux (under `xvfb`)

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
