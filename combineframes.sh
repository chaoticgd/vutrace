#!/bin/bash

# This script uses ffmpeg to combine all the dumped framebuffers from a trace
# session into a single video file.

if [[ ! -f _log.txt ]] ; then
	echo "_log.txt does not exist. You need to run this from the trace directory."
	exit 1
fi

mkdir combineframestemp
cp *.png combineframestemp
iter=0
for f in combineframestemp/* ; do
	mv $f "combineframestemp/fb$iter.png";
	iter=$((iter+1))
done
ffmpeg -r 30 -i "combineframestemp/fb%d.png" -pix_fmt yuv420p -r 30 _video.mp4
rm -r combineframestemp
