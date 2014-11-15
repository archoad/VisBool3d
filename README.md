VisBool3d
=========

Visualizing Boolean functions in 3D

## Compilation

	make

## Launching
	$ ./visBool3d
	Michel Dubois -- visBool3d -- (c) 2013
	Syntaxe: visBool3d <samplesize> <type> <function>
		<samplesize> -> size of the sample space
		<type> -> type of visualization
			avalaible type are: bar, cube
		<function> -> name of the boolean function
			avalaible boolean functions are: xor, and, or, xnor, nand, nor, alea

For example, display a 3D graph of XOR function using cubes and a sample size of 30:

	./visBool3d 30 cube xor

## Using

r-key for rotating

z-key for zooming

Z-key for de-zooming

up|down|right|left-keys for moving

ESC-key to quit