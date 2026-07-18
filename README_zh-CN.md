# PumpedUpKick

一款免费、轻量的侧链压缩插件 —— 融合 KickStarter 的即插即用工作流与 LFOTool 式的包络控制。

**AU / VST3 / Standalone** · macOS & Windows · 基于 [JUCE 8](https://juce.com) 构建 (AGPLv3)

## 功能特性

- **一旋钮工作流** —— 选一个预设、设置速率、拧 Mix 旋钮,搞定。
- **BPM 同步压缩** —— 锁定宿主时间轴 (1/1, 1/2, 1/4, 1/8, 1/16)。宿主走带暂停时压缩暂停(播放头也会隐藏);只有在完全没有走带信息的场景(如 Standalone 独立运行)才会自由运行。
- **波形显示** —— 在包络曲线背后画出最近一个周期的压缩前(灰色)与压缩后(橙色)波形,清楚看到具体被"吃掉"了多少。
- **完整的包络编辑**(LFOTool 风格):
  - 拖动控制点移动(按住 **Shift** 吸附网格)
  - **双击**空白处新增控制点
  - **双击 / 右键 / ⌥ 点击**控制点将其删除
  - 拖动线段的**中点手柄**上下移动来弯曲曲线
- **MIDI one-shot 触发** —— 将 Trigger 切换为 **MIDI**,包络只在收到音符时触发,非常适合非四四拍的节奏型;音符之间音频原样通过不做处理。
- **预设** —— 5 个出厂预设形状(Classic、Soft、Tight、Long Pump、Wobble),并支持保存自定义预设。
- **平滑控制** —— 无爆音的增益平滑处理 (0.1–20 ms)。
- **极小的资源占用** —— 包络被渲染进一张无锁查找表;音频路径每采样只做一次查表和一次乘法,没有内存分配、没有锁、没有 FFT。

## 编译构建

依赖要求:CMake ≥ 3.24,支持 C++17 的编译器(macOS 上的 Xcode 命令行工具,Windows 上的 MSVC)。JUCE 会被自动拉取。

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j8
```

在 macOS 上,AU 和 VST3 会被自动拷贝到:

- `~/Library/Audio/Plug-Ins/Components/PumpedUpKick.component`(Logic Pro)
- `~/Library/Audio/Plug-Ins/VST3/PumpedUpKick.vst3`(FL Studio、Ableton 等)

如果 Logic 没有立即识别到 AU,运行 `auval -a` 或重启 Logic(启动时会重新扫描)。

在 Windows 上,将 `build/PumpedUpKick_artefacts/Release/VST3/PumpedUpKick.vst3` 拷贝到 `C:\Program Files\Common Files\VST3\`。

### 在 Logic 中使用(MIDI 触发模式)

将 PumpedUpKick 作为 **MIDI 控制效果器**插入(乐器插槽 → *AU MIDI-controlled Effects* → PumpedUp Audio → 将音频轨道设为侧链…;或者直接作为普通音频效果器插入并使用 Sync 模式)。在 FL Studio 中,通过匹配 *MIDI output* 端口将任意 MIDI 通道路由到插件,或者直接使用 Sync 模式即可。

## 分发说明

- **macOS**:若要分发给其他机器使用,需用 Developer ID Application 证书对两个 bundle 分别签名并公证:
  ```sh
  codesign --force --deep --sign "Developer ID Application: …" PumpedUpKick.component
  xcrun notarytool submit … && xcrun stapler staple …
  ```
  未签名的本地构建在自己机器上可以正常使用(Gatekeeper 只检查从网络下载的二进制文件)。
- **Windows**:使用 MSVC 构建(`cmake -G "Visual Studio 17 2022"`);代码签名可选。
- **许可证**:JUCE 以 AGPLv3 协议使用,因此本项目同样采用 AGPLv3 —— 源代码必须保持公开可用。VST3 支持基于 Steinberg 的 VST3 SDK(与 GPLv3 兼容)。"VST" 是 Steinberg Media Technologies GmbH 的注册商标。

## 许可证

Copyright (C) 2026 CanisAlpha

PumpedUpKick 是自由软件:你可以在自由软件基金会发布的 [GNU Affero 通用公共许可证](LICENSE)(第 3 版或你选择的更新版本)条款下重新分发和/或修改它。完整条款见 [LICENSE](LICENSE) 文件。

## 用户预设

以 XML 格式保存在:

- macOS:`~/Library/Application Support/PumpedUpKick/Presets/*.pukpreset`
- Windows:`%APPDATA%\PumpedUpKick\Presets\*.pukpreset`

预设文件可移植 —— 直接复制文件即可分享。

## 架构说明

| 模块 | 文件 | 说明 |
|---|---|---|
| 曲线模型 | `src/Curve.h` | 控制点 + 每段张力(tension)、幂函数曲线插值、无锁双缓冲查找表 |
| DSP / 相位引擎 | `src/PluginProcessor.cpp` | Sync 模式下锁定宿主 ppq 的相位计算、MIDI 模式下采样级精度的音符触发、一阶增益平滑 |
| 包络编辑器 | `src/EnvelopeEditor.h` | 矢量绘制,60 fps 播放头动画,带变化检测以跳过多余的重绘 |
| 预设 | `src/Presets.h` | 出厂预设形状 |
| UI | `src/PluginEditor.cpp`, `src/LookAndFeel.h` | 扁平深色主题,可缩放(固定宽高比) |
