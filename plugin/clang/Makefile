LLVM_CONFIG ?= llvm-config
CXX := clang++

BUILD := build
PLUGIN := $(BUILD)/clang-reflect-plugin.so

LLVM_CXXFLAGS := $(shell $(LLVM_CONFIG) --cxxflags)
LLVM_LDFLAGS := $(shell $(LLVM_CONFIG) --ldflags --system-libs --libs)
LLVM_LIBDIR := $(shell $(LLVM_CONFIG) --libdir)
CLANG_CPP := $(firstword $(wildcard $(LLVM_LIBDIR)/libclang-cpp.so $(LLVM_LIBDIR)/libclang-cpp.so.*))

CXXFLAGS += $(LLVM_CXXFLAGS) -fPIC

.PHONY: all clean

all: $(PLUGIN)

$(PLUGIN): clang-reflect-plugin.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -shared $< $(LLVM_LDFLAGS) $(CLANG_CPP) -o $@

clean:
	rm -rf $(BUILD)
