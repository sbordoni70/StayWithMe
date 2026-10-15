# StayWithMe

StayWithMe is a small, lightweight Windows desktop application that keeps the system
awake by continuously resetting the Windows idle system timer. It runs quietly in
the background as a tray (notification area) icon, so you no longer have to keep
moving the mouse or pressing a key just to stop the screen from blanking or the
session from locking.

## What it does

When enabled, it evaluates the idle time to wake the input system by calling the
Windows API `SetThreadExecutionState` on a bit shorter timer interval.

This prevents:
- the display power-save / screen blank timeout, and
- the automatic workstation lock and session idle timeout.

When disabled, the application stops sending those signals and the system returns to
its normal power and idle behavior.

## How to use it

- **Left click** on the tray icon — quickly toggle the "keep awake" feature on/off.
- **Right click** on the tray icon — open the context menu:
  - **Enable** — toggle the idle-timer suspension on and off (the item shows a check
    mark while active).
  - **Exit** — quit the application.

The tray icon changes appearance to reflect the current state:
- an "off" icon when the feature is disabled, and
- the program icon when the feature is enabled and active.

## Building

The project is a native C++ (C++20) Windows application built with the Microsoft
Visual C++ (MSVC) toolchain. [Visual Studio 2026](https://visualstudio.microsoft.com/)
is required to compile it.

- Open `StayWithMe.slnx` in Visual Studio and build the `StayWithMe` target,
  or
- build directly with the MSVC compiler (`cl`) using the settings reflected in
  `compile_commands.json` (Unicode subsystem, `/std:c++20`, `_WINDOWS`).

The result is a single portable executable that only requires the Windows
API libraries present on any modern Windows installation.

## Requirements

- Windows 10 / 11 (any recent Windows release supporting the Windows Shell
  notification area and `SetThreadExecutionState`).
- [Visual Studio 2026](https://visualstudio.microsoft.com/) with the C++ build tools
  (MSVC toolchain) to compile the project.
- No third-party runtime dependencies.
- No installation and no administrative privileges are required to **use** the
  compiled executable — it is a single, portable program that can be run directly
  from any folder or USB drive by the current user.

## Project layout

| File | Description |
| ---- | ----------- |
| `main.cpp` | All application logic: window class, tray icon handling, timers, and the idle-timer suspension. |
| `resource.h` | Resource identifiers (icons, menu IDs). |
| `StayWithMe.rc` | Resource script (icons and resources). |
| `StayWithMe.vcxproj` | Visual Studio project file. |
| `StayWithMe.slnx` | Solution file. |
| `program_icon.ico` | Tray / window icon used when the feature is enabled. |
| `disabled.ico` | Icon used when the feature is disabled. |

## Notes

- **No keyboard or mouse input is tracked.** The application does not hook, monitor,
  record, or simulate any keyboard or mouse activity. It simply tells the system, via
  `SetThreadExecutionState`, that the display and session must stay awake — so your
  typing and clicking remain completely private and untouched.
- StayWithMe requires the process to keep running; closing it via **Exit** restores
  normal system idle behavior.
- The project intentionally keeps no persistent state — a fresh start begins with the
  feature disabled.
