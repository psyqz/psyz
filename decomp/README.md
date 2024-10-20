# PSY-Q decomp

This project aims to decompile the PSY-Q 4.0 SDK by matching individual functions. This is achieved by converting all the existing PSY-Q library object files to GNU-compatible objects and generating an ELF binary. This approach allows the use of modern tooling to aid in the decompilation process. The goal is for the decompiled code to match the generated ELF file exactly.

This project allows to better understand the PSY-Q SDK internals better and to preserve the software for future development and archival purposes.

## Set-up

1. Be sure to clone this repo with `git clone PATH --recurse-submodules` or run `git submodule update --init --recursive` if you cloned it already.
1. In the root directory run `make psyq_libs psyq_incs -j` to download and install PSY-Q in `../nugget/psyq/`.
1. Run `make -j` to verify the decompiled PSY-Q SDK code matches the original binaries.

## Decompile new code

This follows the same decompilation rules from [sotn-decomp](https://github.com/Xeeynamo/sotn-decomp/wiki/Decompilation).

## How does this differ from Sozud' psy-q-decomp

The two decompilation projects follow different philosophies:

This PSY-Q decomp focuses on matching the GNU-converted libraries from the original SDK. It allows matching functions one by one and enables the use of modern tools and compilers.

Sozud's psy-q-decomp aims to match the original unconverted OBJ files using the original Win16/Win32 compilers. The challenge with Sozud's approach is that the entire object file needs to be decompiled and matched, which can be more difficult. But it will produce the original files.

Whenever possible, decompiled code from this repository will be upstreamed to [Sozud decomp](https://github.com/sozud/psy-q-decomp).

## Why PSY-Q 4.0

1. Easier to Match: PSY-Q 4.0 uses GCC 2.7.2, which is more flexible than the GCC 2.6.x used by PSY-Q 3.x, making it easier to match functions.
1. Feature-Rich: It includes PSY-Q 4.x exclusive APIs used in the latest PS1 games and incorporates several fixes from PSY-Q 3.x.
1. More Symbols: There are more symbols embedded in the OBJ files compared to PSY-Q 3.x, which aids in the decompilation process.
1. This project initially started as a PSY-Q 4.7 decompilation. However, matching functions from PSY-Q 4.1 or above proved difficult, possibly due to differences in the compiler or compiler flags used, which are not currently known.

## Contributing

Contributions are welcome! Please ensure that any code you submit matches the generated ELF binaries. When contributing, adhere to the guidelines provided in the sotn-decomp decompilation rules.
