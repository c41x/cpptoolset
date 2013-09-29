# granite makefile
# error syntax highlighting for flymake
check-syntax:
	mingw32-g++ -Wall -mmmx -msse -msse2 -msse3 -Wextra -fsyntax-only -std=c++0x -o nul -S $(CHK_SOURCES)

# program:
all:
	mingw32-g++ -g -std=c++0x -msse -msse2 -msse3 -Wall main.cpp ../../base/math.cpp -o bin/math-test.exe

# run this shit (startup target - somethig like 'startup project' from VS/NetBeans):
run:
	./bin/math-test.exe
