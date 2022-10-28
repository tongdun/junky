#makefile for junky

PLUGIN_SRC=junky.cxx
PLUGIN=junky.so
EXTRA_ARGS=-fplugin-arg-junky-junkpfx=main,fn_test -fplugin-arg-junky-junknum=100 
CC=g++

CCPLUGINS_DIR = $(shell $(CC) -print-file-name=plugin)
CFLAGS += -I$(CCPLUGINS_DIR)/include -fPIC -g3 -O0 \
          -Wall $(EXTRA_CFLAGS) -fno-rtti

$(PLUGIN): $(PLUGIN_SRC)
	$(CC) $(CFLAGS) -g -shared -o $@ $(PLUGIN_SRC)

test: clean $(PLUGIN) test.c
	gcc test.c -o $@ -fplugin=./$(PLUGIN) -g3 -O0 $(EXTRA_ARGS)

.PHONY: clean
clean:
	rm -fv $(PLUGIN) *.o test
