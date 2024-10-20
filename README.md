# PSY-Z SDK

PSY-Z is a drop-in replacement for the PlayStation 1 Runtime Library called PSY-Q, allowing games to target multiple platforms.

This is effectively a porting library written in C that stubs as many API calls as possible. It currently targets Linux and macOS, supporting both 32-bit and 64-bit architectures out of the box. The same applications and games will work on all platforms, including the PlayStation 1, with little to no changes.

## Integrate PSY-Z SDK

There is only one important step to follow to cross-compile your application between PSY-Q and PSY-Z. The PSY-Q SDK uses something called _ordered table_, often abbreviated as `ot`. The type of this variable is `u_long*`. Due to how PSY-Q handles memory, the ordered table will not work on other platforms without modification. You need to replace `u_long*` with `OT_TYPE*` and include the header `psyz.h` in your app.

In rare cases where certain applications or games re-create or re-use GPU primitives defined in `libgpu.h`, be sure to replace the struct field at the beginning of your custom structs from `u_long*` tag to `O_TAG`. You can refer to struct `POLY_F3` or similar for examples of how the original structures were adjusted.

## Compile the PSY-Z SDK

At the root of the directory run `make -j`. This will download PSY-Q SDK 4.7 and populate `nugget/psyq` with the transformed `include` and `lib` folders. PCSX Redux will also be downloaded to compile the tool `psyq-obj-parser`, necessary to generate the libraries. This might take a while.

TODO

## Samples

Most of these examples are adaptations of the original PSY-Q SDK samples, designed to target both PS1 and all PSY-Z platforms. This is achieved via [nugget](https://github.com//pcsx-redux/nugget).

## Architecture

The SDK is structured into three major folders:

### src/psyz

Contains the core of the SDK. The majority of the SDK calls are reimplemented here. Whenever an API call is platform-specific, the call is redirected to an equivalent function with the `My` prefix (e.g., `LoadImage` internally calls `MyLoadProfile`). All platform-specific code is located in `src/platform`.

### src/psyq

All the code in this folder mirrors the original PSY-Q libraries, either logically or directly equivalent. Currently, this targets PSY-Q 4.6.

### src/platform

Platform-specific logic is found here. Each platform uses a subset of the source files in this folder. Ideally the amount of `#ifdef` is reduced and let the linker to decide what platform-specific code needs to be included in the final executable.

## Why develop PSY-Z?

Before starting this project, I tried using both [libValkyrie](https://github.com/Gh0stBlade/libValkyrie) and [PsyCross](https://github.com/OpenDriver2/PsyCross/). However, each presented its own challenges, such as lack of 64-bit support, insufficient documentation, missing samples, or inflexibility when porting to new platforms. PSY-Z aims to solve these issues and more.
