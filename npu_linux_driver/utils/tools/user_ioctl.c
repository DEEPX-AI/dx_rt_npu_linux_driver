#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <byteswap.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

/* Use 'x' as magic number */
#define DX_DMA_IOC_MAGIC	'x'
/* XL OpenCL X->58(ASCII), L->6C(ASCII), O->0 C->C L->6C(ASCII); */
#define DX_DMA_XCL_MAGIC 0X586C0C6C

enum DX_DMA_IOC_TYPES {
	DX_DMA_IOC_NOP,
	DX_DMA_IOC_INFO,
	DX_DMA_IOC_OFFLINE,
	DX_DMA_IOC_ONLINE,
	DX_DMA_IOC_MAX
};

struct dx_dma_ioc_base {
	unsigned int magic;
	unsigned int command;
};

struct dx_dma_ioc_info {
	struct dx_dma_ioc_base	base;
	unsigned short		vendor;
	unsigned short		device;
	unsigned short		subsystem_vendor;
	unsigned short		subsystem_device;
	unsigned int		is_interrupt;
	unsigned int		dma_engine_version;
	unsigned int		driver_version;
	unsigned long long	feature_id;
	unsigned short		domain;
	unsigned char		bus;
	unsigned char		dev;
	unsigned char		func;
};

/* IOCTL codes */
#define DX_DMA_IOCINFO		_IOWR(DX_DMA_IOC_MAGIC, DX_DMA_IOC_INFO, struct dx_dma_ioc_info)

struct dx_dma_ioc_info dx_dma_info;

#define DEVICE_NAME_DEFAULT "/dev/dx_dma0_npu_0"
#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)

inline static void print_version()
{
	printf("==========================================================\n");
	printf("================     [ Version Info ]     ================\n");
	printf("==========================================================\n");
	printf("vendor             : 0x%x\n", dx_dma_info.vendor);
	printf("device             : 0x%x\n", dx_dma_info.device);
	printf("subsystem_vendor   : 0x%x\n", dx_dma_info.subsystem_vendor);
	printf("subsystem_device   : 0x%x\n", dx_dma_info.subsystem_device);
	printf("dma_engine_version : 0x%x\n", dx_dma_info.dma_engine_version);
	printf("driver_version     : %d\n",   dx_dma_info.driver_version);
	printf("feature_id         : 0x%x\n", dx_dma_info.feature_id);
	printf("domain             : 0x%x\n", dx_dma_info.domain);
	printf("bus                : 0x%x\n", dx_dma_info.bus);
	printf("dev                : 0x%x\n", dx_dma_info.dev);
	printf("func               : 0x%x\n", dx_dma_info.func);
	printf("Interrupt          : %d\n",   dx_dma_info.is_interrupt);
	printf("==========================================================\n");
}

int main(int argc, char **argv)
{
	int fd;
	char *device;
	int rc = 0;

	/* not enough arguments given? */
	if (argc < 2) {
		device = DEVICE_NAME_DEFAULT;
		fprintf(stderr,
			" -> device  : character device to access(%s)\n", device);
	} else {
		device = strdup(argv[1]);
	}
	printf("argc = %d\n", argc);

	if ((fd = open(device, O_RDWR | O_SYNC)) == -1)
		FATAL;
	printf("character device %s opened.\n", device);
	fflush(stdout);

	dx_dma_info.base.magic = DX_DMA_XCL_MAGIC;
	rc = ioctl(fd, DX_DMA_IOCINFO, &dx_dma_info);
	if (rc == 0) {
		printf("DX_DMA_IOCINFO succesful.\n");
		print_version();
	} else {
		printf("ioctl(..., DX_DMA_IOCINFO) = %d\n", rc);
	}

	close(fd);
	return 0;
}
