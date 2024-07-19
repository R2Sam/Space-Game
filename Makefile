# Variables
SHELL = cmd
CXX = g++
SRC_DIR = src
OBJ_DIR = src
BIN_DIR = bin
DATA_DIR = data
INCLUDE_DIR = include
LIB_DIR = lib
FILE =

# Source and Object files
SRCS = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Includes
SUB_DIRS = $(wildcard $(INCLUDE_DIR)/*)
FILTER = .h .hpp
SUB_DIRS_FILTERED = $(filter-out %$(FILTER), $(SUB_DIRS))
INCLUDE_FLAGS = $(patsubst %,-I%, $(SUB_DIRS_FILTERED))

# Common compiler flags
COMMON_FLAGS = -std=c++17 -fmax-errors=5 -Werror -Wno-narrowing -Wno-enum-compare --static -I$(INCLUDE_DIR) $(INCLUDE_FLAGS) -L$(LIB_DIR)

# Debug and Release flags
DEBUG_FLAGS = -O0 -gdwarf-4
RELEASE_FLAGS = -O2 -s -mwindows
RES_FLAGS = *.res
FLAGS =

# Libraries to link against
LIBS =$(LIB_DIR)/libraylib.a -lopengl32 -lgdi32 -lwinmm

# Rules
done: debug Run

debug: FLAGS = $(DEBUG_FLAGS)
debug: $(OBJS)
	$(CXX) $(OBJS) $(COMMON_FLAGS) $(FLAGS) -o $(BIN_DIR)/main $(LIBS)

release: FLAGS = $(RELEASE_FLAGS)
release: $(OBJS)
	$(CXX) $(OBJS) $(COMMON_FLAGS) $(FLAGS) -o $(BIN_DIR)/main $(LIBS)

debugRes: FLAGS = $(DEBUG_FLAGS)
debugRes: $(OBJS)
	$(CXX) $(OBJS) $(RES_FLAGS) $(COMMON_FLAGS) $(FLAGS) -o $(BIN_DIR)/"Fall In 2" $(LIBS)

releaseRes: FLAGS = $(RELEASE_FLAGS)
releaseRes: $(OBJS)
	$(CXX) $(OBJS) $(RES_FLAGS) $(COMMON_FLAGS) $(FLAGS) -o $(BIN_DIR)/"Fall In 2" $(LIBS)

single_debug: FLAGS = $(DEBUG_FLAGS)
single_debug:
	$(CXX) -c $(wildcard $(SRC_DIR)/*/$(FILE)) $(wildcard $(SRC_DIR)/$(FILE)) $(COMMON_FLAGS) $(FLAGS)

Run:
	cmd /c start cmd /k "cd $(BIN_DIR) && main.exe"

Cmd:
	cmd /c start cmd

all: clean debug

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(COMMON_FLAGS) $(FLAGS) -c $< -o $@

# Emscripten compiler
EMXX = em++
EMSCRIPTEN_FLAGS = -w -fms-extensions -std=c++17 -D_DEFAULT_SOURCE -Wno-missing-braces -Wunused-result -Os -I$(INCLUDE_DIR) -DPLATFORM_WEB --preload-file $(DATA_DIR) --shell-file $(LIB_DIR)/shell.html -s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=67108864 -s FORCE_FILESYSTEM=1 -s EXPORTED_FUNCTIONS=['_free','_malloc','_main'] -s EXPORTED_RUNTIME_METHODS=ccall -L$(LIB_DIR) -lwebraylib

web: $(SRCS)
	$(EMXX) $(SRCS) $(EMSCRIPTEN_FLAGS) -o $(BIN_DIR)/main.html
	$(MAKE) clean_objs_wsl

clean:
	del /S /Q "$(OBJ_DIR)\*.o"

clean_objs_wsl:
	del /F /Q "$(OBJ_DIR)\*.o"
	del /F /Q "$(SRC_DIR)\*.o