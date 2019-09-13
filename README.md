simgui is an interface for simavr built using dear imgui and other stuff

this was an honours project for school and as such will not see more development beyond what is already here

# Installation/Build instructions

Building this project is a lot of trial and error. First of all, you'll need four repos:

simgui (this one): https://github.com/zachdirk/simgui

imgui: https://github.com/zachdirk/imgui

simavr: https://github.com/zachdirk/simavr

and gl3w: https://github.com/skaslev/gl3w

In order to use the makefile in simgui, place these all in the same high level directory without changing their names. The makefile references `../simgui`, `../imgui`, `../simavr`, and `../gl3w/`. The simplest (but most time consuming) way to build is just to use make and then apt-get to install any missing packages. Eventually it will work.

# notes:

## source repos for code I didn't write

### SIMAVR

https://github.com/buserror/simavr

### gl3w

https://github.com/skaslev/gl3w

### Dear ImGui

https://github.com/ocornut/imgui

### cpp-subprocess

https://github.com/arun11299/cpp-subprocess

only took subprocess.hpp and put it in simgui/include

### json deserializer

https://github.com/nlohmann/json

only took single_include/nlohmann/json.hpp and put it in simgui/include

## assembler flags used by simgui during assembly

* -x -assembler-with-cpp - -x specifies a language, -assembler-with-cpp forces gcc to interpret the input file as an assembly file to be preprocessed

* -no-start-files

## weird bugs

### imgui Assertion `strcmp(version,"x.xx")==0 && "Mismatched version string!"' failed.

You probably just updated imgui, make sure you recompile the entire program otherwise you'll get this issue. `make clean` followed by `make` should do the trick.

### assmebler not applying preprocessing

If you don't specify that your assembler do preprocessing with the proper flags, you must be certain that your file terminates in .S with a capital S. Otherwise, if it ends in .s with a lowercase s, this signifies your assembly file has already been preprocessed.
