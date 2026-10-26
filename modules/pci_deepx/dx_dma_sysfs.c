// SPDX-License-Identifier: GPL-2.0
/*
 * DeepX DMA driver — sysfs attributes
 *
 * Runtime perf-measurement control exposed under
 * /sys/bus/pci/drivers/dx_dma_pcie/perf_{enable,clear,data,stats}
 */

#include <linux/device.h>
#include <linux/pci.h>
#include <linux/mutex.h>
#include "dx_sysfs_compat.h"	//DEEPX MODIFIED: 4.4 sysfs_emit compat
#include "dx_dma_sysfs.h"
#include "dx_util.h"
#include "dx_link_health.h"

#define DX_OUTBOUND_MEM_MIN_SIZE	4096
#define DX_OUTBOUND_MEM_MAX_SIZE	(16 * 1024 * 1024)

static DEFINE_MUTEX(perf_stats_lock);

static struct dw_edma *dx_dma_sysfs_get_dw(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct dw_edma_chip *chip = pci_get_drvdata(pdev);

	if (!chip || !chip->dw)
		return NULL;

	return chip->dw;
}

static ssize_t outbound_mem_alloc_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct dw_edma *dw;
	struct pci_dev *pdev = to_pci_dev(dev);
	dma_addr_t dma_addr;
	u64 requested_size;
	size_t size;
	void *cpu_addr;

	if (kstrtoull(buf, 0, &requested_size))
		return -EINVAL;
	if (requested_size < DX_OUTBOUND_MEM_MIN_SIZE ||
	    requested_size > DX_OUTBOUND_MEM_MAX_SIZE ||
	    !IS_ALIGNED(requested_size, PAGE_SIZE))
		return -EINVAL;

	dw = dx_dma_sysfs_get_dw(dev);
	if (!dw)
		return -ENODEV;

	size = (size_t)requested_size;
	mutex_lock(&dw->outbound_mem_lock);
	if (dw->outbound_mem_cpu_addr) {
		mutex_unlock(&dw->outbound_mem_lock);
		return -EBUSY;
	}

	cpu_addr = dma_alloc_coherent(&pdev->dev, size, &dma_addr,
				     GFP_KERNEL | __GFP_ZERO);
	if (!cpu_addr) {
		mutex_unlock(&dw->outbound_mem_lock);
		return -ENOMEM;
	}

	dw->outbound_mem_cpu_addr = cpu_addr;
	dw->outbound_mem_dma_addr = dma_addr;
	dw->outbound_mem_size = size;
	mutex_unlock(&dw->outbound_mem_lock);

	return count;
}
static DEVICE_ATTR(outbound_mem_alloc, 0200, NULL, outbound_mem_alloc_store);

/* The endpoint outbound engine must be disabled and acknowledge idle first. */
static ssize_t outbound_mem_status_show(struct device *dev,
					struct device_attribute *attr, char *buf)
{
	struct dw_edma *dw;
	ssize_t ret;

	dw = dx_dma_sysfs_get_dw(dev);
	if (!dw)
		return -ENODEV;

	mutex_lock(&dw->outbound_mem_lock);
	if (!dw->outbound_mem_cpu_addr)
		ret = sysfs_emit(buf,
				 "state=unallocated\nsize_bytes=0\ndma_addr=none\n");
	else
		ret = sysfs_emit(buf,
				 "state=allocated\nsize_bytes=%zu\ndma_addr=0x%llx\n",
				 dw->outbound_mem_size,
				 (unsigned long long)dw->outbound_mem_dma_addr);
	mutex_unlock(&dw->outbound_mem_lock);

	return ret;
}
static DEVICE_ATTR(outbound_mem_status, 0400, outbound_mem_status_show, NULL);

static struct attribute *dx_dma_outbound_mem_attrs[] = {
	&dev_attr_outbound_mem_alloc.attr,
	&dev_attr_outbound_mem_status.attr,
	NULL,
};

static const struct attribute_group dx_dma_outbound_mem_attr_group = {
	.attrs = dx_dma_outbound_mem_attrs,
};

int dx_dma_sysfs_device_create(struct pci_dev *pdev)
{
	return sysfs_create_group(&pdev->dev.kobj,
				  &dx_dma_outbound_mem_attr_group);
}

void dx_dma_sysfs_device_remove(struct pci_dev *pdev)
{
	if (!pdev)
		return;

	sysfs_remove_group(&pdev->dev.kobj, &dx_dma_outbound_mem_attr_group);
}

