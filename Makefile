# OS Detection for toolchain 
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Darwin)
  # macOS (Apple Silicon)
  NPROC        := $(shell sysctl -n hw.logicalcpu)
  TOOLCHAIN     = cmake/aarch64-toolchain-mac.cmake
  CROSS_STRIP   = aarch64-unknown-linux-gnu-strip
  CROSS_READELF = aarch64-unknown-linux-gnu-readelf
else
  # Linux (CachyOS / Arch intended..)
  NPROC        := $(shell nproc)
  TOOLCHAIN     = cmake/aarch64-toolchain.cmake
  CROSS_STRIP   = aarch64-linux-gnu-strip
  CROSS_READELF = aarch64-linux-gnu-readelf
endif

# For deployment, set these env vars to ssh target
DEVICE        ?= rg35xxh
DEVICE_USER   ?= root
REMOTE_PATH    = /mnt/mmc/MUOS/application/gplove
BUILD_AARCH64  = build-aarch64
BUILD_NATIVE   = build-native

# Desktop build and run 
run-native:
	cmake -B $(BUILD_NATIVE) -DCMAKE_BUILD_TYPE=Debug -G Ninja --log-level=WARNING
	cmake --build $(BUILD_NATIVE) -j$(NPROC) # --quiet
	cd $(BUILD_NATIVE) && \
	  SDL_GAMECONTROLLERCONFIG_FILE=../assets/gamecontrollerdb.txt \
	  ./gplove

# aarch64 cross-compile 
configure-cross:
	cmake -B $(BUILD_AARCH64) \
	  -DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) \
	  -DCMAKE_BUILD_TYPE=Release \
	  -G Ninja

build-cross: configure-cross
	cmake --build $(BUILD_AARCH64) -j$(NPROC)

# Verify binary 	
check:
	@file $(BUILD_AARCH64)/gplove
	@$(CROSS_READELF) -d $(BUILD_AARCH64)/gplove | grep NEEDED

# Deploy 
deploy: build-cross
	ssh $(DEVICE_USER)@$(DEVICE) "mkdir -p $(REMOTE_PATH)/libs $(REMOTE_PATH)/assets"
	scp $(BUILD_AARCH64)/gplove          $(DEVICE_USER)@$(DEVICE):$(REMOTE_PATH)/
	scp -r assets/                         $(DEVICE_USER)@$(DEVICE):$(REMOTE_PATH)/assets/
	scp -r libs/aarch64/*                  $(DEVICE_USER)@$(DEVICE):$(REMOTE_PATH)/libs/
	scp packaging/muos/gplove.sh         $(DEVICE_USER)@$(DEVICE):$(REMOTE_PATH)/
	ssh $(DEVICE_USER)@$(DEVICE) \
	  "chmod +x $(REMOTE_PATH)/gplove $(REMOTE_PATH)/gplove.sh"
	@echo "DONE: Deployed to $(DEVICE):$(REMOTE_PATH)"

# Run on device 
run-device:
	ssh $(DEVICE_USER)@$(DEVICE) "$(REMOTE_PATH)/gplove.sh"

# Full device loop, build, deploy, run on-device
dev: deploy run-device

# Kill hung process on device 
kill:
	ssh $(DEVICE_USER)@$(DEVICE) "killall gplove 2>/dev/null; echo killed"

# Clean 
clean:
	rm -rf $(BUILD_AARCH64) $(BUILD_NATIVE)

.PHONY: run-native configure-cross build-cross check deploy run-device dev kill clean