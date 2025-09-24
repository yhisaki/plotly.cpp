# ===== Basic Configuration =====
PARALLEL_JOBS ?= $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)

CMAKE                   ?= cmake
BUILD_DIR               ?= build
BUILD_DIR_ASAN          ?= build-asan
BUILD_DIR_TSAN          ?= build-tsan
BUILD_DIR_COV           ?= build-coverage
COVERAGE_OUTPUT_DIR     ?= coverage

# ===== User-Tunable Variables =====
CMAKE_C_COMPILER        ?= clang
CMAKE_CXX_COMPILER      ?= clang++
CLANG_FORMAT            ?= clang-format
CLANG_TIDY              ?= clang-tidy
CMAKE_CXX_STANDARD      ?= 17  # Default to C++17, supports C++17, C++20, C++23, etc.
CMAKE_BUILD_TYPE        ?= Debug

PLOTLY_CPP_BUILD_GALLERY ?= ON
PLOTLY_CPP_BUILD_TESTS   ?= ON

# ===== CMake Options =====
CMAKE_COMMON_OPTIONS = \
	-DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
	-DCMAKE_CXX_STANDARD=$(CMAKE_CXX_STANDARD) \
	-DCMAKE_C_COMPILER=$(CMAKE_C_COMPILER) \
	-DCMAKE_CXX_COMPILER=$(CMAKE_CXX_COMPILER) \
	-DPLOTLY_CPP_BUILD_GALLERY=$(PLOTLY_CPP_BUILD_GALLERY) \
	-DPLOTLY_CPP_BUILD_TESTS=$(PLOTLY_CPP_BUILD_TESTS)

# ===== Helper Functions =====
define cmake_configure
	@echo "Configuring: $(1)"
	$(CMAKE) -S . -B $(1) $(CMAKE_COMMON_OPTIONS) $(2)
endef

define cmake_build
	@echo "Building: $(1) $(if $(2),target=$(2),)"
	$(CMAKE) --build $(1) --parallel $(PARALLEL_JOBS) $(if $(2),--target $(2),)
endef

define sanitizer_flags
	-DCMAKE_BUILD_TYPE=Debug \
	-DCMAKE_C_FLAGS="-fsanitize=$(1) $(2) -g" \
	-DCMAKE_CXX_FLAGS="-fsanitize=$(1) $(2) -g" \
	-DCMAKE_EXE_LINKER_FLAGS="-fsanitize=$(1)" \
	-DCMAKE_SHARED_LINKER_FLAGS="-fsanitize=$(1)"
endef

# ===== Build Targets =====
.PHONY: release debug asan tsan coverage coverage-clang format tidy clean package doxygen

release:
	$(call cmake_configure,$(BUILD_DIR), -DCMAKE_BUILD_TYPE=Release)
	$(call cmake_build,$(BUILD_DIR))

debug:
	$(call cmake_configure,$(BUILD_DIR))
	$(call cmake_build,$(BUILD_DIR))

asan:
	$(call cmake_configure,$(BUILD_DIR_ASAN), $(call sanitizer_flags,address,-fno-omit-frame-pointer))
	$(call cmake_build,$(BUILD_DIR_ASAN))

tsan:
	$(call cmake_configure,$(BUILD_DIR_TSAN), $(call sanitizer_flags,thread,))
	$(call cmake_build,$(BUILD_DIR_TSAN))

coverage:
	$(call cmake_configure,$(BUILD_DIR_COV), \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_CXX_COMPILER=g++ \
		-DCMAKE_C_COMPILER=gcc \
		-DCMAKE_C_FLAGS="--coverage -O0 -g" \
		-DCMAKE_CXX_FLAGS="--coverage -O0 -g" \
		-DCMAKE_EXE_LINKER_FLAGS="--coverage" \
		-DCMAKE_SHARED_LINKER_FLAGS="--coverage")
	$(call cmake_build,$(BUILD_DIR_COV))
	@echo "Running tests for coverage..."
	@cd $(BUILD_DIR_COV) && ctest --output-on-failure
	@echo "Generating coverage report..."
	@mkdir -p $(COVERAGE_OUTPUT_DIR)
	@echo "Finding source files for coverage..."
	@cd $(BUILD_DIR_COV) && find . -name "*.gcda" -exec gcov {} \; > /dev/null 2>&1 || true
	@echo "Generating HTML coverage report with lcov..."
	@if command -v lcov >/dev/null 2>&1 && command -v genhtml >/dev/null 2>&1; then \
		cd $(BUILD_DIR_COV) && \
		lcov --capture --directory . --output-file coverage.info && \
		lcov --remove coverage.info '*/third_party/*' '*/test/*' '*/usr/*' --output-file coverage_filtered.info && \
		genhtml coverage_filtered.info --output-directory ../$(COVERAGE_OUTPUT_DIR) && \
		echo "Coverage report generated: $(COVERAGE_OUTPUT_DIR)/index.html"; \
	else \
		echo "lcov and genhtml are required for HTML coverage reports. Install with:"; \
		echo "  Ubuntu/Debian: sudo apt install lcov"; \
		echo "  RHEL/CentOS: sudo yum install lcov"; \
		echo "  macOS: brew install lcov"; \
		echo ""; \
		echo "Basic coverage data generated in $(BUILD_DIR_COV)/*.gcov files"; \
	fi

format:
	@echo "Formatting source files..."
	@if ! command -v $(CLANG_FORMAT) >/dev/null 2>&1; then \
		echo "Error: $(CLANG_FORMAT) not found. Please install clang-format."; \
		exit 1; \
	fi
	@# Note: See .clang-format-ignore for excluded directories
	@for d in src include gallery test; do \
		[ -d $$d ] && find $$d -type f \( -name "*.cpp" -o -name "*.hpp" \) \
			! -path "*/third_party/*" -exec $(CLANG_FORMAT) -i {} +; \
	done
	@echo "Formatting completed."

tidy:
	$(call cmake_configure,$(BUILD_DIR))
	@echo "Running clang-tidy..."
	@if ! command -v run-$(CLANG_TIDY) >/dev/null 2>&1; then \
		echo "Error: run-$(CLANG_TIDY) not found. Please install run-clang-tidy."; \
		exit 1; \
	fi
	@run-$(CLANG_TIDY) -fix -format -p=$(BUILD_DIR) \
		-source-filter="^(?!.*/build/|.*/third_party/).*" \
		-j $(PARALLEL_JOBS) || echo "clang-tidy completed with warnings."

package:
	@echo "Building Debian package..."
	$(call cmake_configure,$(BUILD_DIR), -DCMAKE_BUILD_TYPE=Release)
	$(call cmake_build,$(BUILD_DIR))
	@echo "Creating Debian package with CPack..."
	@cd $(BUILD_DIR) && cpack -G DEB
	@echo "Package created successfully!"
	@echo "Package files:"
	@find $(BUILD_DIR) -name "*.deb" -type f -exec ls -la {} \;

doxygen:
	@echo "Generating Doxygen documentation..."
	@if ! command -v doxygen >/dev/null 2>&1; then \
		echo "Error: doxygen not found. Please install doxygen."; \
		echo "  Ubuntu/Debian: sudo apt install doxygen graphviz"; \
		exit 1; \
	fi
	@command doxygen Doxyfile
	@echo "Documentation generated successfully in doxygen/html/"
	@echo "Open doxygen/html/index.html in your browser to view the documentation"

clean:
	@rm -rf $(BUILD_DIR) $(BUILD_DIR_ASAN) $(BUILD_DIR_TSAN) $(BUILD_DIR_COV) $(COVERAGE_OUTPUT_DIR) doxygen
