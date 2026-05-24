# ─── Compiler & Flags ────────────────────────────────────
CC      = aarch64-linux-gnu-gcc
LD      = aarch64-linux-gnu-ld
CFLAGS  = -ffreestanding -nostdlib -nostartfiles -mcpu=cortex-a57 \
          -mgeneral-regs-only -Wall -Wextra -g \
          -Isrc/kernel -Isrc/mm -Isrc/fs -Isrc/drivers -Isrc/lib

# ─── Source & Build Directories ─────────────────────────
SRCDIR  = src
BUILDDIR = build

# ─── Automatically find all C and assembly sources ──────
C_SRCS   = $(wildcard $(SRCDIR)/*/*.c)
ASM_SRCS = $(wildcard $(SRCDIR)/*/*.S)

# ─── Transform source paths into object paths inside build/
#     e.g., src/kernel/kernel.c → build/src/kernel/kernel.o
C_OBJS   = $(C_SRCS:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
ASM_OBJS = $(ASM_SRCS:$(SRCDIR)/%.S=$(BUILDDIR)/%.o)
OBJS     = $(C_OBJS) $(ASM_OBJS)

# ─── Targets ─────────────────────────────────────────────
all: kernel.elf

# Pattern rule for C files
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)          # ensures the directory exists
	$(CC) $(CFLAGS) -c $< -o $@

# Pattern rule for assembly files
$(BUILDDIR)/%.o: $(SRCDIR)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

kernel.elf: $(OBJS)
	$(LD) -T linker.ld $(OBJS) -o $@

run: kernel.elf
	qemu-system-aarch64 -machine virt -cpu cortex-a57 -m 256M -kernel kernel.elf -nographic

debug: kernel.elf
	qemu-system-aarch64 -machine virt -cpu cortex-a57 -m 256M -kernel kernel.elf -nographic -s -S

clean:
	rm -rf $(BUILDDIR) kernel.elf

compile_commands.json: clean
	bear -- make
.PHONY: all run debug clean