void dx_dma_sysfs_device_release(struct pci_dev *pdev)
{
	struct dw_edma *dw;

	if (!pdev)
		return;

	dw = dx_dma_sysfs_get_dw(&pdev->dev);
	if (!dw)
		return;

	mutex_lock(&dw->outbound_mem_lock);
	if (dw->outbound_mem_cpu_addr) {
		dma_free_coherent(&pdev->dev, dw->outbound_mem_size,
				  dw->outbound_mem_cpu_addr,
				  dw->outbound_mem_dma_addr);
		dw->outbound_mem_cpu_addr = NULL;
		dw->outbound_mem_dma_addr = 0;
		dw->outbound_mem_size = 0;
	}
	mutex_unlock(&dw->outbound_mem_lock);
}

/* ---- perf_enable: toggle runtime perf measurement ---- */
static ssize_t perf_enable_show(struct device_driver *drv, char *buf)
{
	return sysfs_emit(buf, "%d\n", READ_ONCE(g_perf_enabled) ? 1 : 0);
}

static ssize_t perf_enable_store(struct device_driver *drv,
				 const char *buf, size_t count)
{
	bool val;

	if (kstrtobool(buf, &val))
		return -EINVAL;

	WRITE_ONCE(g_perf_enabled, val);
	if (val)
		pr_info("dx_dma: perf measurement enabled\n");
	else
		pr_info("dx_dma: perf measurement disabled\n");

	return count;
}
static DRIVER_ATTR_RW(perf_enable);

/* ---- perf_clear: write 1 to reset counters ----
 * Disable perf or quiesce DMA first if exact epoch reset semantics are needed;
 * live transfers may race with the best-effort atomic counter reset.
 */
static ssize_t perf_clear_store(struct device_driver *drv,
				const char *buf, size_t count)
{
	int val;

	if (kstrtoint(buf, 0, &val) || val != 1)
		return -EINVAL;

	clear_pcie_profile_info(0, 0, 0, 0, 0);
	pr_info("dx_dma: perf data cleared\n");
	return count;
}
static DRIVER_ATTR_WO(perf_clear);

/* ---- perf_data: read current perf snapshot ---- */
static ssize_t perf_data_show(struct device_driver *drv, char *buf)
{
	char *data;

	if (!READ_ONCE(g_perf_enabled))
		return sysfs_emit(buf, "perf disabled\n");

	data = show_pcie_profile();
	if (!data)
		return sysfs_emit(buf, "no data\n");

	return sysfs_emit(buf, "%s", data);
}
static DRIVER_ATTR_RO(perf_data);

/* ---- perf_stats: low-overhead internal counters for 06_multi analysis ----
 * Unlike perf_data, keep this readable while perf is disabled so users can
 * disable collection first and then read the last captured snapshot.
 */
static ssize_t perf_stats_show(struct device_driver *drv, char *buf)
{
	char *data;
	ssize_t ret;

	mutex_lock(&perf_stats_lock);
	data = show_pcie_internal_stats();
	if (!data)
		ret = sysfs_emit(buf, "no stats\n");
	else
		ret = sysfs_emit(buf, "%s", data);
	mutex_unlock(&perf_stats_lock);

	return ret;
}
static DRIVER_ATTR_RO(perf_stats);

/* ---- perf_concurrency: DMA-transfer latency bucketed by in-flight count ----
 * Reveals whether min/max latency fluctuation in perf_data is caused by
 * multi-channel DMA overlap (contention) or by an unrelated cause. Kept
 * readable while perf is disabled so users can stop collection then read.
 */
static ssize_t perf_concurrency_show(struct device_driver *drv, char *buf)
{
	char *data;
	ssize_t ret;

	mutex_lock(&perf_stats_lock);
	data = show_pcie_concurrency();
	if (!data)
		ret = sysfs_emit(buf, "no data\n");
	else
		ret = sysfs_emit(buf, "%s", data);
	mutex_unlock(&perf_stats_lock);

	return ret;
}
static DRIVER_ATTR_RO(perf_concurrency);

/* ---- perf_outliers / perf_outlier_us: capture slow-sample context ----
 * perf_outlier_us sets the per-stage threshold (microseconds); any stage
 * sample slower than it is logged with CPU / concurrency / timestamp so the
 * cause of large min/max spread can be attributed. Readable while disabled.
 */
