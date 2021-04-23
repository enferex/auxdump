CFLAGS  = -Wall -pedantic -std=gnu17
SRCS    = main.c
OBJS   := $(SRCS:.c=.o)
APP     = auxdump

all: debug

debug: CFLAGS += -g3 -O0
debug: $(APP)

release: CFLAGS += -g3 -O3
release: $(APP)

$(APP): $(OBJS)
	$(CC) $^ -o $@

clean:
	$(RM) $(OBJS) $(APP)
