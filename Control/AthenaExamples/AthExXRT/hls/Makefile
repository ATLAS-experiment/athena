ECHO=@echo

.SECONDARY:

.PHONY: help

help:
	$(ECHO) "Makefile Usage:"
	$(ECHO) ""
	$(ECHO) "  Build all kernels:"
	$(ECHO) "    make all                    [PLATFORM=PLATFORM=xilinx_u250_gen3x16_xdma_4_1_202210_1] [TARGET=hw]"
	$(ECHO) ""
	$(ECHO) "  Build a specific kernel:"
	$(ECHO) "    make krnl_Combined.xclbin   [PLATFORM=PLATFORM=xilinx_u250_gen3x16_xdma_4_1_202210_1] [TARGET=hw]"
	$(ECHO) "    make krnl_VectorAdd.xclbin  [PLATFORM=PLATFORM=xilinx_u250_gen3x16_xdma_4_1_202210_1] [TARGET=hw]"
	$(ECHO) "    make krnl_VectorMult.xclbin [PLATFORM=PLATFORM=xilinx_u250_gen3x16_xdma_4_1_202210_1] [TARGET=hw]"
	$(ECHO) ""
	$(ECHO) "  Clean build artefacts and output:"
	$(ECHO) "    make clean"
	$(ECHO) ""

all: krnl_VectorAdd.xclbin krnl_VectorMult.xclbin krnl_Combined.xclbin

clean:
	rm -rf ./build_*_* *.log .Xil/ .ipcache/ _x/ *.xclbin

TARGET ?= hw
PLATFORM ?= xilinx_u250_gen3x16_xdma_4_1_202210_1

XOCCFLAGS := --platform $(PLATFORM) -t $(TARGET)

XOCCLFLAGS := $(XOCCFLAGS)
XOCCLFLAGS += --kernel_frequency 100

BUILD_DIR := ./build_$(PLATFORM)_$(TARGET)

$(BUILD_DIR)/%.xo: %.cpp
	$(eval OUT_DIR:=$(BUILD_DIR)/xo )
	mkdir -p $(OUT_DIR)
	v++ $(XOCCFLAGS) --kernel $* --temp_dir $(OUT_DIR) --log_dir $(OUT_DIR) -c -o $@ $<

XOS := $(BUILD_DIR)/krnl_VectorAdd.xo $(BUILD_DIR)/krnl_VectorMult.xo

ifneq (,$(findstring vck5000,$(PLATFORM))) # For VCK5000, there is an additional packaging step required

$(BUILD_DIR)/%.xsa: $(XOS)
	$(eval OUT_DIR:=$(BUILD_DIR)/hw/$* )
	mkdir -p $(OUT_DIR)
	v++ -l $(XOCCLFLAGS) --config $*_hw.cfg --temp_dir $(OUT_DIR) --log_dir $(OUT_DIR) -o $@ $^

$(BUILD_DIR)/%.xclbin: $(BUILD_DIR)/%.xsa
	$(eval OUT_DIR:=$(BUILD_DIR)/hw/$* )
	mkdir -p $(OUT_DIR)
	v++ -p -t $(TARGET) -f $(PLATFORM) $^ --temp_dir $(OUT_DIR) --log_dir $(OUT_DIR) -o $@ --package.boot_mode=ospi

else # Standard Alveo case

$(BUILD_DIR)/%.xclbin: $(XOS)
	$(eval OUT_DIR:=$(BUILD_DIR)/hw/$* )
	mkdir -p $(OUT_DIR)
	v++ -l $(XOCCLFLAGS) --config $*_hw.cfg --temp_dir $(OUT_DIR) --log_dir $(OUT_DIR) -o $@ $^

endif

%.xclbin: $(BUILD_DIR)/%.xclbin
	ln -sf $< $@
