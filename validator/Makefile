# XXX  this Makefile is broken
# TODO produce standalone ./validator.py script
# 	   into criu root dir
# TODO integrate with main CRIU Makefile
MAKE = make
RM = rm -f
DESTDIR = ../

all: validate_tests validator

validate_tests:
	$(MAKE) -C ./validate_tests/gen_tree/ 
	mv ./validate_tests $(DESTDIR)

validator: 
	mv ./validator $(DESTDIR)
	mv ./validator.py $(DESTDIR)
	
.PHONY: all validate_tests validator
