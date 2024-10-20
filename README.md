# PSY-Z

PSY-Z SDK is a drop-in replacement for the PlayStation 1 Runtime Library called PSY-Q, allowing games designed for the PlayStation 1 to be compiled and run natively on modern platforms.

PSY-Z is not an emulator or a static recompiler. It is a porting library written in C that replaces as many API calls as possible with counterparts on other platforms. Currently, it targets Linux and macOS, supporting both 32-bit and 64-bit architectures out of the box. The same applications and games will work on all platforms, including the PlayStation 1, with minimal changes.

PSY-Z adheres to the PSY-Q library contracts, focusing on compatibility rather than accuracy. It does not aim to produce 1:1 output compared to real hardware and does not reproduce game bugs that rely on misuse of the PlayStation 1 hardware. The goal is to help port games and run them as native applications.

## PSY-Q decomp

To achieve a high level of API compatibility and accuracy, PSY-Z utilizes decompiled PSY-Q code that matches the original binaries exactly. This decompilation is an ongoing process. Please navigate to the `decomp/` directory for more details.

This decomp is a spin-off of [sotn-decomp psxsdk](https://github.com/Xeeynamo/sotn-decomp/tree/master/src/main/psxsdk) and reuses from the [PSY-Q decomp by Sozud](https://github.com/sozud/psy-q-decomp).

## Integrate PSY-Z SDK

To cross-compile your application between PSY-Q and PSY-Z, there is an important step to follow. The PSY-Q SDK uses something called _ordered table_, often abbreviated as `ot`. The type of this variable is `u_long*`. Due to how PSY-Q handles memory, the ordered table will not work on other platforms without modification. You need to replace `u_long*` with `OT_TYPE*` and include the header `psyz.h`, along with the modified `lib*.h` headers in your software.

In rare cases where certain applications or games recreate or reuse GPU primitives defined in `libgpu.h`, be sure to replace the struct field at the beginning of your custom structs from `u_long*` tag to `O_TAG`. You can refer to struct `POLY_F3` or similar for examples of how the original structures were adjusted.

To include the necessary headers, add `-Ipsyz/include`to your GCC or Clang compiler flags. You can look into the `samples/` directory for examples on how to set up a project to target both PlayStation 1 hardware and PSY-Z on PC.

## Set-up the PSY-Z SDK

At the root of the directory run `make -j`. This will download PSY-Q SDK 4.7 and populate `nugget/psyq` with the transformed `include` and `lib` folders. PCSX Redux will also be downloaded to compile the tool `psyq-obj-parser`, necessary to generate the libraries. This might take a while.

`nugget` is an important step to ensure you can cross-compile your source code to use both PSY-Z and PSY-Q. If you are not interested on cross-compiling your homebrew to both PlayStation 1 and PC, you can skip this step.

## Samples

Most of these examples are adaptations of the original PSY-Q SDK samples, designed to target both PS1 and all PSY-Z platforms. This is achieved via [nugget](https://github.com//pcsx-redux/nugget).

These samples are frequently used to test the quality of PSY-Z, assesting the feature set and to catch regressions.

## Architecture

The SDK is structured into three major folders:

### src/psyz

Contains the core of the SDK. The majority of the SDK calls are reimplemented here. Whenever an API call is platform-specific, the call is redirected to an equivalent function with the `My` prefix (e.g., `LoadImage` internally calls `MyLoadImage`). All platform-specific code is located in `src/platform`.

### src/platform

Platform-specific logic is found here. Each platform uses a subset of the source files in this folder. The build script instructs the linker to decide what platform-specific code needs to be used in the final executable. This means no `#ifdef` spam or function pointers. Everything is statically linked whenever possible.

### decomp/src

All the code in this folder mirrors the original PSY-Q libraries and it is powered by the matching decompilation of PSY-Q. Currently it targets PSY-Q 4.0.

## Why develop PSY-Z?

Before starting this project, I tried using both [libValkyrie](https://github.com/Gh0stBlade/libValkyrie) and [PsyCross](https://github.com/OpenDriver2/PsyCross/). However, each presented its own challenges, such as lack of 64-bit support, insufficient documentation, missing samples, or inflexibility when porting to new platforms. PSY-Z aims to solve these issues and more.

I also became increasingly fascinated by the PlayStation 1 hardware due to its simplicity and versatility. This project is a personal opportunity to learn more.

## Contributing

Contributions are welcome! If you find issues or want to add new features, please open an issue or submit a pull request. Ensure that any code you submit adheres to the project's coding standards and includes appropriate documentation.
