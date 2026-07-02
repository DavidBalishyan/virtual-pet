SHELL := /bin/bash
.POSIX:
PIPX  ?= $(shell command -v pipx 2>/dev/null)
PIO   ?= $(PIPX) run platformio # $(shell command -v pio 2>/dev/null || echo $HOME/.platformio/penv/bin/pio)
ENV   ?= m5stick-c
TEST  ?= native
MONITOR_PORT ?=

CPPCHECK   := $(shell command -v cppcheck 2>/dev/null)
CLANG_FMT  := $(shell command -v clang-format 2>/dev/null || command -v clang-format-19 2>/dev/null)
CLANG_TIDY := $(shell command -v clang-tidy 2>/dev/null || command -v clang-tidy-19 2>/dev/null)
VALGRIND   := $(shell command -v valgrind 2>/dev/null)
DOXYGEN    := $(shell command -v doxygen 2>/dev/null)
GIT        := $(shell command -v git 2>/dev/null)
PERL       := $(shell command -v perl 2>/dev/null)

SRC_DIRS      := src lib
SOURCES       := $(shell find $(SRC_DIRS) -type f \( -name '*.cpp' -o -name '*.c' \) \
                   ! -path '*/dashboard_assets.h' 2>/dev/null | sort)
GEN_HEADER     := lib/Wireless/dashboard_assets.h
EMBED_SCRIPT   := tools/embed_file.pl
SPRITE_SCRIPT  := tools/convert_sprites.pl
SPRITE_SRC     := assets/sprites/raw
SPRITE_OUT     := lib/Display/sprites
BOLD := \033[1m
DIM  := \033[2m
GREEN := \033[32m
YELLOW := \033[33m
CYAN := \033[36m
RED  := \033[31m
RESET := \033[0m

.DEFAULT_GOAL := help

help:
	@printf '\n$(BOLD)Usage:$(RESET) make $(GREEN)<target>$(RESET) [$(DIM)ENV=... TEST=...$(RESET)]\n\n'
	@printf '$(BOLD)Build & flash$(RESET)\n'
	@printf '  $(GREEN)build$(RESET)    Compile firmware ($(DIM)ENV=$(ENV)$(RESET))\n'
	@printf '  $(GREEN)upload$(RESET)   Build & upload to device ($(DIM)ENV=$(ENV)$(RESET))\n'
	@printf '  $(GREEN)monitor$(RESET)  Open serial monitor\n'
	@printf '  $(GREEN)run$(RESET)      upload + monitor in one step\n'
	@printf '  $(GREEN)clean$(RESET)    Remove build artifacts for $(DIM)ENV$(RESET)\n'
	@printf '  $(GREEN)distclean$(RESET) Full clean (build + generated files)\n'
	@printf '\n$(BOLD)Test$(RESET)\n'
	@printf '  $(GREEN)test$(RESET)     Run unit tests ($(DIM)TEST=$(TEST)$(RESET))\n'
	@printf '  $(GREEN)test-all$(RESET) Run tests on every environment\n'
	@if [ -n "$(VALGRIND)" ]; then \
		printf '  $(GREEN)test-mem$(RESET)  Run tests under valgrind\n'; \
	fi
	@printf '\n$(BOLD)Code quality$(RESET)\n'
	@if [ -n "$(CPPCHECK)" ]; then \
		printf '  $(GREEN)lint$(RESET)     Static analysis with cppcheck\n'; \
	fi
	@if [ -n "$(CLANG_FMT)" ]; then \
		printf '  $(GREEN)format$(RESET)   Format all source files with clang-format\n'; \
		printf '  $(GREEN)check-format$(RESET) Check formatting (diff, no write)\n'; \
	fi
	@if [ -n "$(CLANG_TIDY)" ]; then \
		printf '  $(GREEN)tidy$(RESET)     Lint with clang-tidy\n'; \
	fi
	@printf '\n$(BOLD)Utilities$(RESET)\n'
	@printf '  $(GREEN)generate$(RESET)  Regenerate dashboard + sprite asset headers\n'
	@printf '  $(GREEN)deps$(RESET)      Install PlatformIO dependencies\n'
	@printf '  $(GREEN)envs$(RESET)      List available PlatformIO environments\n'
	@printf '  $(GREEN)graph$(RESET)     Show library dependency graph\n'
	@if [ -n "$(GIT)" ]; then \
		printf '  $(GREEN)info$(RESET)      Show project info (git, lines of code)\n'; \
	fi
	@if [ -n "$(DOXYGEN)" ]; then \
		printf '  $(GREEN)docs$(RESET)     Generate Doxygen documentation\n'; \
	fi
	@printf '\n$(BOLD)Options$(RESET)\n'
	@printf '  $(DIM)ENV$(RESET)=m5stick-c    Target environment (default: m5stick-c)\n'
	@printf '  $(DIM)TEST$(RESET)=native      Test environment (default: native)\n\n'

.PHONY: build
build: generate
	@$(PIO) run -e $(ENV)

.PHONY: upload
upload:
	@$(PIO) run -e $(ENV) -t upload

.PHONY: monitor
monitor:
	@if [ -n "$(MONITOR_PORT)" ]; then \
		$(PIO) device monitor -p $(MONITOR_PORT); \
	else \
		$(PIO) device monitor; \
	fi

.PHONY: run
run: upload monitor

.PHONY: clean
clean:
	rm -f $(GEN_HEADER)
	rm -rf $(SPRITE_OUT)
	@$(PIO) run --target clean -e $(ENV)

.PHONY: distclean
distclean: clean
	@rm -rf .pio
	@printf '$(DIM)Removed generated files and .pio/$(RESET)\n'

.PHONY: test
test:
	@$(PIO) test -e $(TEST)

.PHONY: test-all
test-all:
	@$(PIO) test

.PHONY: test-mem
test-mem: test
	@if [ -z "$(VALGRIND)" ]; then \
		printf '$(RED)valgrind not found - skipping memory check$(RESET)\n'; \
	else \
		TEST_BIN=$$(find .pio/build/$(TEST) -name '*test*' -type f 2>/dev/null | head -1); \
		if [ -n "$$TEST_BIN" ]; then \
			$(VALGRIND) --leak-check=full --track-origins=yes --error-exitcode=1 "$$TEST_BIN"; \
		else \
			printf '$(YELLOW)Test binary not found - run make test first$(RESET)\n'; \
		fi \
	fi

.PHONY: lint
lint:
	@if [ -z "$(CPPCHECK)" ]; then \
		printf '$(RED)cppcheck not found - skipping lint$(RESET)\n'; \
		exit 1; \
	fi
	@printf '$(BOLD)Running cppcheck...$(RESET)\n'
	$(CPPCHECK) \
		--std=c++17 \
		--enable=warning,style,performance,portability \
		--suppress=missingIncludeSystem \
		--suppress=unmatchedSuppression \
		--error-exitcode=1 \
		--inline-suppr \
		--language=c++ \
		-q \
		$(SOURCES)

.PHONY: format
format:
	@if [ -z "$(CLANG_FMT)" ]; then \
		printf '$(RED)clang-format not found - skipping format$(RESET)\n'; \
		printf '  Install: sudo apt install clang-format-19$(RESET)\n'; \
		exit 1; \
	fi
	@printf '$(BOLD)Formatting sources...$(RESET)\n'
	@$(CLANG_FMT) -i -style=file $(SOURCES)

.PHONY: check-format
check-format:
	@if [ -z "$(CLANG_FMT)" ]; then \
		printf '$(RED)clang-format not found - skipping format check$(RESET)\n'; \
		exit 1; \
	fi
	@printf '$(BOLD)Checking formatting...$(RESET)\n'
	@$(CLANG_FMT) --dry-run -Werror -style=file $(SOURCES)

.PHONY: tidy
tidy:
	@if [ -z "$(CLANG_TIDY)" ]; then \
		printf '$(RED)clang-tidy not found - skipping tidy$(RESET)\n'; \
		exit 1; \
	fi
	@printf '$(BOLD)Running clang-tidy...$(RESET)\n'
	@$(CLANG_TIDY) $(SOURCES) -- -std=c++17

.PHONY: generate
generate: convert-sprites
	@if [ -n "$(PERL)" ]; then \
		$(PERL) $(EMBED_SCRIPT) $(GEN_HEADER) \
			lib/Wireless/dashboard.css:DASHBOARD_CSS \
			lib/Wireless/dashboard.html:DASHBOARD_HTML \
			lib/Wireless/dashboard.js:DASHBOARD_JS; \
	else \
		printf '$(RED)perl not found - cannot run $(EMBED_SCRIPT)$(RESET)\n'; \
		exit 1; \
	fi

.PHONY: convert-sprites
convert-sprites:
	@if [ -n "$(PERL)" ]; then \
		$(PERL) $(SPRITE_SCRIPT) $(SPRITE_SRC)/*.c; \
	else \
		printf '$(RED)perl not found - cannot run $(SPRITE_SCRIPT)$(RESET)\n'; \
		exit 1; \
	fi

.PHONY: deps
deps:
	@$(PIO) lib install

.PHONY: envs
envs:
	@awk -F'[][]' '/^\[env:/{print $$2}' platformio.ini

.PHONY: graph
graph:
	@$(PIO) lib list 2>/dev/null || printf '(run `pio run` first to install libs)\n'

.PHONY: info
info:
	@if [ -n "$(GIT)" ]; then \
		printf '$(BOLD)Repository:$(RESET)\n'; \
		$(GIT) log --oneline -5 2>/dev/null | sed 's/^/  /'; \
		printf '\n'; \
	fi
	@printf '$(BOLD)Lines of code:$(RESET)\n'
	@wc -l $(SOURCES) 2>/dev/null | tail -1 | sed 's/^/  /'
	@printf '\n$(BOLD)Source files:$(RESET)\n'
	@printf '  %3d  C/C++ source files (.cpp / .c)\n' $$(echo "$(SOURCES)" | wc -w)
	@printf '  %3d  Header files\n'                       $$(find $(SRC_DIRS) -name '*.h' 2>/dev/null | wc -l)
	@printf '  %3d  Sprite files\n'                        $$(find assets -name '*.c' 2>/dev/null | wc -l)
	@printf '\n$(BOLD)PlatformIO:$(RESET)\n'
	@$(PIO) --version 2>/dev/null | sed 's/^/  /'
	@printf '  Environments: '
	@awk -F'[][]' '/^\[env:/{e=e sep $$2; sep=", "} END{print e}' platformio.ini

.PHONY: docs
docs:
	@if [ -z "$(DOXYGEN)" ]; then \
		printf '$(RED)doxygen not found - skipping docs$(RESET)\n'; \
		exit 1; \
	fi
	@if [ ! -f Doxyfile ]; then \
		printf '$(YELLOW)No Doxyfile found - creating default$(RESET)\n'; \
		$(DOXYGEN) -g -q; \
	fi
	$(DOXYGEN)
