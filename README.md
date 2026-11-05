# Dreamy Music VHS (`DreamyMusic.plugin`)

An audio-reactive Adobe After Effects C++ plugin (`Stylize > Dreamy Music VHS`) designed to be edited in **VS Code** and compiled in **Xcode**.

It fuses **Dreamy VHS & Halation Bloom**, **Frequency Modulation (FM) & Rutt-Etra 3D Scanline Synthesis**, **Circuit Bending (Analog Wavefolding & DAC Pin Shorts)**, an **Editable 4-Stop / Preset Gradient Colorizer**, and **CRT Scanlines + Trinitron RGB Phosphor Subpixel Masks**, all reacting dynamically to music.

---

## Key Features & Effect Controls

### 1. Music & Audio Reactivity
* **Audio Source Layer**: Pick any audio/video layer in your composition directly inside the plugin. The built-in 3-band IIR filter bank analyzes the audio waveform in real time into **Bass (< 160 Hz)**, **Mid (160 Hz – 2.5 kHz)**, and **High (> 2.5 kHz)** RMS envelopes.
* **Audio Keyframe Slider**: Keyframable slider (`0 – 200`) that can be pick-whipped directly to After Effects' **Animation > Keyframe Assistant > Convert Audio to Keyframes** (`Audio Amplitude` -> `Both Channels`) or driven via expressions. Both audio sources work simultaneously and fuse smoothly.
* **Master Audio Reactivity & Band Weights**: Independent percentage controls for **Bass Weight** (drives FM depth, Rutt-Etra elevation, and VHS tape wobble), **Mid Weight** (drives dreamy halation bloom, chroma bleed, and gradient cycling), and **High Weight** (drives circuit bending glitches and wavefolding).
* **Audio Window Smooth (ms)**: Temporal analysis window (`15 – 350 ms`) to dial in snappy transient response or dreamy, floating audio envelopes.

### 2. Dreamy Gradient Palette (Editable 4-Stop + Curated Presets)
* **Gradient Palette Mode**:
  1. **Custom 4-Stop Gradient** — Uses the 4 interactive color pickers below.
  2. **Sunset Dream (Ref 1 FM)** — Deep night violet $\rightarrow$ electric magenta $\rightarrow$ fiery orange-gold $\rightarrow$ pastel cyan-white.
  3. **Acid Circuit Synth (Ref 2)** — Deep cobalt blue $\rightarrow$ acid lime green $\rightarrow$ solar yellow $\rightarrow$ hot neon magenta.
  4. **Neon Oscilloscope** — Warm phosphor amber $\rightarrow$ neon coral $\rightarrow$ electric indigo $\rightarrow$ laser cyan.
  5. **Cyberpunk VHS** — Midnight blue $\rightarrow$ hot pink $\rightarrow$ electric cyan $\rightarrow$ warm cream.
  6. **Thermal Phosphor CRT** — Void black $\rightarrow$ deep crimson $\rightarrow$ amber gold $\rightarrow$ incandescent white.
  7. **Pastel Vaporwave** — Twilight plum $\rightarrow$ dreamy rose $\rightarrow$ mint aqua $\rightarrow$ soft lavender white.
* **Shadow / Low Midtone / High Midtone / Highlight Color Pickers**: 4 editable color stops for custom palettes.
* **Gradient Color Mix & Phase Cycling**: Blend between the original footage color and the synthesized gradient, with manual and audio-reactive color phase shifting.

### 3. FM & Rutt-Etra Oscilloscope Synthesis
* **FM Carrier Frequency & Modulation Depth**: True phase/frequency modulation where pixel luminance modulates a high-frequency horizontal carrier wave.
* **Rutt-Etra 3D Elevation & Oscilloscope Line Count**: Deflects horizontal scanlines vertically based on image brightness (Z-extrusion) and renders glowing oscilloscope contour ribbons with adjustable **Beam Ribbon Thickness** and **FM Audio Reactivity**.

### 4. Circuit Bending (Analog Video Synth Glitch)
* **Analog Phase Wavefolder**: Tachyons+ / analog video synth sinusoidal RGB phase wrapping and solarization contours.
* **DAC Pin Short (XOR Crush)**: Emulates shorted digital data pins on a video ADC/DAC chip for authentic hardware bitwise glitch patterns.
* **Horizontal Tear & Smear**: Analog horizontal sync line-hold tearing and color smearing triggered by treble/bass hits.
* **Chroma Matrix Bend**: Cross-channel RGB feedback rotation.

### 5. Dreamy VHS & Halation
* **Dreamy Halation Bloom**: Multi-tap soft highlight glow and warm/magenta halation aura.
* **VHS Chroma Bleed**: Analog YIQ/YUV chroma delay that smears Red/Blue color horizontally while preserving sharp luminance detail.
* **Tape Timebase Wobble & Tracking Static Bar**: 60Hz mains hum wave, high-frequency tape flutter, scrolling VHS tracking noise bar, and magnetic tape grain.

