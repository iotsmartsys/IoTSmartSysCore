MAJOR ?= 0
MINOR ?= 2

tag:
	@MAJOR="$(MAJOR)"; MINOR="$(MINOR)"; PATCH="$(PATCH)"; \
	if [ -z "$$PATCH" ]; then \
		echo "PATCH not provided; auto-incrementing based on library.json..."; \
		OLD_VERSION=$$(python -c 'import json, pathlib; p = pathlib.Path("library.json"); d = json.loads(p.read_text()); print(d.get("version", "0.0.0"))'); \
		OLD_MAJOR=$$(echo "$$OLD_VERSION" | cut -d. -f1); \
		OLD_MINOR=$$(echo "$$OLD_VERSION" | cut -d. -f2); \
		OLD_PATCH=$$(echo "$$OLD_VERSION" | cut -d. -f3); \
		MAJOR="$$OLD_MAJOR"; \
		MINOR="$$OLD_MINOR"; \
		PATCH=$$((OLD_PATCH + 1)); \
	fi; \
	VERSION="$$MAJOR.$$MINOR.$$PATCH"; \
	echo "Using version $$VERSION (MAJOR=$$MAJOR MINOR=$$MINOR PATCH=$$PATCH)"; \
	echo "Updating library.json version to $$VERSION..."; \
	NEW_VERSION="$$VERSION" python -c 'import json, pathlib, os; p = pathlib.Path("library.json"); d = json.loads(p.read_text()); d["version"] = os.environ["NEW_VERSION"]; p.write_text(json.dumps(d, indent=4, ensure_ascii=False) + "\n")'; \
	echo "Generating src/Settings/IoTSmartSysCoreVersion.h..."; \
	mkdir -p src/Settings; \
	printf '%s\n' '#pragma once' '' \
	"#define IOTSMARTSYSCORE_VERSION           \"$$VERSION\"" \
	"#define IOTSMARTSYSCORE_VERSION_MAJOR     $$MAJOR" \
	"#define IOTSMARTSYSCORE_VERSION_MINOR     $$MINOR" \
	"#define IOTSMARTSYSCORE_VERSION_PATCH     $$PATCH" \
	> src/Settings/IoTSmartSysCoreVersion.h; \
	git add library.json src/Settings/IoTSmartSysCoreVersion.h; \
	git commit -m "Bump IoTSmartSysCore version to v$$VERSION"; \
	BRANCH=$$(git rev-parse --abbrev-ref HEAD); \
	echo "Pushing to $$BRANCH..."; \
	git push origin $$BRANCH; \
	echo "Creating tag v$$VERSION..."; \
	git tag -a "v$$VERSION" -m "Release IoTSmartSysCore v$$VERSION"; \
	git push origin "v$$VERSION"

test-u:
	clear
	clear
	pio test -vvv -e esp32_test