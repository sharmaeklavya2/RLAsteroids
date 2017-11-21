sources = rla.cpp
common_flags = -std=c++11
debug_flags = $(common_flags) -g -Wall -Wpedantic
release_flags = $(common_flags) -O2

debug:
	g++ $(debug_flags) $(sources)
release:
	g++ $(release_flags) $(sources)
