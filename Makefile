# 如果是生产环境请把DEBUG设为0
DEBUG ?= 1

CC := gcc
AR := ar
RANLIB := ranlib

ifeq ($(DEBUG), 1)
    CFLAGS := -g -Wall -std=gnu99 -DDEBUG
else
    CFLAGS := -O2 -std=gnu99 -DNDEBUG
endif

EXE_FILES := ywebbench

OBJS := conf.o url.o main.o util.o
SOURCES := $(OBJS:.o=.c)

all:  $(EXE_FILES)

ywebbench: $(OBJS)
	$(CC) -o $@ $^  -lm
%.o:%.c
	@echo "$(CC) -c $(CFLAGS) -o $@ $<"
	@$(CC) -fPIC -shared -c $(CFLAGS) $(CPPFLAGS) $< -o $@ $(OPENCL_INC_PATH) $(INC_PATH)

#自动处理头文件依赖
#SOURCES为所有的源文件列表
%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
# ignore the warn message "XXX.d: No such file or directory"
-include $(SOURCES:.c=.d)

install: all
	@if [ -d $(INSTDIR) ]; \
	then \
	cp myapp $(INSTDIR);\
	chmod a+x $(INSTDIR)/$(EXE_FILE);\
	chmod og-w $(INSTDIR)/$(EXE_FILE);\
	echo “Installed in $(INSTDIR)“;\
	else \
	echo “Sorry, $(INSTDIR) does not exist”;\
	fi

.PHONY: clean
clean:
	-rm -f *.o $(EXE_FILES) *.d *.d.* *.bin *.out new
