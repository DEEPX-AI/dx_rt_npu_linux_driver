/*
 * This file is part of the Xilinx DMA IP Core driver tool for Linux
 *
 * Copyright (c) 2016-present,  Xilinx, Inc.
 * All rights reserved.
 *
 * This source code is licensed under BSD-style license (found in the
 * LICENSE file in the root directory of this source tree)
 */

#define _BSD_SOURCE
#define _XOPEN_SOURCE 500
#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "dma_utils.c"
#include "dx_util.c"

// #define DEVICE_NAME_DEFAULT "/dev/xdma0_c2h_0"
#define DEVICE_NAME_DEFAULT "/dev/dx_dma0_c2h_0"
#define SIZE_DEFAULT (32)
#define COUNT_DEFAULT (1)

static struct option const long_opts[] = {
	{"device", required_argument, NULL, 'd'},
	{"address", required_argument, NULL, 'a'},
	{"aperture", required_argument, NULL, 'k'},
	{"size", required_argument, NULL, 's'},
	{"offset", required_argument, NULL, 'o'},
	{"count", required_argument, NULL, 'c'},
	{"file", required_argument, NULL, 'f'},
	{"help", no_argument, NULL, 'h'},
	{"verbose", required_argument, NULL, 'v'},
	{"print_buf", no_argument, NULL, 'p'},
	{0, 0, 0, 0}
};

static int test_dma(char *devname, uint64_t addr, uint64_t aperture, 
					uint64_t size, uint64_t offset, uint64_t count,
					char *ofname);
static int no_write = 0;

static void usage(const char *name)
{
	int i = 0;
	fprintf(stdout, "%s\n\n", name);
	fprintf(stdout, "usage: %s [OPTIONS]\n\n", name);
	fprintf(stdout, "Read via SGDMA, optionally save output to a file\n\n");

	fprintf(stdout, "  -%c (--%s) device (defaults to %s)\n",
		long_opts[i].val, long_opts[i].name, DEVICE_NAME_DEFAULT);
	i++;
	fprintf(stdout, "  -%c (--%s) the start address on the AXI bus\n",
	       long_opts[i].val, long_opts[i].name);
	i++;
	fprintf(stdout, "  -%c (--%s) memory address aperture\n",
	       long_opts[i].val, long_opts[i].name);
	i++;
	fprintf(stdout,
		"  -%c (--%s) size of a single transfer in bytes, default %d.\n",
		long_opts[i].val, long_opts[i].name, SIZE_DEFAULT);
	i++;
	fprintf(stdout, "  -%c (--%s) page offset of transfer\n",
		long_opts[i].val, long_opts[i].name);
	i++;
	fprintf(stdout, "  -%c (--%s) number of transfers, default is %d.\n",
	       long_opts[i].val, long_opts[i].name, COUNT_DEFAULT);
	i++;
	fprintf(stdout,
		"  -%c (--%s) file to write the data of the transfers\n",
		long_opts[i].val, long_opts[i].name);
	i++;
	fprintf(stdout, "  -%c (--%s) print usage help and exit\n",
		long_opts[i].val, long_opts[i].name);
	i++;
	fprintf(stdout, "  -%c (--%s) verbose output(1:max, 2:all)\n",
		long_opts[i].val, long_opts[i].name);
	i++;
}

int main(int argc, char *argv[])
{
	int cmd_opt;
	char *device = DEVICE_NAME_DEFAULT;
	uint64_t address = 0;
	uint64_t aperture = 0;
	uint64_t size = SIZE_DEFAULT;
	uint64_t offset = 0;
	uint64_t count = COUNT_DEFAULT;
	char *ofname = NULL;

	while ((cmd_opt = getopt_long(argc, argv, "v:phxc:f:d:a:k:s:o:", long_opts,
			    NULL)) != -1) {
		switch (cmd_opt) {
		case 0:
			/* long option */
			break;
		case 'd':
			/* device node name */
			device = strdup(optarg);
			break;
		case 'a':
			/* RAM address on the AXI bus in bytes */
			address = getopt_integer(optarg);
			break;
		case 'k':
			/* memory aperture windows size */
			aperture = getopt_integer(optarg);
			break;
		case 's':
			/* RAM size in bytes */
			size = getopt_integer(optarg);
			break;
		case 'o':
			offset = getopt_integer(optarg) & 4095;
			break;
			/* count */
		case 'c':
			count = getopt_integer(optarg);
			break;
			/* count */
		case 'f':
			ofname = strdup(optarg);
			break;
			/* print usage help and exit */
    		case 'x':
			no_write++;
			break;
		case 'v':
			verbose = getopt_integer(optarg);
			if (verbose < 1 || verbose > 2) {
				usage(argv[0]);
				exit(0);
			}
			break;
		case 'p':
			print_buf = 1;
			break;
		case 'h':
		default:
			usage(argv[0]);
			exit(0);
			break;
		}
	}
	if (verbose)
#if 0
	fprintf(stdout,
		"dev %s, addr 0x%lx, aperture 0x%lx, size 0x%lx, offset 0x%lx, "
		"count %lu\n",
		device, address, aperture, size, offset, count);
#else
	fprintf(stdout,
		"dma-read : %s, 0x%lx, 0x%lx (%s)\n", ofname, address, size, device);
#endif

	return test_dma(device, address, aperture, size, offset, count, ofname);
}

