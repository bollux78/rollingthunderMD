PYTHON ?= python3
JOBS ?= 4
.PHONY: build audit clean
build:
	$(PYTHON) tools/build.py --rom "$(ROM)" --sgdk "$(GDK)" --jobs $(JOBS)
audit:
	$(PYTHON) tools/audit_repo.py
clean:
	$(PYTHON) -c 'import shutil; shutil.rmtree(".build", ignore_errors=True); shutil.rmtree("dist", ignore_errors=True)'
