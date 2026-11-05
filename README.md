# Dreamy Music VHS (`DreamyMusic.plugin`)

An audio-reactive Adobe After Effects C++ plugin (**Effect > Stylize > Dreamy Music VHS**, match name `ADBE DreamyMusicVHS`) built for **macOS (`arm64` + `x86_64` Universal Binary)**.

**Dreamy Music VHS** fuses six analog video synthesis and tape emulation modules into a single real-time pipeline that reacts dynamically to music:
1. **3-Band Music & Audio Reactivity** (Direct Audio Layer DSP + Keyframe Slider fusion)
2. **Dreamy Color Gradient Synthesizer** (Editable 4-Stop Color Pickers + 6 Curated Presets)
3. **FM & Rutt-Etra 3D Scanline Oscilloscope** (Luminance-driven Z-elevation & phase modulation)
4. **Circuit Bending Glitch Engine** (Analog wavefolder, bitwise XOR DAC pin-shorting, H-sync tearing, and RGB matrix feedback)
5. **Dreamy VHS & Halation Bloom** (Multi-tap highlight aura, YIQ/YUV chroma delay bleed, 60 Hz tape edge wobble, scrolling static bar, and magnetic grain)
6. **CRT Scanlines & RGB Phosphor Subpixel Masks** (Sony Trinitron Aperture Grille, Shadow Mask Delta Triad, Arcade Slot Mask, and CRT glass curvature)

---

## Table of Contents

