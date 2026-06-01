MAJOR ?= 1
MINOR ?= 0
PYTHON ?= python3

tag:
	@set -e; \
	MAJOR="$(MAJOR)"; MINOR="$(MINOR)"; PATCH="$(PATCH)"; \
	OLD_VERSION_VARS=$$($(PYTHON) -c "import json, pathlib; p = pathlib.Path('library.json'); v = json.loads(p.read_text()).get('version','0.0.0'); parts = v.split('.'); parts += ['0'] * (3 - len(parts)); print(f'OLD_MAJOR={parts[0]} OLD_MINOR={parts[1]} OLD_PATCH={parts[2]}')"); \
	eval "$$OLD_VERSION_VARS"; \
	[ -n "$$MAJOR" ] || MAJOR="$$OLD_MAJOR"; \
	[ -n "$$MINOR" ] || MINOR="$$OLD_MINOR"; \
	if [ -z "$$PATCH" ]; then \
		echo "PATCH not provided; auto-incrementing based on library.json..."; \
		PATCH=$$((OLD_PATCH + 1)); \
	fi; \
	VERSION="$$MAJOR.$$MINOR.$$PATCH"; \
	echo "Using version $$VERSION (MAJOR=$$MAJOR MINOR=$$MINOR PATCH=$$PATCH)"; \
	echo "Updating library.json version to $$VERSION..."; \
	NEW_VERSION="$$VERSION" $(PYTHON) -c 'import json, pathlib, os; p = pathlib.Path("library.json"); d = json.loads(p.read_text()); d["version"] = os.environ["NEW_VERSION"]; p.write_text(json.dumps(d, indent=4, ensure_ascii=False) + "\n")'; \
	echo "Generating src/Version/IoTSmartSysCoreVersion.h..."; \
	mkdir -p src/Settings; \
	printf '%s\n' '#pragma once' '' \
	"#define IOTSMARTSYSCORE_VERSION           \"$$VERSION\"" \
	"#define IOTSMARTSYSCORE_VERSION_MAJOR     $$MAJOR" \
	"#define IOTSMARTSYSCORE_VERSION_MINOR     $$MINOR" \
	"#define IOTSMARTSYSCORE_VERSION_PATCH     $$PATCH" \
	> src/Version/IoTSmartSysCoreVersion.h; \
	git add library.json src/Version/IoTSmartSysCoreVersion.h; \
	git commit -m "Bump IoTSmartSysCore version to v$$VERSION"; \
	BRANCH=$$(git rev-parse --abbrev-ref HEAD); \
	echo "Pushing to $$BRANCH..."; \
	git push origin $$BRANCH; \
	echo "Creating tag v$$VERSION..."; \
	git tag -a "v$$VERSION" -m "Release IoTSmartSysCore v$$VERSION"; \
	git push origin "v$$VERSION"

utest:
	clear
	clear
	pio test -e esp32s3_test --filter test_temperature

erase:
# 	clear
# 	clear
	@PORTS="$$(pio device list --serial --json-output | python3 -c 'import json, sys; print("\n".join(device["port"] for device in json.load(sys.stdin)))')"; \
	if [ -z "$$PORTS" ]; then \
		echo "Nenhuma porta serial encontrada."; \
		exit 1; \
	fi; \
	echo "Portas seriais disponíveis:"; \
	printf '%s\n' "$$PORTS" | awk '{ printf "  %d) %s\n", NR, $$0 }'; \
	printf "Escolha a porta do ESP32: "; \
	read CHOICE; \
	case "$$CHOICE" in \
		''|*[!0-9]*) echo "Opção inválida."; exit 1 ;; \
	esac; \
	PORT="$$(printf '%s\n' "$$PORTS" | sed -n "$${CHOICE}p")"; \
	if [ -z "$$PORT" ]; then \
		echo "Opção inválida."; \
		exit 1; \
	fi; \
	echo "Apagando a flash do ESP32 usando o ambiente padrão do PlatformIO em $$PORT..."; \
	for ATTEMPT in 1 2 3; do \
		echo "Tentativa $$ATTEMPT de 3..."; \
		if pio run -t erase --upload-port "$$PORT"; then \
			exit 0; \
		fi; \
		if [ "$$ATTEMPT" -lt 3 ]; then \
			echo "Falha ao apagar. Tentando novamente..."; \
			sleep 1; \
		fi; \
	done; \
	echo "Não foi possível apagar a flash após 3 tentativas."; \
	exit 1
