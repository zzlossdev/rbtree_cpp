CXX := clang++
TARGET ?= debug
CXXFLAGS := -std=c++23 -Werror -Wall -g

ifneq (,$(filter test%,$(MAKECMDGOALS)))
CXXFLAGS += -D_TC_ENABLE
endif

ifeq (rel, $(TARGET))
  CXXFLAGS += -O2
else
  LDFLAGS := -fsanitize=address -fno-omit-frame-pointer
endif

LDFLAGS += -fuse-ld=mold

LLVM_SYMBOLIZER := $(shell which llvm-symbolizer)

testcase: testcase.o file_stream.o rb_tree.o
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o testcase

.PHONY: clean testcase
