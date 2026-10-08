# make         build everything into bin/
# make check   build, then run the tests and the verifier
# make clean   remove build/ and bin/

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -pedantic
override CXXFLAGS += -pthread
CPPFLAGS += -Isrc -MMD -MP

BUILD := build
BIN   := bin

LIB_SRC := $(wildcard src/*.cpp)
LIB_OBJ := $(patsubst src/%.cpp,$(BUILD)/%.o,$(LIB_SRC))

TOOLS := $(patsubst tools/%.cpp,%,$(wildcard tools/*.cpp))
TESTS := $(patsubst tests/%.cpp,%,$(wildcard tests/*.cpp))

PROGRAMS := $(addprefix $(BIN)/,$(TOOLS) $(TESTS))

all: $(PROGRAMS)

$(BUILD)/%.o: src/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

$(BUILD)/%.o: tools/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

$(BUILD)/%.o: tests/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

$(BIN)/%: $(BUILD)/%.o $(LIB_OBJ) | $(BIN)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD) $(BIN):
	mkdir -p $@

check: all
	@for test in $(addprefix $(BIN)/,$(TESTS)); do ./$$test || exit 1; done
	./$(BIN)/verifier

clean:
	rm -rf $(BUILD) $(BIN)

-include $(wildcard $(BUILD)/*.d)

.SECONDARY:  # keep object files between builds

.PHONY: all check clean
