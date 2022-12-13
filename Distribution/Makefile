SHELL = /bin/bash

bindist-mac:
	-@echo "Making bindist for Mac..."
	$(SHELL) makedistr-mac.sh

deployqt:
	-@echo "Making a bundle with QT embedded"
	$(SHELL) make-standalone-bundle.sh

bindist-linux:
	-@echo "Making bindist for Linux..."
	$(SHELL) makedistr-linux.sh

clean:
	-@/bin/rm *~
