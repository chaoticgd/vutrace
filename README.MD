# vutrace

A tracing debugger for the PS2's second vector processing unit (VPU1). A modified version of PCSX2 can be used to dump out the state of VU1 after each instruction is executed, and the resultant traces can be viewed with the main vutrace application.

It works on Linux. It might work on Windows but I haven't tested it.

## Usage

1. Checkout PCSX2 commit `58f5a5b915915293b90e7e8f34c33f9c0424d1e8` and apply the patch `pcsx2_58f5a5b915915293b90e7e8f34c33f9c0424d1e8.patch` to it using `git apply`, then build PCSX2.

2. Build vutrace using cmake: `cmake . && cmake --build .`.

3. Launch PCSX2 with a command similar to `./pcsx2/PCSX2 --gs=plugins/GSdx/libGSdx.so`.

4. In PCSX2, use the VU1 interpreter, don't use the VU0 interpreter, and set GSdx to software mode. I haven't tested it with MTVU enabled, so don't use that.

5. Create a directory called `vutrace_output` in the working directory you ran PCSX2 from (it won't create the directory itself).

6. Trace a frame using `Begin VU trace...` (warning: traces may be very large).

7. Open a trace: `./vutrace (PCSX2 working dir)/vutrace_output/traceN.bin` where N is the index of the trace.
