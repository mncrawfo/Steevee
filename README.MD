# Steevee

This repository contains a starter JUCE audio plug-in project. The Projucer
project file is `TestPlugin.jucer`; its C++ source files are in the repository
root. Projucer is JUCE's project manager: it creates native IDE/build files
from the shared project settings.

## What you need

- The same JUCE release as the rest of your team, which includes Projucer and
  the modules used by this project
- A compiler for your operating system: Xcode on macOS, Visual Studio 2022 on
  Windows, or a C++ build toolchain on Linux
- A plug-in host to try the built plug-in (optional)

The JUCE framework itself is not included in this repository. Each teammate
needs their own JUCE installation; do not copy a computer-specific JUCE path
into the shared project.

## Open the project

1. In Projucer, set the JUCE modules path to the `modules` folder inside your
   JUCE installation. On Windows and Linux this is under **File > Global Search
   Paths**; on macOS, use **Projucer > Global Search Paths**.
2. Open `TestPlugin.jucer` in Projucer. The four source files should appear in
   the project and the JUCE modules should resolve without missing-module
   warnings.
3. Save the project in Projucer.

If Projucer reports a missing module, check that the modules path points to the
`modules` folder from the JUCE release your team agreed to use. This project
needs every module listed in its Modules section, including
`juce_audio_processors_headless`.

The project has exporters for Xcode (macOS), Visual Studio 2022 (Windows), and
Linux Makefiles. In Projucer, choose the exporter for your operating system and
use its generated project/build files. If you change project settings, save the
`.jucer` file and share that file with your teammates; each person should set
their own JUCE modules path in Projucer.

Projucer-generated files, including `Builds/` and `JuceLibraryCode/`, are
intentionally ignored by Git. The project keeps the `.jucer` file and C++ source
files in the repository instead.
