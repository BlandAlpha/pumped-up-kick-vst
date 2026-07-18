The first release of **PumpedUpKick** — a free, open-source sidechain-ducking plugin that pairs KickStarter's instant workflow with LFOTool-style envelope editing.

## Highlights

- **Insert → preset → done.** BPM-synced volume ducking that locks to your host's timeline (1/1 – 1/16), with a global Mix knob.
- **Draw your own pump.** Full envelope editor: drag points, double-click to add, right-click to delete, bend segments with their midpoint handles, Shift to snap. Save your shapes as presets.
- **MIDI one-shot trigger** for anything that isn't four-on-the-floor — each Note-On fires the envelope exactly once, sample-accurately. Per-host routing guides are in the [README](https://github.com/BlandAlpha/pumped-up-kick-vst#using-the-midi-one-shot-trigger).
- **See what you're cutting.** Live waveform display shows pre-duck input (grey) against post-duck output (orange) behind the curve.
- **Practically free CPU.** The audio path is one table lookup and one multiply per sample — no locks, no allocations.
- 5 factory shapes (Classic / Soft / Tight / Long Pump / Wobble), click-free smoothing, resizable UI set in Special Gothic.

## Downloads

| File | Platform | Formats |
|---|---|---|
| `PumpedUpKick-macOS-v1.0.0.zip` | macOS 10.13+ (Intel & Apple Silicon, universal) | AU · VST3 · Standalone |
| `PumpedUpKick-Windows-v1.0.0.zip` | Windows 10+ | VST3 · Standalone |
| `PumpedUpKick-Linux-v1.0.0.zip` | Linux (x86_64) | VST3 · Standalone |

Every binary in this release passed [pluginval](https://github.com/Tracktion/pluginval) at strictness level 10, and the AU passes `auval`.

## macOS users: one required step

These builds are not notarized (no paid Apple Developer account — this is free software). After copying the plugins into place, clear the quarantine flag or your DAW will refuse to load them:

```sh
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/Components/PumpedUpKick.component
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/PumpedUpKick.vst3
```

The "right-click → Open" trick does **not** work for plugins. Full instructions ship inside the zip (`INSTALL.txt`). Prefer not to run commands? [Build from source](https://github.com/BlandAlpha/pumped-up-kick-vst#building) — two commands, and locally built binaries need no workaround.

---

首个正式版本。**PumpedUpKick** 是一款免费开源的侧链压缩插件,融合 KickStarter 的即插即用与 LFOTool 式的包络编辑。

- **BPM 同步压缩**:锁定宿主时间轴(1/1–1/16),全局 Mix 旋钮
- **自由绘制包络**:拖点 / 双击加点 / 右键删点 / 中点手柄弯曲曲线 / Shift 吸附,可保存自定义预设
- **MIDI one-shot 触发**:非四四拍场景下,每个音符采样级精确触发一次包络,各宿主路由方法见 [中文 README](https://github.com/BlandAlpha/pumped-up-kick-vst/blob/main/README_zh-CN.md)
- **实时波形对比**:压缩前(灰)/压缩后(橙)直接画在曲线背后
- **接近零的 CPU 占用**:音频路径每采样仅一次查表一次乘法

**macOS 用户必读**:本构建未经 Apple 公证,安装后需在终端执行上方的 `xattr` 命令解除隔离,否则宿主会拒绝加载("右键打开"对插件无效,详见 zip 内 `INSTALL.txt`)。