### 6. CRT Scanlines & Trinitron Phosphor Mask
* **Scanline Pitch (px) & Intensity**: Raised-cosine CRT scanline beam profile with highlight bloom compensation.
* **RGB Phosphor Mask Type**:
  * **None (Scanlines Only)**
  * **Trinitron Aperture Grille (Ref 3)** — Discrete vertical R-G-B subpixel phosphor stripes.
  * **Shadow Mask Delta Triad** — Staggered RGB phosphor triads.
  * **Arcade Slot Mask** — Staggered vertical slot mask with horizontal bridges.
* **CRT Glass Curvature**: Optional barrel distortion and CRT bezel corner vignette.

---

## Editing in VS Code & Building in Xcode

### Project Layout
* [`src/DreamyMusic.h`](src/DreamyMusic.h) — Parameter IDs, data structures, and plugin metadata.
* [`src/DreamyDSP.h`](src/DreamyDSP.h) — 3-band audio analyzer, FM/Rutt-Etra synthesizer, circuit bending, VHS bloom, gradient mapper, and CRT phosphor mask DSP engine.
* [`src/DreamyMusic.cpp`](src/DreamyMusic.cpp) — After Effects `PluginDataEntryFunction2` & `EffectMain` entry points, parameter registration, diagnostic file logger, and multi-threaded 8-bit/16-bit renderer.
* [`src/DreamyMusicPiPL.r`](src/DreamyMusicPiPL.r) — Adobe PiPL resource definition (`ma64` / `mi64`) compiled via Apple's `Rez` tool.
* [`include/AE_SDK/`](include/AE_SDK/) — Self-contained 64-bit After Effects SDK header subset so the project builds out-of-the-box without external dependencies (also pre-configured to search `../AfterEffectsSDK/Examples/Headers` if present).
* [`dreamymusic.log`](dreamymusic.log) — Live diagnostic log written inside the project root (and mirrored to `/tmp/dreamymusic.log`).
* [`DreamyMusic.xcodeproj`](DreamyMusic.xcodeproj) — Xcode project configured to build a Universal (`arm64` + `x86_64`) `.plugin` bundle.
* [`.vscode/`](.vscode/) — VS Code IntelliSense (`c_cpp_properties.json`), build tasks (`tasks.json`), and LLDB debugger attachment (`launch.json`).

### Build from VS Code
Press **`Cmd + Shift + B`** in VS Code to run the default build task (`Build DreamyMusic.plugin (Release)`), or open the Command Palette (`Cmd + Shift + P` -> `Tasks: Run Task`) to choose:
* **Build DreamyMusic.plugin (Release)**
* **Build DreamyMusic.plugin (Debug)**
* **Install Plugin to Adobe After Effects MediaCore**
* **Open Project in Xcode**

### Build from Terminal or Xcode
```bash
xcodebuild -project DreamyMusic.xcodeproj -scheme DreamyMusic -configuration Release -derivedDataPath build/DerivedData build
```
The compiled Universal binary bundle is output to:
```
build/Release/DreamyMusic.plugin
```

### Install into Adobe After Effects
Copy `build/Release/DreamyMusic.plugin` into Adobe's shared MediaCore plug-ins folder (and set ownership so subsequent rebuilds can overwrite it cleanly):
```bash
sudo rm -rf "/Library/Application Support/Adobe/Common/Plug-ins/7.0/MediaCore/DreamyMusic.plugin" && \
sudo cp -R "build/Release/DreamyMusic.plugin" "/Library/Application Support/Adobe/Common/Plug-ins/7.0/MediaCore/" && \
sudo chown -R $(whoami):admin "/Library/Application Support/Adobe/Common/Plug-ins/7.0/MediaCore/DreamyMusic.plugin"
```
Restart After Effects (hold **`Shift + Option`** on launch if After Effects previously cached an older plugin scan), apply **Effect > Stylize > Dreamy Music VHS** to any footage or solid/adjustment layer, and select your audio track in **Audio Source Layer** (or link **Audio Keyframe Slider** to audio keyframes).

---

## Diagnostic Logging & Troubleshooting

The plugin includes a built-in diagnostic logger (`DreamyLog` in [`src/DreamyMusic.cpp`](src/DreamyMusic.cpp)) that automatically records:
* Mach-O binary load events (`OnDreamyPluginLoaded`)
* `PluginDataEntryFunction2` host handshake and version verification
* `PF_Cmd_GLOBAL_SETUP` flags and struct ABI verification (`sizeof(PF_InData)`, `sizeof(PF_OutData)`, `sizeof(PF_ParamDef)`)
* `PF_Cmd_PARAMS_SETUP` parameter registration and any `PF_ADD_PARAM` error codes
* Any runtime or `PF_Cmd_RENDER` errors

### Where to View the Logs
Logs are written simultaneously to two locations so they are always accessible:
1. **In your VS Code Workspace**: Open [`dreamymusic.log`](dreamymusic.log) directly in the VS Code editor.
2. **In Terminal (`/tmp/dreamymusic.log`)**: Stream live events in real time with:
   ```bash
   tail -f /tmp/dreamymusic.log
   ```
   Or stream the workspace log:
   ```bash
   tail -f "/Users/niall/Developer/AE Plugins/dreamymusic/dreamymusic.log"
   ```