static ssize_t perf_outliers_show(struct device_driver *drv, char *buf)
{
	char *data;
	ssize_t ret;

	mutex_lock(&perf_stats_lock);
	data = show_pcie_outliers();
	if (!data)
		ret = sysfs_emit(buf, "no data\n");
	else
		ret = sysfs_emit(buf, "%s", data);
	mutex_unlock(&perf_stats_lock);

	return ret;
}
static DRIVER_ATTR_RO(perf_outliers);

static ssize_t perf_outlier_us_show(struct device_driver *drv, char *buf)
{
	return sysfs_emit(buf, "%u\n", dx_pcie_outlier_thresh_us());
}

static ssize_t perf_outlier_us_store(struct device_driver *drv,
				     const char *buf, size_t count)
{
	unsigned int val;

	if (kstrtouint(buf, 0, &val))
		return -EINVAL;

	dx_pcie_outlier_set_thresh_us(val);
	return count;
}
static DRIVER_ATTR_RW(perf_outlier_us);

/* ---- perf_floor_us: capture too-fast/suspect samples (0 disables) ----
 * Any stage sample faster than this floor is logged (marked FAST) with its
 * size and implied MB/s, so measurement artifacts that corrupt min / BW-max
 * (e.g. a DMA-transfer window far below the PCIe line rate) become visible.
 */
static ssize_t perf_floor_us_show(struct device_driver *drv, char *buf)
{
	return sysfs_emit(buf, "%u\n", dx_pcie_outlier_floor_us());
}

static ssize_t perf_floor_us_store(struct device_driver *drv,
				   const char *buf, size_t count)
{
	unsigned int val;

	if (kstrtouint(buf, 0, &val))
		return -EINVAL;

	dx_pcie_outlier_set_floor_us(val);
	return count;
}
static DRIVER_ATTR_RW(perf_floor_us);

/* ---- perf_size_reset: clear a channel's stats when transfer size changes ----
 * Default (1): avoids mixing timings from different sizes into one avg/min/max,
 * which otherwise makes the Bandwidth row in perf_data meaningless. Set to 0
 * for mixed-size traffic where a running average across sizes is preferred.
 */
static ssize_t perf_size_reset_show(struct device_driver *drv, char *buf)
{
	return sysfs_emit(buf, "%d\n", dx_pcie_size_reset_enabled() ? 1 : 0);
}

static ssize_t perf_size_reset_store(struct device_driver *drv,
				     const char *buf, size_t count)
{
	bool val;

	if (kstrtobool(buf, &val))
		return -EINVAL;

	dx_pcie_set_size_reset_enabled(val);
	return count;
}
static DRIVER_ATTR_RW(perf_size_reset);

/* ---- perf_top: 10 slowest transfers with per-stage breakdown ----
 * Shows WHERE each slow transfer spent its time (dominant stage), so a large
 * ioctl total latency can be attributed at a glance. Readable while disabled.
 */
static ssize_t perf_top_show(struct device_driver *drv, char *buf)
{
	char *data;
	ssize_t ret;

	mutex_lock(&perf_stats_lock);
	data = show_pcie_top();
	if (!data)
		ret = sysfs_emit(buf, "no data\n");
	else
		ret = sysfs_emit(buf, "%s", data);
	mutex_unlock(&perf_stats_lock);

	return ret;
}
static DRIVER_ATTR_RO(perf_top);

struct link_health_summary {
	int devices;
	int enabled;
};

static int link_health_count_device(struct device *dev, void *data)
{
	struct link_health_summary *summary = data;
	struct pci_dev *pdev = to_pci_dev(dev);
	struct dw_edma_chip *chip = pci_get_drvdata(pdev);

	if (!chip || !chip->dw)
		return 0;

	summary->devices++;
	if (dx_link_health_is_enabled(chip->dw))
		summary->enabled++;

	return 0;
}

static int link_health_apply_device(struct device *dev, void *data)
{
	bool enable = *(bool *)data;
	struct pci_dev *pdev = to_pci_dev(dev);
	struct dw_edma_chip *chip = pci_get_drvdata(pdev);

	if (!chip || !chip->dw)
		return 0;

	dx_link_health_set_enabled(chip->dw, enable);
	return 0;
}

/* ---- link_health_enable: toggle link-health worker + auto-recovery ----
 * Driver-level switch for early deployments.  It updates the default for
 * future devices and all currently-bound devices.
 *
 *   1: periodic link-health polling and link-down auto-recovery enabled
 *   0: disabled; AER/sysfs-reset callbacks still run, but health worker
 *      will not poll, schedule recovery_work, or re-arm after reset.
 */
