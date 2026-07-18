# PumpedUpKick

[简体中文](README_zh-CN.md)

A free, lightweight sidechain-ducking plugin — KickStarter's instant workflow with LFOTool-style envelope control.

**AU / VST3 / Standalone** · macOS & Windows · built with [JUCE 8](https://juce.com) (AGPLv3)

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

## Building

Requirements: CMake ≥ 3.24, a C++17 compiler (Xcode command-line tools on macOS, MSVC on Windows). JUCE is fetched automatically.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j8
```

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

## License

Copyright (C) 2026 CanisAlpha

PumpedUpKick is free software: you can redistribute it and/or modify it under the terms of the [GNU Affero General Public License](LICENSE) as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. See [LICENSE](LICENSE) for the full text.

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
