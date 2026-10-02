CXX      := clang++
AR       := llvm-ar
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -fPIC -Iinclude

BUILD    := build
SRC      := src
INC      := include

LIB      := $(BUILD)/libreflect.a
SHARED_LIB := $(BUILD)/libreflect.so

SRCS     := $(wildcard $(SRC)/*.cpp)
OBJS     := $(patsubst $(SRC)/%.cpp,$(BUILD)/obj/%.o,$(SRCS))

HEADERS  := $(wildcard $(INC)/*.hpp)

.PHONY: all clean

all: $(LIB) $(SHARED_LIB)

$(BUILD)/obj/%.o: $(SRC)/%.cpp $(HEADERS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(LIB): $(OBJS)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^

$(SHARED_LIB): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) -shared -o $@ $^

clean:
	rm -rf $(BUILD)