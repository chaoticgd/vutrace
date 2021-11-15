# vutrace

A tracing debugger for the PS2's second vector processing unit (VPU1). A modified version of PCSX2 can be used to dump out the state of VU1 after each instruction is executed, and the resultant traces can be viewed with the main vutrace application.

It should build on Linux (GCC) or Windows (MSVC) but has mainly been tested on Linux.

## Screenshot

![Screenshot](screenshot.png)

## vutrace Usage

1. Checkout PCSX2 commit `58f5a5b915915293b90e7e8f34c33f9c0424d1e8` and apply the patch `pcsx2_58f5a5b915915293b90e7e8f34c33f9c0424d1e8.patch` to it using `git apply`, then build PCSX2.

2. Build vutrace using cmake: `cmake . && cmake --build .`.

3. Launch PCSX2 with a command similar to `./pcsx2/PCSX2 --gs=plugins/GSdx/libGSdx.so`.

4. In PCSX2, use the VU1 interpreter, don't use the VU0 interpreter, and set GSdx to software mode. I haven't tested it with MTVU enabled, so don't use that.

5. Create a directory called `vutrace_output` in the working directory you ran PCSX2 from (it won't create the directory itself).

6. Trace a frame using `Begin VU trace...` (warning: traces may be very large).

7. Open a trace: `./vutrace (PCSX2 working dir)/vutrace_output/traceN.bin` where N is the index of the trace.

## vudis Usage

This is the disassembler split out into a seperate component.

1. Build vutrace using cmake: `cmake . && cmake --build .`.

2. Open a memory dump: `./vudis vu0MicroMem.bin`.

where `vu0MicroMem.bin` or `vu1MicroMem.bin` can be extracted from a PCSX2 save state.

## Keyboard Controls

- W - Step back one instruction.
- S - Step forward one instruction.
- A - Step back one loop iteration (until the PC is the same as it was originally).
- D - Step forward one loop iteration (until the PC is the same as it was originally).

## Known Issues

- vutrace: The GS packet parser assumes that the data transfer to the GS is instant.
- vutrace: Parsing of REGLIST primitives is not supported.
- patch: The version of PCSX2 that is supported is a bit out of date.
- patch: Traces don't start and end on vsync so it doesn't always trace an entire frame.
- patch: The framebuffer dumps aren't synced with the VU state dumps.
