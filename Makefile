BUILD_DIR = build
INCLUDE_DIR = include


CPP_SRC = src/main.cpp src/tree_isomorphism.cpp
CPP_EXEC = tree_isomorphism
CXX = g++
CXXFLAGS = -std=c++17 -I$(INCLUDE_DIR) -I/opt/homebrew/include
CLANG = clang++

build:
	$(CXX) $(CXXFLAGS) $(CPP_SRC) -o $(CPP_EXEC)

.PHONY: all run build clean tree_iso_build