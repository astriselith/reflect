CLANGXX ?= clang++
ifeq ($(origin CXX),default)
CXX := clang++
endif

BUILD := build
TEST_DIR := $(BUILD)/case
ROOT := $(abspath ../..)
CLANG_DIR := $(ROOT)/plugin/clang
CLANG_PLUGIN := $(CLANG_DIR)/build/clang-reflect-plugin.so
REFLECT_INCLUDE := $(ROOT)/include
REFLECT_LIB := $(ROOT)/build/libreflect.a

MK := $(TEST_DIR)/Makefile.reflect.mk
BIN := $(BUILD)/reflection-test

SRCS := main.cpp Inspector.cpp
HEADERS := $(wildcard *.hpp)

-include $(MK)

.PHONY: all test clean distclean rebuild

all: test

test: $(BIN)
	$(BIN)

$(BIN): $(MK) $(REFLECT_LIB)
	cd $(TEST_DIR) && $(CXX) -std=c++17 -I"$(REFLECT_INCLUDE)" \
		$(SRCS) $(REFLECT_GENERATED) \
		"$(REFLECT_LIB)" -o "../reflection-test"

$(MK): $(SRCS) $(HEADERS) $(CLANG_PLUGIN)
	rm -rf $(TEST_DIR)
	mkdir -p $(TEST_DIR)
	cp $(SRCS) $(HEADERS) $(TEST_DIR)/
	@cd $(TEST_DIR) && \
		for h in $(HEADERS); do \
			echo "==> reflect: $$h"; \
			$(CXX) -std=c++17 -fsyntax-only -x c++-header \
				-Xclang -load -Xclang "$(CLANG_PLUGIN)" \
				-Xclang -plugin -Xclang reflect \
				-I"$(REFLECT_INCLUDE)" $$h || exit 1; \
		done

$(CLANG_PLUGIN):
	make -C $(CLANG_DIR) all

$(REFLECT_LIB):
	make -C $(ROOT) all

clean:
	rm -rf $(BUILD)

distclean: clean
	make -C $(ROOT) clean
	make -C $(CLANG_DIR) clean

rebuild: clean all