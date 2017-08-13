ifeq ($(strip $(DEVKITARM)),)
$(error "请将DEVKITARM添加到环境变量")
endif

PREFIX = $(DEVKITARM)/bin/arm-none-eabi-

CC = $(PREFIX)gcc
LD = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump

INCLUDE = $(DEVKITPRO)/libnds/include

LIBS	=	../../lib
SOURCE	=	../../JPN/arm9/source

# define options for compilation
CFLAGS = -std=c99 -Wall -Os -march=armv5te -mtune=arm946e-s -D=SPA -D=ARM9 -mthumb-interwork -fomit-frame-pointer -ffast-math -I $(INCLUDE)


# define options for linkage. Prevent the inclusion of standard start
# code and libraries.
LDFLAGS = -T $(CURDIR)/symbol.sym -T ../../JPN/arm9/linker.ld -nostartfiles -Wl,--gc-sections,--use-blx


# define options for the objdump
DUMPFLAGS = -xdsS

# use variables to refer to init code in case it changes
OBJS = $(SOURCE)/arm9.o entry.o $(SOURCE)/arm7loader.o $(SOURCE)/common.o $(SOURCE)/backup.o $(SOURCE)/server.o


#
# define build targets
#
all: 
	touch $(SOURCE)/common.c $(SOURCE)/arm9.c
	make arm9.bin

clean:
	rm -f *.o *.elf *.bin *.dump $(SOURCE)/*.o


# build s-record with init code and c files linked together
arm9.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o arm9.elf $(OBJS) -L$(LIBS) -ldswifi9
	$(OBJCOPY) -O binary arm9.elf arm9.bin

# handle compilation of C files
$(SOURCE)/server.o:$(SOURCE)/server.c
	$(CC) $(CFLAGS) -mthumb -c $< -o $@

$(SOURCE)/backup.o:$(SOURCE)/backup.c
	$(CC) $(CFLAGS) -mthumb -c $< -o $@

%.o:%.S
	$(CC) -D__ASSEMBLY__ $(CFLAGS) -c $< -o $@

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@