1. [Quick Start: Applying the Effect & Connecting Music](#1-quick-start-applying-the-effect--connecting-music)
2. [Complete Feature & Control Reference](#2-complete-feature--control-reference)
   - [Group 1: Music & Audio Reactivity](#group-1-music--audio-reactivity-starts-open)
   - [Group 2: Dreamy Color Gradient](#group-2-dreamy-color-gradient-starts-open)
   - [Group 3: FM & Rutt-Etra Waves](#group-3-fm--rutt-etra-waves-starts-open)
   - [Group 4: Circuit Bending Glitch](#group-4-circuit-bending-glitch-starts-collapsed)
   - [Group 5: Dreamy VHS & Halation](#group-5-dreamy-vhs--halation-starts-collapsed)
   - [Group 6: CRT Scanlines & Phosphor](#group-6-crt-scanlines--phosphor-starts-collapsed)
3. [Creative Recipes & Look Guides](#3-creative-recipes--look-guides)
4. [Building, Editing & Installing (VS Code & Xcode)](#4-building-editing--installing-vs-code--xcode)
5. [Two-Tier Live Diagnostic Logging System](#5-two-tier-live-diagnostic-logging-system)
6. [Project Architecture & Technical Notes](#6-project-architecture--technical-notes)
7. [Troubleshooting & FAQ](#7-troubleshooting--faq)

---

## 1. Quick Start: Applying the Effect & Connecting Music

1. **Apply the Effect**:
   - Select any video layer, solid, or adjustment layer in your After Effects composition and choose **Effect > Stylize > Dreamy Music VHS**.
   - Immediately upon application, the default **Sunset Dream** gradient, soft **FM Rutt-Etra waves**, **Dreamy VHS halation**, and **Trinitron CRT scanlines** are active.
2. **Connect Your Audio (Two Ways — Use Either or Both!)**:
   - **Method A — Direct Audio Layer Selection (Recommended, Zero Setup)**:
     - In the **Music & Audio Reactivity** group, set **Audio Source Layer** to your music `.mp3`/`.wav` layer (or leave it on the current video layer if the video itself contains audio).
     - The plugin checks out the raw 32-bit floating-point audio waveform directly from After Effects and runs a real-time 3-band IIR crossover filter splitting the track into **Bass (< 160 Hz)**, **Mid (160 Hz – 2.5 kHz)**, and **Treble (> 2.5 kHz)** envelopes.
   - **Method B — Keyframe / Expression Slider**:
     - Right-click your audio track and select **Keyframe Assistant > Convert Audio to Keyframes**.
     - Option-click (Alt-click) the stopwatch on **Audio Keyframe Slider** in **Dreamy Music VHS** and pick-whip it to the `Both Channels` slider on the generated `Audio Amplitude` layer.
     - Both methods work simultaneously and fuse smoothly.

---

## 2. Complete Feature & Control Reference

The Effect Controls panel is organized into **6 collapsible topic groups**. The 3 core look groups start expanded for immediate tweaking, while the 3 secondary hardware/tape groups start collapsed to keep the UI tidy.

### Group 1: Music & Audio Reactivity *(Starts Open)*

| Control | Range | Default | How It Works & How to Use It |
| :--- | :--- | :--- | :--- |
| **Audio Source Layer** | Layer Picker | `Self` | Selects the composition layer whose audio waveform drives the 3-band DSP analyzer. Select `None` if you want a purely static/manual look. |
| **Audio Keyframe Slider** | `0 – 200` (Slider `0 – 50`) | `0.0` | Manual or expression-driven audio amplitude input (`0–50` typical keyframe range). Adds directly to the live waveform analyzer. |
| **Master Reactivity** | `0% – 500%` | `125.0%` | Global multiplier for all audio-reactive motion across all modules. Set to `0%` to freeze all audio modulation. |
| **Bass Response (FM & VHS)** | `0% – 300%` | `130.0%` | Controls how strongly sub-bass and kick drums (`< 160 Hz`) drive **FM Wave Depth**, **3D Scanline Elevation**, and **VHS Tape Edge Wobble**. |
| **Mid Response (Glow & Color)** | `0% – 300%` | `100.0%` | Controls how strongly vocals, synths, and snares (`160 Hz – 2.5 kHz`) drive **Dreamy Halation Glow**, **VHS Color Bleed**, and **Audio Color Cycling**. |
| **Treble Response (Glitch)** | `0% – 300%` | `120.0%` | Controls how strongly hi-hats and transients (`> 2.5 kHz`) trigger **Circuit Bending** wavefolding, XOR pin-shorts, and horizontal sync tears. |
| **Audio Smoothing (ms)** | `15 – 350 ms` | `75 ms` | Duration of the sliding RMS audio analysis window. Lower values (`15–35 ms`) create snappy, jittery beat reactions; higher values (`150–350 ms`) create slow, floating, dreamy swells. |

---

### Group 2: Dreamy Color Gradient *(Starts Open)*

Maps image luminance (plus FM wave ridges and circuit-bent phase contours) through a smooth hermite-interpolated 4-stop color palette while preserving natural highlight roll-off. You can turn off the gradient colors at any time to keep 100% of your original footage colors while keeping FM/Rutt-Etra, VHS, Circuit Bending, and CRT scanlines active.

| Control | Range | Default | How It Works & How to Use It |
| :--- | :--- | :--- | :--- |
| **Enable Color Gradient** | Checkbox | `On` | Master toggle for the color gradient synthesizer. Uncheck this to turn off the gradient colors and keep 100% of your original video colors. |
| **Palette Preset** | Popup (8 modes) | `Sunset Dream (Ref 1 FM)` | Choose **Custom 4-Stop Gradient** to use your own color pickers below, pick one of the 6 built-in curated palettes, or choose **Off (Original Footage Colors)** to bypass gradient colorization. |
| **Shadow Color (Stop 1)** | RGB Picker | `#140524` (Deep Violet) | Color mapped to dark shadows (`Luma 0.00 – 0.33`) when **Palette Preset** is set to `Custom 4-Stop Gradient`. |
| **Low Midtone (Stop 2)** | RGB Picker | `#E026A6` (Electric Magenta) | Color mapped to lower midtones (`Luma 0.33 – 0.66`) in Custom mode. |
| **High Midtone (Stop 3)** | RGB Picker | `#FF8F1A` (Fiery Orange-Gold) | Color mapped to upper midtones (`Luma 0.66 – 1.00`) in Custom mode. |
| **Highlight Color (Stop 4)** | RGB Picker | `#B8E6FF` (Pastel Cyan-White) | Color mapped to bright specular highlights and oscilloscope beam peaks in Custom mode. |
| **Gradient Blend** | `0% – 100%` | `85.0%` | Blends between the original video colors (`0%`) and the dreamy gradient synthesizer (`100%`). |
| **Color Phase Shift** | `-500% – 500%` | `0.0%` | Manually scrolls/rotates the 4 stops across the luminance spectrum. Keyframe this for continuous rainbow palette evolution. |
| **Audio Color Cycling** | `0% – 200%` | `35.0%` | Shifts the gradient phase dynamically in response to **Mid** and **Bass** energy so colors pulse with the music. |

#### Built-in Palette Presets:
1. **Custom 4-Stop Gradient**: Uses your 4 custom RGB pickers (`Stop 1` through `Stop 4`).
2. **Sunset Dream (Ref 1 FM)**: Deep night violet $\rightarrow$ electric magenta $\rightarrow$ fiery orange-gold $\rightarrow$ pastel cyan-white.
3. **Acid Circuit Synth (Ref 2)**: Deep cobalt blue $\rightarrow$ acid lime green $\rightarrow$ solar yellow $\rightarrow$ hot neon magenta.
4. **Neon Oscilloscope**: Warm phosphor amber $\rightarrow$ neon coral $\rightarrow$ electric indigo $\rightarrow$ laser cyan.
5. **Cyberpunk VHS**: Midnight blue $\rightarrow$ hot pink $\rightarrow$ electric cyan $\rightarrow$ warm cream.
6. **Thermal Phosphor CRT**: Void black $\rightarrow$ deep crimson $\rightarrow$ amber gold $\rightarrow$ incandescent white.
7. **Pastel Vaporwave**: Twilight plum $\rightarrow$ dreamy rose $\rightarrow$ mint aqua $\rightarrow$ soft lavender white.
8. **Off (Original Footage Colors)**: Turns off the gradient colorizer so all effects render using the layer's original colors.

---

### Group 3: FM & Rutt-Etra Waves *(Starts Open)*

Emulates classic analog Rutt/Etra video synthesizers and frequency-modulated CRT oscilloscopes.

| Control | Range | Default | How It Works & How to Use It |
| :--- | :--- | :--- | :--- |
| **Enable FM & Rutt-Etra** | Checkbox | `On` | Master bypass toggle for the FM & Rutt-Etra synthesizer stage. |
| **FM Wave Frequency** | `1 – 400` | `85.0` | Base horizontal carrier frequency of the FM sine oscillator. Higher values create finer, tightly packed ripple textures. |
| **FM Wave Depth** | `0% – 100%` | `25.0%` | Controls how strongly image brightness warps horizontal pixel coordinates and superimposes FM harmonic ripples. |
| **3D Scanline Elevation** | `0% – 200%` | `35.0%` | Rutt-Etra Z-axis vertical deflection: bright areas of the video physically lift scanlines upward into 3D topographic ridges. |
| **Oscilloscope Lines** | `0 – 400` | `90` | Number of discrete horizontal oscilloscope contour ribbons drawn across the frame. Set to `0` for smooth continuous FM displacement without discrete ribbon gaps. |
| **Oscilloscope Beam Width** | `5% – 100%` | `62.0%` | Thickness of the glowing phosphor beam relative to the dark inter-line gap. Lower values (`15%–35%`) produce thin, sharp vector lines on a dark background. |
| **FM Music Reactivity** | `0% – 300%` | `90.0%` | Scales how strongly **Bass** and **Mid** audio energy boost FM Wave Depth and 3D Scanline Elevation on beats. |

---

### Group 4: Circuit Bending Glitch *(Starts Collapsed)*

Emulates hardware-bent analog/digital video processors (such as Tachyons+ / dirty video mixers / shorted ADC data pins).

| Control | Range | Default | How It Works & How to Use It |
| :--- | :--- | :--- | :--- |
| **Enable Circuit Bending** | Checkbox | `On` | Master bypass toggle for all circuit-bending processors. |
| **Psychedelic Wavefolder** | `0% – 100%` | `18.0%` | Analog sinusoidal wavefolder: pushes RGB channels past `1.0` and folds them back down (`0.5 - 0.5 * cos(...)`), creating liquid solarized color bands. |
| **Digital Pin-Short (XOR)** | `0% – 100%` | `0.0%` | Simulates shorting digital bits on an 8-bit video DAC chip using bitwise `XOR` (`^`) patterns for crunchy hardware quantization artifacts. |
| **Horizontal Sync Tear** | `0% – 100%` | `15.0%` | Analog H-sync line-hold instability that tears random horizontal scanline bands sideways and smears their chroma trail. |
| **RGB Matrix Shift** | `0% – 100%` | `12.0%` | Rotates and cross-feeds Red, Green, and Blue channels (`R->G->B`) in a time-varying phase loop. |
| **Bend Music Reactivity** | `0% – 300%` | `100.0%` | Scales how strongly **Treble** and **Bass** peaks trigger wavefolding, H-sync tearing, and XOR pin glitches. |

---

### Group 5: Dreamy VHS & Halation *(Starts Collapsed)*

Models magnetic VHS tape transport, analog YIQ color under-carrier smearing, and dreamy optical highlight bloom.

| Control | Range | Default | How It Works & How to Use It |
| :--- | :--- | :--- | :--- |
| **Enable Dreamy VHS** | Checkbox | `On` | Master bypass toggle for the VHS tape and dreamy bloom stage. |
| **Dreamy Halation Glow** | `0% – 150%` | `55.0%` | Wide multi-tap horizontal/vertical highlight diffusion that surrounds bright areas with a warm magenta-tinted dreamy aura. |
| **VHS Color Bleed** | `0% – 100%` | `42.0%` | Separates luma (`Y`) from chroma (`R-Y`, `B-Y`) and delays the color signal horizontally to the right while keeping luma edges sharp. |
| **Tape Edge Wobble** | `0% – 100%` | `24.0%` | Combines 60 Hz mains-hum wave distortion with high-frequency capstan flutter and audio-driven head-switching jitter. |
| **VHS Tracking Static Bar** | `0% – 100%` | `20.0%` | Renders a scrolling horizontal VHS tracking error band with white snow particles and horizontal sync pull. |
| **Magnetic Tape Grain** | `0% – 100%` | `18.0%` | Temporal analog luma/chroma tape hiss and grain. |

---

### Group 6: CRT Scanlines & Phosphor *(Starts Collapsed)*

Simulates a physical cathode-ray tube television monitor with discrete RGB subpixel phosphor geometry.

| Control | Range | Default | How It Works & How to Use It |
| :--- | :--- | :--- | :--- |
| **Enable CRT Display** | Checkbox | `On` | Master bypass toggle for CRT scanlines, phosphor masks, and screen curvature. |
| **Scanline Size (px)** | `2.0 – 24.0 px` | `4.0 px` | Vertical height of each CRT scanline cycle in pixels. Increase to `6–12 px` for bold, macro-zoom CRT lines. |
| **Scanline Contrast** | `0% – 100%` | `45.0%` | Darkness of the gap between scanlines, featuring automatic highlight beam-bloom compensation so bright whites widen the electron beam naturally. |
| **Phosphor Subpixel Mask** | Popup (4 modes) | `Trinitron Aperture Grille (Ref 3)` | Selects the physical RGB phosphor subpixel layout:<br>• **None (Scanlines Only)**<br>• **Trinitron Aperture Grille (Ref 3)**: Continuous vertical `R | G | B` stripes<br>• **Shadow Mask Delta Triad**: Brick-staggered RGB triads<br>• **Arcade Slot Mask**: Staggered slot mask with horizontal dark bridges |
| **Phosphor Mask Intensity** | `0% – 100%` | `40.0%` | Visibility of the individual Red, Green, and Blue phosphor subpixels. Push to `75%–100%` for extreme macro close-up CRT aesthetics. |
| **CRT Screen Curvature** | `0% – 100%` | `15.0%` | Applies spherical CRT glass barrel distortion and darkened tube-bezel corners. Set to `0%` for a flat-screen framing. |

---

## 3. Creative Recipes & Look Guides

* **Look 1 — Glowing Rutt-Etra Vector Flower (Reference Image 1)**:
  - **Palette Preset**: `Sunset Dream (Ref 1 FM)` (`Gradient Blend`: `95%`)
  - **FM & Rutt-Etra Waves**: `FM Wave Frequency`: `110`, `FM Wave Depth`: `35%`, `3D Scanline Elevation`: `75%`, `Oscilloscope Lines`: `85`, `Oscilloscope Beam Width`: `38%`
  - **Dreamy VHS & Halation**: `Dreamy Halation Glow`: `80%`
* **Look 2 — Psychedelic Circuit-Bent TV Broadcast (Reference Image 2)**:
  - **Palette Preset**: `Acid Circuit Synth (Ref 2)` (`Gradient Blend`: `90%`)
  - **Circuit Bending Glitch**: `Psychedelic Wavefolder`: `65%`, `Horizontal Sync Tear`: `45%`, `RGB Matrix Shift`: `35%`, `Digital Pin-Short (XOR)`: `15%`
  - **Dreamy VHS & Halation**: `VHS Color Bleed`: `75%`, `Tape Edge Wobble`: `40%`
* **Look 3 — Macro Sony Trinitron CRT Monitor (Reference Image 3)**:
  - **CRT Scanlines & Phosphor**: `Scanline Size (px)`: `6.0 px`, `Scanline Contrast`: `75%`, `Phosphor Subpixel Mask`: `Trinitron Aperture Grille (Ref 3)`, `Phosphor Mask Intensity`: `85%`, `CRT Screen Curvature`: `20%`
  - **Dreamy VHS & Halation**: `Dreamy Halation Glow`: `45%`, `VHS Color Bleed`: `35%`

---

## 4. Building, Editing & Installing (VS Code & Xcode)

### Prerequisites
* **macOS** (Apple Silicon `arm64` or Intel `x86_64`)
* **Xcode** / Xcode Command Line Tools (`xcodebuild` and Apple's `Rez` resource compiler)
* **Visual Studio Code** (optional, with the C/C++ extension)
* **Adobe After Effects** (2022 / 2023 / 2024 / 2025 / 2026)

### Option A: Build & Install from VS Code
Open the `/Users/niall/Developer/AE Plugins/dreamymusic` folder in VS Code:
* Press **`Cmd + Shift + B`** to run the default build task (`Build DreamyMusic.plugin (Release)`).
* Or open the Command Palette (**`Cmd + Shift + P` $\rightarrow$ `Tasks: Run Task`**) to run any of the configured tasks in [`.vscode/tasks.json`](.vscode/tasks.json):
  1. `Build DreamyMusic.plugin (Release)`
  2. `Build DreamyMusic.plugin (Debug)`
  3. `Install Plugin to Adobe After Effects MediaCore`
  4. `Open Project in Xcode`

### Option B: Build from Terminal
Run `xcodebuild` from the project root:
```bash
xcodebuild -project DreamyMusic.xcodeproj \
           -scheme DreamyMusic \
           -configuration Release \
           -derivedDataPath build/DerivedData \
           SYMROOT="$(pwd)/build" \
           build
```
This compiles the Universal (`arm64` + `x86_64`) Mach-O binary, runs `Rez` on [`src/DreamyMusicPiPL.r`](src/DreamyMusicPiPL.r) to generate the `DreamyMusic.rsrc` PiPL resource, and outputs the bundle to:
```text
build/Release/DreamyMusic.plugin
```

### Option C: Build in Xcode IDE
Open [`DreamyMusic.xcodeproj`](DreamyMusic.xcodeproj) in Xcode:
```bash
open DreamyMusic.xcodeproj
```
Select the **DreamyMusic** scheme and press **`Cmd + B`** to build.

### Installing into Adobe After Effects
Copy the built `.plugin` bundle into Adobe's shared **MediaCore** directory so After Effects loads it automatically:
```bash
cp -R "build/Release/DreamyMusic.plugin" "/Library/Application Support/Adobe/Common/Plug-ins/7.0/MediaCore/"
```
*(If `/Library/Application Support/Adobe/Common/Plug-ins/7.0/MediaCore/` requires administrator permissions on your machine, prefix with `sudo` and set ownership with `sudo chown -R $(whoami):admin "/Library/Application Support/Adobe/Common/Plug-ins/7.0/MediaCore/DreamyMusic.plugin"` so future builds can overwrite it without `sudo`.)*

---

## 5. Two-Tier Live Diagnostic Logging System

To make both casual monitoring and deep low-level debugging effortless, the plugin writes **two separate timestamped log files simultaneously** inside the dedicated [`logs/`](logs/) folder every time After Effects loads the plugin (implemented in [`include/AE_SDK/AE_Macros.h`](include/AE_SDK/AE_Macros.h) and [`src/DreamyMusic.cpp`](src/DreamyMusic.cpp)):

* **Timestamped Filename Format**:
  - Friendly Live Log: `logs/dreamymusic_YYYY-MM-DD_HH-MM-SS.log`
  - Detailed Technical Log: `logs/dreamymusic_detailed_YYYY-MM-DD_HH-MM-SS.log`
* **Automatic Latest Symlinks**:
  - `logs/latest.log` always points to the newest session's friendly log.
  - `logs/latest_detailed.log` always points to the newest session's detailed technical log.

### Tier 1: Friendly Live Monitor Log (`logs/dreamymusic_YYYY-MM-DD_HH-MM-SS.log`)
* **Locations**:
  - Workspace Folder: [`/Users/niall/Developer/AE Plugins/dreamymusic/logs/`](logs/) (`dreamymusic_YYYY-MM-DD_HH-MM-SS.log` & `latest.log`)
  - Temp Mirror: `/tmp/dreamymusic_logs/`
* **Purpose**: Clean, human-readable real-time status updates with icons—never floods your terminal during playback.
* **What It Logs**:
  - `✨ Dreamy Music VHS v1.0 Loaded`: Mach-O load, `PluginDataEntryFunction2` host registration, and active session log file paths.
  - `✅ [INIT]`: `PF_Cmd_GLOBAL_SETUP` version check (`0x80001 == 524289`), Deep Color (8/16-bit), and Multi-Frame Rendering flags.
  - `🎛️ [UI SETUP]`: Confirmation of all registered Effect Controls across the 6 collapsible topic groups.
  - `🎚️ [CONTROL CHANGED]` / `🎨 [PRESET CHANGED]` / `☑️ [TOGGLE CHANGED]`: Immediate feedback whenever you drag a slider, switch a palette/CRT preset, pick a color stop, or toggle a module in After Effects.
  - `🎬 [RENDER #N]`: Periodic render heartbeats (frames 1–5 and every 30th frame thereafter) summarizing resolution, bit depth, composition timestamp, live 3-band audio levels (`Audio[Bass, Mid, High]`), active gradient preset, and enabled visual modules.
* **Watch Live in Terminal**:
  ```bash
  tail -f "/Users/niall/Developer/AE Plugins/dreamymusic/logs/latest.log"
  ```

### Tier 2: In-Depth Technical Diagnostic Log (`logs/dreamymusic_detailed_YYYY-MM-DD_HH-MM-SS.log`)
* **Locations**:
  - Workspace Folder: [`/Users/niall/Developer/AE Plugins/dreamymusic/logs/`](logs/) (`dreamymusic_detailed_YYYY-MM-DD_HH-MM-SS.log` & `latest_detailed.log`)
  - Temp Mirror: `/tmp/dreamymusic_logs/`
* **Purpose**: Granular engineering telemetry for verifying ABI struct alignment, DSP audio checkout math, render performance, and pixel buffer changes.
* **What It Logs**:
  - `[PARAM_DEF]`: Logs every parameter index, display name, `[valid_min .. valid_max]`, `slider=[slider_min .. slider_max]`, and `default` during `PF_Cmd_PARAMS_SETUP`.
  - `[PERF & BUFFER]`: Frame synthesis duration in milliseconds (`RenderTime=X.XX ms`), multi-threading mode (`MultiThread(PF_Iterate8)` vs `OpenMP/HostFallback`), `in_buf` and `out_buf` memory addresses, `rowbytes`, and a **Center Pixel `RGB(in) -> RGB(out)`** sample proving that output pixels are being modified.
  - `[AUDIO DSP]`: `checkout_layer_audio` return code, audio layer index, sample rate (`48000 Hz`), checked-out sample count, raw waveform peak amplitude, raw 3-band IIR filter RMS (`Raw[B, M, H]`), user weight multipliers, and final modulation envelopes (`Out[B, M, H]`).
  - `[MODULE PARAMS]`: Full snapshot of all parameter values across Gradient, FM/Rutt-Etra, Circuit Bending, VHS, and CRT modules on logged frames.
  - `[UI_EVENT]`: Raw parameter index, type code, and internal floating-point/integer values on every `PF_Cmd_USER_CHANGED_PARAM`.
* **Watch Live in Terminal**:
  ```bash
  tail -f "/Users/niall/Developer/AE Plugins/dreamymusic/logs/latest_detailed.log"
  ```

---

## 6. Project Architecture & Technical Notes

```text
dreamymusic/
├── DreamyMusic.xcodeproj/       # Xcode project (Universal arm64 + x86_64 .plugin target)
├── .vscode/                     # VS Code tasks, IntelliSense config & LLDB launch profile
├── include/AE_SDK/              # Self-contained After Effects CC 2025/2026 C/C++ SDK headers
│   ├── A.h                      # Core Adobe types (A_long, PF_FpLong, PF_FpShort, etc.)
│   ├── AE_Effect.h              # Binary-verified PF_InData, PF_OutData, PF_ParamDef & slider structs
│   ├── AE_EffectCB.h            # Host callback suites (PF_InteractCallbacks, PF_Iterate8Suite1)
│   ├── AE_EffectCBSuites.h      # SweetPea PICA suite acquisition definitions
│   └── AE_Macros.h              # Parameter builder macros & Two-Tier Diagnostic Logger
├── src/
│   ├── DreamyMusic.h            # Parameter enum IDs, constants & DreamyRenderParams struct
│   ├── DreamyDSP.h              # Real-time 3-band IIR audio analyzer & 6-stage pixel synth engine
│   ├── DreamyMusic.cpp          # Entry points, GlobalSetup, ParamsSetup, Audio checkout & Render
│   ├── DreamyMusicPiPL.r        # PiPL resource definition compiled via Apple Rez
│   └── Info.plist               # macOS bundle metadata (CFBundlePackageType = eFKT)
└── logs/                        # Session-timestamped diagnostic logs
    ├── dreamymusic_YYYY-MM-DD_HH-MM-SS.log          # Tier 1 Friendly Live Monitor log
    ├── dreamymusic_detailed_YYYY-MM-DD_HH-MM-SS.log # Tier 2 In-Depth Technical log
    ├── latest.log                                   # Symlink to newest Tier 1 log
    └── latest_detailed.log                          # Symlink to newest Tier 2 log
```

### Key Engineering Highlights
* **Compile-Time ABI Verification (`static_assert`)**: [`src/DreamyMusic.cpp`](src/DreamyMusic.cpp) enforces exact byte offsets against Adobe After Effects 2025/2026 (`arm64` / `x86_64`), including `sizeof(PF_ParamDef) == 0xB0`, `PF_FloatSliderDef` range offsets (`valid_min` at `+0x68`, `valid_max` at `+0x6C`, `slider_min` at `+0x70`, `slider_max` at `+0x74`, `dephault` at `+0x78`), `PF_InData` audio callbacks (`+0x30`..`+0x40`), and `effect_ref` (`+0xB8`).
* **8-bit & 16-bit Deep Color Pipeline**: Native support for both 8-bit (`PF_Pixel8`, `0..255`) and 16-bit (`PF_Pixel16`, `0..32768`) After Effects project color depths with internal 32-bit floating-point synthesis.
* **Multi-Frame Rendering (MFR) Safe**: Advertises `PF_OutFlag2_SUPPORTS_THREADED_RENDERING` with stateless per-frame audio checkout and row-parallel rendering.

---

## 7. Troubleshooting & FAQ

* **I opened an older `.aep` project and the sliders show `0.0%` or `-100.0%`**:
  - After Effects serializes parameter values inside `.aep` project files when an effect is first added. While [`Render()`](src/DreamyMusic.cpp) includes automatic recovery for layers saved with legacy clamped values, simply click the blue **`Reset`** link next to **Dreamy Music VHS** in the Effect Controls panel to snap all UI slider readouts to their factory defaults.
* **The effect doesn't react to my music layer**:
  1. Ensure **Audio Source Layer** in **Music & Audio Reactivity** is set to the layer that actually contains audio (e.g. your `.mp3`/`.wav` track or video layer with audio), not a silent layer.
  2. Check [`logs/latest_detailed.log`](logs/latest_detailed.log) for the `[AUDIO DSP]` line—if `Peak=0.0000`, verify the selected layer's audio isn't muted or trimmed out at the current composition time.
* **After Effects doesn't show recent code changes after rebuilding**:
  - Make sure you copied `build/Release/DreamyMusic.plugin` to `/Library/Application Support/Adobe/Common/Plug-ins/7.0/MediaCore/DreamyMusic.plugin` and restarted After Effects. You can verify the exact session log at the top of `logs/latest.log` (`✨ Dreamy Music VHS v1.0 Loaded into Adobe After Effects`).
