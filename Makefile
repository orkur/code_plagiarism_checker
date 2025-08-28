BUILD_DIR = build
INCLUDE_DIR = include
NLOHMANN_JSON = $(INCLUDE_DIR)/nlohmann/json.hpp

CPP_SRC = src/main.cpp src/tree_isomorphism.cpp
CPP_EXEC = tree_isomorphism
CXX = g++
CXXFLAGS = -std=c++17 -I$(INCLUDE_DIR)
CLANG = clang++


build: $(NLOHMANN_JSON)
	$(CXX) $(CXXFLAGS) $(CPP_SRC) -o $(CPP_EXEC)

$(NLOHMANN_JSON):
	mkdir -p $(INCLUDE_DIR)/nlohmann
	curl -L https://github.com/nlohmann/json/releases/download/v3.12.0/json.hpp -o $(NLOHMANN_JSON)

create-tree $(FILE):
	clang++ -Xclang -ast-dump=json -fsyntax-only $(CXXFLAGS) $(FILE) > $(FILE:.cpp=.json)


clean:
	rm -fr $(CPP_EXEC) $(INCLUDE_DIR)/nlohmann

.PHONY: build clean create-tree