static ssize_t link_health_enable_show(struct device_driver *drv, char *buf)
{
	struct link_health_summary summary = {0};
	int ret;

	ret = driver_for_each_device(drv, NULL, &summary,
				     link_health_count_device);
	if (ret)
		return ret;
	if (!summary.devices)
		return sysfs_emit(buf, "%d\n",
				  dx_link_health_default_enabled() ? 1 : 0);

	return sysfs_emit(buf, "%d\n",
			  summary.enabled == summary.devices ? 1 : 0);
}

static ssize_t link_health_enable_store(struct device_driver *drv,
						const char *buf, size_t count)
{
	bool enable;
	int ret;

	if (kstrtobool(buf, &enable))
		return -EINVAL;

	dx_link_health_set_default_enabled(enable);
	ret = driver_for_each_device(drv, NULL, &enable,
				     link_health_apply_device);
	if (ret)
		return ret;
	pr_info("dx_dma: link health monitor %s\n",
		enable ? "enabled" : "disabled");

	return count;
}
static DRIVER_ATTR_RW(link_health_enable);

/* ---- create / remove helpers (called from module init/exit) ---- */
void dx_dma_sysfs_create(struct pci_driver *pdrv)
{
	int ret;

	ret = driver_create_file(&pdrv->driver, &driver_attr_perf_enable);
	if (ret)
		pr_warn("dx_dma: failed to create perf_enable sysfs (%d)\n", ret);
	ret = driver_create_file(&pdrv->driver, &driver_attr_perf_clear);
	if (ret)
		pr_warn("dx_dma: failed to create perf_clear sysfs (%d)\n", ret);
	ret = driver_create_file(&pdrv->driver, &driver_attr_perf_data);
	if (ret)
		pr_warn("dx_dma: failed to create perf_data sysfs (%d)\n", ret);
	ret = driver_create_file(&pdrv->driver, &driver_attr_perf_stats);
	if (ret)
		pr_warn("dx_dma: failed to create perf_stats sysfs (%d)\n", ret);
	ret = driver_create_file(&pdrv->driver, &driver_attr_perf_concurrency);
	if (ret)
		pr_warn("dx_dma: failed to create perf_concurrency sysfs (%d)\n", ret);
	ret = driver_create_file(&pdrv->driver, &driver_attr_perf_outliers);
	if (ret)
		pr_warn("dx_dma: failed to create perf_outliers sysfs (%d)\n", ret);
	ret = driver_create_file(&pdrv->driver, &driver_attr_perf_outlier_us);
	if (ret)
		pr_warn("dx_dma: failed to create perf_outlier_us sysfs (%d)\n", ret);
	ret = driver_create_file(&pdrv->driver, &driver_attr_perf_floor_us);
	if (ret)
		pr_warn("dx_dma: failed to create perf_floor_us sysfs (%d)\n", ret);
	ret = driver_create_file(&pdrv->driver, &driver_attr_perf_size_reset);
	if (ret)
		pr_warn("dx_dma: failed to create perf_size_reset sysfs (%d)\n", ret);
	ret = driver_create_file(&pdrv->driver, &driver_attr_perf_top);
	if (ret)
		pr_warn("dx_dma: failed to create perf_top sysfs (%d)\n", ret);
	ret = driver_create_file(&pdrv->driver, &driver_attr_link_health_enable);
	if (ret)
		pr_warn("dx_dma: failed to create link_health_enable sysfs (%d)\n", ret);
}

void dx_dma_sysfs_remove(struct pci_driver *pdrv)
{
	driver_remove_file(&pdrv->driver, &driver_attr_link_health_enable);
	driver_remove_file(&pdrv->driver, &driver_attr_perf_top);
	driver_remove_file(&pdrv->driver, &driver_attr_perf_size_reset);
	driver_remove_file(&pdrv->driver, &driver_attr_perf_floor_us);
	driver_remove_file(&pdrv->driver, &driver_attr_perf_outlier_us);
	driver_remove_file(&pdrv->driver, &driver_attr_perf_outliers);
	driver_remove_file(&pdrv->driver, &driver_attr_perf_concurrency);
	driver_remove_file(&pdrv->driver, &driver_attr_perf_stats);
	driver_remove_file(&pdrv->driver, &driver_attr_perf_data);
	driver_remove_file(&pdrv->driver, &driver_attr_perf_clear);
	driver_remove_file(&pdrv->driver, &driver_attr_perf_enable);
}
