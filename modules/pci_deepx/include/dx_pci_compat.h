/* SPDX-License-Identifier: GPL-2.0 */
/*
 * pci_alloc_irq_vectors / pci_irq_vector / pci_* logging for older kernels
 * (e.g. Ubuntu 4.4.x before unified MSI helpers landed in 4.8).
 */
#ifndef DX_PCI_COMPAT_H
#define DX_PCI_COMPAT_H

#include <linux/version.h>
#include <linux/pci.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0)

#include <linux/msi.h>

#ifndef PCI_IRQ_MSI
#define PCI_IRQ_MSI  (1 << 0)
#endif

#ifdef CONFIG_PCI_MSI

static inline int pci_alloc_irq_vectors(struct pci_dev *pdev,
					unsigned int min_vecs,
					unsigned int max_vecs,
					unsigned long flags)
{
	(void)flags;
	return pci_enable_msi_range(pdev, (int)min_vecs, (int)max_vecs);
}

static inline void pci_free_irq_vectors(struct pci_dev *pdev)
{
	pci_disable_msi(pdev);
}

/*
 * Linux 4.4 MSI: multiple vectors are often one msi_desc with nvec_used > 1
 * and consecutive Linux IRQs (entry->irq, entry->irq+1, ...).  Do not treat
 * each list node as exactly one vector — that makes pci_irq_vector(1) fail
 * with -EINVAL and breaks multi-vector request_irq().
 */
static inline int pci_irq_vector(struct pci_dev *pdev, unsigned int nr)
{
	struct msi_desc *entry;
	unsigned int base = 0;

	for_each_pci_msi_entry(entry, pdev) {
		unsigned int n = entry->nvec_used ? entry->nvec_used : 1;

		if (nr < base + n)
			return entry->irq + (nr - base);
		base += n;
	}
	return -EINVAL;
}

#endif /* CONFIG_PCI_MSI */

#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)

#include <linux/delay.h>

/*
 * pcie_flr() became a public, ready-waiting helper in mainline 4.12.  On older
 * kernels (e.g. 4.4) it exists only as a private function in pci/pci.c.  This
 * mirrors the 4.12 implementation: verify FLR capability, drain pending
 * transactions, set the Initiate Function Level Reset bit, then wait the
 * mandated 100ms for the function to become ready.
 */
static inline int pcie_flr(struct pci_dev *dev)
{
	u32 cap;

	pcie_capability_read_dword(dev, PCI_EXP_DEVCAP, &cap);
	if (!(cap & PCI_EXP_DEVCAP_FLR))
		return -ENOTTY;

	if (!pci_wait_for_pending_transaction(dev))
		dev_err(&dev->dev,
			"timed out waiting for pending transaction; performing function level reset anyway\n");

	pcie_capability_set_word(dev, PCI_EXP_DEVCTL, PCI_EXP_DEVCTL_BCR_FLR);
	msleep(100);
	return 0;
}

#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0) */

#ifndef pci_dbg
#define pci_dbg(pdev, fmt, ...) \
	dev_dbg(&(pdev)->dev, fmt, ##__VA_ARGS__)
#endif
#ifndef pci_err
#define pci_err(pdev, fmt, ...) \
	dev_err(&(pdev)->dev, fmt, ##__VA_ARGS__)
#endif
#ifndef pci_warn
#define pci_warn(pdev, fmt, ...) \
	dev_warn(&(pdev)->dev, fmt, ##__VA_ARGS__)
#endif
#ifndef pci_info
#define pci_info(pdev, fmt, ...) \
	dev_info(&(pdev)->dev, fmt, ##__VA_ARGS__)
#endif

#endif /* DX_PCI_COMPAT_H */
