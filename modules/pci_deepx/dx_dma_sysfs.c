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
#include "dx_dma_sysfs.h"
#include "dx_util.h"
#include "dx_link_health.h"

static DEFINE_MUTEX(perf_stats_lock);

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
	ret = driver_create_file(&pdrv->driver, &driver_attr_link_health_enable);
	if (ret)
		pr_warn("dx_dma: failed to create link_health_enable sysfs (%d)\n", ret);
}

void dx_dma_sysfs_remove(struct pci_driver *pdrv)
{
	driver_remove_file(&pdrv->driver, &driver_attr_link_health_enable);
	driver_remove_file(&pdrv->driver, &driver_attr_perf_stats);
	driver_remove_file(&pdrv->driver, &driver_attr_perf_data);
	driver_remove_file(&pdrv->driver, &driver_attr_perf_clear);
	driver_remove_file(&pdrv->driver, &driver_attr_perf_enable);
}
