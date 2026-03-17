.PHONY: start setup build clean deps db client

VCPKG_ROOT := $(or $(VCPKG_ROOT),$(HOME)/vcpkg)
CMAKE_ARGS := -DCMAKE_TOOLCHAIN_FILE=$(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake -G Ninja

deps:
	$(VCPKG_ROOT)/vcpkg install

start:
	watchexec -e cpp,hpp -r "ninja -C build && ./build/vendbunny_backend"

setup:
	cmake -S . -B build $(CMAKE_ARGS) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

build:
	ninja -C build

clean:
	rm -rf build

db:
	@./scripts/db.sh $(filter-out db,$(MAKECMDGOALS))

client:
	@./scripts/client.sh $(filter-out client,$(MAKECMDGOALS))

create migrate seed fresh status list remove disable enable:
	@:

%:
	@:
