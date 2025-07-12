# Makefile for Gravity Simulator
# Assumes you have activated your Conda env (so $CONDA_PREFIX is set)

# C++ compiler & flags
CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall

# Conda env root
PREFIX      ?= $(CONDA_PREFIX)
INCLUDE_DIR := $(PREFIX)/include
LIB_DIR     := $(PREFIX)/lib

# Sources and target
SRC    := gravity_sum.cpp
OBJ    := $(SRC:.cpp=.o)
TARGET := sim

# Include & library flags
INCLUDES := -I$(INCLUDE_DIR)
LIBS     := -L$(LIB_DIR) \
            -lglfw \
            -lGL \
            -ldl \
            -lpthread \
            -lm \
            -lX11 \
            -lXrandr \
            -lXxf86vm \
            -lXcursor

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	 $(CXX) $(CXXFLAGS) $(OBJ) -o $@ $(LIBS) -Wl,-rpath,$(LIB_DIR)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

