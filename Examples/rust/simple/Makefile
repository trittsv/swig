TOP        = ../..
SWIG       = $(TOP)/../preinst-swig
SRCS       = example.c
TARGET     = example
INTERFACE  = example.i
RUNME      = runme

all::
	$(MAKE) -f $(TOP)/Makefile rust

check: all
	$(MAKE) -f $(TOP)/Makefile rust_compile
	$(MAKE) -f $(TOP)/Makefile rust_run

clean:
	$(MAKE) -f $(TOP)/Makefile rust_clean
