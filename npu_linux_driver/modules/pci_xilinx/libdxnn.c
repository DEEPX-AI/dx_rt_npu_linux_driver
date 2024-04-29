#include <linux/types.h>
#include <linux/kthread.h>
#include <linux/sched/signal.h>

#include "libdxnn.h"

#define NPU_MAX_DEVICES		4
/* Signaling to Application */
static struct task_struct *task = {NULL,};
static int signum = 0;
static int npu_irq_signum[NPU_MAX_DEVICES] = {35, 36, 37, 38};

void set_task_pid(int pcie_num)
{
    struct task_struct *task_p = get_current();
    task = task_p;
    signum = npu_irq_signum[0];
    pr_info("[DXNN] set_task_pid, task:%p, id:%d (%d)\n", task_p, task_p->pid, npu_irq_signum[0]);
}

void send_signal_to_app(int irq_num)
{
    if (task != NULL) {
        pr_info("[DXNN] Sending signal to app, task:%p, id:%d (%d)\n", task, task->pid, npu_irq_signum[0]);
        if(kill_pid(find_vpid(task->pid), npu_irq_signum[irq_num], SEND_SIG_PRIV) < 0)
		{
			pr_err("[DXNN] pcie irq: can't send signal\n");
		}
    }
}
