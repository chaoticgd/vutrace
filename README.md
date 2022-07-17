# vutrace

A tracing debugger for the PS2's second vector processing unit (VPU1). A modified version of PCSX2 can be used to dump out the state of VU1 after each instruction is executed, and the resultant traces can be viewed with the main vutrace application.

This is useful for reversing the renderers and model formats of PS2 games.

It should build on Linux (GCC) or Windows (MSVC) but has mainly been tested on Linux.

**The patch was last updated on 2022-07-17.**

## Screenshot

![Screenshot](screenshot.png)

## vutrace Usage

1. Checkout PCSX2 commit `011d6bebfa15e077cfde91af14d3ae7e57d348ed` and apply the patch `pcsx2_011d6bebfa15e077cfde91af14d3ae7e57d348ed.patch` to it using `git apply`, then build PCSX2.

2. Build vutrace using cmake: `cmake . && cmake --build .`.

3. Launch PCSX2.

4. In PCSX2, use the VU1 interpreter, don't use the VU0 interpreter, and set GSdx to software mode. I haven't tested it with MTVU enabled, so don't use that.

5. Create a directory called `vutrace_output` in the working directory you ran PCSX2 from (it won't create the directory itself).

6. Trace a frame using the menu item `System->Begin VU trace...` (warning: traces may be very large).

7. Open a trace: `./vutrace (PCSX2 working dir)/vutrace_output/traceN.bin` where N is the index of the trace.

## vudis Usage

This is the disassembler split out into a seperate component.

1. Build vutrace using cmake: `cmake . && cmake --build .`.

2. Open a memory dump: `./vudis vu0MicroMem.bin`.

where `vu0MicroMem.bin` or `vu1MicroMem.bin` can be extracted from a PCSX2 save state.

## Tips

- A _log.txt file is written out with the trace with logs of all the VIF1 DMA transfers in it. If you know the address of one of the VIF command lists you're interested in, you can use this file to find the associated trace file.
- If you have the data you're interested in but not its address, you can VIF unpack (see EE User's Manual section 6.3.4) the data manually and binary grep for it.

## Keyboard Controls

- W - Step back one instruction.
- S - Step forward one instruction.
- A - Step back one loop iteration (until the PC is the same as it was originally).
- D - Step forward one loop iteration (until the PC is the same as it was originally).

## Known Issues

- vutrace: The GS packet parser assumes that the data transfer to the GS is instant.
- vutrace: Parsing of REGLIST primitives is not supported.
- patch: Traces don't start and end on vsync so it doesn't always trace an entire frame.
- patch: The framebuffer dumps aren't synced with the VU state dumps.
