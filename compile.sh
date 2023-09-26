#!/bin/bash
FILES=./resources/shaders/*

for file in $FILES
do
	if [[ "$file" == *.vert ]] || [[ "$file" == *.frag ]]
	then
		glslc "$file" -o "$file".spv
		echo "Compiled $file"
	fi
done