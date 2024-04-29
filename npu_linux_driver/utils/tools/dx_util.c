/*
 * This file is part of the Xilinx DMA IP Core driver tools for Linux
 *
 * Copyright (c) 2016-present,  Xilinx, Inc.
 * All rights reserved.
 *
 * This source code is licensed under BSD-style license (found in the
 * LICENSE file in the root directory of this source tree)
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sched.h>

#define FILE_BUF_MAX	100
#define PCI_LOCK		1
#define PCI_UNLOCK		0

char lock_file[FILE_BUF_MAX] = "/usr/local/etc/pci_lock";
char lock_owner[FILE_BUF_MAX] = "/usr/local/etc/pci_lock";

void dx_pci_lock(void);
void dx_pci_unlock(void);
int get_cpu_num(void);
uint64_t get_cpu_frequency(void);

// Check PCI lock 
void dx_pci_lock(void)
{
	int lock;
	FILE *fp = fopen(lock_file, "w+");
	FILE *fp2 = fopen(lock_owner, "w+");

	/*read lock*/
	fscanf(fp, "%d", &lock);
	printf("lock value : %d\n", lock);

	char owner[10] = {0,};
	while(1) {
		if(lock == PCI_UNLOCK) {
			fprintf(fp, "%d", PCI_LOCK);
			break;
		} else if (lock == PCI_LOCK) {
			printf("wait until the lock is relased. (owner:%s)\n", owner);
			sleep(1);
		} else {
			printf("undefined state : lock-%d\n", lock);
			break;
		}
	}
	fclose(fp);
}

void dx_pci_unlock(void)
{
	FILE *fp = fopen(lock_file, "w+");
	fprintf(fp, "%d", PCI_UNLOCK);
	fclose(fp);
}

int get_cpu_num(void)
{
	int cpu = sched_getcpu();
	return cpu;
}

uint64_t get_cpu_frequency(void)
{
	int cpu = sched_getcpu();
	uint64_t freq;
	{
		char freq_file_path[1024];
		snprintf(freq_file_path, sizeof(freq_file_path), "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq", cpu);
		FILE* fp = fopen(freq_file_path, "r");
		if (fp == NULL) {
		    perror("Failed to open scaling_cur_freq file");
		    return -1;
		}
		fscanf(fp, "%llu", &freq);
		// printf("CPU%d Frequency : %llu kHz\n", cpu, freq / 1000);
		fclose(fp);
	}
	return freq / 1000;
}

int change_scheduler() {
    struct sched_param param;
    int result;
    pid_t pid = getpid();

    param.sched_priority = 1;  // 우선순위 설정

    result = sched_setscheduler(pid, SCHED_FIFO, &param);
    if (result == -1) {
        perror("sched_setscheduler failed");
        exit(EXIT_FAILURE);
    }
    printf("SCHED_FIFO policy applied successfully(pid:%d).\n", pid);

    return 0;
}