static int test_dma(char *devname, uint64_t addr, uint64_t aperture,
					uint64_t size, uint64_t offset, uint64_t count,
					char *ofname)
{
	ssize_t rc;
	uint64_t i;
	uint64_t apt_loop = aperture ? (size + aperture - 1) / aperture : 0;
	char *buffer = NULL;
	char *allocated = NULL;
	struct timespec ts_start, ts_end;
	int out_fd = -1;
	int fpga_fd = open(devname, O_RDWR | O_NONBLOCK);
	long total_time = 0, max_time = 0;
	float result;
	float avg_time = 0;
	int cpu_n = get_cpu_num();

	if (fpga_fd < 0) {
                fprintf(stderr, "unable to open device %s, %d.\n",
                        devname, fpga_fd);
		perror("open device");
                return -EINVAL;
        }

	/* create file to write data to */
	if (ofname) {
		out_fd = open(ofname, O_RDWR | O_CREAT | O_TRUNC | O_SYNC,
				0666);
		if (out_fd < 0) {
                        fprintf(stderr, "unable to open output file %s, %d.\n",
                                ofname, out_fd);
			perror("open output file");
                        rc = -EINVAL;
                        goto out;
                }
	}

	posix_memalign((void **)&allocated, 4096 /*alignment */ , size + 4096);
	if (!allocated) {
		fprintf(stderr, "OOM %lu.\n", size + 4096);
		rc = -ENOMEM;
		goto out;
	}

	buffer = allocated + offset;
	if (verbose)
	fprintf(stdout, "host buffer 0x%lx, %p.\n", size + 4096, buffer);

	for (i = 0; i < count; i++) {
		uint64_t freq = get_cpu_frequency();
		rc = clock_gettime(CLOCK_MONOTONIC, &ts_start);
		if (apt_loop) {
			uint64_t j;
			uint64_t len = size;
			char *buf = buffer;

			for (j = 0; j < apt_loop; j++, len -= aperture, buf += aperture) {
				/* lseek & read data from AXI MM into buffer using SGDMA */
				rc = read_to_buffer(devname, fpga_fd, buf,
									(len > aperture) ? aperture : len,
									addr);
				if (rc < 0)
					goto out;
			}
		} else {
			/* lseek & read data from AXI MM into buffer using SGDMA */
			rc = read_to_buffer(devname, fpga_fd, buffer, size, addr);

			if (rc < 0)
				goto out;
		}
		clock_gettime(CLOCK_MONOTONIC, &ts_end);

		/*print buffer data*/
		if (print_buf) {
			// printf("[caution: APB area of npu is only supported 4bytes when you access.                          ]\n");
			// printf("[         If you read these reg with over size 4bytes then you must ignore over 4bytes datas.]\n");
			for (int i = 0; i < size/4; i++) {
				// printf("Addr(0x%lX) Data:0x%08X\n", addr + i*4, ((uint32_t *)buffer)[i]);
				printf("0x%08lX : 0x%08X\n", addr + i*4, ((uint32_t *)buffer)[i]);
			}
		}

		/* subtract the start time from the end time */
		timespec_sub(&ts_end, &ts_start);
		total_time += ts_end.tv_nsec;
		if (i > 0) {
			if (max_time < ts_end.tv_nsec) {
				max_time = ts_end.tv_nsec;
				/* a bit less accurate but side-effects are accounted for */
				if (verbose == 1) {
					fprintf(stdout,
						"#%lu: CLOCK_MONOTONIC %ld.%09ld sec. read %ld bytes, CPU%d Freq:%ld\n",
						i, ts_end.tv_sec, ts_end.tv_nsec, size, cpu_n, freq);
				}
			}
		}
		/* a bit less accurate but side-effects are accounted for */
		if (verbose > 1) {
			fprintf(stdout,
				"#%lu: CLOCK_MONOTONIC %ld.%09ld sec. read %ld bytes, CPU%d Freq:%ld\n",
				i, ts_end.tv_sec, ts_end.tv_nsec, size, cpu_n, freq);
		}
		/* file argument given? */
		if ((out_fd >= 0) & (no_write == 0)) {
			rc = write_from_buffer(ofname, out_fd, buffer,
					 size, i*size);
			if (rc < 0)
				goto out;
		}
	}
	avg_time = (float)total_time/(float)count;
	result = ((float)size)*1000/avg_time;
	if (verbose)
	{
		printf("** [Read] Avg time device %s, total time %ld nsec, avg_time = %f, max_time = %ld, size = %lu, BW = %f \n",
			devname, total_time, avg_time, max_time, size, result);
		printf("** Average BW = %lu, %f\n", size, result);
	}
	rc = 0;

out:
	close(fpga_fd);
	if (out_fd >= 0)
		close(out_fd);
	free(allocated);

	return rc;
}
