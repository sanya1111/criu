MAKE = make
RM = rm -f
SRCDIR = ../

all: validate_tests validator

validate_tests:
	$(MAKE) -C ./validate_tests/gen_tree/ 
	mv ./validate_tests $(SRCDIR)

validator: 
	mv ./validator $(SRCDIR)
	mv ./validator.py $(SRCDIR)
	
.PHONY: all validate_tests validator
