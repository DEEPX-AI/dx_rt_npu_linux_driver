#ifndef DXNN_LIB_H
#define DXNN_LIB_H
#include <linux/kernel.h>

#ifdef __LIBXDMA_DEBUG__
#define dbg_dxnn	pr_err
#else
/* disable debugging */
#define dbg_dxnn(...)
#endif

#define SIGETX 44
#define XDMA_CURRENT_TASK    _IOW('x', 10, int)

extern void set_task_pid(int user);
extern void send_signal_to_app(int irq_num);

#endif /* DXNN_LIB_H */
