/* Automatically generated files */
/* registers are not duplicated and register unit : 32bits */

#ifndef __PCIE_PCIE_TOP_H
#define __PCIE_PCIE_TOP_H
/* Address */
#define PCIE0_PCIE_TOP_BASE     0xc1080000
#define PCIE1_PCIE_TOP_BASE     0xc1280000

/* Register Definition */
typedef volatile union _init_crg_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int init_crg:1;
        /* Default Value : 0x0, TYPE : RW, Position : 0 */
        /* Description :
        pcie_syn_top_crg register initialize signal. when assert init_crg to 1'b1, initialize crg register
        */
        volatile unsigned int RESERVED_1:31;
        /* Default Value : x, TYPE : U, Position : 1 */
        /* Description :
        RESERVED
        */
    };
} init_crg_t;

typedef volatile union _reset_reg_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int pcie_cr_clk_rstn:1;
        /* Default Value : 0x1, TYPE : RW, Position : 0 */
        /* Description :
        cr resetn (not used)
        */
        volatile unsigned int RESERVED_1:3;
        /* Default Value : x, TYPE : U, Position : 1 */
        /* Description :
        RESERVED
        */
        volatile unsigned int pcie_auxclk_rstn:1;
        /* Default Value : 0x1, TYPE : RW, Position : 4 */
        /* Description :
        aux resetn (not used)
        */
        volatile unsigned int RESERVED_5:3;
        /* Default Value : x, TYPE : U, Position : 5 */
        /* Description :
        RESERVED
        */
        volatile unsigned int pcie_pclk_rstn:1;
        /* Default Value : 0x1, TYPE : RW, Position : 8 */
        /* Description :
        apb resetn
        */
        volatile unsigned int RESERVED_9:3;
        /* Default Value : x, TYPE : U, Position : 9 */
        /* Description :
        RESERVED
        */
        volatile unsigned int pcie_axi_clk_rstn:1;
        /* Default Value : 0x1, TYPE : RW, Position : 12 */
        /* Description :
        apb resetn (not used)
        */
        volatile unsigned int RESERVED_13:19;
        /* Default Value : x, TYPE : U, Position : 13 */
        /* Description :
        RESERVED
        */
    };
} reset_reg_0_t;

typedef volatile union _clk_en_reg_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int pcie_cr_clk_en:1;
        /* Default Value : 0x1, TYPE : RW, Position : 0 */
        /* Description :
        cr clock enable signal
        */
        volatile unsigned int RESERVED_1:3;
        /* Default Value : x, TYPE : U, Position : 1 */
        /* Description :
        RESERVED
        */
        volatile unsigned int pcie_aux_clk_en:1;
        /* Default Value : 0x1, TYPE : RW, Position : 4 */
        /* Description :
        aux clock enable signal
        */
        volatile unsigned int RESERVED_5:7;
        /* Default Value : x, TYPE : U, Position : 5 */
        /* Description :
        RESERVED
        */
        volatile unsigned int pcie0_axi_clk_en:1;
        /* Default Value : 0x1, TYPE : RW, Position : 12 */
        /* Description :
        axi clock enable signal
        */
        volatile unsigned int RESERVED_13:3;
        /* Default Value : x, TYPE : U, Position : 13 */
        /* Description :
        RESERVED
        */
        volatile unsigned int main_1ghz_in_div2_en:1;
        /* Default Value : 0x1, TYPE : RW, Position : 16 */
        /* Description :
        The enable signal of the divider that divides the clock of 1Ghz to 500MhZ
        */
        volatile unsigned int RESERVED_17:15;
        /* Default Value : x, TYPE : U, Position : 17 */
        /* Description :
        RESERVED
        */
    };
} clk_en_reg_0_t;

typedef volatile union _reset_reg_1_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int sw_button_rst_n:1;
        /* Default Value : 0x0, TYPE : RW, Position : 0 */
        /* Description :
        software button reset_n
        */
        volatile unsigned int RESERVED_1:3;
        /* Default Value : x, TYPE : U, Position : 1 */
        /* Description :
        RESERVED
        */
        volatile unsigned int sw_power_up_rst_n:1;
        /* Default Value : 0x0, TYPE : RW, Position : 4 */
        /* Description :
        software power_up reset_n
        */
        volatile unsigned int RESERVED_5:3;
        /* Default Value : x, TYPE : U, Position : 5 */
        /* Description :
        RESERVED
        */
        volatile unsigned int sw_perst_n:1;
        /* Default Value : 0x0, TYPE : RW, Position : 8 */
        /* Description :
        software perst reset_n
        */
        volatile unsigned int RESERVED_9:3;
        /* Default Value : x, TYPE : U, Position : 9 */
        /* Description :
        RESERVED
        */
        volatile unsigned int apb_muxd_aux_clk_sel:1;
        /* Default Value : 0x0, TYPE : RW, Position : 12 */
        /* Description :
        not used
        */
        volatile unsigned int RESERVED_13:3;
        /* Default Value : x, TYPE : U, Position : 13 */
        /* Description :
        RESERVED
        */
        volatile unsigned int sw_niu_rst_n:1;
        /* Default Value : 0x0, TYPE : RW, Position : 16 */
        /* Description :
        software pcie niu reset_n
        */
        volatile unsigned int RESERVED_17:15;
        /* Default Value : x, TYPE : U, Position : 17 */
        /* Description :
        RESERVED
        */
    };
} reset_reg_1_t;

typedef volatile union _count_reg_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int reg_count:16;
        /* Default Value : 0xff, TYPE : RW, Position : 0 */
        /* Description :
        If the muxd_aux clock does not appear for a certain period of time, it is converted to an apb clock. The count value that determines the time to be converted.
        */
        volatile unsigned int RESERVED_16:16;
        /* Default Value : x, TYPE : U, Position : 16 */
        /* Description :
        RESERVED
        */
    };
} count_reg_0_t;


/* PCIe PCIE_TOP Registers */
typedef volatile struct _pcie_pcie_top_reg_t
{
    init_crg_t init_crg; /* 0x0 */
    reset_reg_0_t reset_reg_0; /* 0x4 */
    clk_en_reg_0_t clk_en_reg_0; /* 0x8 */
    reset_reg_1_t reset_reg_1; /* 0xc */
    count_reg_0_t count_reg_0; /* 0x10 */
} pcie_pcie_top_reg_t;

#endif /* __PCIE_PCIE_TOP_H */