BUILD=build

default: all test

$(BUILD):
	mkdir -p $(BUILD)

all: $(BUILD)
	cd $(BUILD) && cmake ../ && make

test: all
	cd $(BUILD) && env GTEST_COLOR=1 make testcommon test ARGS=-VV

doc: all
	cd $(BUILD) && make doc

clean:
	rm -rf $(BUILD)

veryclean: clean
	rm -rf gitdeps

.PHONY:
