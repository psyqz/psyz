# PSX test output generator

Some unit tests for psyz expects an image to verify if the rendered output matches what an accurate PlayStation 1 emulator would produce.

This small project automates part of the process to produce the expected output image by leveraging the PCSX Redux web server debugger.

## Generate PNG output

1. Create a file named `.emu` containing the full path to the PCSX-Redux executable (e.g. `/usr/bin/pcsx-redux`)
2. Edit `test.c` with the setup and test code for the unit test you want to target.
3. Run `make gen` and wait and wait for the tool to generate the PNG files.

Do not commit the modified `test.c`.

The emulator is pre-configured with a `pcsx.json`, and it is automatically reset at every `make gen` execution via `git checkout` to ensure a clean state.

Three images gets generated:

* `output_fb1.png` first framebuffer at 0x0, size 256x240.
* `output_fb2.png` second framebuffer at 256x0, size 256x240.
* `output_vram.png` full 1024x512 VRAM dump.

