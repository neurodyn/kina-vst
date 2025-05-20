# KINA VST

KINA VST is a powerful audio effect plugin that combines modulation, filtering, distortion, and time-based effects in a creative signal chain.

## Features

- **VCA with LFO Modulation**
  - LFO shapes: Sine, Triangle, Saw, Square, Random
  - LFO rate up to 10,000Hz for ring-mod effects
  - DAW tempo sync with divisions up to 1/64
  - Amount control

- **VCF (Voltage Controlled Filter)**
  - Filter types: Low Pass, Band Pass, High Pass
  - Cutoff and resonance controls
  - LFO modulation up to 1,000Hz
  - DAW tempo sync with divisions up to 1/64

- **Dual Trasher Distortion**
  - Two independent distortion modules
  - Modes: Fuzz and Scream
  - Amount and tone controls for each

- **Echo**
  - Time, feedback, and amount controls
  - DAW tempo sync with divisions up to 1/64

- **Reverb**
  - Room size control
  - Damping control
  - Width control
  - Amount control

- **Global Features**
  - Dry/Wet mix control
  - Oversampling options: Off, 2x, 4x, 8x
  - Randomize button for creative sound design

## Signal Chain

The audio signal flows through the modules in this order:
1. VCA (modulated by LFO)
2. VCF (with optional LFO modulation)
3. Trasher 1
4. Trasher 2
5. Echo
6. Reverb

## Building

This project uses CMake for building. Make sure you have CMake 3.22 or higher installed.

1. Clone the repository with submodules:
```bash
git clone --recursive [repository-url]
```

2. Create a build directory and navigate to it:
```bash
mkdir build
cd build
```

3. Configure and build the project:
```bash
cmake ..
cmake --build .
```

## System Requirements

- C++17 compatible compiler
- CMake 3.22 or higher
- JUCE 8.0 or higher

## Supported Platforms

- macOS (VST3, AU)
- Windows (VST3)
- Linux (VST3)

## License

This project is licensed under the terms of the GNU General Public License v3.0. 