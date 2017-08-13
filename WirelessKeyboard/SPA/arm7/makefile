ifeq ($(strip $(DEVKITARM)),)
$(error "请将DEVKITARM添加到环境变量")
endif

PREFIX = $(DEVKITARM)/bin/arm-none-eabi-

CC = $(PREFIX)gcc
LD = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump

INCLUDE = $(DEVKITPRO)/libnds/include

SOURCE	:=	../../JPN/arm7/source
LIBS	:=	-L../../lib -ldswifi7

# define options for compilation
CFLAGS = -I $(SOURCE) -I $(INCLUDE) -std=c99 -Wall -O2 -mtune=arm7tdmi -fomit-frame-pointer -ffast-math

# define options for linkage. Prevent the inclusion of standard start
# code and libraries.
LDFLAGS = -T $(CURDIR)/symbol.sym -T ../../JPN/arm7/linker.ld -nostartfiles


# define options for the objdump
DUMPFLAGS = -xdsS

# use variables to refer to init code in case it changes
OBJS = $(SOURCE)/arm7.o $(SOURCE)/entry.o $(SOURCE)/hook.o

#
# define build targets
#
all: arm7.bin

clean:
	rm -f *.o *.elf *.bin *.dump $(SOURCE)/*.o


# build s-record with init code and c files linked together
arm7.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o arm7.elf $(OBJS) $(LIBS)
	$(OBJCOPY) -O binary arm7.elf arm7.bin

# handle compilation of C files
%.o:%.S
	$(CC) -D__ASSEMBLY__ $(CFLAGS) -c $< -o $@

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@


