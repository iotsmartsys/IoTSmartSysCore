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
