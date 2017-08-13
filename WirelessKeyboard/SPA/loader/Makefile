ifeq ($(strip $(DEVKITARM)),)
$(error "请将DEVKITARM添加到环境变量")
endif

PREFIX = $(DEVKITARM)/bin/arm-none-eabi-

CC = $(PREFIX)gcc
LD = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy

# define options for linkage. Prevent the inclusion of standard start
# code and libraries.
LDFLAGS = -nostartfiles -Ttext=0x20006A8

# use variables to refer to init code in case it changes
OBJS = loader.o

#
# define build targets
#
all: loader.bin

clean:
	rm -f *.o *.elf *.bin *.dump


# build s-record with init code and c files linked together
loader.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o loader.elf $(OBJS)
	$(OBJCOPY) -O binary loader.elf loader.bin

# handle compilation of C files
%.o:%.S
	$(CC) -D__ASSEMBLY__ -c $< -o $@

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@


