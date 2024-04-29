CC ?= gcc
CFLAGS := -g

all: reg_rw dma_to_device dma_from_device poll user_ioctl

dma_to_device: dma_to_device.o
	$(CC) -lrt -o $@ $< -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D_LARGE_FILE_SOURCE

dma_from_device: dma_from_device.o
	$(CC) -lrt -o $@ $< -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D_LARGE_FILE_SOURCE

reg_rw: reg_rw.o
	$(CC) -o $@ $<

user_ioctl: user_ioctl.o
	$(CC) -o $@ $< -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D_LARGE_FILE_SOURCE

%.o: %.c
	$(CC) $(CFLAGS) -c -std=c99 -o $@ $< -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D_LARGE_FILE_SOURCE

clean:
	rm -rf reg_rw *.o *.bin dma_to_device dma_from_device poll user_ioctl

