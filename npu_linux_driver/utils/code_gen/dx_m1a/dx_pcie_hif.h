/* Automatically generated files */

#ifndef __DX_PCIE_HIF_H
#define __DX_PCIE_HIF_H

/* Address */
#define PCIE0_HIF_BASE     0xc1000000
#define PCIE1_HIF_BASE     0xc1200000

/* Register Definition */
/* offset : 0x0 */
typedef volatile union _acc_ctrl00_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int sys_aux_pwr_det:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Auxiliary Power Detected. Used to report to the host software that auxiliary power (Vaux) is present.
        */
        volatile unsigned int RESERVED_1:31; /* [Position:1] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_ctrl00_0_t;
/* offset : 0x4 */
typedef volatile union _acc_ctrl01_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int clkreq_in_n_sel:2; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
         Status of the CLKREQ# bidirectional CMOS board-level signal. Used by the controller to determine when to enter and exit L1 Substates when using the CLKREQ#-based mechanism. For more information, see 'L1 Substates'. This signal is syncronized using aux_clk and can be driven/supplied asynchronously to the controller in certain low-power modes.
When this bit is 2'b00, clkreq_in_n = ext_clkreq_in_n.
When this bit is 2'b01, clkreq_in_n = !ext_clkreq_in_n.
        */
        volatile unsigned int RESERVED_2:30; /* [Position:2] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_ctrl01_0_t;
/* offset : 0x10 */
typedef volatile union _acc_intr_en0_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int intr_en:19; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        intr_en[00] = cfg_send_cor_err
intr_en[01] = cfg_send_nf_err
intr_en[02] = cfg_send_f_err
intr_en[03] = assert_inta_grt
intr_en[04] = assert_intb_grt
intr_en[05] = assert_intc_grt
intr_en[06] = assert_intd_grt
intr_en[07] = deassert_inta_grt
intr_en[08] = deassert_intb_grt
intr_en[09] = deassert_intc_grt
intr_en[10] = deassert_intd_grt
intr_en[11] = flr_pf_active_assert_intr
intr_en[12] = flr_vf0_active_assert_intr
intr_en[13] = flr_vf1_active_assert_intr
intr_en[14] = flr_vf2_active_assert_intr
intr_en[15] = flr_vf3_active_assert_intr
intr_en[16] = cfg_link_eq_req_int
intr_en[17] = usp_eq_redo_executed_int
intr_en[18] = parity_intr 
        */
        volatile unsigned int RESERVED_19:13; /* [Position:19] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_intr_en0_0_t;
/* offset : 0x14 */
typedef volatile union _acc_intr_cl0_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int intr_cl0:16; /* [Position:0] */
        /* Default Value : 0, TYPE : WO */
        /* Description :
        intr_cl[00] = cfg_send_cor_err
intr_cl[01] = cfg_send_nf_err
intr_cl[02] = cfg_send_f_err
intr_cl[03] = assert_inta_grt
intr_cl[04] = assert_intb_grt
intr_cl[05] = assert_intc_grt
intr_cl[06] = assert_intd_grt
intr_cl[07] = deassert_inta_grt
intr_cl[08] = deassert_intb_grt
intr_cl[09] = deassert_intc_grt
intr_cl[10] = deassert_intd_grt
intr_cl[11] = flr_pf_active_assert_intr
intr_cl[12] = flr_vf0_active_assert_intr clear
intr_cl[13] = flr_vf1_active_assert_intr clear
intr_cl[14] = flr_vf2_active_assert_intr clear
intr_cl[15] = flr_vf3_active_assert_intr clear
        */
        volatile unsigned int RESERVED_16:2; /* [Position:16] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
        volatile unsigned int intr_cl1:1; /* [Position:18] */
        /* Default Value : 0, TYPE : WO */
        /* Description :
        intr_cl[18] = parity_intr
        */
        volatile unsigned int RESERVED_20:12; /* [Position:20] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_intr_cl0_0_t;
/* offset : 0x18 */
typedef volatile union _acc_intr_st_part_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int intr_st_part:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        intr_st_part[0] = cfg_send_cor_err
intr_st_part[1] = cfg_send_nf_err
intr_st_part[2] = cfg_send_f_err
intr_st_part[3] = assert_inta_grt
intr_st_part[4] = assert_intb_grt
intr_st_part[5] = assert_intc_grt
intr_st_part[6] = assert_intd_grt
intr_st_part[7] = deassert_inta_grt
intr_st_part[8] = deassert_intb_grt
intr_st_part[9] = deassert_intc_grt
intr_st_part[10] = deassert_intd_grt
intr_st_part[11] = flr_pf_assert_intr_st
intr_st_part[12] = flr_vf0_assert_intr_st
intr_st_part[13] = flr_vf1_assert_intr_st
intr_st_part[14] = flr_vf2_assert_intr_st
intr_st_part[15] = flr_vf3_assert_intr_st
intr_st_part[16] = cfg_link_eq_req_int
intr_st_part[17] = usp_eq_redo_executed_int
intr_st_part[18] = parity_intr
intr_st_part[31:19] = RESERVED
        */
    };
} acc_intr_st_part_0_t;
/* offset : 0x1c */
typedef volatile union _acc_perst_n_intr_en_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int perst_n_intr_en:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        perst_n_intr interrupt enable signal
        */
        volatile unsigned int RESERVED_1:31; /* [Position:1] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_perst_n_intr_en_0_t;
/* offset : 0x20 */
typedef volatile union _acc_perst_n_intr_clr_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int perst_n_intr_clr:1; /* [Position:0] */
        /* Default Value : 0, TYPE : WO */
        /* Description :
        perst_n_intr interrupt clear signal
        */
        volatile unsigned int RESERVED_1:31; /* [Position:1] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_perst_n_intr_clr_0_t;
/* offset : 0x24 */
typedef volatile union _acc_perst_n_intr_st_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int perst_n_intr_st:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        perst_n_intr interrupt status signal
        */
        volatile unsigned int RESERVED_1:31; /* [Position:1] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_perst_n_intr_st_0_t;
/* offset : 0x28 */
typedef volatile union _acc_edma_wr_intr_en_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int edma_wr_intr_en:4; /* [Position:0] */
        /* Default Value : 15, TYPE : RW */
        /* Description :
        edma write channel interrupt enable signal
        */
        volatile unsigned int RESERVED_4:28; /* [Position:4] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_edma_wr_intr_en_0_t;
/* offset : 0x2c */
typedef volatile union _acc_edma_wr_intr_st_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int edma_wr_intr_st:4; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        edma write channel interrupt status signal
        */
        volatile unsigned int RESERVED_4:28; /* [Position:4] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_edma_wr_intr_st_0_t;
/* offset : 0x34 */
typedef volatile union _acc_edma_rd_intr_en_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int edma_rd_intr_en:4; /* [Position:0] */
        /* Default Value : 15, TYPE : RW */
        /* Description :
        edma read channel interrupt enable signal
        */
        volatile unsigned int RESERVED_4:28; /* [Position:4] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_edma_rd_intr_en_0_t;
/* offset : 0x38 */
typedef volatile union _acc_edma_rd_intr_st_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int edma_rd_intr_st:4; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        edma read channel interrupt status signal
        */
        volatile unsigned int RESERVED_4:28; /* [Position:4] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_edma_rd_intr_st_0_t;
/* offset : 0x48 */
typedef volatile union _acc_pipe02_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int phy_cfg_status:4; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Input bus that can optionally be used to read PHY status. The phy_cfg_status bus maps to the PHY Status register
phy_cfg_status[0] = pipe_lane0_phystatus
phy_cfg_status[1] = pipe_lane1_phystatus
phy_cfg_status[2] = pipe_lane2_phystatus
phy_cfg_status[3] = pipe_lane3_phystatus
        */
        volatile unsigned int data:28; /* [Position:4] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        data
        */
    };
} acc_pipe02_0_t;
/* offset : 0x50 */
typedef volatile union _acc_pipe04_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mac_phy_fs:24; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Provides the FS value advertised by the link partner.
        */
        volatile unsigned int RESERVED_24:8; /* [Position:24] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_pipe04_0_t;
/* offset : 0x54 */
typedef volatile union _acc_pipe05_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mac_phy_lf:24; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Provides the LF value advertised by the link partner.
        */
        volatile unsigned int RESERVED_24:8; /* [Position:24] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_pipe05_0_t;
/* offset : 0x58 */
typedef volatile union _acc_pipe06_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mac_phy_rxpresethint:12; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Provides the RX equalization preset hint for the receiver.
        */
        volatile unsigned int mac_phy_dirchange:4; /* [Position:12] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates the PHY to perform Figure Of Merit or Direction Change evaluation. 
0: when mac_phy_rxeqeval = 1, the PHY performs Figure Of Merit 
1: when mac_phy_rxeqeval = 1, the PHY performs Direction Change
        */
        volatile unsigned int mac_phy_pclk_rate:3; /* [Position:16] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Controls the PIPE PCLK rate
        */
        volatile unsigned int RESERVED_19:13; /* [Position:19] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_pipe06_0_t;
/* offset : 0x84 */
typedef volatile union _acc_etc_rw01_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int app_sris_mode:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        SRIS operating mode: 0b: non-SRIS mode 1b: SRIS mode
        */
        volatile unsigned int app_ltssm_enable:1; /* [Position:1] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Driven low by your application after cold, warm or hot reset to hold the LTSSM in the Detect state until your application is ready for the link training to begin.
        */
        volatile unsigned int app_hold_phy_rst:1; /* [Position:2] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Set this signal to one before the de-assertion of power on reset to hold the PHY in reset.
        */
        volatile unsigned int app_clk_req_n:1; /* [Position:3] */
        /* Default Value : 1, TYPE : RW */
        /* Description :
        Indicates that the application logic is ready to have reference clock removed. In designs which support reference clock removal through either L1 PM Sub-states or L1 CPM, the application should set this signal to 1'b1 if it supports reference clock removal.
        */
        volatile unsigned int app_clk_pm_en:1; /* [Position:4] */
        /* Default Value : 1, TYPE : RW */
        /* Description :
        Clock PM feature enabled by application. Used to inhibit the programming of the Clock PM in Link Control Register.
        */
        volatile unsigned int app_init_rst:1; /* [Position:5] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Request from your application to send a hot reset to the upstream port. The hot reset request is sent when a single cycle pulse is applied to this pin. In an upstream port, you should set this input to '0'.
        */
        volatile unsigned int app_req_entr_l1:1; /* [Position:6] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Application request to Enter L1 ASPM state. The app_req_entr_l1 signal is for use by applications that need to control L1 entry instead of using the L1 entry timer as defined in the PCI Express Specification.
        */
        volatile unsigned int app_ready_entr_l23:1; /* [Position:7] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Application Ready to Enter L23. Indication from your application that it is ready to enter the L23 state.
        */
        volatile unsigned int app_req_exit_l1:1; /* [Position:8] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Application request to Exit L1. Request from your application to exit L1. It is only effective when L1 is enabled.
        */
        volatile unsigned int app_xfer_pending:1; /* [Position:9] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Indicates that your application has transfers pending and prevents the controller from entering L1
        */
        volatile unsigned int app_req_retry_en:1; /* [Position:10] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Provides a capability to defer incoming configuration requests until initialization is complete. When app_req_retry_en is asserted, the controller completes incoming configuration requests with a configuration request retry status.
        */
        volatile unsigned int app_pf_req_retry_en:4; /* [Position:11] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Provides a per Physical Function (PF) capability to defer incoming configuration requests until initialization is complete. When app_pf_req_retry_en is asserted for a certain PF, the controller completes incoming configuration requests with a configuration request retry status other incoming requests are not affected.
        */
        volatile unsigned int app_drs_ready:1; /* [Position:15] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Defers DRS messaging when set to '0'.
        */
        volatile unsigned int app_pf_frs_ready:1; /* [Position:16] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Defers FRS messaging when set to '0'.
        */
        volatile unsigned int exp_rom_validation_status_strobe:1; /* [Position:17] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        This strobe sets the Expansion ROM Validation Status field (ROM_BAR_VALIDATION_STATUS) of the Expansion ROM BAR register as specified on the exp_rom_validation_status input signal.
        */
        volatile unsigned int exp_rom_validation_details_strobe:1; /* [Position:18] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        This strobe sets the Expansion ROM Validation Details field (ROM_BAR_VALIDATION_DETAILS) of the Expansion ROM BAR register as specified on the exp_rom_validation_details input signal.
        */
        volatile unsigned int RESERVED_19:13; /* [Position:19] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw01_0_t;
/* offset : 0x88 */
typedef volatile union _acc_etc_rw02_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int exp_rom_validation_status:3; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        You can use this signal to notify the controller about the expansion ROM validation status. On assertion of exp_rom_validation_status_strobe, the controller stores the value of this signal in Expansion ROM Validation Status field (ROM_BAR_VALIDATION_STATUS) of the Expansion ROM BAR register.
        */
        volatile unsigned int exp_rom_validation_details:4; /* [Position:3] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        You can use this signal to notify the controller about the expansion ROM validation details. On assertion of exp_rom_validation_details_strobe, the controller stores the value of this signal in Expansion ROM Validation Details field (ROM_BAR_VALIDATION_DETAILS) of the Expansion ROM BAR register.
        */
        volatile unsigned int RESERVED_7:25; /* [Position:7] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw02_0_t;
/* offset : 0x94 */
typedef volatile union _acc_etc_rw05_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int prs_res_failure:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Pulse per function(PF) to indicate that the function (PF or associated VF) has received a PRG Response Failure.
        */
        volatile unsigned int prs_uprgi:1; /* [Position:1] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Pulse per function(PF) to indicate that the function (PF or associated VF) has received a response with Unexpected Page Request Group Index.
        */
        volatile unsigned int prs_stopped:1; /* [Position:2] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Pulse per function(PF) to indicate that the function (PF or associated VF) has completed all previously issued Page Requests and that it has stopped requests for additional pages. Only valid when the PRS enable bit is clear.
        */
        volatile unsigned int RESERVED_3:29; /* [Position:3] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw05_0_t;
/* offset : 0x98 */
typedef volatile union _acc_etc_rw06_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mstr_bmisc_info_tlpln:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Master Write Response TLP Lightweight Notification (LN) bit.
        */
        volatile unsigned int mstr_bmisc_info_cpl_stat:3; /* [Position:1] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Master Write Response selection bus. This controls the response to be sent on the wire in the case of successful write requests.
000: SC (Successful Completion) 001: UR (Unsupported Request)
010: RRS (Request Retry Status) 100: CA (Completer Abort) All others: Reserved
        */
        volatile unsigned int mstr_rmisc_info_tlpln:1; /* [Position:4] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Master Read Response TLP Lightweight Notification (LN) bit.
        */
        volatile unsigned int mstr_rmisc_info:13; /* [Position:5] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Master Read Response Transaction Associated Misc Information.
[1:0]: Reserved [2]: Reserved. [6:3]: Reserved [7]: TLP's EP bit [12:8]: Reserved
        */
        volatile unsigned int mstr_rmisc_info_cpl_stat:3; /* [Position:18] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Master Read Response selection bus. This bus controls the response sent on the PCIe wire in the case of successful read requests.
000: SC (Successful Completion) 001: UR (Unsupported Request)
010: RRS (Request Retry Status) 100: CA (Completer Abort) All others: Reserved
        */
        volatile unsigned int mstr_csysreq:1; /* [Position:21] */
        /* Default Value : 1, TYPE : RW */
        /* Description :
        AXI Master System Low-Power Request. System low-power request from system clock controller for AXI bridge master to enter low-power state.
        */
        volatile unsigned int mstr_csysack:1; /* [Position:22] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Low-Power Request Acknowledgment. Indicates acknowledgment from AXI bridge master to a system low-power request. The controller does not support AXI power modes and always denies any request the mstr_csysreq input.
        */
        volatile unsigned int mstr_cactive:1; /* [Position:23] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Clock Active. Indicates that AXI bridge master requires a clock signal. The controller does not support AXI power modes and always drives this output high.
        */
        volatile unsigned int RESERVED_24:8; /* [Position:24] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw06_0_t;
/* offset : 0x9c */
typedef volatile union _acc_etc_rw07_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int slv_awmisc_info:22; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave Write Transaction Associated Misc Information.
[4:0]: TYPE bits of the TLP. For more information, see 'I/O and CFG Transaction Handling'
[5]: Serialize NP Requests [6]: EP bit of the TLP
[7]: Reserved [8]: NS bit of the TLP [9]: RO bit of the TLP
[12:10]: TC bits of the TLP [20:13]: MSG code of the TLP
[21]: AXI transaction is a DBI access. This is for SHARED DBI mode only.
[22]: Function number of the TLP. Function numbering starts at 
'0'. Not used when CX_SRIOV_ENABLE =1 or CX_ARI_ENABLE =1. 
        */
        volatile unsigned int RESERVED_22:10; /* [Position:22] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw07_0_t;
/* offset : 0xa0 */
typedef volatile union _acc_etc_rw08_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int slv_awmisc_info_hdr_34dw0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave 3rd and 4th header DWs. The application drives this bus with the 3rd and 4th Header DWs it intends to send on a PCIe Msg/MsgD. Note: The data is in big endian format, that is, slv_awmisc_info_hdr_34dw[7:0] contains byte 15 of header DW.[LSB 32bit]
        */
    };
} acc_etc_rw08_0_t;
/* offset : 0xa4 */
typedef volatile union _acc_etc_rw09_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int slv_awmisc_info_hdr_34dw1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave 3rd and 4th header DWs. The application drives this bus with the 3rd and 4th Header DWs it intends to send on a PCIe Msg/MsgD. Note: The data is in big endian format, that is, slv_awmisc_info_hdr_34dw[7:0] contains byte 15 of header DW.[MSB 32bit]
        */
    };
} acc_etc_rw09_0_t;
/* offset : 0xa8 */
typedef volatile union _acc_etc_rw10_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int slv_awmisc_info_nw:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        ATS No Write (NW) Bit of the AXI Slave Write Transaction Request.
        */
        volatile unsigned int RESERVED_1:31; /* [Position:1] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw10_0_t;
/* offset : 0xac */
typedef volatile union _acc_etc_rw11_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int RESERVED_0:1; /* [Position:0] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
        volatile unsigned int slv_awmisc_info_ats:2; /* [Position:1] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AT Bits of the AXI Slave Write Transaction Request.
        */
        volatile unsigned int slv_awmisc_info_p_tag:8; /* [Position:3] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave Write Request Tag. Sets the TAG number for output posted requests. It is expected that your application normally sets this to '0' except when generating ATS invalidate requests.
        */
        volatile unsigned int slv_awmisc_info_tph:11; /* [Position:11] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave Write Request TLP Processing Hints. The bits are mapped as follows: 
[0]: TH (TLP Processing Hint present) [2:1] PH (TLP Processing Hint) 
[10:3] ST (Steering Tag)
        */
        volatile unsigned int slv_awmisc_info_tlpln:1; /* [Position:22] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave Write Request TLP Lightweight Notification (LN) bit.
        */
        volatile unsigned int slv_awmisc_info_atu_bypass:1; /* [Position:23] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave Write Request Internal ATU Bypass. When set it indicates that this request should not be processed by the internal address translation unit
        */
        volatile unsigned int slv_awmisc_info_ido:1; /* [Position:24] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        IDO bit of the AXI Slave Write Transaction. Enables ID-base ordering on outbound requests.
        */
        volatile unsigned int slv_wmisc_info_ep:1; /* [Position:25] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave Write Data transaction related misc information. When asserted, the controller sets the Poisoned TLP (EP) bit in the TLP header of the current and subsequent Write Data transactions.
        */
        volatile unsigned int slv_wmisc_info_silentDrop:1; /* [Position:26] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave Write Data transaction related misc information.
        */
        volatile unsigned int RESERVED_27:5; /* [Position:27] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw11_0_t;
/* offset : 0xb0 */
typedef volatile union _acc_etc_rw12_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int slv_awmisc_info_tlpprfx0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave Write Request TLP Prefixes. The field [31:0] represents the first prefix to be transmitted [31:0]
        */
    };
} acc_etc_rw12_0_t;
/* offset : 0xb4 */
typedef volatile union _acc_etc_rw13_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int slv_awmisc_info_tlpprfx1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave Write Request TLP Prefixes. The field [31:0] represents the first prefix to be transmitted [63:32]
        */
    };
} acc_etc_rw13_0_t;
/* offset : 0xb8 */
typedef volatile union _acc_etc_rw14_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int slv_awmisc_info_tlpprfx2:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave Write Request TLP Prefixes. The field [31:0] represents the first prefix to be transmitted [95:64]
        */
    };
} acc_etc_rw14_0_t;
/* offset : 0xbc */
typedef volatile union _acc_etc_rw15_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int slv_awmisc_info_tlpprfx3:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave Write Request TLP Prefixes. The field [31:0] represents the first prefix to be transmitted [127:96]
        */
    };
} acc_etc_rw15_0_t;
/* offset : 0xc0 */
typedef volatile union _acc_etc_rw16_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int slv_awmisc_info_tlpprfx4:16; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave Write Request TLP Prefixes. The field [31:0] represents the first prefix to be transmitted [143:128]
        */
        volatile unsigned int RESERVED_16:16; /* [Position:16] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw16_0_t;
/* offset : 0xc4 */
typedef volatile union _acc_etc_rw17_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int slv_armisc_info:22; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave Read Transaction Associated Miscellaneous Information.
[4:0]: TLP's TYPE [5]: Serialize NP Requests [6]: Reserved
[7]: Reserved [8]: TLP's NS bit [9]: TLP's RO bit
[12:10]: TLP's TC bits [20:13]: TLP's MSG code
[21]: AXI transaction is a DBI access. This is for SHARED DBI mode only.
[24:22]: TLP's function number. Function numbering starts at '0'. Not 
used when CX_SRIOV_ENABLE =1 or CX_ARI_ENABLE =1. For more information, see slv_armisc_info_func_num. This field is only present when multifunction support (CX_NFUNC > 1) is enabled.
        */
        volatile unsigned int RESERVED_22:4; /* [Position:22] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
        volatile unsigned int slv_armisc_info_nw:1; /* [Position:26] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AT No Write (NW) Bit of the AXI Slave Read Transaction Request.
        */
        volatile unsigned int slv_armisc_info_ats:2; /* [Position:27] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AT Bits of the AXI Slave Read Transaction Request.
        */
        volatile unsigned int RESERVED_29:3; /* [Position:29] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw17_0_t;
/* offset : 0xc8 */
typedef volatile union _acc_etc_rw18_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int slv_armisc_info_tph:11; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave Read Request TLP Processing Hints. The bits are mapped as follows: 
[0]: TH (TLP Processing Hint present) [2:1]: PH (TLP Processing Hint) 
[10:3]: ST (Steering Tag)
        */
        volatile unsigned int slv_armisc_info_tlpln:1; /* [Position:11] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave Read Request TLP Lightweight Notification (LN) bit
        */
        volatile unsigned int slv_armisc_info_atu_bypass:1; /* [Position:12] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave Read Request Internal ATU Bypass. When set it indicates that this request should not be processed by the internal address translation unit
        */
        volatile unsigned int slv_armisc_info_ido:1; /* [Position:13] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        IDO Bit of the AXI Slave Read Transaction. Enables ID-base ordering on outbound requests.
        */
        volatile unsigned int slv_csysreq:1; /* [Position:14] */
        /* Default Value : 1, TYPE : RW */
        /* Description :
        AXI Slave System Low-Power Request. System low-power request from system clock controller for AXI bridge slave to enter low-power state. AXI power modes are not currently supported.
        */
        volatile unsigned int slv_csysack:1; /* [Position:15] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Slave Low-Power Request Acknowledgment. Indicates acknowledgment from AXI bridge slave to a system low-power request.
        */
        volatile unsigned int slv_cactive:1; /* [Position:16] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Slave Clock Active. Indicates that AXI bridge slave requires a clock signal
        */
        volatile unsigned int RESERVED_17:15; /* [Position:17] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw18_0_t;
/* offset : 0xcc */
typedef volatile union _acc_etc_rw19_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int slv_armisc_info_tlpprfx0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave Read Request TLP Prefixes. The field [31:0] represents the first prefix to be transmitted. [31:0]
        */
    };
} acc_etc_rw19_0_t;
/* offset : 0xd0 */
typedef volatile union _acc_etc_rw20_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int slv_armisc_info_tlpprfx1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave Read Request TLP Prefixes. The field [31:0] represents the first prefix to be transmitted. [63:32]
        */
    };
} acc_etc_rw20_0_t;
/* offset : 0xd4 */
typedef volatile union _acc_etc_rw21_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int slv_armisc_info_tlpprfx2:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave Read Request TLP Prefixes. The field [31:0] represents the first prefix to be transmitted. [95:64]
        */
    };
} acc_etc_rw21_0_t;
/* offset : 0xd8 */
typedef volatile union _acc_etc_rw22_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int slv_armisc_info_tlpprfx3:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave Read Request TLP Prefixes. The field [31:0] represents the first prefix to be transmitted. [127:96]
        */
    };
} acc_etc_rw22_0_t;
/* offset : 0xdc */
typedef volatile union _acc_etc_rw23_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int slv_armisc_info_tlpprfx4:16; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave Read Request TLP Prefixes. The field [31:0] represents the first prefix to be transmitted. [143:128]
        */
        volatile unsigned int RESERVED_16:16; /* [Position:16] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw23_0_t;
/* offset : 0xe0 */
typedef volatile union _acc_etc_rw24_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int app_dbi_ro_wr_disable:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        DBI Read-only Write Disable 
0: MISC_CONTROL_1_OFF.DBI_RO_WR_EN register field is read-write. 
1: MISC_CONTROL_1_OFF.DBI_RO_WR_EN register field is forced to 0 and is read-only.
        */
        volatile unsigned int ext_lbc_ack:4; /* [Position:1] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Indicates that the requested read or write operation to an external register block is complete. There is no time interval requirement between the request (when lbc_ext_cs is asserted) and the acknowledgement (when ext_lbc_ack is asserted).
        */
        volatile unsigned int RESERVED_5:27; /* [Position:5] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw24_0_t;
/* offset : 0xe4 */
typedef volatile union _acc_etc_rw25_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int ext_lbc_din0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Data bus from the external register block. Depending on value of CX_LBC_NW, there are 32/64/128 bits of ext_lbc_din for each function in your controller configuration. [31:0]
        */
    };
} acc_etc_rw25_0_t;
/* offset : 0xe8 */
typedef volatile union _acc_etc_rw26_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int ven_msi_func_num:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        The function number of the MSI request. Function numbering starts at '0'.
        */
        volatile unsigned int RESERVED_1:31; /* [Position:1] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw26_0_t;
/* offset : 0xf8 */
typedef volatile union _acc_etc_rw30_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int app_ltr_msg_latency:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        LTR message that your application is requesting to send.
        */
    };
} acc_etc_rw30_0_t;
/* offset : 0xfc */
typedef volatile union _acc_etc_rw31_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int app_ltr_msg_req:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Indicates that your application is requesting to send an LTR message. Once asserted, app_ ltr_msg_req must remain asserted until the controller asserts app_ltr_msg_grant.
        */
        volatile unsigned int app_ltr_msg_func_num:1; /* [Position:1] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Function number in your application that is requesting to send an LTR message. Function numbering starts at '0'.
        */
        volatile unsigned int RESERVED_2:30; /* [Position:2] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw31_0_t;
/* offset : 0x104 */
typedef volatile union _acc_etc_rw33_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int ven_msg_req:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Request from your application to send a vendor-defined Message. Once asserted, ven_msg_req must remain asserted until the controller asserts ven_msg_grant.
        */
        volatile unsigned int ven_msg_td:1; /* [Position:1] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        The TLP Digest (TD) bit for the vendor-defined Message TLP, valid when ven_msg_req is asserted.
        */
        volatile unsigned int ven_msg_ep:1; /* [Position:2] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        The Poisoned TLP (EP) bit for the vendor-defined Message TLP.
        */
        volatile unsigned int ven_msg_fmt:2; /* [Position:3] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        The Format field for the vendor-defined Message TLP. Should be set to 0x1.
        */
        volatile unsigned int ven_msg_type:5; /* [Position:5] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        The Type field for the vendor-defined Message TLP.
        */
        volatile unsigned int ven_msg_tc:3; /* [Position:10] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        The Traffic Class field for the vendor-defined Message TLP.
        */
        volatile unsigned int ven_msg_attr:3; /* [Position:13] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        The Attributes field for the vendor-defined Message TLP. 
Width is three bits when CX_IDO_ENABLE =1 
Width is two bits when CX_IDO_ENABLE =0
        */
        volatile unsigned int ven_msg_len:10; /* [Position:16] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        The Length field for the vendor-defined Message TLP (indicates length of data payload in dwords). Should be set to 0x0.
        */
        volatile unsigned int ven_msg_func_num:1; /* [Position:26] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Function Number for the vendor-defined Message TLP. Function numbering starts at '0'.
        */
        volatile unsigned int RESERVED_27:5; /* [Position:27] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw33_0_t;
/* offset : 0x110 */
typedef volatile union _acc_etc_rw36_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int ven_msg_tag:8; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Tag for the vendor-defined Message TLP.
        */
        volatile unsigned int ven_msg_code:8; /* [Position:8] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        The Message Code for the vendor-defined Message TLP.
        */
        volatile unsigned int RESERVED_16:16; /* [Position:16] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw36_0_t;
/* offset : 0x114 */
typedef volatile union _acc_etc_rw37_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int ven_msg_data0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Third and fourth dwords of the Vendor Defined Message header where: 
Bytes 8-11 (third header dword) =ven_msg_data[63:32] 
Bytes 12-15 (fourth header dword) =ven_msg_data[31:0], where ven_msg_data[7:0] =byte 15 // [31:0]
        */
    };
} acc_etc_rw37_0_t;
/* offset : 0x118 */
typedef volatile union _acc_etc_rw38_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int ven_msg_data1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Third and fourth dwords of the Vendor Defined Message header where: 
Bytes 8-11 (third header dword) =ven_msg_data[63:32] 
Bytes 12-15 (fourth header dword) =ven_msg_data[31:0], where ven_msg_data[7:0] =byte 15 // [64:32]
        */
    };
} acc_etc_rw38_0_t;
/* offset : 0x11c */
typedef volatile union _acc_etc_rw39_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int rx_lane_flip_en:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Performs manual lane reversal for receive lanes. For use when automatic lane reversal does not occur because lane 0 is not detected.
        */
        volatile unsigned int tx_lane_flip_en:1; /* [Position:1] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Performs manual lane reversal for transmit lanes. For use when automatic lane reversal does not occur because lane 0 is not detected.
        */
        volatile unsigned int sys_int:1; /* [Position:2] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        When sys_int goes from low to high, the controller generates an Assert_INTx Message. When sys_int goes from high to low, the controller generates a Deassert_INTx Message.
        */
        volatile unsigned int apps_pm_xmt_pme:1; /* [Position:3] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Wake Up. If PME is enabled and PME support is configured for current PMCSR D-state asserting this signal causes the controller to wake from either L1 or L2 state.
        */
        volatile unsigned int RESERVED_4:28; /* [Position:4] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw39_0_t;
/* offset : 0x120 */
typedef volatile union _acc_etc_rw40_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int outband_pwrup_cmd:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Wake Up. If PME is enabled and PME support is configured for current PMCSR D-state asserting this signal causes the controller to wake from either L1 or L2 state.
        */
        volatile unsigned int app_hdr_valid:1; /* [Position:1] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        One-clock-cycle pulse indicating that the data app_hdr_log, app_err_bus, app_err_func_num, and app_tlp_prfx_log is valid.
        */
        volatile unsigned int RESERVED_2:30; /* [Position:2] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw40_0_t;
/* offset : 0x124 */
typedef volatile union _acc_etc_rw41_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int app_hdr_log0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        The header of the TLP that contained the error indicated app_err_bus, valid when 1. app_hdr_valid is asserted. [31:0]
        */
    };
} acc_etc_rw41_0_t;
/* offset : 0x128 */
typedef volatile union _acc_etc_rw42_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int app_hdr_log1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        The header of the TLP that contained the error indicated app_err_bus, valid when 1. app_hdr_valid is asserted. [63:32]
        */
    };
} acc_etc_rw42_0_t;
/* offset : 0x12c */
typedef volatile union _acc_etc_rw43_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int app_hdr_log2:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        The header of the TLP that contained the error indicated app_err_bus, valid when 1. app_hdr_valid is asserted. [95:64]
        */
    };
} acc_etc_rw43_0_t;
/* offset : 0x130 */
typedef volatile union _acc_etc_rw44_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int app_hdr_log3:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        The header of the TLP that contained the error indicated app_err_bus, valid when 1. app_hdr_valid is asserted. [127:96]
        */
    };
} acc_etc_rw44_0_t;
/* offset : 0x134 */
typedef volatile union _acc_etc_rw45_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int app_tlp_prfx_log0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        End-End TLP prefixes of the TLP that contained an error. Valid when 1. app_hdr_valid is asserted. The header of the TLP that contained the error indicated on app_err_bus. [31:0]
        */
    };
} acc_etc_rw45_0_t;
/* offset : 0x138 */
typedef volatile union _acc_etc_rw46_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int app_tlp_prfx_log1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        End-End TLP prefixes of the TLP that contained an error. Valid when 1. app_hdr_valid is asserted. The header of the TLP that contained the error indicated on app_err_bus. [63:32]
        */
    };
} acc_etc_rw46_0_t;
/* offset : 0x13c */
typedef volatile union _acc_etc_rw47_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int app_tlp_prfx_log2:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        End-End TLP prefixes of the TLP that contained an error. Valid when 1. app_hdr_valid is asserted. The header of the TLP that contained the error indicated on app_err_bus. [95:64]
        */
    };
} acc_etc_rw47_0_t;
/* offset : 0x140 */
typedef volatile union _acc_etc_rw48_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int app_tlp_prfx_log3:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        End-End TLP prefixes of the TLP that contained an error. Valid when 1. app_hdr_valid is asserted. The header of the TLP that contained the error indicated on app_err_bus. [127:96]
        */
    };
} acc_etc_rw48_0_t;
/* offset : 0x144 */
typedef volatile union _acc_etc_rw49_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int app_err_bus:27; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        The type of error that your application detected. The controller combines the values the app_err_bus bits with the internally-detected error signals to set the corresponding bit in the Uncorrectable or Correctable Error Status Registers:
        */
        volatile unsigned int app_err_advisory:1; /* [Position:27] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Indicates that your application error is an advisory error. Your application should assert app_err_advisory under either of the following conditions: 
- The controller is configured to mask completion timeout errors, your application is reporting a completion timeout error app_err_bus, and your application intends to resend the request. In such cases the error is an advisory error, as described in PCI Express Specification. When your application does not intend to resend the request, then your application must keep app_err_advisory de-asserted when reporting a completion timeout error.
- The controller is configured to forward poisoned TLPs to your appli_x0002_cation and your application is going to treat the poisoned TLP as a normal TLP, as described in PCI Express Specification. Upon receipt of a poisoned TLP, your application must report the error app_er_x0002_r_bus, and either assert app_err_advisory (to indicate an advisory error) or de-assert app_err_advisory (to indicate that your applica_x0002_tion is dropping the TLP).
        */
        volatile unsigned int app_err_func_num:1; /* [Position:28] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        The number of the function that is reporting the error indicated app_err_bus, valid when app_hdr_valid is asserted.
        */
        volatile unsigned int RESERVED_29:3; /* [Position:29] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw49_0_t;
/* offset : 0x150 */
typedef volatile union _acc_etc_rw52_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int app_l1sub_disable:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        The application can set this input to 1'b1 to prevent entry to L1 Sub-states. This pin is used to gate the L1 sub-state enable bits from the L1 PM Substates Control 1 Register.
        */
        volatile unsigned int RESERVED_1:31; /* [Position:1] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw52_0_t;
/* offset : 0x154 */
typedef volatile union _acc_etc_ro000_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int pf_frs_grant:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicator of when an FRS message for this function has been scheduled for transmission.
        */
        volatile unsigned int RESERVED_1:31; /* [Position:1] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro000_0_t;
/* offset : 0x168 */
typedef volatile union _acc_etc_ro005_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_ats_stu:5; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Contents of the "Smallest Translation Unit" field (ATU) in the ATS_CAPABILITIES_CTRL_REG register.
        */
        volatile unsigned int cfg_ats_cache_en:1; /* [Position:5] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Contents of the "Cache Enable" (ENABLE) field in the ATS_CAPABILITIES_CTRL_REG register.
        */
        volatile unsigned int cfg_prs_enable:1; /* [Position:6] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that the Page Request Interface is allowed to make page requests. This output pin reflects the value of the PRS_ENABLE field in the PRS_CONTROL_STATUS_REG register.
        */
        volatile unsigned int cfg_prs_reset:1; /* [Position:7] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Pulse to indicate to the application that the associated page request credit counter and pending request state must be cleared. This output pin reflects the value of the PRS_RESET field in the PRS_CONTROL_STATUS_REG register.
        */
        volatile unsigned int RESERVED_8:24; /* [Position:8] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro005_0_t;
/* offset : 0x16c */
typedef volatile union _acc_etc_ro006_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_prs_stopped:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        This output pin reflects the value of the PRS_STOPPED field in the Page Request Status Register (PRS_CONTROL_STATUS_REG) register.
        */
        volatile unsigned int cfg_prs_uprgi:1; /* [Position:1] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        This output pin reflects the value of the PRS_UPRGI field in the Page Request Status (PRS_CONTROL_STATUS_REG) register.
        */
        volatile unsigned int cfg_prs_response_failure:1; /* [Position:2] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        This output pin reflects the value of the PRS_RESP_FAILURE field in the Page Request Status (PRS_CONTROL_STATUS_REG) register.
        */
        volatile unsigned int cfg_hp_slot_ctrl_access:1; /* [Position:3] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Slot Control Accessed.
        */
        volatile unsigned int cfg_dll_state_chged_en:1; /* [Position:4] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Slot Control DLL State Change Enable.
        */
        volatile unsigned int cfg_cmd_cpled_int_en:1; /* [Position:5] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Slot Control Command Completed Interrupt Enable.
        */
        volatile unsigned int cfg_hp_int_en:1; /* [Position:6] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Slot Control Hot Plug Interrupt Enable.
        */
        volatile unsigned int cfg_pre_det_chged_en:1; /* [Position:7] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Slot Control Presence Detect Changed Enable.
        */
        volatile unsigned int RESERVED_8:24; /* [Position:8] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro006_0_t;
/* offset : 0x170 */
typedef volatile union _acc_etc_ro007_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_prs_outstanding_allocation0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Signal to indicate the number of outstanding page request messages the associated Page Request Interface is allowed to issue. This output pin reflects the value of the PRS_OUTSTANDING_ALLOCATION field in the PRS_REQ_ALLOCATION_REG register. [31:0]
        */
    };
} acc_etc_ro007_0_t;
/* offset : 0x180 */
typedef volatile union _acc_etc_ro011_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_mrl_sensor_chged_en:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Slot Control MRL Sensor Changed Enable.
        */
        volatile unsigned int cfg_pwr_fault_det_en:1; /* [Position:1] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Slot Control Power Fault Detect Enable.
        */
        volatile unsigned int cfg_atten_button_pressed_en:1; /* [Position:2] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Slot Control Attention Button Pressed Enable.
        */
        volatile unsigned int local_ref_clk_req_n:1; /* [Position:3] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        This signal may be connected to the CLKREQ# driver to negotiate entry into L1 sub-states. When this signal is set to 1 the controller is requesting entry into L1 sub-states and CLKREQ# may be de-asserted.
        */
        volatile unsigned int cfg_hw_auto_sp_dis:1; /* [Position:4] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Autonomous speed disable. Used in downstream ports only.
        */
        volatile unsigned int RESERVED_5:27; /* [Position:5] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro011_0_t;
/* offset : 0x184 */
typedef volatile union _acc_etc_ro012_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mstr_awmisc_info0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Write Misc Information. AXI master write transaction associated miscellaneous information from the TLP received by the native PCIe controller. [31:0]
        */
    };
} acc_etc_ro012_0_t;
/* offset : 0x188 */
typedef volatile union _acc_etc_ro013_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mstr_awmisc_info1:16; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Write Misc Information. AXI master write transaction associated miscellaneous information from the TLP received by the native PCIe controller. [63:32]
        */
        volatile unsigned int RESERVED_16:16; /* [Position:16] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro013_0_t;
/* offset : 0x18c */
typedef volatile union _acc_etc_ro014_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mstr_awmisc_info_hdr_34dw0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master 3rd and 4th header DW. The bus conveys the 3rd and 4th header DWs in a PCIe Msg/MsgD to the application. [31:0]
        */
    };
} acc_etc_ro014_0_t;
/* offset : 0x190 */
typedef volatile union _acc_etc_ro015_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mstr_awmisc_info_hdr_34dw1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master 3rd and 4th header DW. The bus conveys the 3rd and 4th header DWs in a PCIe Msg/MsgD to the application. [63:32]
        */
    };
} acc_etc_ro015_0_t;
/* offset : 0x194 */
typedef volatile union _acc_etc_ro016_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mstr_awmisc_info_ep:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Write mapping to the EP bit in the PCIe protocol domain.
        */
        volatile unsigned int mstr_awmisc_info_last_dcmp_tlp:1; /* [Position:1] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Write Request last TLP bit. The signal indicates that the TLP in transaction is the last TLP of the write request.
        */
        volatile unsigned int mstr_awmisc_info_dma:5; /* [Position:2] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        DMA Bits of the AXI Write Master Transaction.
        */
        volatile unsigned int mstr_awmisc_info_nw:1; /* [Position:7] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        ATS No Write (NW) Bit of the AXI Master Write Transaction Request.
        */
        volatile unsigned int mstr_awmisc_info_ats:2; /* [Position:8] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AT Bits of the AXI Master Write Transaction Request.
        */
        volatile unsigned int mstr_awmisc_info_tph:11; /* [Position:10] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Write Request TLP Processing Hints. The bits are mapped as follows:  [0]: TH (TLP Processing Hint present) 
[2:1] PH (TLP Processing Hint)
[10:3] ST (Steering Tag)
        */
        volatile unsigned int mstr_awmisc_info_tlpln:1; /* [Position:21] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Write Request TLP Lightweight Notification (LN) bit.
        */
        volatile unsigned int mstr_awmisc_info_ido:1; /* [Position:22] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        IDO Bit of the AXI Master Write Transaction. Enables ID-base ordering on inbound requests.
        */
        volatile unsigned int RESERVED_23:9; /* [Position:23] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro016_0_t;
/* offset : 0x198 */
typedef volatile union _acc_etc_ro017_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mstr_awmisc_info_tlpprfx0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Write Request TLP Prefixes. The bits [31:0] represent the first prefix received. [31:0]
        */
    };
} acc_etc_ro017_0_t;
/* offset : 0x19c */
typedef volatile union _acc_etc_ro018_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mstr_awmisc_info_tlpprfx1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Write Request TLP Prefixes. The bits [31:0] represent the first prefix received. [63:32]
        */
    };
} acc_etc_ro018_0_t;
/* offset : 0x1a0 */
typedef volatile union _acc_etc_ro019_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mstr_awmisc_info_tlpprfx2:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Write Request TLP Prefixes. The bits [31:0] represent the first prefix received. [95:64]
        */
    };
} acc_etc_ro019_0_t;
/* offset : 0x1a4 */
typedef volatile union _acc_etc_ro020_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mstr_awmisc_info_tlpprfx3:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Write Request TLP Prefixes. The bits [31:0] represent the first prefix received. [127:96]
        */
    };
} acc_etc_ro020_0_t;
/* offset : 0x1a8 */
typedef volatile union _acc_etc_ro021_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mstr_awmisc_info_tlpprfx4:16; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Write Request TLP Prefixes. The bits [31:0] represent the first prefix received. [143:128]
        */
        volatile unsigned int RESERVED_16:16; /* [Position:16] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro021_0_t;
/* offset : 0x1ac */
typedef volatile union _acc_etc_ro022_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mstr_armisc_info0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Read Transaction Associated Misc Information (from the TLP received by the native PCIe controller). [31:0]
        */
    };
} acc_etc_ro022_0_t;
/* offset : 0x1b0 */
typedef volatile union _acc_etc_ro023_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mstr_armisc_info1:16; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Read Transaction Associated Misc Information (from the TLP received by the native PCIe controller). [63:32]
        */
        volatile unsigned int RESERVED_16:16; /* [Position:16] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro023_0_t;
/* offset : 0x1b4 */
typedef volatile union _acc_etc_ro024_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mstr_armisc_info_last_dcmp_tlp:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Read Request last TLP bit. The signal indicates that the TLP in transaction is the last TLP of the read request.
        */
        volatile unsigned int mstr_armisc_info_dma:5; /* [Position:1] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        DMA bits of the AXI Read Master Transaction.
        */
        volatile unsigned int mstr_armisc_info_nw:1; /* [Position:6] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        ATS No Write (NW) Bit of the AXI Master Read Transaction Request.
        */
        volatile unsigned int mstr_armisc_info_ats:2; /* [Position:7] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        ATS Bits of the AXI Master Read Transaction Request.
        */
        volatile unsigned int mstr_armisc_info_tph:11; /* [Position:9] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Read Request TLP Processing Hints. The bits are mapped as follows: 
[0]: TH (TLP Processing Hint present) 
[2:1] PH (TLP Processing Hint) 
[10:3] ST (Steering Tag)
        */
        volatile unsigned int RESERVED_20:12; /* [Position:20] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro024_0_t;
/* offset : 0x1b8 */
typedef volatile union _acc_etc_ro025_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mstr_armisc_info_tlpprfx0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Read Request TLP Prefixes. The field [31:0] represents the first prefix received. [31:0]
        */
    };
} acc_etc_ro025_0_t;
/* offset : 0x1bc */
typedef volatile union _acc_etc_ro026_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mstr_armisc_info_tlpprfx1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Read Request TLP Prefixes. The field [31:0] represents the first prefix received. [63:32]
        */
    };
} acc_etc_ro026_0_t;
/* offset : 0x1c0 */
typedef volatile union _acc_etc_ro027_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mstr_armisc_info_tlpprfx2:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Read Request TLP Prefixes. The field [31:0] represents the first prefix received. [95:64]
        */
    };
} acc_etc_ro027_0_t;
/* offset : 0x1c4 */
typedef volatile union _acc_etc_ro028_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mstr_armisc_info_tlpprfx3:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Read Request TLP Prefixes. The field [31:0] represents the first prefix received. [127:96]
        */
    };
} acc_etc_ro028_0_t;
/* offset : 0x1c8 */
typedef volatile union _acc_etc_ro029_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mstr_armisc_info_tlpprfx4:16; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Read Request TLP Prefixes. The field [31:0] represents the first prefix received. [143:128]
        */
        volatile unsigned int RESERVED_16:16; /* [Position:16] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro029_0_t;
/* offset : 0x1cc */
typedef volatile union _acc_etc_ro030_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mstr_armisc_info_tlpln:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Read Request TLP Lightweight Notification (LN) bit.
        */
        volatile unsigned int mstr_armisc_info_ido:1; /* [Position:1] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        IDO bit of the AXI Master Read Transaction. Enables ID-base ordering on inbound requests.
        */
        volatile unsigned int mstr_armisc_info_zeroread:1; /* [Position:2] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Read mapping to a zero length read in the PCIe protocol domain.
        */
        volatile unsigned int slv_bmisc_info_tlpln:1; /* [Position:3] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Slave Write Response TLP Lightweight Notification (LN) bit.
        */
        volatile unsigned int slv_bmisc_info:11; /* [Position:4] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Associated Misc Information of the AXI Slave Write Transaction Response.
[0]: Completion timeout bit [1]: EP bit of the TLP
[4:2]: TC bits of the TLP [5]: NS bit of the TLP
[6]: RO bit of the TLP [9:7]: Completion status field of the TLP
[10]: Non-posted request
[13:11]: Function number of the TLP. This field [13:11] is only present 
when multifunction support (CX_NFUNC > 1) is enabled.
        */
        volatile unsigned int slv_rmisc_info:11; /* [Position:15] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Associated Miscellaneous Information of the AXI Slave Read Transaction Response.
[0]: Completion timeout bit [1]: EP bit of the TLP
[4:2]: TC bits of the TLP [5]: NS bit of the TLP
[6]: RO bit of the TLP [9:7]: Completion status field of the TLP
[10]: Non-posted request. 
[13:11]: Function number of the TLP. The bits [13:11] are present 
only when multifunction support (CX_NFUNC > 1) is enabled.
        */
        volatile unsigned int RESERVED_26:6; /* [Position:26] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro030_0_t;
/* offset : 0x1d0 */
typedef volatile union _acc_etc_ro031_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int slv_rmisc_info_tlpln:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Slave Read Response TLP Lightweight Notification (LN).
        */
        volatile unsigned int slv_rmisc_info_ido:1; /* [Position:1] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        IDO bit of the AXI Slave Write Transaction Response. Indicates if the response should make use of ID-based ordering.
        */
        volatile unsigned int radm_trgt1_vc:3; /* [Position:2] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Virtual Channel Number of a received TLP on TRGT1. 
[2:0]: Virtual Channel number
        */
        volatile unsigned int RESERVED_5:27; /* [Position:5] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro031_0_t;
/* offset : 0x1d4 */
typedef volatile union _acc_etc_ro032_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int lbc_ext_addr:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Address bus to the external register block. The width of the address bus is the value you select for the CX_LBC_EXT_AW parameter.
        */
    };
} acc_etc_ro032_0_t;
/* offset : 0x1d8 */
typedef volatile union _acc_etc_ro033_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int lbc_ext_dout:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Write data bus to the external register block, driven to all functions in a multi-function configuration.
        */
    };
} acc_etc_ro033_0_t;
/* offset : 0x1dc */
typedef volatile union _acc_etc_ro034_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int lbc_ext_cs:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The controller asserts lbc_ext_cs when a received TLP for a read or write request has an address in the range of your application device, as determined by the BAR configuration.
        */
        volatile unsigned int lbc_ext_wr:4; /* [Position:1] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates when the external register access is a read or a write. For writes, lbc_ext_wr also indicates the byte enables.
        */
        volatile unsigned int lbc_ext_dbi_access:1; /* [Position:5] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that the current ELBI access is initiated using DBI.
        */
        volatile unsigned int lbc_ext_rom_access:1; /* [Position:6] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that the current ELBI access is for expansion ROM.
        */
        volatile unsigned int lbc_ext_io_access:1; /* [Position:7] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that the current ELBI access is an I/O access.
        */
        volatile unsigned int lbc_ext_bar_num:3; /* [Position:8] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The BAR number of the current ELBI access: 
000b: BAR 0, 001b: BAR 1, 010b: BAR 2, 011b: BAR 3 
100b: BAR 4, 101b: BAR 5, 110b is not used. 
111b is used to indicate a configuration access. This is designed for an application with application-specific configuration registers such as vendor capability. lbc_ext_bar_num is set to 3'b111 for all DBI accesses.
        */
        volatile unsigned int RESERVED_11:21; /* [Position:11] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro034_0_t;
/* offset : 0x1e0 */
typedef volatile union _acc_etc_ro035_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_tph_req_en:2; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The 2-bit TPH Requester Enabled field of each TPH Requester Control register.
        */
        volatile unsigned int cfg_pf_pasid_en:1; /* [Position:2] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The value of the PASID Enable field in each PF PASID Control Register.
        */
        volatile unsigned int cfg_pf_pasid_execute_perm_en:1; /* [Position:3] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The value of the Execute Permission Enable field in each PF PASID Control Register.
        */
        volatile unsigned int cfg_pf_pasid_priv_mode_en:1; /* [Position:4] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The value of the Privileged Mode Enable field in each PF PASID Control Register.
        */
        volatile unsigned int dpa_substate_update:1; /* [Position:5] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        DPA Substate-Updated Indication Signal Bus. The controller asserts it for one core_clk cycle when the Substate Control field of the DPA Control Register has been updated with a new value for the associated function. The controller asserts it when the updated value of the Substate Control field does not match the Substate Status field of the Status Register when the Substate Control Enable bit field of the Status Register is set to "1".
        */
        volatile unsigned int cfg_ltr_m_en:1; /* [Position:6] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The LTR Mechanism Enable field of the Device Control 2 register of function 0.
        */
        volatile unsigned int app_ltr_msg_grant:1; /* [Position:7] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that the controller has accepted your request to send an LTR message.
        */
        volatile unsigned int RESERVED_8:24; /* [Position:8] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro035_0_t;
/* offset : 0x1e4 */
typedef volatile union _acc_etc_ro036_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_ltr_max_latency0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The concatenated contents of 'LTR Max No-Snoop Latency Register' and 'LTR Max Snoop Latency Register'. The 'LTR Max Snoop Latency ' of function 0 occupies the lower 16-bits. [31:0]
        */
    };
} acc_etc_ro036_0_t;
/* offset : 0x1ec */
typedef volatile union _acc_etc_ro038_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int app_ltr_latency:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The current LTR values reported and in-use by the downstream device.
        */
    };
} acc_etc_ro038_0_t;
/* offset : 0x1f0 */
typedef volatile union _acc_etc_ro039_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_disable_ltr_clr_msg:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Disable the autonomous generation of LTR clear message.
        */
        volatile unsigned int ven_msg_grant:1; /* [Position:1] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        One-cycle pulse that indicates that the controller has accepted the request to send the vendor-defined Message.
        */
        volatile unsigned int RESERVED_2:30; /* [Position:2] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro039_0_t;
/* offset : 0x204 */
typedef volatile union _acc_etc_ro044_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int pm_xtlh_block_tlp:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that your application must stop generating new outgoing request TLPs due to the current power management state.
        */
        volatile unsigned int radm_q_not_empty:1; /* [Position:1] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Level indicating that the receive queues contain TLP header/data.
        */
        volatile unsigned int RESERVED_2:30; /* [Position:2] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro044_0_t;
/* offset : 0x208 */
typedef volatile union _acc_etc_ro045_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int radm_qoverflow:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Pulse indicating that one or more of the P/NP/CPL receive queues have overflowed. There is a 1-bit indication for each configured virtual channel.
        */
        volatile unsigned int RESERVED_1:31; /* [Position:1] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro045_0_t;
/* offset : 0x20c */
typedef volatile union _acc_etc_ro046_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar0_start0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 0 (a memory or I/O BAR). There are 64 bits of cfg_bar0_start assigned to each configured function. The actual BAR start address present any 64-bit segment can be either a 64-bit or 32-bit BAR start address. [31:0]
        */
    };
} acc_etc_ro046_0_t;
/* offset : 0x210 */
typedef volatile union _acc_etc_ro047_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar0_start1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 0 (a memory or I/O BAR). There are 64 bits of cfg_bar0_start assigned to each configured function. The actual BAR start address present any 64-bit segment can be either a 64-bit or 32-bit BAR start address. [63:32]
        */
    };
} acc_etc_ro047_0_t;
/* offset : 0x22c */
typedef volatile union _acc_etc_ro054_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar0_limit0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 0 (a memory or I/O BAR). There are 64 bits of cfg_bar0_limit assigned to each configured function. The actual BAR end address present any 64-bit segment can be either a 64-bit or 32-bit BAR end address. [31:0]
        */
    };
} acc_etc_ro054_0_t;
/* offset : 0x230 */
typedef volatile union _acc_etc_ro055_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar0_limit1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 0 (a memory or I/O BAR). There are 64 bits of cfg_bar0_limit assigned to each configured function. The actual BAR end address present any 64-bit segment can be either a 64-bit or 32-bit BAR end address. [63:32]
        */
    };
} acc_etc_ro055_0_t;
/* offset : 0x24c */
typedef volatile union _acc_etc_ro062_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar1_start0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 1 (a memory or I/O BAR). There are 32 bits of cfg_bar1_start assigned to each configured function. [31:0]
        */
    };
} acc_etc_ro062_0_t;
/* offset : 0x25c */
typedef volatile union _acc_etc_ro066_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar1_limit4:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 1 (a memory or I/O BAR). There are 32 bits of cfg_bar1_start assigned to each configured function. [159:128]
        */
    };
} acc_etc_ro066_0_t;
/* offset : 0x26c */
typedef volatile union _acc_etc_ro070_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar2_start0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 2 (a memory or I/O BAR). There are 64 bits of cfg_bar2_start assigned to each configured function. The actual BAR start address present any 64-bit segment can be either a 64-bit or 32-bit BAR start address. [31:0]
        */
    };
} acc_etc_ro070_0_t;
/* offset : 0x270 */
typedef volatile union _acc_etc_ro071_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar2_start1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 2 (a memory or I/O BAR). There are 64 bits of cfg_bar2_start assigned to each configured function. The actual BAR start address present any 64-bit segment can be either a 64-bit or 32-bit BAR start address. [63:32]
        */
    };
} acc_etc_ro071_0_t;
/* offset : 0x28c */
typedef volatile union _acc_etc_ro078_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar2_limit0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 2 (a memory or I/O BAR). There are 64 bits of cfg_bar2_limit assigned to each configured function. The actual BAR end address present any 64-bit segment can be either a 64-bit or 32-bit BAR end address. [31:0]
        */
    };
} acc_etc_ro078_0_t;
/* offset : 0x290 */
typedef volatile union _acc_etc_ro079_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar2_limit1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 2 (a memory or I/O BAR). There are 64 bits of cfg_bar2_limit assigned to each configured function. The actual BAR end address present any 64-bit segment can be either a 64-bit or 32-bit BAR end address. [63:32]
        */
    };
} acc_etc_ro079_0_t;
/* offset : 0x2ac */
typedef volatile union _acc_etc_ro086_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar3_start0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 3 (a memory or I/O BAR). There are 32 bits of cfg_bar3_start assigned to each configured function.[31:0]
        */
    };
} acc_etc_ro086_0_t;
/* offset : 0x2bc */
typedef volatile union _acc_etc_ro090_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar3_limit0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 3 (a memory or I/O BAR). There are 32 bits of cfg_bar3_limit assigned to each configured function.[31:0]
        */
    };
} acc_etc_ro090_0_t;
/* offset : 0x2cc */
typedef volatile union _acc_etc_ro094_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar4_start0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 4 (a memory or I/O BAR). There are 64 bits of cfg_bar4_start assigned to each configured function. The actual BAR start address present any 64-bit segment can be either a 64-bit or 32-bit BAR start address. [31:0]
        */
    };
} acc_etc_ro094_0_t;
/* offset : 0x2d0 */
typedef volatile union _acc_etc_ro095_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar4_start1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 4 (a memory or I/O BAR). There are 64 bits of cfg_bar4_start assigned to each configured function. The actual BAR start address present any 64-bit segment can be either a 64-bit or 32-bit BAR start address. [63:32]
        */
    };
} acc_etc_ro095_0_t;
/* offset : 0x2ec */
typedef volatile union _acc_etc_ro102_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar4_limit0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 4 (a memory or I/O BAR). There are 64 bits of cfg_bar4_limit assigned to each configured function. The actual BAR end address present any 64-bit segment can be either a 64-bit or 32-bit BAR end address. [31:0]
        */
    };
} acc_etc_ro102_0_t;
/* offset : 0x2f0 */
typedef volatile union _acc_etc_ro103_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar4_limit1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 4 (a memory or I/O BAR). There are 64 bits of cfg_bar4_limit assigned to each configured function. The actual BAR end address present any 64-bit segment can be either a 64-bit or 32-bit BAR end address. [63:32]
        */
    };
} acc_etc_ro103_0_t;
/* offset : 0x30c */
typedef volatile union _acc_etc_ro110_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar5_start0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 5 (a memory or I/O BAR). There are 32 bits of cfg_bar5_start assigned to each configured function.[31:0]
        */
    };
} acc_etc_ro110_0_t;
/* offset : 0x31c */
typedef volatile union _acc_etc_ro114_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar5_limit0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 5 (a memory or I/O BAR). There are 32 bits of cfg_bar5_limit assigned to each configured function.[31:0]
        */
    };
} acc_etc_ro114_0_t;
/* offset : 0x32c */
typedef volatile union _acc_etc_ro118_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_exp_rom_start0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of expansion ROM. There are 32 bits of cfg_exp_rom_start assigned to each configured function.[31:0]
        */
    };
} acc_etc_ro118_0_t;
/* offset : 0x33c */
typedef volatile union _acc_etc_ro122_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_exp_rom_limit0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of expansion ROM. There are 32 bits of cfg_exp_rom_limit assigned to each configured function.[31:0]
        */
    };
} acc_etc_ro122_0_t;
/* offset : 0x34c */
typedef volatile union _acc_etc_ro126_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bus_master_en:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The state of the bus master enable bit in the PCI-compatible Command register. There is 1 bit of cfg_bus_master_en assigned to each configured function.
        */
        volatile unsigned int cfg_rcb:1; /* [Position:1] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The value of the RCB bit in the Link Control register. There is 1 bit of cfg_rcb assigned to each configured function.
        */
        volatile unsigned int cfg_max_payload_size:3; /* [Position:2] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The value of the Max_Payload_Size field in the Device Control register. There are 3 bits of cfg_max_payload_size assigned to each configured function.
        */
        volatile unsigned int RESERVED_5:27; /* [Position:5] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro126_0_t;
/* offset : 0x364 */
typedef volatile union _acc_etc_ro132_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_mem_space_en:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The state of the Memory Space Enable bit in the PCI-compatible Command register. There is 1 bit of cfg_mem_space_en assigned to each configured function.
        */
        volatile unsigned int cfg_max_rd_req_size:3; /* [Position:1] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The value of the Max_Read_Request_Size field in the Device Control register. There are 3 bits of cfg_max_rd_req_size assigned to each configured function.
        */
        volatile unsigned int cfg_reg_serren:1; /* [Position:4] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        PF's SERR# Enable registers value in Command Register of Type0 Header, for setting error status registers of external VFs.
        */
        volatile unsigned int cfg_cor_err_rpt_en:1; /* [Position:5] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        PF's Correctable Error Reporting Enable registers value in Device Control Register of PCIe Capability, for sending ERR_MSG of external VFs.
        */
        volatile unsigned int cfg_nf_err_rpt_en:1; /* [Position:6] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        PF's Non-Fatal Error Reporting Enable registers value in Device Control Register of PCIe Capability, for sending ERR_MSG of external VFs.
        */
        volatile unsigned int cfg_f_err_rpt_en:1; /* [Position:7] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        PF's Fatal Error Reporting Enable registers value in Device Control Register of PCIe Capability, for sending ERR_MSG of external VFs.
        */
        volatile unsigned int RESERVED_8:24; /* [Position:8] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro132_0_t;
/* offset : 0x368 */
typedef volatile union _acc_etc_ro133_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int rdlh_link_up:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Data link layer up/down indicator: This status from the flow control initialization state machine indicates that flow control has been initiated and the Data link layer is ready to transmit and receive packets. For multi-VC designs, this signal indicates status for VC0 only. - 1: Link is up - 0: Link is down
        */
        volatile unsigned int smlh_ltssm_state:6; /* [Position:1] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Current state of the LTSSM. 
        */
        volatile unsigned int pm_curnt_state:3; /* [Position:7] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates the current power state. The pm_curnt_state output is intended for debugging purposes, not for system operation.
        */
        volatile unsigned int smlh_ltssm_state_rcvry_eq:1; /* [Position:10] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        This status signal is asserted during all Recovery Equalization states.
        */
        volatile unsigned int smlh_link_up:1; /* [Position:11] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        PHY Link up/down indicator: - 1: Link is up - 0: Link is down
        */
        volatile unsigned int smlh_req_rst_not:1; /* [Position:12] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Early version of the link_req_rst_not signal.
        */
        volatile unsigned int link_req_rst_not:1; /* [Position:13] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Reset request because the link has gone down or the controller received a hot-reset request. A low level indicates that the controller is requesting external logic to reset the controller because the PHY link is down.
        */
        volatile unsigned int brdg_slv_xfer_pending:1; /* [Position:14] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Slave non-DBI transfer pending status. Indicates AXI non-DBI Slave Read or Write transfers are pending, that is, AXI Slave transfers are awaiting a response from the controller.
        */
        volatile unsigned int brdg_dbi_xfer_pending:1; /* [Position:15] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Slave DBI transfer pending status. Indicates AXI DBI Slave Read or Write transfers are pending, that is, AXI Slave transfers are awaiting a response from the controller.
        */
        volatile unsigned int edma_xfer_pending:1; /* [Position:16] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        eDMA transfer pending status. Indicates eDMA Write or Read Channel transfers are pending, that is, DMA Write or Read Channels have not finished transferring data.
        */
        volatile unsigned int radm_xfer_pending:1; /* [Position:17] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Receive request pending status. Indicates Receive TLP requests are pending, that is, requests sent to the RTRGT1 or RTRGT0 interfaces are awaiting a response from your application.
        */
        volatile unsigned int RESERVED_18:14; /* [Position:18] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro133_0_t;
/* offset : 0x36c */
typedef volatile union _acc_etc_ro134_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int smlh_ltssm_state:6;        /* [Position:0] */
        /* : smlh_ltssm_state: LTSSM current state. Encoding is same as the dedicated smlh_ltssm_state output. */
        volatile unsigned int mac_phy_txdatak:2;        /* [Position:6] */
        /* : mac_phy_txdatak: PIPE transmit K indication */
        volatile unsigned int mac_phy_txdata:16;        /* [Position:8] */
        /* : mac_phy_txdata: PIPE Transmit data */
        volatile unsigned int rplh_rcvd_idle1:1;        /* [Position:24] */
        /* : rplh_rcvd_idle[1]: 2n symbol is also idle (16bit PHY interface only) */
        volatile unsigned int rplh_rcvd_idle0:1;        /* [Position:25] */
        /* : rplh_rcvd_idle[0]: Receiver is receiving logical idle */
        volatile unsigned int smlh_ts_link_num_is_k237:1;        /* [Position:26] */
        /* : smlh_ts_link_num_is_k237: Currently receiving k237 (PAD) in place of link number */
        volatile unsigned int smlh_ts_lane_num_is_k237:1;        /* [Position:27] */
        /* : smlh_ts_lane_num_is_k237: Currently receiving k237 (PAD) in place of lane number */
        volatile unsigned int smlh_ts_link_ctrl:4;        /* [Position:28] */
        /* : smlh_ts_link_ctrl: Link control bits advertised by link partner */
    };
} acc_etc_ro134_0_t;
/* offset : 0x370 */
typedef volatile union _acc_etc_ro135_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int smlh_rcvd_lane_rev:1;        /* [Position:0] */
        /* : smlh_rcvd_lane_rev: Receiver detected lane reversal */
        volatile unsigned int smlh_ts2_rcvd:1;        /* [Position:1] */
        /* : smlh_ts2_rcvd: TS2 training sequence received (pulse) */
        volatile unsigned int smlh_ts1_rcvd:1;        /* [Position:2] */
        /* : smlh_ts1_rcvd: TS1 training sequence received (pulse) */
        volatile unsigned int smlh_inskip_rcv:1;        /* [Position:3] */
        /* : smlh_inskip_rcv: Receiver reports skip reception */
        volatile unsigned int smlh_link_up:1;        /* [Position:4] */
        /* : smlh_link_up: LTSSM reports PHY link up or LTSSM is in Loopback.Active for Loopback Master */
        volatile unsigned int xmtbyte_skip_sent:1;        /* [Position:5] */
        /* : xmtbyte_skip_sent: A skip ordered set has been transmitted */
        volatile unsigned int Reserved_6:2;        /* [Position:6] */
        /* : 00b: Reserved */
        volatile unsigned int rmlh_ts_link_num:8;        /* [Position:8] */
        /* : rmlh_ts_link_num: Link number advertised/confirmed by link partner */
        volatile unsigned int Reserved_16:3;        /* [Position:16] */
        /* : 000b: Reserved */
        volatile unsigned int app_init_rst:1;        /* [Position:19] */
        /* : app_init_rst: Application request to initiate training reset */
        volatile unsigned int mac_phy_txcompliance0:1;        /* [Position:20] */
        /* : mac_phy_txcompliance[0]: PIPE transmit compliance request */
        volatile unsigned int mac_phy_txelecidle0:1;        /* [Position:21] */
        /* : mac_phy_txelecidle[0]: PIPE transmit electrical idle request */
        volatile unsigned int mac_phy_txdetectrx_loopback:1;        /* [Position:22] */
        /* : mac_phy_txdetectrx_loopback: PIPE receiver detect/loopback request */
        volatile unsigned int Reserved_23:4;        /* [Position:23] */
        /* : 0000b: Reserved */
        volatile unsigned int smlh_training_rst_n:1;        /* [Position:27] */
        /* : smlh_training_rst_n: LTSSM-negotiated link reset */
        volatile unsigned int smlh_ltssm_in_pollconfig:1;        /* [Position:28] */
        /* : smlh_ltssm_in_pollconfig: LTSSM is in Polling.Configuration state */
        volatile unsigned int smlh_link_in_training:1;        /* [Position:29] */
        /* : smlh_link_in_training: LTSSM performing link training */
        volatile unsigned int smlh_link_disable:1;        /* [Position:30] */
        /* : smlh_link_disable: TSSM in DISABLE state. Link inoperable */
        volatile unsigned int smlh_scrambler_disable:1;        /* [Position:31] */
        /* : smlh_scrambler_disable: Scrambling disabled for the link */
    };
} acc_etc_ro135_0_t;
/* offset : 0x374 */
typedef volatile union _acc_etc_ro136_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int bit0:1;        /* [Position:0] */
        /* : EIOS detected Group 2 (level) - LTSSM is in one of the states that depend rxelecidle =0: */
        volatile unsigned int bit1:1;        /* [Position:1] */
        /* : L1 */
        volatile unsigned int bit2:1;        /* [Position:2] */
        /* : L2 */
        volatile unsigned int bit3:1;        /* [Position:3] */
        /* : RxL0s */
        volatile unsigned int bit4:1;        /* [Position:4] */
        /* : Disabled */
        volatile unsigned int bit5:1;        /* [Position:5] */
        /* : Detect.Quiet */
        volatile unsigned int bit6:1;        /* [Position:6] */
        /* : Polling.Active */
        volatile unsigned int bit7:1;        /* [Position:7] */
        /* : Polling.Compliance Group 3 (level) - LTSSM is in one of the states that depend rxelecidle =1: */
        volatile unsigned int bit8:1;        /* [Position:8] */
        /* : LTSSM is in a transitory state prior to L1 or L2 */
        volatile unsigned int bit9:1;        /* [Position:9] */
        /* : LTSSM is in a transitory state prior to Disabled */
        volatile unsigned int bit10:1;        /* [Position:10] */
        /* : LTSSM is in Loopback.Active as a Slave at Gen1 */
        volatile unsigned int bit11:1;        /* [Position:11] */
        /* : LTSSM is in Polling.Active Group 4 (pulse) - LTSSM state transitions with EI inferred: */
        volatile unsigned int bit12:1;        /* [Position:12] */
        /* : LTSSM enters Recovery from L0 with EI inferred, first row in base spec Table 4-11 */
        volatile unsigned int bit13:1;        /* [Position:13] */
        /* : LTSSM enters Recovery.Speed from Recovery.RcvrCfg with EI inferred, second row in Table 4-11 of PCI Express  Specification */
        volatile unsigned int bit14:1;        /* [Position:14] */
        /* : EI inferred while LTSSM in Recovery.Speed, third/fourth rows in base spec Table 4-11 */
        volatile unsigned int bit15:1;        /* [Position:15] */
        /* : EI inferred while LTSSM */
        volatile unsigned int training_rst_n:1; /* [Position:16] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Hot reset from upstream component. When the controller LTSSM receives two consecutive TS1 ordered sets with the hot_reset bit asserted, it asserts training_rst_n for one clock cycle.
        */
        volatile unsigned int radm_pm_turnoff:1; /* [Position:17] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        One-clock-cycle pulse that indicates that the controller received a PME Turnoff message. When FX_TLP > 1 and when two messages of the same type are received in the same clock cycle (back-to-back), then no indication is given for the second message.
        */
        volatile unsigned int radm_msg_unlock:1; /* [Position:18] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        One-cycle pulse that indicates that the controller received an Unlock message. When FX_TLP > 1 and when two messages of the same type are received in the same clock cycle (back-to-back), then no separate indication is given for the second message.
        */
        volatile unsigned int RESERVED_19:13; /* [Position:19] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro136_0_t;
/* offset : 0x378 */
typedef volatile union _acc_etc_ro137_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int pm_dstate:3; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The current power management D-state of the function: 
000b: D0, 001b: D1, 010b: D2, 
011b: D3, 100b: Uninitialized, Other values: Not applicable
        */
        volatile unsigned int aux_pm_en:1; /* [Position:3] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Auxiliary Power Enable bit in the Device Control register. There is 1 bit of aux_pm_en for each configured function.
        */
        volatile unsigned int pm_pme_en:1; /* [Position:4] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        PME Enable bit in the PMCSR. There is 1 bit of pm_pme_en for each configured function.
        */
        volatile unsigned int pm_linkst_in_l0s:1; /* [Position:5] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Power management is in L0s state.
        */
        volatile unsigned int pm_linkst_in_l1:1; /* [Position:6] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Power management is in L1 state.
        */
        volatile unsigned int pm_l1_entry_started:1; /* [Position:7] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        L1 entry process is in progress.
        */
        volatile unsigned int pm_linkst_in_l2:1; /* [Position:8] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Power management is in L2 state
        */
        volatile unsigned int pm_linkst_l2_exit:1; /* [Position:9] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Power management is exiting L2 state. Not applicable for downstream port.
        */
        volatile unsigned int pm_linkst_in_l1sub:1; /* [Position:10] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Power management is in L1 substate. Indicates when the link has entered L1 substates. It is used in DWC_pcie_clkrst.v (see 'Clock Generation and Gating Design Example') to ensure that the switching back of aux_clk from AUXCLK to PCLK occurs only after L1 substates have been exited.
        */
        volatile unsigned int cfg_l1sub_en:1; /* [Position:11] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that any of the L1 Substates are enabled in the L1 Substates Control 1 Register. Could be used by your application in a downstream port to determine when not to drive CLKREQ# such as when L1 Substates are not enabled.
        */
        volatile unsigned int pm_status:1; /* [Position:12] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        PME Status bit from the PMCSR. There is 1 bit of pm_status for each configured function.
        */
        volatile unsigned int RESERVED_13:19; /* [Position:13] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro137_0_t;
/* offset : 0x37c */
typedef volatile union _acc_etc_ro138_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_pbus_num:8; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The primary bus number assigned to the function. The number of bits depends the value of MULTI_DEVICE_AND_BUS_PER_FUNC_EN: 
- If MULTI_DEVICE_AND_BUS_PER_FUNC_EN =0, there are eight bits of cfg_pbus_num ([7:0]), regardless of the number of functions. 
- If MULTI_DEVICE_AND_BUS_PER_FUNC_EN =1, there are eight bits of cfg_pbus_num for each configured function.
        */
        volatile unsigned int cfg_pbus_dev_num:5; /* [Position:8] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The device number assigned to the function. The number of bits depends the value of MULTI_DEVICE_AND_BUS_PER_FUNC_EN: 
- If MULTI_DEVICE_AND_BUS_PER_FUNC_EN =0, there are five bits of cfg_pbus_dev_num ([4:0]), regardless of the number of func_x0002_tions. 
- If MULTI_DEVICE_AND_BUS_PER_FUNC_EN =1, there are five bits of cfg_pbus_dev_num for each configured function.
        */
        volatile unsigned int cfg_obff_en:2; /* [Position:13] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The OBFF Enable field of the Device Control 2 register of function 0.
        */
        volatile unsigned int radm_msg_cpu_active:1; /* [Position:15] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        One-clock-cycle pulse that indicates that the controller received a 'CPU Active' OBFF message. When FX_TLP > 1 and when two messages of the same type are received in the same clock cycle (back-to-back), then no separate indication is given for the second message. Only usable in an upstream port
        */
        volatile unsigned int radm_msg_obff:1; /* [Position:16] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        One-clock-cycle pulse that indicates that the controller received an 'OBFF' OBFF message. When FX_TLP > 1 and when two messages of the same type are received in the same clock cycle (back-to-back), then no separate indication is given for the second message. Only usable in an upstream port
        */
        volatile unsigned int radm_msg_idle:1; /* [Position:17] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        One-clock-cycle pulse that indicates that the controller received an 'IDLE' OBFF message. When FX_TLP > 1 and when two messages of the same type are received in the same clock cycle (back-to-back), then no separate indication is given for the second message. Only usable in an upstream port
        */
        volatile unsigned int radm_vendor_msg:1; /* [Position:18] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        One-cycle pulse that indicates the controller received a vendor-defined message. The controller makes the message header available the radm_msg_payload output. When FX_TLP > 1 and when two messages of the same type are received in the same clock cycle (back-to-back), then both bits are asserted.
        */
        volatile unsigned int RESERVED_19:13; /* [Position:19] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro138_0_t;
/* offset : 0x380 */
typedef volatile union _acc_etc_ro139_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int radm_msg_payload0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Received message header information.[31:0]
        */
    };
} acc_etc_ro139_0_t;
/* offset : 0x384 */
typedef volatile union _acc_etc_ro140_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int radm_msg_payload1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Received message header information.[63:32]
        */
    };
} acc_etc_ro140_0_t;
/* offset : 0x388 */
typedef volatile union _acc_etc_ro141_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int wake:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Wake Up. Wake up from power management unit. The controller generates wake to request the system to restore power and clock when a wakeup event has been detected such as apps_pm_xmt_pme, apps_pm_vf_xmt_pme, or outband_pwrup_cmd.
        */
        volatile unsigned int radm_msg_req_id:16; /* [Position:1] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The requester ID of the received Message. 
- [15:8]: Bus number - [7:3]: Device number - [2:0]: Function number
        */
        volatile unsigned int trgt_cpl_timeout:1; /* [Position:17] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that the application has not generated a completion for an incoming request within the required time interval.
        */
        volatile unsigned int trgt_timeout_cpl_func_num:1; /* [Position:18] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The function number of the timed out completion. Function numbering starts at '0'.
        */
        volatile unsigned int RESERVED_19:13; /* [Position:19] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro141_0_t;
/* offset : 0x38c */
typedef volatile union _acc_etc_ro144_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int trgt_timeout_cpl_tc:3; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The TC of the timed out completion.
        */
        volatile unsigned int trgt_timeout_cpl_attr:2; /* [Position:3] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The Attributes value of the timed out completion. Does not include IDO bit.
        */
        volatile unsigned int trgt_timeout_cpl_len:12; /* [Position:5] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The Length of the timed out completion.
        */
        volatile unsigned int trgt_timeout_lookup_id:8; /* [Position:17] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The target completion LUT lookup ID of the timed out completion
        */
        volatile unsigned int RESERVED_25:7; /* [Position:25] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro144_0_t;
/* offset : 0x390 */
typedef volatile union _acc_etc_ro145_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int trgt_lookup_id:8; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The target completion LUT lookup ID for the incoming request TLP. When using the optional target completion lookup table feature, the application must save the lookup ID and assert the same lookup ID client0/1/2_cpl_lookup_id when generating a completion for the request.
        */
        volatile unsigned int trgt_lookup_empty:1; /* [Position:8] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        When this signal is asserted with radm_trgt1_hv, it indicates that the target completion LUT is not full.
        */
        volatile unsigned int radm_cpl_timeout:1; /* [Position:9] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that the completion TLP for a request has not been received within the expected time window.
        */
        volatile unsigned int radm_timeout_func_num:1; /* [Position:10] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The function Number of the timed out completion. Function numbering starts at '0'.
        */
        volatile unsigned int RESERVED_11:21; /* [Position:11] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro145_0_t;
/* offset : 0x3a4 */
typedef volatile union _acc_etc_ro148_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int radm_timeout_cpl_tc:3; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The Traffic Class of the timed out completion.
        */
        volatile unsigned int radm_timeout_cpl_attr:2; /* [Position:3] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The Attributes field of the timed out completion.
        */
        volatile unsigned int radm_timeout_cpl_ln:1; /* [Position:5] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The Lightweight Notification (LN) field of the timed out completion.
        */
        volatile unsigned int radm_timeout_cpl_len:12; /* [Position:6] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Length (in bytes) of the timed out completion. For a split completion, it indicates the number of bytes remaining to be delivered when the completion timed out.
        */
        volatile unsigned int radm_timeout_cpl_tag:8; /* [Position:18] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The Tag field of the timed out completion.
        */
        volatile unsigned int RESERVED_26:6; /* [Position:26] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro148_0_t;
/* offset : 0x3a8 */
typedef volatile union _acc_etc_ro149_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int edma_int:8; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        DMA Interrupt. Indicates that the DMA transfer has completed or that an error has occurred. This is a level interrupt. For more information, see 'Interrupts and Error Handling'.
        */
        volatile unsigned int RESERVED_8:1; /* [Position:8] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
        volatile unsigned int assert_inta_grt:1; /* [Position:9] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The signal assert_inta_grt is a one-clock-cycle pulse that indicates that the controller sent an Assert_INTA Message to the upstream device.
        */
        volatile unsigned int assert_intb_grt:1; /* [Position:10] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The signal assert_intb_grt is a one-clock-cycle pulse that indicates that the controller sent an Assert_INTB Message to the upstream device.
        */
        volatile unsigned int assert_intc_grt:1; /* [Position:11] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The signal assert_intc_grt is a one-clock-cycle pulse that indicates that the controller sent an Assert_INTC Message to the upstream device.
        */
        volatile unsigned int assert_intd_grt:1; /* [Position:12] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The signal assert_intd_grt is a one-clock-cycle pulse that indicates that the controller sent an Assert_INTD Message to the upstream device.
        */
        volatile unsigned int deassert_inta_grt:1; /* [Position:13] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The signal deassert_inta_grt is a one-clock-cycle pulse that indicates that the controller sent an Deassert_INTA Message to the upstream device.
        */
        volatile unsigned int deassert_intb_grt:1; /* [Position:14] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The signal deassert_intb_grt is a one-clock-cycle pulse that indicates that the controller sent an Deassert_INTB Message to the upstream device.
        */
        volatile unsigned int deassert_intc_grt:1; /* [Position:15] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The signal deassert_intc_grt is a one-clock-cycle pulse that indicates that the controller sent an Deassert_INTC Message to the upstream device.
        */
        volatile unsigned int deassert_intd_grt:1; /* [Position:16] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The signal deassert_intd_grt is a one-clock-cycle pulse that indicates that the controller sent an Deassert_INTD Message to the upstream device.
        */
        volatile unsigned int RESERVED_17:15; /* [Position:17] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro149_0_t;
/* offset : 0x3ac */
typedef volatile union _acc_etc_ro150_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_int_pin:8; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The cfg_int_pin indicates the configured value for the Interrupt Pin Register field in the BRIDGE_CTRL_INT_PIN_INT_LINE register.
        */
        volatile unsigned int RESERVED_8:24; /* [Position:8] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro150_0_t;
/* offset : 0x3b0 */
typedef volatile union _acc_etc_ro151_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_send_cor_err:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Sent Correctable Error. Controller has sent a message towards the Root Complex indicating that an Rx TLP that contained an error, and that can be corrected, has been received by the Endpoint
        */
        volatile unsigned int cfg_send_nf_err:1; /* [Position:1] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Sent Non-Fatal Error. Controller has sent a message towards the Root Complex indicating that an Rx TLP that contained an non-fatal error, and that can not be corrected, has been received by the Endpoint
        */
        volatile unsigned int cfg_send_f_err:1; /* [Position:2] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Sent Fatal Error. Controller has sent a message towards the Root Complex indicating that an Rx TLP that contained a fatal error, and that can not be corrected, has been received by the Endpoint
        */
        volatile unsigned int cfg_int_disable:1; /* [Position:3] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        When high a functions ability to generate INTx messages is Disabled
        */
        volatile unsigned int cfg_no_snoop_en:1; /* [Position:4] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Contents of the "Enable No Snoop" field (PCIE_CAP_EN_NO_SNOOP) in the "Device Control and Status" register (DEVICE_CONTROL_DEVICE_STATUS) register.
        */
        volatile unsigned int cfg_relax_order_en:1; /* [Position:5] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Contents of the "Enable Relaxed Ordering" field (PCIE_CAP_EN_REL_ORDER) in the "Device Control and Status" register (DEVICE_CONTROL_DEVICE_STATUS) register.
        */
        volatile unsigned int cfg_link_eq_req_int:1; /* [Position:6] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Interrupt indicating to your application that the Link Equalization Request bit in the Link Status 2 Register has been set and the Link Equalization Request Interrupt Enable (Link Control 3 Register bit 1) is set.
        */
        volatile unsigned int usp_eq_redo_executed_int:1; /* [Position:7] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Interrupt indicating the EQ redo is executed by USP. You can use mac_phy_rate to know the data rate at which the interrupt occured. You can leave this pin unconnected.
        */
        volatile unsigned int RESERVED_8:24; /* [Position:8] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro151_0_t;
/* offset : 0x3b8 */
typedef volatile union _acc_etc_ro153_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int pm_l1sub_state:3; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Power management L1 sub-states FSM state
        */
        volatile unsigned int pm_master_state:5; /* [Position:3] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Power management master FSM state.
        */
        volatile unsigned int pm_slave_state:5; /* [Position:8] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Power management slave FSM state
        */
        volatile unsigned int RESERVED_13:19; /* [Position:13] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro153_0_t;
/* offset : 0x3bc */
typedef volatile union _acc_etc_ro154_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_uncor_internal_err_sts:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indication from the controller that the controller has detected an Uncorrectable Internal Error.
        */
        volatile unsigned int cfg_rcvr_overflow_err_sts:1; /* [Position:1] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indication from the controller that the controller has detected an Receiver Overflow Error.
        */
        volatile unsigned int cfg_fc_protocol_err_sts:1; /* [Position:2] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indication from the controller that the controller has detected an Flow Control Protocol Error.
        */
        volatile unsigned int cfg_mlf_tlp_err_sts:1; /* [Position:3] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indication from the controller that the controller detected an Malformed TLP Error.
        */
        volatile unsigned int cfg_surprise_down_er_sts:1; /* [Position:4] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indication from the controller that the controller detected an Surprise Down Error.
        */
        volatile unsigned int cfg_dl_protocol_err_sts:1; /* [Position:5] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indication from the controller that the controller detected an Data Link Protocol Error.
        */
        volatile unsigned int cfg_ecrc_err_sts:1; /* [Position:6] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indication from the controller that the controller detected an ECRC Error.
        */
        volatile unsigned int cfg_corrected_internal_err_sts:1; /* [Position:7] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indication from the controller that the controller detected an Corrected Internal Error.
        */
        volatile unsigned int cfg_replay_number_rollover_err_sts:1; /* [Position:8] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indication from the controller that the controller detected an REPLAY_NUMBER Rollover Error.
        */
        volatile unsigned int cfg_replay_timer_timeout_err_sts:1; /* [Position:9] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indication from the controller that the controller detected an Replay Timer Timeout.
        */
        volatile unsigned int cfg_bad_dllp_err_sts:1; /* [Position:10] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indication from the controller that the controller detected an Bad DLLP Error.
        */
        volatile unsigned int cfg_bad_tlp_err_sts:1; /* [Position:11] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indication from the controller that the controller detected an Bad TLP Error.
        */
        volatile unsigned int cfg_rcvr_err_sts:1; /* [Position:12] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indication from the controller that the controller detected an Receiver Error.
        */
        volatile unsigned int radm_trgt1_atu_sloc_match:8; /* [Position:13] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        ATU Single Location match indication per ATU region. Set for 1 core_clk period pulse when the ATU region matched a received VDM Single Location Address translation.
        */
        volatile unsigned int radm_trgt1_atu_cbuf_err:8; /* [Position:21] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        ATU Error indication per ATU region. Set for 1 core_clk period pulse when the ATU region matched received VDM size (payload + 3rd and 4th DW of Header) is greater than the programmed Circular Buffer Increment size (CBUF_INCR) for Single Location Address translation.
        */
        volatile unsigned int pm_aspm_l1_enter_ready:1; /* [Position:29] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates the controller is idle in L0 or L0s, and it is ready to enter L1 as soon as application releases app_xfer_pending AND the L1 ASPM timer expires.
        */
        volatile unsigned int radm_slot_pwr_limit:1; /* [Position:30] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        One-Clock-Cycle pulse that indicates the controller received a Set_Slot_Power_Limit message.
        */
        volatile unsigned int RESERVED_31:1; /* [Position:31] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro154_0_t;
/* offset : 0x3c0 */
typedef volatile union _acc_etc_ro155_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int radm_slot_pwr_payload:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        DW data of Set_Slot_Power_Limit message. This signal is only valid when radm_slot_pwr_limit is asserted.
        */
    };
} acc_etc_ro155_0_t;
/* offset : 0x3c4 */
typedef volatile union _acc_etc_ro156_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_ido_req_en:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        ID-Based Ordering Requests Enabled. Independent ordering based on Requester/Completer ID
        */
        volatile unsigned int cfg_ido_cpl_en:1; /* [Position:1] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        ID-Based Ordering Completions Enabled. Independent ordering based on Requester/Completer ID
        */
        volatile unsigned int cfg_pf_tph_st_mode:3; /* [Position:2] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Steering Tag Mode of Operation for Physical Function. Steering Tags are system-specific values used to identify a processing resource that a Requester explicitly targets
        */
        volatile unsigned int cfg_ln_enable:1; /* [Position:5] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        This signal reflects the contents of the LNR_ENABLE field in the LN Control and Capabilities register in a EP device.
        */
        volatile unsigned int radm_idle:1; /* [Position:6] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        RADM activity status signal. The controller creates the en_radm_clk_g output by gating this signal with the output of the RADM_CLK_GATING_EN field in the CLOCK_GATING_CTRL_OFF register. For debug purposes only.
        */
        volatile unsigned int cfg_pm_no_soft_rst:1; /* [Position:7] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        This is the value of the No Soft Reset bit in the Power Management Control and Status Register. When set, you should not reset any controller registers when transitioning from D3hot to D0. Therefore, you should not assert the non_sticky_rst_n and sticky_rst_n inputs.
        */
        volatile unsigned int RESERVED_8:24; /* [Position:8] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro156_0_t;
/* offset : 0x3cc */
typedef volatile union _acc_etc_ro158_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int app_parity_errs:3; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that the controller detected a datapath parity error, one bit for each of the following parity errors: ▪ app_parity_errs[0]: Parity error at front end of the transmit datapath. ▪ app_parity_errs[1]: Parity error at back end of the transmit data_x0002_path. ▪ app_parity_errs[2]: Parity error the receive datapath. The app_parity_errs signals are one-cycle pulses that indicate the as_x0002_sociated parity error occurred they do not indicate which packet con_x0002_tained the parity error. A suggested usage of the app_parity_errs sig_x0002_nals is to register each bit and to provide a control to turn off system notification of parity errors. By doing so, your application can choose to only respond to the first detection of a parity error. The controller per_x0002_forms transmit and receive datapath parity if data path protection is se_x0002_lected.
        */
        volatile unsigned int axi_parity_errs:7; /* [Position:3] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        axi_parity_errs
        */
        volatile unsigned int cfg_ext_tag_en:1; /* [Position:10] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        When enabled, controller supports up to 8-bit tag values.
        */
        volatile unsigned int slv_rasdp_err_mode:1; /* [Position:11] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indication (slv_aclk version) from the controller that it has entered RASDP error mode. The controller enters RASDP error mode (if the ERROR_MODE_EN register field =1) upon detection of the first uncor_x0002_rectable error. During this mode: ▪ Controller sets mstr_rasdp_error_mode =1 ▪ Controller sets slv_rasdp_error_mode =1 ▪ Controller sets dbi_rasdp_error_mode =1 ▪ Rx TLPs that are forwarded to your application are not guaranteed to be correct you must discard them.
        */
        volatile unsigned int mstr_rasdp_err_mode:1; /* [Position:12] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates (mstr_aclk version) that the controller has entered the RASDP error mode. The controller enters RASDP error mode (if the ERROR_MODE_EN register field =1) upon detection of the first uncor_x0002_rectable error. During this mode: ▪ Controller sets slv_rasdp_err_mode =1 ▪ Controller sets mstr_rasdp_err_mode =1 ▪ Controller sets dbi_rasdp_err_mode =1 ▪ Rx TLPs that are forwarded to your application are not guaranteed to be correct you must discard them.
        */
        volatile unsigned int cfg_neg_link_width:6; /* [Position:13] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Negotiated Link Width Field of Link Status Register Indicates the nego_x0002_tiated width of the given PCI Express Link
        */
        volatile unsigned int cfg_advisory_nf_sts:1; /* [Position:19] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indication from the controller that the controller detected an Advisory Non-Fatal Error.
        */
        volatile unsigned int cfg_hdr_log_overflow_sts:1; /* [Position:20] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indication from the controller that the controller detected a Header Log Overflow Error.
        */
        volatile unsigned int RESERVED_21:11; /* [Position:21] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro158_0_t;
/* offset : 0x400 */
typedef volatile union _acc_ras_rw00_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int diag_ctrl_bus:3; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Diagnostic Control Bus
- x01: Insert LCRC error by inverting the LSB of LCRC
- x10: Insert ECRC error by inverting the LSB of ECRC
- 1xx: Select Fast Link Mode.
        */
        volatile unsigned int app_ras_des_tba_ctrl:2; /* [Position:3] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Controls the start/end of time based analysis. You must only set the pins to the required value for the duration of one clock cycle. This signal must be 2'b00 while the TIMER_START field in TIME_BASED_ANALYSIS_CONTROL_REG register is controlled by the DBI interface or the accesses from the wire side.
-2'b00: No action
-2'b01: Start
-2'b10: End. This setting is only used when theTIME_BASED_DURATION_SELECT field of TIME_BASED_ANALYSIS_CONTROL_REG is set to "manual control".
-2'b11: Reserved
These pins also set the contents of the TIMER_START field in TIME_BASED_ANALYSIS_CONTROL_REG register.
        */
        volatile unsigned int RESERVED_5:3; /* [Position:5] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
        volatile unsigned int app_ras_des_sd_hold_ltssm:1; /* [Position:8] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Hold and release LTSSM. For as long as this signal is '1', the controller stays in the current LTSSM.
        */
        volatile unsigned int r_app_ras_des_sd_hold_ltssm_start:1; /* [Position:9] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        The start bit. When r_app_ras_des_sd_hold_ltssm_start is 1'b1, app_ras_des_sd_hold_ltssm signal is de-asserted (1->0) and the counter is started.
        */
        volatile unsigned int RESERVED_10:6; /* [Position:10] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
        volatile unsigned int r_app_ras_des_sd_hold_ltssm_time:16; /* [Position:16] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        The counter value. When the counter value reached 0, 
the app_ras_des_sd_hold_ltssm signal is asserted. (0->1)
        */
    };
} acc_ras_rw00_0_t;
/* offset : 0xc00 */
typedef volatile union _acc_ras_st_bus_ro00_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int rtfcgen_incr_amt:18;        /* [Position:0] */
        /*  = Payload flow control credits consumed. NOTE: hdr credits consumed is always 1 */
        volatile unsigned int rtfcgen_incr_enable:2;        /* [Position:18] */
        /*  = Header flow control credit consumed */
        volatile unsigned int rtfcgen_fctype:4;        /* [Position:20] */
        /*  = Flow control type consumed (P=0, NP=1, CPL=2) */
        volatile unsigned int rtcheck_rtfcgen_vc:6;        /* [Position:24] */
        /*  = Virtual channel of received TLP */
        volatile unsigned int xdlh_xtlh_halt:1;        /* [Position:30] */
        /* = Layer2 is not accepting data to transmit this cycle  */
        volatile unsigned int xtlh_xdlh_data:1; /* [Position:31] */
    };
} acc_ras_st_bus_ro00_0_t;

typedef volatile union _acc_ras_st_bus_ro00_1_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Transmit data at the interface between Layer3 and Layer2. If RASDP is enabled, includes the ECC or parity protection code bits. */
        volatile unsigned int xtlh_xdlh_data:32; /* [Position:32] */
    };
} acc_ras_st_bus_ro00_1_t;

typedef volatile union _acc_ras_st_bus_ro00_2_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Transmit data at the interface between Layer3 and Layer2. If RASDP is enabled, includes the ECC or parity protection code bits. */
        volatile unsigned int xtlh_xdlh_data:32; /* [Position:64] */
    };
} acc_ras_st_bus_ro00_2_t;

typedef volatile union _acc_ras_st_bus_ro00_3_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Transmit data at the interface between Layer3 and Layer2. If RASDP is enabled, includes the ECC or parity protection code bits. */
        volatile unsigned int xtlh_xdlh_data:32; /* [Position:96] */
    };
} acc_ras_st_bus_ro00_3_t;

typedef volatile union _acc_ras_st_bus_ro00_4_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Transmit data at the interface between Layer3 and Layer2. If RASDP is enabled, includes the ECC or parity protection code bits. */
        volatile unsigned int xtlh_xdlh_data:32; /* [Position:128] */
    };
} acc_ras_st_bus_ro00_4_t;

typedef volatile union _acc_ras_st_bus_ro00_5_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Transmit data at the interface between Layer3 and Layer2. If RASDP is enabled, includes the ECC or parity protection code bits. */
        volatile unsigned int xtlh_xdlh_data:32; /* [Position:160] */
    };
} acc_ras_st_bus_ro00_5_t;

typedef volatile union _acc_ras_st_bus_ro00_6_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Transmit data at the interface between Layer3 and Layer2. If RASDP is enabled, includes the ECC or parity protection code bits. */
        volatile unsigned int xtlh_xdlh_data:32; /* [Position:192] */
    };
} acc_ras_st_bus_ro00_6_t;

typedef volatile union _acc_ras_st_bus_ro00_7_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Transmit data at the interface between Layer3 and Layer2. If RASDP is enabled, includes the ECC or parity protection code bits. */
        volatile unsigned int xtlh_xdlh_data:32; /* [Position:224] */
    };
} acc_ras_st_bus_ro00_7_t;

typedef volatile union _acc_ras_st_bus_ro00_8_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Transmit data at the interface between Layer3 and Layer2. If RASDP is enabled, includes the ECC or parity protection code bits. */
        volatile unsigned int xtlh_xdlh_data:32; /* [Position:256] */
    };
} acc_ras_st_bus_ro00_8_t;

typedef volatile union _acc_ras_st_bus_ro00_9_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Transmit data at the interface between Layer3 and Layer2. If RASDP is enabled, includes the ECC or parity protection code bits. */
        volatile unsigned int xtlh_xdlh_data:31;        /* [Position:288] */
        /*  = Transmit data at the interface between Layer3 and Layer2. If RASDP is enabled, includes the ECC or parity protection code bits. */
        volatile unsigned int xtlh_xdlh_badeot:1; /* [Position:319] */
    };
} acc_ras_st_bus_ro00_9_t;

typedef volatile union _acc_ras_st_bus_ro00_10_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int xtlh_xdlh_badeot:7;       /* [Position:319] */
        /*  = Nullify this transmit TLP (invert CRC, append EDB) */
        volatile unsigned int xtlh_xdlh_eot:8;        /* [Position:327] */
        /*  = Transmit End of TLP this cycle */
        volatile unsigned int xtlh_xdlh_sot:8;        /* [Position:335] */
        /*  = Transmit Start of TLP this cycle */
        volatile unsigned int ecrc_err_asserted:1;        /* [Position:343] */
        /* = End-to-end CRC corrupted for this packet */
        volatile unsigned int lcrc_err_asserted:1;        /* [Position:344] */
        /* = Link CRC corrupted for this packet */
        volatile unsigned int xplh_xdlh_halt:1;        /* [Position:345] */
        /* = PHY Layer not accepting data this cycle */
        volatile unsigned int xdlh_xplh_data:6; /* [Position:346] */
    };
} acc_ras_st_bus_ro00_10_t;

typedef volatile union _acc_ras_st_bus_ro00_11_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Transmit packet payload (completely framed) */
        volatile unsigned int xdlh_xplh_data:32; /* [Position:352] */
    };
} acc_ras_st_bus_ro00_11_t;

typedef volatile union _acc_ras_st_bus_ro00_12_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Transmit packet payload (completely framed) */
        volatile unsigned int xdlh_xplh_data:32; /* [Position:384] */
    };
} acc_ras_st_bus_ro00_12_t;

typedef volatile union _acc_ras_st_bus_ro00_13_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Transmit packet payload (completely framed) */
        volatile unsigned int xdlh_xplh_data:32; /* [Position:416] */
    };
} acc_ras_st_bus_ro00_13_t;

typedef volatile union _acc_ras_st_bus_ro00_14_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Transmit packet payload (completely framed) */
        volatile unsigned int xdlh_xplh_data:32; /* [Position:448] */
    };
} acc_ras_st_bus_ro00_14_t;

typedef volatile union _acc_ras_st_bus_ro00_15_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Transmit packet payload (completely framed) */
        volatile unsigned int xdlh_xplh_data:32; /* [Position:480] */
    };
} acc_ras_st_bus_ro00_15_t;

typedef volatile union _acc_ras_st_bus_ro00_16_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Transmit packet payload (completely framed) */
        volatile unsigned int xdlh_xplh_data:32; /* [Position:512] */
    };
} acc_ras_st_bus_ro00_16_t;

typedef volatile union _acc_ras_st_bus_ro00_17_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Transmit packet payload (completely framed) */
        volatile unsigned int xdlh_xplh_data:32; /* [Position:544] */
    };
} acc_ras_st_bus_ro00_17_t;

typedef volatile union _acc_ras_st_bus_ro00_18_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Transmit packet payload (completely framed) */
        volatile unsigned int xdlh_xplh_data:26;        /* [Position:576] */
        /*  = Transmit packet payload (completely framed) */
        volatile unsigned int xdlh_xplh_sdp:6; /* [Position:602] */
    };
} acc_ras_st_bus_ro00_18_t;

typedef volatile union _acc_ras_st_bus_ro00_19_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int xdlh_xplh_sdp:2;       /* [Position:602] */
        /*  = Transmit Start of DLLP (per dword) */
        volatile unsigned int xdlh_xplh_stp:8;        /* [Position:610] */
        /*  = Transmit Start of TLP (per dword) */
        volatile unsigned int xdlh_xplh_eot:8;        /* [Position:618] */
        /*  = Transmit end of TLP/DLLP (per dword) */
        volatile unsigned int rdlh_xdlh_req_acknack_seqnum:12;        /* [Position:626] */
        /*  = Sequence Number for ACK/NAK DLLP */
        volatile unsigned int rdlh_xdlh_req2send_nack:1;        /* [Position:638] */
        /* = DataLink Layer request to send NAK */
        volatile unsigned int rdlh_xdlh_req2send_ack_due2dup:1; /* [Position:639] */
    };
} acc_ras_st_bus_ro00_19_t;

typedef volatile union _acc_ras_st_bus_ro00_20_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* = Request to send ACK due to duplicate TLP */
        volatile unsigned int rdlh_xdlh_req2send_ack:1;        /* [Position:640] */
        /* = DataLink Layer request to send ACK */
        volatile unsigned int rdlh_xdlh_rcvd_acknack_seqnum:12;        /* [Position:641] */
        /*  = Sequence number corresponding to NAK/ACK */
        volatile unsigned int rdlh_xdlh_rcvd_ack:1;        /* [Position:653] */
        /* = DataLink Layer received ACK DLLP */
        volatile unsigned int rdlh_xdlh_rcvd_nack:1;        /* [Position:654] */
        /* = DataLink Layer received NAK DLLP */
        volatile unsigned int cfg_link_retrain:1;        /* [Position:655] */
        /* = Software programmed link retrain request */
        volatile unsigned int rtlh_req_link_retrain:1;        /* [Position:656] */
        /* = Receive watchdog timer expired, retrain link */
        volatile unsigned int xdlh_smlh_start_link_retrain:1;        /* [Position:657] */
        /* = vMax retries attempted, request to retrain link */
        volatile unsigned int rdlh_rtlh_tlp_dv:1;        /* [Position:658] */
        /* = Receive data interface from DLL to Transaction Layer valid this cycle (rdlh_rtlh_*) */
        volatile unsigned int rdlh_rtlh_tlp_eot:8;        /* [Position:659] */
        /*  = End of TLP (per dword) */
        volatile unsigned int rdlh_rtlh_tlp_sot:5; /* [Position:667] */
    };
} acc_ras_st_bus_ro00_20_t;

typedef volatile union _acc_ras_st_bus_ro00_21_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int rdlh_rtlh_tlp_sot:3;       /* [Position:667] */
        /*  = Start of TLP (per dword)  */
        volatile unsigned int rplh_rdlh_pkt_data:29; /* [Position:675] */
    };
} acc_ras_st_bus_ro00_21_t;

typedef volatile union _acc_ras_st_bus_ro00_22_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Received packet payload */
        volatile unsigned int rplh_rdlh_pkt_data:32; /* [Position:704] */
    };
} acc_ras_st_bus_ro00_22_t;

typedef volatile union _acc_ras_st_bus_ro00_23_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Received packet payload */
        volatile unsigned int rplh_rdlh_pkt_data:32; /* [Position:736] */
    };
} acc_ras_st_bus_ro00_23_t;

typedef volatile union _acc_ras_st_bus_ro00_24_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Received packet payload */
        volatile unsigned int rplh_rdlh_pkt_data:32; /* [Position:768] */
    };
} acc_ras_st_bus_ro00_24_t;

typedef volatile union _acc_ras_st_bus_ro00_25_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Received packet payload */
        volatile unsigned int rplh_rdlh_pkt_data:32; /* [Position:800] */
    };
} acc_ras_st_bus_ro00_25_t;

typedef volatile union _acc_ras_st_bus_ro00_26_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Received packet payload */
        volatile unsigned int rplh_rdlh_pkt_data:32; /* [Position:832] */
    };
} acc_ras_st_bus_ro00_26_t;

typedef volatile union _acc_ras_st_bus_ro00_27_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Received packet payload */
        volatile unsigned int rplh_rdlh_pkt_data:32; /* [Position:864] */
    };
} acc_ras_st_bus_ro00_27_t;

typedef volatile union _acc_ras_st_bus_ro00_28_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Received packet payload */
        volatile unsigned int rplh_rdlh_pkt_data:32; /* [Position:896] */
    };
} acc_ras_st_bus_ro00_28_t;

typedef volatile union _acc_ras_st_bus_ro00_29_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Received packet payload */
        volatile unsigned int rplh_rdlh_pkt_data:3;        /* [Position:928] */
        /*  = Received packet payload */
        volatile unsigned int rplh_rdlh_pkt_err:8;        /* [Position:931] */
        /*  = Physical Error detected (per dword) */
        volatile unsigned int rplh_rdlh_pkt_dv:1;        /* [Position:939] */
        /* = Receive data interface from PHY to DLL valid this cycle (rplh_rdlh_*) */
        volatile unsigned int rplh_rdlh_pkt_edb:8;        /* [Position:940] */
        /*  = Packet terminated with EDB (per dword) */
        volatile unsigned int rplh_rdlh_pkt_end:8;        /* [Position:948] */
        /*  = DLLP/TLP ending (per dword) */
        volatile unsigned int rplh_rdlh_tlp_start:4; /* [Position:956] */
    };
} acc_ras_st_bus_ro00_29_t;

typedef volatile union _acc_ras_st_bus_ro00_30_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int rplh_rdlh_tlp_start:4;       /* [Position:956] */
        /*  = DLLP starting (per dword) */
        volatile unsigned int rplh_rdlh_dllp_start:8;        /* [Position:964] */
        /*  = MAC layer detected runt STP (per dword) */
        volatile unsigned int rplh_rdlh_nak:8;        /* [Position:972] */
        /*  = MAC layer detected runt STP (per dword) */
        volatile unsigned int rmlh_lanes_rcving:4;        /* [Position:980] */
        /*  = lanes active in link training */
        volatile unsigned int rmlh_rcvd_eidle_set:1;        /* [Position:984] */
        /* = Received EIDLE ordered set, any active lane */
        volatile unsigned int rplh_rcvd_idle1:1;        /* [Position:985] */
        /* = Logical Idle seen for 1+ symbols on all active lanes */
        volatile unsigned int rplh_rcvd_idle0:1;        /* [Position:986] */
        /* = Logical Idle seen for 8+ symbols on all active lanes */
        volatile unsigned int smlh_rcvd_lane_rev:1;        /* [Position:987] */
        /* = Receive logic detected logical lane reversal */
        volatile unsigned int rmlh_ts_link_num_is_k237:1;        /* [Position:988] */
        /* = Received Link number (lane 0) is PAD (k237) */
        volatile unsigned int rmlh_deskew_alignment_err:1;        /* [Position:989] */
        /* = Deskew logic overflow. Unable to align lanes [level] */
        volatile unsigned int rmlh_ts_lane_num_is_k237:1;        /* [Position:990] */
        /* = Received lane number (lane 0) is PAD (k237) */
        volatile unsigned int rmlh_ts2_rcvd:1; /* [Position:991] */
    };
} acc_ras_st_bus_ro00_30_t;

typedef volatile union _acc_ras_st_bus_ro00_31_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* = At least one active lane received TS2 this cycle */
        volatile unsigned int rmlh_ts1_rcvd:1;        /* [Position:992] */
        /* = At least one active lane received TS1 this cycle */
        volatile unsigned int rmlh_ts_rcv_err:1;        /* [Position:993] */
        /* = Assert when the received data is not an expected */
        volatile unsigned int rmlh_inskip_rcv:1;        /* [Position:994] */
        /* = Skip character is received this cyclePM Diagnostic Bus */
        volatile unsigned int xadm_no_fc_credit:1;        /* [Position:995] */
        /*  = No credits of any type are available (per VC). */
        volatile unsigned int xadm_tlp_pending:1;        /* [Position:996] */
        /* = No TLP transmit requests currently pending (from internal or external clients). */
        volatile unsigned int xadm_had_enough_credit:1;        /* [Position:997] */
        /*  = The controller has enough transmit credits of each type (per VC) to meet the PM entry criteria. */
        volatile unsigned int xdlh_not_expecting_ack:1;        /* [Position:998] */
        /* = All transmitted TLPs have been acknowledged by the link partner. */
        volatile unsigned int xdlh_xmt_pme_ack:1;        /* [Position:999] */
        /* = DataLink layer just transmitted a PME_TO_ACK msg. */
        volatile unsigned int xdlh_nodllp_pending:1;        /* [Position:1000] */
        /* = There are no pending or in-progress packets going to the PHY. */
        volatile unsigned int l1sub_state:3;        /* [Position:1001] */
        /*  = L1 FSM sub-state (Tied to 3'b000 when !CX_L1_SUBSTATES_ENABLE) */
        volatile unsigned int unexpected_cpl_err:1;        /* [Position:1004] */
        /* = Received a completion that was unexpected */
        volatile unsigned int cpl_ca_err:1;        /* [Position:1005] */
        /* = Received completion with CA status */
        volatile unsigned int cpl_ur_err:1;        /* [Position:1006] */
        /* = Received completion with UR status */
        volatile unsigned int flt_q_cpl_last:1;        /* [Position:1007] */
        /* = Final completion for the transaction */
        volatile unsigned int flt_q_cpl_abort:1;        /* [Position:1008] */
        /* = Current completion is being aborted (trashed) */
        volatile unsigned int cpl_mlf_err:1;        /* [Position:1009] */
        /* = Malformed completion */
        volatile unsigned int flt_q_header_cpl_status:3;        /* [Position:1010] */
        /*  = Indicates the filtering result for the current RX request or the completion status for the current RX received completion. */
        volatile unsigned int flt_q_header_destination:2;        /* [Position:1013] */
        /*  = Destination interface. */
        volatile unsigned int form_filt_ecrc_err:1;        /* [Position:1015] */
        /* = This TLP had an ECRC error */
        volatile unsigned int form_filt_malform_tlp_err:1;        /* [Position:1016] */
        /* = This TLP is malformed */
        volatile unsigned int form_filt_dllp_err:1;        /* [Position:1017] */
        /* = This TLP has a DataLink Layer Error (for example, LCRC) */
        volatile unsigned int form_filt_eot:1;        /* [Position:1018] */
        /* = End of TLP received this cycle */
        volatile unsigned int form_filt_dwen:5; /* [Position:1019] */
    };
} acc_ras_st_bus_ro00_31_t;

typedef volatile union _acc_ras_st_bus_ro00_32_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int form_filt_dwen:3;       /* [Position:1019] */
        /*  = Dword enables */
        volatile unsigned int form_filt_data:29; /* [Position:1027] */
    };
} acc_ras_st_bus_ro00_32_t;

typedef volatile union _acc_ras_st_bus_ro00_33_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Packet data from receive Transaction Layer */
        volatile unsigned int form_filt_data:32; /* [Position:1056] */
    };
} acc_ras_st_bus_ro00_33_t;

typedef volatile union _acc_ras_st_bus_ro00_34_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Packet data from receive Transaction Layer */
        volatile unsigned int form_filt_data:32; /* [Position:1088] */
    };
} acc_ras_st_bus_ro00_34_t;

typedef volatile union _acc_ras_st_bus_ro00_35_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Packet data from receive Transaction Layer */
        volatile unsigned int form_filt_data:32; /* [Position:1120] */
    };
} acc_ras_st_bus_ro00_35_t;

typedef volatile union _acc_ras_st_bus_ro00_36_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Packet data from receive Transaction Layer */
        volatile unsigned int form_filt_data:32; /* [Position:1152] */
    };
} acc_ras_st_bus_ro00_36_t;

typedef volatile union _acc_ras_st_bus_ro00_37_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Packet data from receive Transaction Layer */
        volatile unsigned int form_filt_data:32; /* [Position:1184] */
    };
} acc_ras_st_bus_ro00_37_t;

typedef volatile union _acc_ras_st_bus_ro00_38_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Packet data from receive Transaction Layer */
        volatile unsigned int form_filt_data:32; /* [Position:1216] */
    };
} acc_ras_st_bus_ro00_38_t;

typedef volatile union _acc_ras_st_bus_ro00_39_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Packet data from receive Transaction Layer */
        volatile unsigned int form_filt_data:32; /* [Position:1248] */
    };
} acc_ras_st_bus_ro00_39_t;

typedef volatile union _acc_ras_st_bus_ro00_40_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Packet data from receive Transaction Layer */
        volatile unsigned int form_filt_data:3;        /* [Position:1280] */
        /*  = Packet data from receive Transaction Layer */
        volatile unsigned int form_filt_dv:1;        /* [Position:1283] */
        /* = Packet is in payload stage */
        volatile unsigned int form_filt_hdr:28; /* [Position:1284] */
    };
} acc_ras_st_bus_ro00_40_t;

typedef volatile union _acc_ras_st_bus_ro00_41_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Header data (size of stored header is configurable) */
        volatile unsigned int form_filt_hdr:32; /* [Position:1312] */
    };
} acc_ras_st_bus_ro00_41_t;

typedef volatile union _acc_ras_st_bus_ro00_42_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Header data (size of stored header is configurable) */
        volatile unsigned int form_filt_hdr:32; /* [Position:1344] */
    };
} acc_ras_st_bus_ro00_42_t;

typedef volatile union _acc_ras_st_bus_ro00_43_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Header data (size of stored header is configurable) */
        volatile unsigned int form_filt_hdr:32; /* [Position:1376] */
    };
} acc_ras_st_bus_ro00_43_t;

typedef volatile union _acc_ras_st_bus_ro00_44_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Header data (size of stored header is configurable) */
        volatile unsigned int form_filt_hdr:4;        /* [Position:1408] */
        /*  = Header data (size of stored header is configurable) */
        volatile unsigned int form_filt_hv:1;        /* [Position:1412] */
        /* = Information to the receive packet filter block is valid */
        volatile unsigned int unexpected_cpl_err:1;        /* [Position:1413] */
        /* = Received a completion that was unexpected */
        volatile unsigned int cpl_ca_err:1;        /* [Position:1414] */
        /* = Received completion with CA status */
        volatile unsigned int cpl_ur_err:1;        /* [Position:1415] */
        /* = Received completion with UR status */
        volatile unsigned int flt_q_cpl_last:1;        /* [Position:1416] */
        /* = Final completion for the transaction */
        volatile unsigned int flt_q_cpl_abort:1;        /* [Position:1417] */
        /* = Current completion is being aborted (trashed) */
        volatile unsigned int cpl_mlf_err:1;        /* [Position:1418] */
        /* = Malformed completion */
        volatile unsigned int flt_q_header_cpl_status:3;        /* [Position:1419] */
        /*  = Indicates the filtering result for the current RX request or the completion status for the current RX received completion. */
        volatile unsigned int flt_q_header_destination:2;        /* [Position:1422] */
        /*  = Destination interface. */
        volatile unsigned int form_filt_ecrc_err:1;        /* [Position:1424] */
        /* = This TLP had an ECRC error */
        volatile unsigned int form_filt_malform_tlp_err:1;        /* [Position:1425] */
        /* = This TLP is malformed */
        volatile unsigned int form_filt_dllp_err:1;        /* [Position:1426] */
        /* = This TLP has a DataLink Layer Error (for example, LCRC) */
        volatile unsigned int form_filt_eot:1;        /* [Position:1427] */
        /* = End of TLP received this cycle */
        volatile unsigned int form_filt_dwen:8;        /* [Position:1428] */
        /*  = Dword enables */
        volatile unsigned int form_filt_data:4; /* [Position:1436] */
    };
} acc_ras_st_bus_ro00_44_t;

typedef volatile union _acc_ras_st_bus_ro00_45_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Packet data from receive Transaction Layer */
        volatile unsigned int form_filt_data:32; /* [Position:1440] */
    };
} acc_ras_st_bus_ro00_45_t;

typedef volatile union _acc_ras_st_bus_ro00_46_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Packet data from receive Transaction Layer */
        volatile unsigned int form_filt_data:32; /* [Position:1472] */
    };
} acc_ras_st_bus_ro00_46_t;

typedef volatile union _acc_ras_st_bus_ro00_47_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Packet data from receive Transaction Layer */
        volatile unsigned int form_filt_data:32; /* [Position:1504] */
    };
} acc_ras_st_bus_ro00_47_t;

typedef volatile union _acc_ras_st_bus_ro00_48_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Packet data from receive Transaction Layer */
        volatile unsigned int form_filt_data:32; /* [Position:1536] */
    };
} acc_ras_st_bus_ro00_48_t;

typedef volatile union _acc_ras_st_bus_ro00_49_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Packet data from receive Transaction Layer */
        volatile unsigned int form_filt_data:32; /* [Position:1568] */
    };
} acc_ras_st_bus_ro00_49_t;

typedef volatile union _acc_ras_st_bus_ro00_50_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Packet data from receive Transaction Layer */
        volatile unsigned int form_filt_data:32; /* [Position:1600] */
    };
} acc_ras_st_bus_ro00_50_t;

typedef volatile union _acc_ras_st_bus_ro00_51_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Packet data from receive Transaction Layer */
        volatile unsigned int form_filt_data:32; /* [Position:1632] */
    };
} acc_ras_st_bus_ro00_51_t;

typedef volatile union _acc_ras_st_bus_ro00_52_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Packet data from receive Transaction Layer */
        volatile unsigned int form_filt_data:28;        /* [Position:1664] */
        /*  = Packet data from receive Transaction Layer */
        volatile unsigned int form_filt_dv:1;        /* [Position:1692] */
        /* = Packet is in payload stage */
        volatile unsigned int form_filt_hdr:3; /* [Position:1693] */
    };
} acc_ras_st_bus_ro00_52_t;

typedef volatile union _acc_ras_st_bus_ro00_53_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Header data (size of stored header is configurable) */
        volatile unsigned int form_filt_hdr:32; /* [Position:1696] */
    };
} acc_ras_st_bus_ro00_53_t;

typedef volatile union _acc_ras_st_bus_ro00_54_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Header data (size of stored header is configurable) */
        volatile unsigned int form_filt_hdr:32; /* [Position:1728] */
    };
} acc_ras_st_bus_ro00_54_t;

typedef volatile union _acc_ras_st_bus_ro00_55_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Header data (size of stored header is configurable) */
        volatile unsigned int form_filt_hdr:32; /* [Position:1760] */
    };
} acc_ras_st_bus_ro00_55_t;

typedef volatile union _acc_ras_st_bus_ro00_56_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Header data (size of stored header is configurable) */
        volatile unsigned int form_filt_hdr:29;        /* [Position:1792] */
        /*  = Header data (size of stored header is configurable) */
        volatile unsigned int form_filt_hv:1;        /* [Position:1821] */
        /* = Information to the receive packet filter block is valid RADM Diagnostic Dual Bus Selector. */
        volatile unsigned int form_filt_formation:1;        /* [Position:1822] */
        /* = Indicates which TLP was received first by the controller for 256-bit configurations. */
        volatile unsigned int active_grant:1; /* [Position:1823] */
    };
} acc_ras_st_bus_ro00_56_t;

typedef volatile union _acc_ras_st_bus_ro00_57_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int active_grant:4;       /* [Position:1823] */
        /*  = Currently granted client (1-hot) */
        volatile unsigned int grant_ack:5;        /* [Position:1828] */
        /*  = This client has completed his request (1-hot) */
        volatile unsigned int fc_cds_pass:5;        /* [Position:1833] */
        /*  = Credit check passed for each request */
        volatile unsigned int arb_reqs:5;        /* [Position:1838] */
        /*  = Transmit requests from each client  */
        volatile unsigned int cfg_pbus_num:8;        /* [Position:1843] */
        /*  = Bus number; 8 bits per function when MULTI_DEVICE_AND_BUS_PER_FUNC_EN=1, else 8 bits in total. */
        volatile unsigned int cfg_pbus_dev_num:5; /* [Position:1851] */
    };
} acc_ras_st_bus_ro00_57_t;

typedef volatile union _acc_ras_st_bus_ro00_58_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Device number; 5 bits per function when MULTI_DEVICE_AND_BUS_PER_FUNC_EN=1, else 5 bits in total. */
        volatile unsigned int xdlh_replay_timeout_err:1;        /* [Position:1856] */
        /* = Retry timer expired [pulse] */
        volatile unsigned int xdlh_replay_num_rlover_err:1;        /* [Position:1857] */
        /* = Max retries exceeded [pulse] */
        volatile unsigned int rdlh_bad_dllp_err:1;        /* [Position:1858] */
        /* = Received DLLP with DataLink Layer error [pulse] */
        volatile unsigned int rdlh_bad_tlp_err:1;        /* [Position:1859] */
        /* = Received TLP with DataLink Layer error [pulse] */
        volatile unsigned int rdlh_prot_err:1;        /* [Position:1860] */
        /* = DLLP protocol error (out of sequence DLLP) [pulse] */
        volatile unsigned int rtlh_fc_prot_err:1;        /* [Position:1861] */
        /* = Flow control protocol violation (watchdog timer) [pulse] */
        volatile unsigned int rmlh_rcvd_err:1;        /* [Position:1862] */
        /* = Received PHY error this cycle [pulse] */
        volatile unsigned int int_xadm_fc_prot_err:1;        /* [Position:1863] */
        /* = Flow control update protocol violation (opt.checks) */
        volatile unsigned int radm_cpl_timeout:1;        /* [Position:1864] */
        /* = An request failed to complete in the allotted time */
        volatile unsigned int radm_qoverflow:1;        /* [Position:1865] */
        /*  = Receive queue overflow. Normally happens only when flow control advertisements are ignored */
        volatile unsigned int radm_unexp_cpl_err:1;        /* [Position:1866] */
        /*  = Received an unexpected completion [pulse] */
        volatile unsigned int radm_rcvd_cpl_ur:1;        /* [Position:1867] */
        /*  = Received a completion with UR status */
        volatile unsigned int radm_rcvd_cpl_ca:1;        /* [Position:1868] */
        /*  = Received a completion with CA status */
        volatile unsigned int radm_rcvd_req_ca:1;        /* [Position:1869] */
        /*  = Completer aborted request */
        volatile unsigned int radm_rcvd_req_ur:1;        /* [Position:1870] */
        /*  = Received a request which device does not support */
        volatile unsigned int radm_ecrc_err:1;        /* [Position:1871] */
        /*  = Received a TLP with ECRC error */
        volatile unsigned int radm_mlf_tlp_err:1;        /* [Position:1872] */
        /*  = Received a malformed TLP [pulse] */
        volatile unsigned int radm_rcvd_cpl_poisoned:1;        /* [Position:1873] */
        /*  = Received a completion with poisoned payload */
        volatile unsigned int radm_rcvd_wreq_poisoned:1;        /* [Position:1874] */
        /*  = Received a write with poisoned payload */
        volatile unsigned int cdm_lbc_ack:1;        /* [Position:1875] */
        /*  = Local bus Acknowledge */
        volatile unsigned int lbc_cdm_wr:4;        /* [Position:1876] */
        /*  = Local bus Write enable (per byte) */
        volatile unsigned int lbc_cdm_cs:1;        /* [Position:1880] */
        /*  = Local bus chip select */
        volatile unsigned int lbc_cdm_data:7; /* [Position:1881] */
    };
} acc_ras_st_bus_ro00_58_t;

typedef volatile union _acc_ras_st_bus_ro00_59_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int lbc_cdm_data:25;       /* [Position:1881] */
        /*  = Local bus Data */
        volatile unsigned int lbc_cdm_addr:7; /* [Position:1913] */
    };
} acc_ras_st_bus_ro00_59_t;

typedef volatile union _acc_ras_st_bus_ro00_60_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int lbc_cdm_addr:25;       /* [Position:1913] */
        /*  = Local bus Address */
        volatile unsigned int cfg_sys_err_rc_cor:1;        /* [Position:1945] */
        /* = System Error caused by Correctable  */
        volatile unsigned int cfg_sys_err_rc_nf:1;        /* [Position:1946] */
        /* = System Error caused by Non-Fatal */
        volatile unsigned int cfg_sys_err_rc_f:1;        /* [Position:1947] */
        /* = System Error caused by Fatal */
        volatile unsigned int radm_ide_pcrc_err:1;        /* [Position:1948] */
        /* = Received a TLP with PCRC error (Tied 0 if PCIe IDE is not enabled) */
        volatile unsigned int radm_ide_misrouted:1;        /* [Position:1949] */
        /* = Received an IDE Misrouted TLP (Tied 0 if PCIe IDE is not enabled) */
        volatile unsigned int radm_ide_check_failed:1;        /* [Position:1950] */
        /* = Received an IDE TLP with an IDE Check */
    };
} acc_ras_st_bus_ro00_60_t;
/* offset : 0x45c */
typedef volatile union _acc_ras_ec_com_ro00_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int bit0:1;        /* [Position:0] */
        /* : Reserved (fixed """"0"""") */
        volatile unsigned int bit1:1;        /* [Position:1] */
        /* : Reserved (fixed """"0"""") */
        volatile unsigned int bit2:1;        /* [Position:2] */
        /* : Reserved (fixed """"0"""") */
        volatile unsigned int bit3:1;        /* [Position:3] */
        /* : Reserved (fixed """"0"""") */
        volatile unsigned int bit4:1;        /* [Position:4] */
        /* : Reserved (fixed """"0"""") */
        volatile unsigned int smlh_eidle_inferred_in_l0:1;        /* [Position:5] */
        /* : smlh_eidle_inferred_in_l0: Level: Detect EI Infer */
        volatile unsigned int rmlh_rcvd_err:1;        /* [Position:6] */
        /* : rmlh_rcvd_err: Pulse: Receiver Error */
        volatile unsigned int smlh_rx_rcvry_req:1;        /* [Position:7] */
        /* : smlh_rx_rcvry_req: Level: Rx Recovery Request */
        volatile unsigned int smlh_timeout_nfts:1;        /* [Position:8] */
        /* : smlh_timeout_nfts: Level: FTS Timeout */
        volatile unsigned int rmlh_framing_err:1;        /* [Position:9] */
        /* : rmlh_framing_err: Pulse: Framing Error */
        volatile unsigned int rmlh_deskew_alignment_err:1;        /* [Position:10] */
        /* : rmlh_deskew_alignment_err: Level: Deskew Error */
        volatile unsigned int rdlh_bad_tlp_err_pertlp:4;        /* [Position:11] */
        /* : rdlh_bad_tlp_err_pertlp : Pulse: BAD TLP */
        volatile unsigned int rdlh_lcrc_tlp_err_pertlp:4;        /* [Position:15] */
        /* : rdlh_lcrc_tlp_err_pertlp : Pulse: LCRC Error */
        volatile unsigned int rdlh_bad_dllp_err_perdllp:8;        /* [Position:19] */
        /* : rdlh_bad_dllp_err_perdllp: Pulse: BAD DLLP */
        volatile unsigned int xdlh_replay_num_rlover_err:1;        /* [Position:27] */
        /* : xdlh_replay_num_rlover_err: Pulse: Replay_Num Rollover */
        volatile unsigned int xdlh_replay_timeout_err:1;        /* [Position:28] */
        /* : xdlh_replay_timeout_err: Pulse: Replay Timeout */
        volatile unsigned int rdlh_rcvd_nack_perdllp:3; /* [Position:29] */
    };
} acc_ras_ec_com_ro00_0_t;

typedef volatile union _acc_ras_ec_com_ro00_1_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : rdlh_rcvd_nack_perdllp: Pulse: Rx Nak DLLP */
        volatile unsigned int rdlh_rcvd_nack_perdllp:5;        /* [Position:32] */
        /* : rdlh_rcvd_nack_perdllp: Pulse: Rx Nak DLLP */
        volatile unsigned int xdlh_nak_sent:1;        /* [Position:37] */
        /* : xdlh_nak_sent: Pulse: Tx Nak DLLP */
        volatile unsigned int xdlh_retry_req:1;        /* [Position:38] */
        /* : xdlh_retry_req: Pulse: Retry TLP */
        volatile unsigned int rtlh_req_link_retrain:1;        /* [Position:39] */
        /* : rtlh_req_link_retrain: Level: FC Timeout */
        volatile unsigned int cfg_poisned_tlp:4;        /* [Position:40] */
        /* : cfg_poisned_tlp: Pulse: Poisoned TLP */
        volatile unsigned int cfg_ecrc_tlp_err:4;        /* [Position:44] */
        /* : cfg_ecrc_tlp_err: Pulse: ECRC Error */
        volatile unsigned int cfg_ur_tlp:4;        /* [Position:48] */
        /* : cfg_ur_tlp: Pulse: Unsupported Request */
        volatile unsigned int cfg_ca_tlp:4;        /* [Position:52] */
        /* : cfg_ca_tlp: Pulse: Completer Abort */
        volatile unsigned int cfg_cpl_timeout_1_:2;        /* [Position:56] */
        /* : cfg_cpl_timeout[1:0]: Pulse: Completion Timeout */
        volatile unsigned int smlh_l0_to_recovery:1;        /* [Position:58] */
        /* : smlh_l0_to_recovery: Pulse: L0 to Recovery Entry */
        volatile unsigned int smlh_l1_to_recovery:1;        /* [Position:59] */
        /* : smlh_l1_to_recovery: Pulse: L1 to Recovery Entry */
        volatile unsigned int smlh_in_l0s:1;        /* [Position:60] */
        /* : smlh_in_l0s: Level: Tx L0s Entry */
        volatile unsigned int smlh_in_rl0s:1;        /* [Position:61] */
        /* : smlh_in_rl0s: Level: Rx L0s Entry */
        volatile unsigned int pm_asnak:1;        /* [Position:62] */
        /* : pm_asnak: Level: ASPM L1 reject */
        volatile unsigned int smlh_in_l1:1; /* [Position:63] */
    };
} acc_ras_ec_com_ro00_1_t;

typedef volatile union _acc_ras_ec_com_ro00_2_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : smlh_in_l1: Level: L1 Entry */
        volatile unsigned int pm_in_l11:1;        /* [Position:64] */
        /* : pm_in_l11: Level: L1.1 Entry"" */
        volatile unsigned int pm_in_l12:1;        /* [Position:65] */
        /* : pm_in_l12: Level: L1.2 Entry */
        volatile unsigned int pm_in_l1_short:1;        /* [Position:66] */
        /* : pm_in_l1_short: Level: L1 short duration */
        volatile unsigned int pm_in_l1_cpm:1;        /* [Position:67] */
        /* : pm_in_l1_cpm: Level: L1 Clock PM (L1 with REFCLK removal/PLL Off) */
        volatile unsigned int pm_in_l1_abort:1;        /* [Position:68] */
        /* : pm_in_l1_abort: Level: L1.2 abort */
        volatile unsigned int smlh_in_l23:1;        /* [Position:69] */
        /* : smlh_in_l23: Level: L2 Entry */
        volatile unsigned int smlh_spd_change:1;        /* [Position:70] */
        /* : smlh_spd_change: Pulse: Speed Change */
        volatile unsigned int smlh_lwd_change:1;        /* [Position:71] */
        /* : smlh_lwd_change: Pulse: Link width Change */
        volatile unsigned int xdlh_ack_sent:1;        /* [Position:72] */
        /* : xdlh_ack_sent: Pulse: Tx Ack DLLP */
        volatile unsigned int xdlh_update_fc_sent:1;        /* [Position:73] */
        /* : xdlh_update_fc_sent: Pulse: Tx Update FC DLLP */
        volatile unsigned int rdlh_rcvd_ack_perdllp:8;        /* [Position:74] */
        /* : rdlh_rcvd_ack_perdllp: Pulse: Rx Ack DLLP */
        volatile unsigned int rtlh_rcvd_ufc_perdllp:8;        /* [Position:82] */
        /* : rtlh_rcvd_ufc_perdllp: Pulse: Rx Update FC DLLP */
        volatile unsigned int rdlh_nulified_tlp_err_pertlp:4;        /* [Position:90] */
        /* : rdlh_nulified_tlp_err_pertlp : Pulse: Rx Nullified TLP */
        volatile unsigned int xtlh_xadm_restore_enable:1;        /* [Position:94] */
        /* : xtlh_xadm_restore_enable: Pulse: Tx Nullified TLP */
        volatile unsigned int rdlh_duplicate_tlp_err_pertlp:1; /* [Position:95] */
    };
} acc_ras_ec_com_ro00_2_t;

typedef volatile union _acc_ras_ec_com_ro00_3_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : rdlh_duplicate_tlp_err_pertlp: Pulse: Rx Duplicate TLP */
        volatile unsigned int rdlh_duplicate_tlp_err_pertlp:3;        /* [Position:96] */
        /* : rdlh_duplicate_tlp_err_pertlp: Pulse: Rx Duplicate TLP */
        volatile unsigned int xtlh_tx_memwr_evt:2;        /* [Position:99] */
        /* : xtlh_tx_memwr_evt: Pulse: Tx Memory Write */
        volatile unsigned int xtlh_tx_memrd_evt:2;        /* [Position:101] */
        /* : xtlh_tx_memrd_evt: Pulse: Tx Memory Read */
        volatile unsigned int xtlh_tx_cfgwr_evt:2;        /* [Position:103] */
        /* : xtlh_tx_cfgwr_evt: Pulse: Tx Config Write */
        volatile unsigned int xtlh_tx_cfgrd_evt:2;        /* [Position:105] */
        /* : xtlh_tx_cfgrd_evt: Pulse: Tx Config Read */
        volatile unsigned int xtlh_tx_iowr_evt:2;        /* [Position:107] */
        /* : xtlh_tx_iowr_evt: Pulse: Tx IO Write */
        volatile unsigned int xtlh_tx_iord_evt:2;        /* [Position:109] */
        /* : xtlh_tx_iord_evt: Pulse: Tx IO Read */
        volatile unsigned int xtlh_tx_cplwod_evt:2;        /* [Position:111] */
        /* : xtlh_tx_cplwod_evt: Pulse: Tx Completion wo data */
        volatile unsigned int xtlh_tx_cplwd_evt:2;        /* [Position:113] */
        /* : xtlh_tx_cplwd_evt: Pulse: Tx Completion w data */
        volatile unsigned int xtlh_tx_msg_evt:2;        /* [Position:115] */
        /* : xtlh_tx_msg_evt: Pulse: Tx Message */
        volatile unsigned int xtlh_tx_atmcop_evt:2;        /* [Position:117] */
        /* : xtlh_tx_atmcop_evt: Pulse: Tx AtomicOp */
        volatile unsigned int xtlh_tx_tlpwprefix_evt:2;        /* [Position:119] */
        /* : xtlh_tx_tlpwprefix_evt: Pulse: Tx TLP with Prefix */
        volatile unsigned int rtlh_rx_memwr_evt:4;        /* [Position:121] */
        /* : rtlh_rx_memwr_evt: Pulse: Rx Memory Write */
        volatile unsigned int rtlh_rx_memrd_evt:3; /* [Position:125] */
    };
} acc_ras_ec_com_ro00_3_t;

typedef volatile union _acc_ras_ec_com_ro00_4_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : rtlh_rx_memrd_evt: Pulse: Rx Memory Read */
        volatile unsigned int rtlh_rx_memrd_evt:1;        /* [Position:128] */
        /* : rtlh_rx_memrd_evt: Pulse: Rx Memory Read+J402 */
        volatile unsigned int rtlh_rx_cfgwr_evt:4;        /* [Position:129] */
        /* : rtlh_rx_cfgwr_evt: Pulse: Rx Config Write */
        volatile unsigned int rtlh_rx_cfgrd_evt:4;        /* [Position:133] */
        /* : rtlh_rx_cfgrd_evt: Pulse: Rx Config Read */
        volatile unsigned int rtlh_rx_iowr_evt:4;        /* [Position:137] */
        /* : rtlh_rx_iowr_evt: Pulse: Rx IO Write */
        volatile unsigned int rtlh_rx_iord_evt:4;        /* [Position:141] */
        /* : rtlh_rx_iord_evt: Pulse: Rx IO Read */
        volatile unsigned int rtlh_rx_cplwod_evt:4;        /* [Position:145] */
        /* : rtlh_rx_cplwod_evt: Pulse: Rx Completion wo data */
        volatile unsigned int rtlh_rx_cplwd_evt:4;        /* [Position:149] */
        /* : rtlh_rx_cplwd_evt: Pulse: Rx Completion w data */
        volatile unsigned int rtlh_rx_msg_evt:4;        /* [Position:153] */
        /* : rtlh_rx_msg_evt: Pulse: Rx Message TLP */
        volatile unsigned int rtlh_rx_atmcop_evt:3; /* [Position:157] */
    };
} acc_ras_ec_com_ro00_4_t;

typedef volatile union _acc_ras_ec_com_ro00_5_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : rtlh_rx_atmcop_evt: Pulse: Rx Atomic */
        volatile unsigned int rtlh_rx_atmcop_evt:1;        /* [Position:160] */
        /* : rtlh_rx_atmcop_evt: Pulse: Rx Atomic */
        volatile unsigned int rtlh_rx_tlpwprefix_evt:4;        /* [Position:161] */
        /* : rtlh_rx_tlpwprefix_evt: Pulse: Rx TLP with Prefix */
        volatile unsigned int xtlh_tx_ccix_tlp_evt:2;        /* [Position:165] */
        /* : xtlh_tx_ccix_tlp_evt: Pulse: Tx CCIX TLP */
        volatile unsigned int rtlh_rx_ccix_tlp_evt:4;        /* [Position:167] */
        /* : rtlh_rx_ccix_tlp_evt: Pulse: Rx CCIX TLP */
        volatile unsigned int xtlh_tx_defmemwr_evt:2;        /* [Position:171] */
        /* : xtlh_tx_defmemwr_evt: Pulse: Tx Deferrable Memory Write  */
        volatile unsigned int rtlh_rx_defmemwr_evt:4;        /* [Position:173] */
        /* : rtlh_rx_defmemwr_evt: Pulse: Rx Deferrable Memory Write */
    };
} acc_ras_ec_com_ro00_5_t;
/* offset : 0x474 */
typedef volatile union _acc_ras_ec_lxx_ro00_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int l0_rmlh_ebuf_rxoverflow:1;        /* [Position:0] */
        /* : l0_rmlh_ebuf_rxoverflow: Pulse: EBUF Overflow */
        volatile unsigned int l0_rmlh_ebuf_rxunderflow:1;        /* [Position:1] */
        /* : l0_rmlh_ebuf_rxunderflow: Pulse: EBUF underrun */
        volatile unsigned int l0_rmlh_phy_rxcodeerror:1;        /* [Position:2] */
        /* : l0_rmlh_phy_rxcodeerror: Pulse: Decode Error */
        volatile unsigned int l0_rmlh_phy_rxdisperror:1;        /* [Position:3] */
        /* : l0_rmlh_phy_rxdisperror: Pulse: Running Disparity Error */
        volatile unsigned int l0_rmlh_skp_parity_err:1;        /* [Position:4] */
        /* : l0_rmlh_skp_parity_err: Pulse: SKP OS Parity Error */
        volatile unsigned int l0_rmlh_sync_header_err:1;        /* [Position:5] */
        /* : l0_rmlh_sync_header_err: Pulse: SYNC Header Error */
        volatile unsigned int l0_rmlh_rxvalid_deasserted:1;        /* [Position:6] */
        /* : l0_rmlh_rxvalid_deasserted: Pulse: Rx Valid de-assertion */
        volatile unsigned int l0_rmlh_ebuf_rxskipadded:1;        /* [Position:7] */
        /* : l0_rmlh_ebuf_rxskipadded: Pulse: EBUF SKP Add */
        volatile unsigned int l0_rmlh_ebuf_rxskipremoved:1;        /* [Position:8] */
        /* : l0_rmlh_ebuf_rxskipremoved: Pulse: EBUF SKP Del */
        volatile unsigned int l0_rmlh_deskew_ctlskp_erri_9:1;        /* [Position:9] */
        /* : l0_rmlh_deskew_ctlskp_err[i*5]: Pulse: CTL SKP OS Data Parity Error */
        volatile unsigned int l0_rmlh_deskew_ctlskp_erri_10:1;        /* [Position:10] */
        /* : l0_rmlh_deskew_ctlskp_err[i*5+1]: Pulse: 1st Retimer Parity Error */
        volatile unsigned int l0_rmlh_deskew_ctlskp_erri_11:1;        /* [Position:11] */
        /* : l0_rmlh_deskew_ctlskp_err[i*5+2]: Pulse: 2nd Retimer Parity Error */
        volatile unsigned int l0_rmlh_deskew_ctlskp_err_i_12:1;        /* [Position:12] */
        /* : l0_rmlh_deskew_ctlskp_err[i*5+4:i*5+3]: Pulse: Margin CRC and Parity Error */
        volatile unsigned int l1_rmlh_ebuf_rxoverflow:1;        /* [Position:13] */
        /* : l1_rmlh_ebuf_rxoverflow: Pulse: EBUF Overflow */
        volatile unsigned int l1_rmlh_ebuf_rxunderflow:1;        /* [Position:14] */
        /* : l1_rmlh_ebuf_rxunderflow: Pulse: EBUF underrun */
        volatile unsigned int l1_rmlh_phy_rxcodeerror:1;        /* [Position:15] */
        /* : l1_rmlh_phy_rxcodeerror: Pulse: Decode Error */
        volatile unsigned int l1_rmlh_phy_rxdisperror:1;        /* [Position:16] */
        /* : l1_rmlh_phy_rxdisperror: Pulse: Running Disparity Error */
        volatile unsigned int l1_rmlh_skp_parity_err:1;        /* [Position:17] */
        /* : l1_rmlh_skp_parity_err: Pulse: SKP OS Parity Error */
        volatile unsigned int l1_rmlh_sync_header_err:1;        /* [Position:18] */
        /* : l1_rmlh_sync_header_err: Pulse: SYNC Header Error */
        volatile unsigned int l1_rmlh_rxvalid_deasserted:1;        /* [Position:19] */
        /* : l1_rmlh_rxvalid_deasserted: Pulse: Rx Valid de-assertion */
        volatile unsigned int l1_rmlh_ebuf_rxskipadded:1;        /* [Position:20] */
        /* : l1_rmlh_ebuf_rxskipadded: Pulse: EBUF SKP Add */
        volatile unsigned int l1_rmlh_ebuf_rxskipremoved:1;        /* [Position:21] */
        /* : l1_rmlh_ebuf_rxskipremoved: Pulse: EBUF SKP Del */
        volatile unsigned int l1_rmlh_deskew_ctlskp_erri_22:1;        /* [Position:22] */
        /* : l1_rmlh_deskew_ctlskp_err[i*5]: Pulse: CTL SKP OS Data Parity Error */
        volatile unsigned int l1_rmlh_deskew_ctlskp_erri_23:1;        /* [Position:23] */
        /* : l1_rmlh_deskew_ctlskp_err[i*5+1]: Pulse: 1st Retimer Parity Error */
        volatile unsigned int l1_rmlh_deskew_ctlskp_erri_24:1;        /* [Position:24] */
        /* : l1_rmlh_deskew_ctlskp_err[i*5+2]: Pulse: 2nd Retimer Parity Error */
        volatile unsigned int l1_rmlh_deskew_ctlskp_err_i_25:1;        /* [Position:25] */
        /* : l1_rmlh_deskew_ctlskp_err[i*5+4:i*5+3]: Pulse: Margin CRC and Parity Error */
        volatile unsigned int RESERVED_26:6; /* [Position:26] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_ras_ec_lxx_ro00_0_t;
/* offset : 0x478 */
typedef volatile union _acc_ras_ec_lxx_ro01_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int l2_rmlh_ebuf_rxoverflow:1;        /* [Position:0] */
        /* : l2_rmlh_ebuf_rxoverflow: Pulse: EBUF Overflow */
        volatile unsigned int l2_rmlh_ebuf_rxunderflow:1;        /* [Position:1] */
        /* : l2_rmlh_ebuf_rxunderflow: Pulse: EBUF underrun */
        volatile unsigned int l2_rmlh_phy_rxcodeerror:1;        /* [Position:2] */
        /* : l2_rmlh_phy_rxcodeerror: Pulse: Decode Error */
        volatile unsigned int l2_rmlh_phy_rxdisperror:1;        /* [Position:3] */
        /* : l2_rmlh_phy_rxdisperror: Pulse: Running Disparity Error */
        volatile unsigned int l2_rmlh_skp_parity_err:1;        /* [Position:4] */
        /* : l2_rmlh_skp_parity_err: Pulse: SKP OS Parity Error */
        volatile unsigned int l2_rmlh_sync_header_err:1;        /* [Position:5] */
        /* : l2_rmlh_sync_header_err: Pulse: SYNC Header Error */
        volatile unsigned int l2_rmlh_rxvalid_deasserted:1;        /* [Position:6] */
        /* : l2_rmlh_rxvalid_deasserted: Pulse: Rx Valid de-assertion */
        volatile unsigned int l2_rmlh_ebuf_rxskipadded:1;        /* [Position:7] */
        /* : l2_rmlh_ebuf_rxskipadded: Pulse: EBUF SKP Add */
        volatile unsigned int l2_rmlh_ebuf_rxskipremoved:1;        /* [Position:8] */
        /* : l2_rmlh_ebuf_rxskipremoved: Pulse: EBUF SKP Del */
        volatile unsigned int l2_rmlh_deskew_ctlskp_erri_9:1;        /* [Position:9] */
        /* : l2_rmlh_deskew_ctlskp_err[i*5]: Pulse: CTL SKP OS Data Parity Error */
        volatile unsigned int l2_rmlh_deskew_ctlskp_erri_10:1;        /* [Position:10] */
        /* : l2_rmlh_deskew_ctlskp_err[i*5+1]: Pulse: 1st Retimer Parity Error */
        volatile unsigned int l2_rmlh_deskew_ctlskp_erri_11:1;        /* [Position:11] */
        /* : l2_rmlh_deskew_ctlskp_err[i*5+2]: Pulse: 2nd Retimer Parity Error */
        volatile unsigned int l2_rmlh_deskew_ctlskp_err_i_12:1;        /* [Position:12] */
        /* : l2_rmlh_deskew_ctlskp_err[i*5+4:i*5+3]: Pulse: Margin CRC and Parity Error */
        volatile unsigned int l3_rmlh_ebuf_rxoverflow:1;        /* [Position:13] */
        /* : l3_rmlh_ebuf_rxoverflow: Pulse: EBUF Overflow */
        volatile unsigned int l3_rmlh_ebuf_rxunderflow:1;        /* [Position:14] */
        /* : l3_rmlh_ebuf_rxunderflow: Pulse: EBUF underrun */
        volatile unsigned int l3_rmlh_phy_rxcodeerror:1;        /* [Position:15] */
        /* : l3_rmlh_phy_rxcodeerror: Pulse: Decode Error */
        volatile unsigned int l3_rmlh_phy_rxdisperror:1;        /* [Position:16] */
        /* : l3_rmlh_phy_rxdisperror: Pulse: Running Disparity Error */
        volatile unsigned int l3_rmlh_skp_parity_err:1;        /* [Position:17] */
        /* : l3_rmlh_skp_parity_err: Pulse: SKP OS Parity Error */
        volatile unsigned int l3_rmlh_sync_header_err:1;        /* [Position:18] */
        /* : l3_rmlh_sync_header_err: Pulse: SYNC Header Error */
        volatile unsigned int l3_rmlh_rxvalid_deasserted:1;        /* [Position:19] */
        /* : l3_rmlh_rxvalid_deasserted: Pulse: Rx Valid de-assertion */
        volatile unsigned int l3_rmlh_ebuf_rxskipadded:1;        /* [Position:20] */
        /* : l3_rmlh_ebuf_rxskipadded: Pulse: EBUF SKP Add */
        volatile unsigned int l3_rmlh_ebuf_rxskipremoved:1;        /* [Position:21] */
        /* : l3_rmlh_ebuf_rxskipremoved: Pulse: EBUF SKP Del */
        volatile unsigned int l3_rmlh_deskew_ctlskp_erri_22:1;        /* [Position:22] */
        /* : l3_rmlh_deskew_ctlskp_err[i*5]: Pulse: CTL SKP OS Data Parity Error */
        volatile unsigned int l3_rmlh_deskew_ctlskp_erri_23:1;        /* [Position:23] */
        /* : l3_rmlh_deskew_ctlskp_err[i*5+1]: Pulse: 1st Retimer Parity Error */
        volatile unsigned int l3_rmlh_deskew_ctlskp_erri_24:1;        /* [Position:24] */
        /* : l3_rmlh_deskew_ctlskp_err[i*5+2]: Pulse: 2nd Retimer Parity Error */
        volatile unsigned int l3_rmlh_deskew_ctlskp_err_i_25:1;        /* [Position:25] */
        /* : l3_rmlh_deskew_ctlskp_err[i*5+4:i*5+3]: Pulse: Margin CRC and Parity Error */
        volatile unsigned int RESERVED_26:6; /* [Position:26] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_ras_ec_lxx_ro01_0_t;
/* offset : 0x47c */
typedef volatile union _acc_ras_sd_com_ro00_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int pm_master_state_4_:5;        /* [Position:0] */
        /* : pm_master_state[4:0]: Level: PM Internal State (Master)" */
        volatile unsigned int pm_slave_state_3_:4;        /* [Position:5] */
        /* : pm_slave_state[3:0]: Level: PM Internal State (Slave) */
        volatile unsigned int rmlh_framing_err_ptr_6_:7;        /* [Position:9] */
        /* : rmlh_framing_err_ptr[6:0]: Pulse: 1st Framing Error Pointer */
        volatile unsigned int smlh_lane_reversed:1;        /* [Position:16] */
        /* : smlh_lane_reversed: Level: Lane Reversal Operation */
        volatile unsigned int pm_pme_resend_flag:1;        /* [Position:17] */
        /* : pm_pme_resend_flag: Pulse: PME Re-Send flag */
        volatile unsigned int bit31:14; /* [Position:18] */
    };
} acc_ras_sd_com_ro00_0_t;

typedef volatile union _acc_ras_sd_com_ro00_1_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : smlh_ltssm_variable [15:0]: Level: LTSSM Variable */
        volatile unsigned int bit33:2;        /* [Position:32] */
        /* : smlh_ltssm_variable [15:0]: Level: LTSSM Variable" */
        volatile unsigned int ltssm_powerdown_1_:3;        /* [Position:34] */
        /* : ltssm_powerdown[1:0]: Level: PIPE: Power Down */
        volatile unsigned int latched_ts_nfts_7_:8;        /* [Position:37] */
        /* : latched_ts_nfts[7:0]: Level: Latched NFTS */
        volatile unsigned int bit46:2;        /* [Position:45] */
        /* : rdlh_dlcntrl_state [1:0]: Level: DLCM */
        volatile unsigned int rdlh_vc0_initfc1_status:1;        /* [Position:47] */
        /* : rdlh_vc0_initfc1_status: Level: Init-FC Flag1 VC0 */
        volatile unsigned int rdlh_vc0_initfc2_status:1;        /* [Position:48] */
        /* : rdlh_vc0_initfc2_status: Level: Init-FC Flag2 VC0 */
        volatile unsigned int rdlh_curnt_rx_ack_seqnum_11_:12;        /* [Position:49] */
        /* : rdlh_curnt_rx_ack_seqnum[11:0]: Level: Rx ACK SEQ# */
        volatile unsigned int bit63:3; /* [Position:61] */
    };
} acc_ras_sd_com_ro00_1_t;

typedef volatile union _acc_ras_sd_com_ro00_2_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : xdlh_curnt_seqnum [11:0]: Level: Tx TLP SEQ# */
        volatile unsigned int bit72:9;        /* [Position:64] */
        /* : xdlh_curnt_seqnum [11:0]: Level: Tx TLP SEQ#" */
        volatile unsigned int init_eq_pending:1;        /* [Position:73] */
        /* : init_eq_pending: Level: Equalization sequence Gen3 */
        volatile unsigned int init_eq_pending_g4:1;        /* [Position:74] */
        /* : init_eq_pending_g4: Level: Equalization sequence Gen4 */
        volatile unsigned int l1sub_state:3;        /* [Position:75] */
        /* : l1sub_state: Level: L1 sub state */
        volatile unsigned int init_eq_pending_g5:1;        /* [Position:78] */
        /* : init_eq_pending_g5: Level: Equalization sequence Gen5 */
    };
} acc_ras_sd_com_ro00_2_t;
/* offset : 0x488 */
typedef volatile union _acc_ras_sd_lxx_ro00_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mac_phy_txelec_txburst:1;        /* [Position:0] */
        /* : mac_phy_txelec_txburst: Level: PIPE: TxElecIdle/TxBurst */
        volatile unsigned int phy_mac_rxelec_rxh8exit:1;        /* [Position:1] */
        /* : phy_mac_rxelec_rxh8exit: Level: PIPE: RxElecIdle/RxHibern8ExitType1 */
        volatile unsigned int phy_mac_rxvalid_rxburst:1;        /* [Position:2] */
        /* : phy_mac_rxvalid_rxburst: Level: PIPE: RxValid/RxBurst */
        volatile unsigned int latched_rxdetected:1;        /* [Position:3] */
        /* : latched_rxdetected: Level: PIPE: Detect Lane */
        volatile unsigned int mac_phy_rxpolarity:1;        /* [Position:4] */
        /* : mac_phy_rxpolarity: Level: PIPE: RxPolarity */
        volatile unsigned int rmlh_deskew_fifo_ptr:8;        /* [Position:5] */
        /* : rmlh_deskew_fifo_ptr: Level: Deskew Pointer */
        volatile unsigned int mac_cdm_ras_des_fs:6;        /* [Position:13] */
        /* : mac_cdm_ras_des_fs: Level: Remote Device FS Gen3 */
        volatile unsigned int mac_cdm_ras_des_lf:6;        /* [Position:19] */
        /* : mac_cdm_ras_des_lf: Level: Remote Device LF Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_17_1:6;        /* [Position:25] */
        /* : mac_cdm_ras_des_coef_rtx[17:12]: Level: Current Remote Transmitter Post Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_11_:1; /* [Position:31] */
    };
} acc_ras_sd_lxx_ro00_0_t;

typedef volatile union _acc_ras_sd_lxx_ro00_1_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : mac_cdm_ras_des_coef_rtx[11:6]: Level: Current Remote Transmitter Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_11_:5;        /* [Position:32] */
        /* : mac_cdm_ras_des_coef_rtx[11:6]: Level: Current Remote Transmitter Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_5_:6;        /* [Position:37] */
        /* : mac_cdm_ras_des_coef_rtx[5:0]: Level: Current Remote Transmitter Pre Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_17_1:6;        /* [Position:43] */
        /* : mac_cdm_ras_des_coef_ltx[17:12]: Level: Current Local Transmitter Post Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_11_:6;        /* [Position:49] */
        /* : mac_cdm_ras_des_coef_ltx[11:6]: Level: Current Local Transmitter Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_5_:6;        /* [Position:55] */
        /* : mac_cdm_ras_des_coef_ltx[5:0]: Level: Current Local Transmitter Pre Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_pset_lrx:3; /* [Position:61] */
    };
} acc_ras_sd_lxx_ro00_1_t;

typedef volatile union _acc_ras_sd_lxx_ro00_2_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : mac_cdm_ras_des_pset_lrx: Level: Current Local Receiver Preset Hint Gen3 */
        volatile unsigned int phy_cdm_ras_des_fomfeedback:8;        /* [Position:64] */
        /* : phy_cdm_ras_des_fomfeedback: Level: Current Figure of Merit Gen3 */
        volatile unsigned int mac_cdm_ras_des_reject_rtx:1;        /* [Position:72] */
        /* : mac_cdm_ras_des_reject_rtx: Level: Receive Reject Coefficient Event status Gen3 */
        volatile unsigned int eqpa_violate_rule_1230:1;        /* [Position:73] */
        /* : eqpa_violate_rule_123[0]: Level: Rule A Violation Event Status Gen3 */
        volatile unsigned int eqpa_violate_rule_1231:1;        /* [Position:74] */
        /* : eqpa_violate_rule_123[1]: Level: Rule B Violation Event Status Gen3 */
        volatile unsigned int eqpa_violate_rule_1232:1;        /* [Position:75] */
        /* : eqpa_violate_rule_123[2]: Level: Rule C Violation Event Status Gen3 */
        volatile unsigned int bit77:2;        /* [Position:76] */
        /* : eq_convergence_sts [1:0]: Level: Equalization convergence information Gen3 */
        volatile unsigned int mac_cdm_ras_des_fs_g4:6;        /* [Position:78] */
        /* : mac_cdm_ras_des_fs_g4: Level: Remote Device FS Gen4 */
        volatile unsigned int mac_cdm_ras_des_lf_g4:6;        /* [Position:84] */
        /* : mac_cdm_ras_des_lf_g4: Level: Remote Device LF Gen4 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g4_17_1:6; /* [Position:90] */
    };
} acc_ras_sd_lxx_ro00_2_t;

typedef volatile union _acc_ras_sd_lxx_ro00_3_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : mac_cdm_ras_des_coef_rtx_g4[17:12]: Level: Current Remote Transmitter Post Cursor coefficient Gen4 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g4_11_:6;        /* [Position:96] */
        /* : mac_cdm_ras_des_coef_rtx_g4[11:6]: Level: Current Remote Transmitter Cursor coefficient Gen4 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g4_5_:6;        /* [Position:102] */
        /* : mac_cdm_ras_des_coef_rtx_g4[5:0]: Level: Current Remote Transmitter Pre Cursor coefficient Gen4 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_g4_17_1:6;        /* [Position:108] */
        /* : mac_cdm_ras_des_coef_ltx_g4[17:12]: Level: Current Local Transmitter Post Cursor coefficient Gen4 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_g4_11_:6;        /* [Position:114] */
        /* : mac_cdm_ras_des_coef_ltx_g4[11:6]: Level: Current Local Transmitter Cursor coefficient Gen4 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_g4_5_:6;        /* [Position:120] */
        /* : mac_cdm_ras_des_coef_ltx_g4[5:0]: Level: Current Local Transmitter Pre Cursor coefficient Gen4 */
        volatile unsigned int Reserved_126:2; /* [Position:126] */
    };
} acc_ras_sd_lxx_ro00_3_t;

typedef volatile union _acc_ras_sd_lxx_ro00_4_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : 3'b000: Reserved */
        volatile unsigned int Reserved_128:1;        /* [Position:128] */
        /* : 3'b000: Reserved */
        volatile unsigned int phy_cdm_ras_des_fomfeedback_g4:8;        /* [Position:129] */
        /* : phy_cdm_ras_des_fomfeedback_g4: Level: Current Figure of Merit Gen4 */
        volatile unsigned int mac_cdm_ras_des_reject_rtx_g4:1;        /* [Position:137] */
        /* : mac_cdm_ras_des_reject_rtx_g4: Level: Receive Reject Coefficient Event status Gen4 */
        volatile unsigned int eqpa_violate_rule_123_g40:1;        /* [Position:138] */
        /* : eqpa_violate_rule_123_g4[0]: Level: Rule A Violation Event Status Gen4 */
        volatile unsigned int eqpa_violate_rule_123_g41:1;        /* [Position:139] */
        /* : eqpa_violate_rule_123_g4[1]: Level: Rule B Violation Event Status Gen4 */
        volatile unsigned int eqpa_violate_rule_123_g42:1;        /* [Position:140] */
        /* : eqpa_violate_rule_123_g4[2]: Level: Rule C Violation Event Status Gen4 */
        volatile unsigned int bit142:2;        /* [Position:141] */
        /* : eq_convergence_sts_g4 [1:0]: Level: Equalization convergence information Gen4 */
        volatile unsigned int mac_cdm_ras_des_fs_g5:6;        /* [Position:143] */
        /* : mac_cdm_ras_des_fs_g5: Level: Remote Device FS Gen5 */
        volatile unsigned int mac_cdm_ras_des_lf_g5:6;        /* [Position:149] */
        /* : mac_cdm_ras_des_lf_g5: Level: Remote Device LF Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g5_17_1:5; /* [Position:155] */
    };
} acc_ras_sd_lxx_ro00_4_t;

typedef volatile union _acc_ras_sd_lxx_ro00_5_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : mac_cdm_ras_des_coef_rtx_g5[17:12]: Level: Current Remote Transmitter Post Cursor coefficient Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g5_17_1:1;        /* [Position:160] */
        /* : mac_cdm_ras_des_coef_rtx_g5[17:12]: Level: Current Remote Transmitter Post Cursor coefficient Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g5_11_:6;        /* [Position:161] */
        /* : mac_cdm_ras_des_coef_rtx_g5[11:6]: Level: Current Remote Transmitter Cursor coefficient Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g5_5_:6;        /* [Position:167] */
        /* : mac_cdm_ras_des_coef_rtx_g5[5:0]: Level: Current Remote Transmitter Pre Cursor coefficient Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_g5_17_1:6;        /* [Position:173] */
        /* : mac_cdm_ras_des_coef_ltx_g5[17:12]: Level: Current Local Transmitter Post Cursor coefficient Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_g5_11_:6;        /* [Position:179] */
        /* : mac_cdm_ras_des_coef_ltx_g5[11:6]: Level: Current Local Transmitter Cursor coefficient Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_g5_5_:6;        /* [Position:185] */
        /* : mac_cdm_ras_des_coef_ltx_g5[5:0]: Level: Current Local Transmitter Pre Cursor coefficient Gen5 */
        volatile unsigned int phy_cdm_ras_des_fomfeedback_g5:1; /* [Position:191] */
    };
} acc_ras_sd_lxx_ro00_5_t;

typedef volatile union _acc_ras_sd_lxx_ro00_6_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : phy_cdm_ras_des_fomfeedback_g5: Level: Current Figure of Merit Gen5 */
        volatile unsigned int phy_cdm_ras_des_fomfeedback_g5:7;        /* [Position:192] */
        /* : phy_cdm_ras_des_fomfeedback_g5: Level: Current Figure of Merit Gen5 */
        volatile unsigned int mac_cdm_ras_des_reject_rtx_g5:1;        /* [Position:199] */
        /* : mac_cdm_ras_des_reject_rtx_g5: Level: Receive Reject Coefficient Event status Gen5 */
        volatile unsigned int eqpa_violate_rule_123_g50:1;        /* [Position:200] */
        /* : eqpa_violate_rule_123_g5[0]: Level: Rule A Violation Event Status Gen5 */
        volatile unsigned int eqpa_violate_rule_123_g51:1;        /* [Position:201] */
        /* : eqpa_violate_rule_123_g5[1]: Level: Rule B Violation Event Status Gen5 */
        volatile unsigned int eqpa_violate_rule_123_g52:1;        /* [Position:202] */
        /* : eqpa_violate_rule_123_g5[2]: Level: Rule C Violation Event Status Gen5 */
        volatile unsigned int bit204:2;        /* [Position:203] */
        /* : eq_convergence_sts_g5 [1:0]: Level: Equalization convergence information Gen5 */
        volatile unsigned int mac_cdm_ras_des_fs_g6:6;        /* [Position:205] */
        /* : mac_cdm_ras_des_fs_g6: Level: Remote Device FS Gen6 */
        volatile unsigned int mac_cdm_ras_des_lf_g6:6;        /* [Position:211] */
        /* : mac_cdm_ras_des_lf_g6: Level: Remote Device LF Gen6 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g6_17_1:5;        /* [Position:217] */
        /* : mac_cdm_ras_des_coef_rtx_g6[17:13]: Level: Current Remote Transmitter Post Cursor coefficient Gen6 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g6_8_:2; /* [Position:222] */
    };
} acc_ras_sd_lxx_ro00_6_t;

typedef volatile union _acc_ras_sd_lxx_ro00_7_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : mac_cdm_ras_des_coef_rtx_g6[8:7]: Level: Current Remote Transmitter Cursor coefficient Gen6 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g6_12_:4;        /* [Position:224] */
        /* : mac_cdm_ras_des_coef_rtx_g6[12:9]: Level: Current Remote Transmitter Cursor coefficient Gen6 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g6_4_:2;        /* [Position:228] */
        /* : mac_cdm_ras_des_coef_rtx_g6[4:3]: Level: Current Remote Transmitter Pre Cursor coefficient Gen6 */
    };
} acc_ras_sd_lxx_ro00_7_t;
/* offset : 0x4a8 */
typedef volatile union _acc_ras_sd_lxx_ro08_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mac_phy_txelec_txburst:1;        /* [Position:0] */
        /* : mac_phy_txelec_txburst: Level: PIPE: TxElecIdle/TxBurst */
        volatile unsigned int phy_mac_rxelec_rxh8exit:1;        /* [Position:1] */
        /* : phy_mac_rxelec_rxh8exit: Level: PIPE: RxElecIdle/RxHibern8ExitType1 */
        volatile unsigned int phy_mac_rxvalid_rxburst:1;        /* [Position:2] */
        /* : phy_mac_rxvalid_rxburst: Level: PIPE: RxValid/RxBurst */
        volatile unsigned int latched_rxdetected:1;        /* [Position:3] */
        /* : latched_rxdetected: Level: PIPE: Detect Lane */
        volatile unsigned int mac_phy_rxpolarity:1;        /* [Position:4] */
        /* : mac_phy_rxpolarity: Level: PIPE: RxPolarity */
        volatile unsigned int rmlh_deskew_fifo_ptr:8;        /* [Position:5] */
        /* : rmlh_deskew_fifo_ptr: Level: Deskew Pointer */
        volatile unsigned int mac_cdm_ras_des_fs:6;        /* [Position:13] */
        /* : mac_cdm_ras_des_fs: Level: Remote Device FS Gen3 */
        volatile unsigned int mac_cdm_ras_des_lf:6;        /* [Position:19] */
        /* : mac_cdm_ras_des_lf: Level: Remote Device LF Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_17_1:6;        /* [Position:25] */
        /* : mac_cdm_ras_des_coef_rtx[17:12]: Level: Current Remote Transmitter Post Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_11_:1; /* [Position:31] */
    };
} acc_ras_sd_lxx_ro08_0_t;

typedef volatile union _acc_ras_sd_lxx_ro08_1_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : mac_cdm_ras_des_coef_rtx[11:6]: Level: Current Remote Transmitter Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_11_:5;        /* [Position:32] */
        /* : mac_cdm_ras_des_coef_rtx[11:6]: Level: Current Remote Transmitter Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_5_:6;        /* [Position:37] */
        /* : mac_cdm_ras_des_coef_rtx[5:0]: Level: Current Remote Transmitter Pre Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_17_1:6;        /* [Position:43] */
        /* : mac_cdm_ras_des_coef_ltx[17:12]: Level: Current Local Transmitter Post Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_11_:6;        /* [Position:49] */
        /* : mac_cdm_ras_des_coef_ltx[11:6]: Level: Current Local Transmitter Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_5_:6;        /* [Position:55] */
        /* : mac_cdm_ras_des_coef_ltx[5:0]: Level: Current Local Transmitter Pre Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_pset_lrx:3; /* [Position:61] */
    };
} acc_ras_sd_lxx_ro08_1_t;

typedef volatile union _acc_ras_sd_lxx_ro08_2_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : mac_cdm_ras_des_pset_lrx: Level: Current Local Receiver Preset Hint Gen3 */
        volatile unsigned int phy_cdm_ras_des_fomfeedback:8;        /* [Position:64] */
        /* : phy_cdm_ras_des_fomfeedback: Level: Current Figure of Merit Gen3" */
        volatile unsigned int mac_cdm_ras_des_reject_rtx:1;        /* [Position:72] */
        /* : mac_cdm_ras_des_reject_rtx: Level: Receive Reject Coefficient Event status Gen3 */
        volatile unsigned int eqpa_violate_rule_1230:1;        /* [Position:73] */
        /* : eqpa_violate_rule_123[0]: Level: Rule A Violation Event Status Gen3 */
        volatile unsigned int eqpa_violate_rule_1231:1;        /* [Position:74] */
        /* : eqpa_violate_rule_123[1]: Level: Rule B Violation Event Status Gen3 */
        volatile unsigned int eqpa_violate_rule_1232:1;        /* [Position:75] */
        /* : eqpa_violate_rule_123[2]: Level: Rule C Violation Event Status Gen3 */
        volatile unsigned int bit77:2;        /* [Position:76] */
        /* : eq_convergence_sts [1:0]: Level: Equalization convergence information Gen3 */
        volatile unsigned int mac_cdm_ras_des_fs_g4:6;        /* [Position:78] */
        /* : mac_cdm_ras_des_fs_g4: Level: Remote Device FS Gen4 */
        volatile unsigned int mac_cdm_ras_des_lf_g4:6;        /* [Position:84] */
        /* : mac_cdm_ras_des_lf_g4: Level: Remote Device LF Gen4 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g4_17_1:6; /* [Position:90] */
    };
} acc_ras_sd_lxx_ro08_2_t;

typedef volatile union _acc_ras_sd_lxx_ro08_3_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : mac_cdm_ras_des_coef_rtx_g4[17:12]: Level: Current Remote Transmitter Post Cursor coefficient Gen4 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g4_11_:6;        /* [Position:96] */
        /* : mac_cdm_ras_des_coef_rtx_g4[11:6]: Level: Current Remote Transmitter Cursor coefficient Gen4" */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g4_5_:6;        /* [Position:102] */
        /* : mac_cdm_ras_des_coef_rtx_g4[5:0]: Level: Current Remote Transmitter Pre Cursor coefficient Gen4 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_g4_17_1:6;        /* [Position:108] */
        /* : mac_cdm_ras_des_coef_ltx_g4[17:12]: Level: Current Local Transmitter Post Cursor coefficient Gen4 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_g4_11_:6;        /* [Position:114] */
        /* : mac_cdm_ras_des_coef_ltx_g4[11:6]: Level: Current Local Transmitter Cursor coefficient Gen4 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_g4_5_:6;        /* [Position:120] */
        /* : mac_cdm_ras_des_coef_ltx_g4[5:0]: Level: Current Local Transmitter Pre Cursor coefficient Gen4 */
        volatile unsigned int Reserved_126:2; /* [Position:126] */
    };
} acc_ras_sd_lxx_ro08_3_t;

typedef volatile union _acc_ras_sd_lxx_ro08_4_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : 3'b000: Reserved */
        volatile unsigned int Reserved_128:1;        /* [Position:128] */
        /* : 3'b000: Reserved */
        volatile unsigned int phy_cdm_ras_des_fomfeedback_g4:8;        /* [Position:129] */
        /* : phy_cdm_ras_des_fomfeedback_g4: Level: Current Figure of Merit Gen4 */
        volatile unsigned int mac_cdm_ras_des_reject_rtx_g4:1;        /* [Position:137] */
        /* : mac_cdm_ras_des_reject_rtx_g4: Level: Receive Reject Coefficient Event status Gen4 */
        volatile unsigned int eqpa_violate_rule_123_g40:1;        /* [Position:138] */
        /* : eqpa_violate_rule_123_g4[0]: Level: Rule A Violation Event Status Gen4 */
        volatile unsigned int eqpa_violate_rule_123_g41:1;        /* [Position:139] */
        /* : eqpa_violate_rule_123_g4[1]: Level: Rule B Violation Event Status Gen4 */
        volatile unsigned int eqpa_violate_rule_123_g42:1;        /* [Position:140] */
        /* : eqpa_violate_rule_123_g4[2]: Level: Rule C Violation Event Status Gen4 */
        volatile unsigned int bit142:2;        /* [Position:141] */
        /* : eq_convergence_sts_g4 [1:0]: Level: Equalization convergence information Gen4 */
        volatile unsigned int mac_cdm_ras_des_fs_g5:6;        /* [Position:143] */
        /* : mac_cdm_ras_des_fs_g5: Level: Remote Device FS Gen5 */
        volatile unsigned int mac_cdm_ras_des_lf_g5:6;        /* [Position:149] */
        /* : mac_cdm_ras_des_lf_g5: Level: Remote Device LF Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g5_17_1:5; /* [Position:155] */
    };
} acc_ras_sd_lxx_ro08_4_t;

typedef volatile union _acc_ras_sd_lxx_ro08_5_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : mac_cdm_ras_des_coef_rtx_g5[17:12]: Level: Current Remote Transmitter Post Cursor coefficient Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g5_17_1:1;        /* [Position:160] */
        /* : mac_cdm_ras_des_coef_rtx_g5[17:12]: Level: Current Remote Transmitter Post Cursor coefficient Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g5_11_:6;        /* [Position:161] */
        /* : mac_cdm_ras_des_coef_rtx_g5[11:6]: Level: Current Remote Transmitter Cursor coefficient Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g5_5_:6;        /* [Position:167] */
        /* : mac_cdm_ras_des_coef_rtx_g5[5:0]: Level: Current Remote Transmitter Pre Cursor coefficient Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_g5_17_1:6;        /* [Position:173] */
        /* : mac_cdm_ras_des_coef_ltx_g5[17:12]: Level: Current Local Transmitter Post Cursor coefficient Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_g5_11_:6;        /* [Position:179] */
        /* : mac_cdm_ras_des_coef_ltx_g5[11:6]: Level: Current Local Transmitter Cursor coefficient Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_g5_5_:6;        /* [Position:185] */
        /* : mac_cdm_ras_des_coef_ltx_g5[5:0]: Level: Current Local Transmitter Pre Cursor coefficient Gen5 */
        volatile unsigned int phy_cdm_ras_des_fomfeedback_g5:1; /* [Position:191] */
    };
} acc_ras_sd_lxx_ro08_5_t;

typedef volatile union _acc_ras_sd_lxx_ro08_6_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : phy_cdm_ras_des_fomfeedback_g5: Level: Current Figure of Merit Gen5 */
        volatile unsigned int phy_cdm_ras_des_fomfeedback_g5:7;        /* [Position:192] */
        /* : phy_cdm_ras_des_fomfeedback_g5: Level: Current Figure of Merit Gen5 */
        volatile unsigned int mac_cdm_ras_des_reject_rtx_g5:1;        /* [Position:199] */
        /* : mac_cdm_ras_des_reject_rtx_g5: Level: Receive Reject Coefficient Event status Gen5 */
        volatile unsigned int eqpa_violate_rule_123_g50:1;        /* [Position:200] */
        /* : eqpa_violate_rule_123_g5[0]: Level: Rule A Violation Event Status Gen5 */
        volatile unsigned int eqpa_violate_rule_123_g51:1;        /* [Position:201] */
        /* : eqpa_violate_rule_123_g5[1]: Level: Rule B Violation Event Status Gen5 */
        volatile unsigned int eqpa_violate_rule_123_g52:1;        /* [Position:202] */
        /* : eqpa_violate_rule_123_g5[2]: Level: Rule C Violation Event Status Gen5 */
        volatile unsigned int bit204:2;        /* [Position:203] */
        /* : eq_convergence_sts_g5 [1:0]: Level: Equalization convergence information Gen5 */
        volatile unsigned int mac_cdm_ras_des_fs_g6:6;        /* [Position:205] */
        /* : mac_cdm_ras_des_fs_g6: Level: Remote Device FS Gen6 */
        volatile unsigned int mac_cdm_ras_des_lf_g6:6;        /* [Position:211] */
        /* : mac_cdm_ras_des_lf_g6: Level: Remote Device LF Gen6 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g6_17_1:5;        /* [Position:217] */
        /* : mac_cdm_ras_des_coef_rtx_g6[17:13]: Level: Current Remote Transmitter Post Cursor coefficient Gen6 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g6_8_:2; /* [Position:222] */
    };
} acc_ras_sd_lxx_ro08_6_t;

typedef volatile union _acc_ras_sd_lxx_ro08_7_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : mac_cdm_ras_des_coef_rtx_g6[8:7]: Level: Current Remote Transmitter Cursor coefficient Gen6 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g6_12_:4;        /* [Position:224] */
        /* : mac_cdm_ras_des_coef_rtx_g6[12:9]: Level: Current Remote Transmitter Cursor coefficient Gen6 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g6_4_:2;        /* [Position:228] */
        /* : mac_cdm_ras_des_coef_rtx_g6[4:3]: Level: Current Remote Transmitter Pre Cursor coefficient Gen6 */
    };
} acc_ras_sd_lxx_ro08_7_t;
/* offset : 0x4c8 */
typedef volatile union _acc_ras_sd_lxx_ro16_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mac_phy_txelec_txburst:1;        /* [Position:0] */
        /* : mac_phy_txelec_txburst: Level: PIPE: TxElecIdle/TxBurst */
        volatile unsigned int phy_mac_rxelec_rxh8exit:1;        /* [Position:1] */
        /* : phy_mac_rxelec_rxh8exit: Level: PIPE: RxElecIdle/RxHibern8ExitType1 */
        volatile unsigned int phy_mac_rxvalid_rxburst:1;        /* [Position:2] */
        /* : phy_mac_rxvalid_rxburst: Level: PIPE: RxValid/RxBurst */
        volatile unsigned int latched_rxdetected:1;        /* [Position:3] */
        /* : latched_rxdetected: Level: PIPE: Detect Lane */
        volatile unsigned int mac_phy_rxpolarity:1;        /* [Position:4] */
        /* : mac_phy_rxpolarity: Level: PIPE: RxPolarity */
        volatile unsigned int rmlh_deskew_fifo_ptr:8;        /* [Position:5] */
        /* : rmlh_deskew_fifo_ptr: Level: Deskew Pointer */
        volatile unsigned int mac_cdm_ras_des_fs:6;        /* [Position:13] */
        /* : mac_cdm_ras_des_fs: Level: Remote Device FS Gen3 */
        volatile unsigned int mac_cdm_ras_des_lf:6;        /* [Position:19] */
        /* : mac_cdm_ras_des_lf: Level: Remote Device LF Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_17_1:6;        /* [Position:25] */
        /* : mac_cdm_ras_des_coef_rtx[17:12]: Level: Current Remote Transmitter Post Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_11_:1; /* [Position:31] */
    };
} acc_ras_sd_lxx_ro16_0_t;

typedef volatile union _acc_ras_sd_lxx_ro16_1_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : mac_cdm_ras_des_coef_rtx[11:6]: Level: Current Remote Transmitter Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_11_:5;        /* [Position:32] */
        /* : mac_cdm_ras_des_coef_rtx[11:6]: Level: Current Remote Transmitter Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_5_:6;        /* [Position:37] */
        /* : mac_cdm_ras_des_coef_rtx[5:0]: Level: Current Remote Transmitter Pre Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_17_1:6;        /* [Position:43] */
        /* : mac_cdm_ras_des_coef_ltx[17:12]: Level: Current Local Transmitter Post Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_11_:6;        /* [Position:49] */
        /* : mac_cdm_ras_des_coef_ltx[11:6]: Level: Current Local Transmitter Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_5_:6;        /* [Position:55] */
        /* : mac_cdm_ras_des_coef_ltx[5:0]: Level: Current Local Transmitter Pre Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_pset_lrx:3; /* [Position:61] */
    };
} acc_ras_sd_lxx_ro16_1_t;

typedef volatile union _acc_ras_sd_lxx_ro16_2_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : mac_cdm_ras_des_pset_lrx: Level: Current Local Receiver Preset Hint Gen3 */
        volatile unsigned int phy_cdm_ras_des_fomfeedback:8;        /* [Position:64] */
        /* : phy_cdm_ras_des_fomfeedback: Level: Current Figure of Merit Gen3 */
        volatile unsigned int mac_cdm_ras_des_reject_rtx:1;        /* [Position:72] */
        /* : mac_cdm_ras_des_reject_rtx: Level: Receive Reject Coefficient Event status Gen3 */
        volatile unsigned int eqpa_violate_rule_1230:1;        /* [Position:73] */
        /* : eqpa_violate_rule_123[0]: Level: Rule A Violation Event Status Gen3 */
        volatile unsigned int eqpa_violate_rule_1231:1;        /* [Position:74] */
        /* : eqpa_violate_rule_123[1]: Level: Rule B Violation Event Status Gen3 */
        volatile unsigned int eqpa_violate_rule_1232:1;        /* [Position:75] */
        /* : eqpa_violate_rule_123[2]: Level: Rule C Violation Event Status Gen3 */
        volatile unsigned int bit77:2;        /* [Position:76] */
        /* : eq_convergence_sts [1:0]: Level: Equalization convergence information Gen3 */
        volatile unsigned int mac_cdm_ras_des_fs_g4:6;        /* [Position:78] */
        /* : mac_cdm_ras_des_fs_g4: Level: Remote Device FS Gen4 */
        volatile unsigned int mac_cdm_ras_des_lf_g4:6;        /* [Position:84] */
        /* : mac_cdm_ras_des_lf_g4: Level: Remote Device LF Gen4 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g4_17_1:6; /* [Position:90] */
    };
} acc_ras_sd_lxx_ro16_2_t;

typedef volatile union _acc_ras_sd_lxx_ro16_3_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : mac_cdm_ras_des_coef_rtx_g4[17:12]: Level: Current Remote Transmitter Post Cursor coefficient Gen4 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g4_11_:6;        /* [Position:96] */
        /* : mac_cdm_ras_des_coef_rtx_g4[11:6]: Level: Current Remote Transmitter Cursor coefficient Gen4 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g4_5_:6;        /* [Position:102] */
        /* : mac_cdm_ras_des_coef_rtx_g4[5:0]: Level: Current Remote Transmitter Pre Cursor coefficient Gen4 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_g4_17_1:6;        /* [Position:108] */
        /* : mac_cdm_ras_des_coef_ltx_g4[17:12]: Level: Current Local Transmitter Post Cursor coefficient Gen4 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_g4_11_:6;        /* [Position:114] */
        /* : mac_cdm_ras_des_coef_ltx_g4[11:6]: Level: Current Local Transmitter Cursor coefficient Gen4 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_g4_5_:6;        /* [Position:120] */
        /* : mac_cdm_ras_des_coef_ltx_g4[5:0]: Level: Current Local Transmitter Pre Cursor coefficient Gen4 */
        volatile unsigned int Reserved_126:2; /* [Position:126] */
    };
} acc_ras_sd_lxx_ro16_3_t;

typedef volatile union _acc_ras_sd_lxx_ro16_4_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : 3'b000: Reserved */
        volatile unsigned int Reserved_128:1;        /* [Position:128] */
        /* : 3'b000: Reserved */
        volatile unsigned int phy_cdm_ras_des_fomfeedback_g4:8;        /* [Position:129] */
        /* : phy_cdm_ras_des_fomfeedback_g4: Level: Current Figure of Merit Gen4 */
        volatile unsigned int mac_cdm_ras_des_reject_rtx_g4:1;        /* [Position:137] */
        /* : mac_cdm_ras_des_reject_rtx_g4: Level: Receive Reject Coefficient Event status Gen4 */
        volatile unsigned int eqpa_violate_rule_123_g40:1;        /* [Position:138] */
        /* : eqpa_violate_rule_123_g4[0]: Level: Rule A Violation Event Status Gen4 */
        volatile unsigned int eqpa_violate_rule_123_g41:1;        /* [Position:139] */
        /* : eqpa_violate_rule_123_g4[1]: Level: Rule B Violation Event Status Gen4 */
        volatile unsigned int eqpa_violate_rule_123_g42:1;        /* [Position:140] */
        /* : eqpa_violate_rule_123_g4[2]: Level: Rule C Violation Event Status Gen4 */
        volatile unsigned int bit142:2;        /* [Position:141] */
        /* : eq_convergence_sts_g4 [1:0]: Level: Equalization convergence information Gen4 */
        volatile unsigned int mac_cdm_ras_des_fs_g5:6;        /* [Position:143] */
        /* : mac_cdm_ras_des_fs_g5: Level: Remote Device FS Gen5 */
        volatile unsigned int mac_cdm_ras_des_lf_g5:6;        /* [Position:149] */
        /* : mac_cdm_ras_des_lf_g5: Level: Remote Device LF Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g5_17_1:5; /* [Position:155] */
    };
} acc_ras_sd_lxx_ro16_4_t;

typedef volatile union _acc_ras_sd_lxx_ro16_5_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : mac_cdm_ras_des_coef_rtx_g5[17:12]: Level: Current Remote Transmitter Post Cursor coefficient Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g5_17_1:1;        /* [Position:160] */
        /* : mac_cdm_ras_des_coef_rtx_g5[17:12]: Level: Current Remote Transmitter Post Cursor coefficient Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g5_11_:6;        /* [Position:161] */
        /* : mac_cdm_ras_des_coef_rtx_g5[11:6]: Level: Current Remote Transmitter Cursor coefficient Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g5_5_:6;        /* [Position:167] */
        /* : mac_cdm_ras_des_coef_rtx_g5[5:0]: Level: Current Remote Transmitter Pre Cursor coefficient Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_g5_17_1:6;        /* [Position:173] */
        /* : mac_cdm_ras_des_coef_ltx_g5[17:12]: Level: Current Local Transmitter Post Cursor coefficient Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_g5_11_:6;        /* [Position:179] */
        /* : mac_cdm_ras_des_coef_ltx_g5[11:6]: Level: Current Local Transmitter Cursor coefficient Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_g5_5_:6;        /* [Position:185] */
        /* : mac_cdm_ras_des_coef_ltx_g5[5:0]: Level: Current Local Transmitter Pre Cursor coefficient Gen5 */
        volatile unsigned int phy_cdm_ras_des_fomfeedback_g5:1; /* [Position:191] */
    };
} acc_ras_sd_lxx_ro16_5_t;

typedef volatile union _acc_ras_sd_lxx_ro16_6_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : phy_cdm_ras_des_fomfeedback_g5: Level: Current Figure of Merit Gen5 */
        volatile unsigned int phy_cdm_ras_des_fomfeedback_g5:7;        /* [Position:192] */
        /* : phy_cdm_ras_des_fomfeedback_g5: Level: Current Figure of Merit Gen5 */
        volatile unsigned int mac_cdm_ras_des_reject_rtx_g5:1;        /* [Position:199] */
        /* : mac_cdm_ras_des_reject_rtx_g5: Level: Receive Reject Coefficient Event status Gen5 */
        volatile unsigned int eqpa_violate_rule_123_g50:1;        /* [Position:200] */
        /* : eqpa_violate_rule_123_g5[0]: Level: Rule A Violation Event Status Gen5 */
        volatile unsigned int eqpa_violate_rule_123_g51:1;        /* [Position:201] */
        /* : eqpa_violate_rule_123_g5[1]: Level: Rule B Violation Event Status Gen5 */
        volatile unsigned int eqpa_violate_rule_123_g52:1;        /* [Position:202] */
        /* : eqpa_violate_rule_123_g5[2]: Level: Rule C Violation Event Status Gen5 */
        volatile unsigned int bit204:2;        /* [Position:203] */
        /* : eq_convergence_sts_g5 [1:0]: Level: Equalization convergence information Gen5 */
        volatile unsigned int mac_cdm_ras_des_fs_g6:6;        /* [Position:205] */
        /* : mac_cdm_ras_des_fs_g6: Level: Remote Device FS Gen6 */
        volatile unsigned int mac_cdm_ras_des_lf_g6:6;        /* [Position:211] */
        /* : mac_cdm_ras_des_lf_g6: Level: Remote Device LF Gen6 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g6_17_1:5;        /* [Position:217] */
        /* : mac_cdm_ras_des_coef_rtx_g6[17:13]: Level: Current Remote Transmitter Post Cursor coefficient Gen6 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g6_8_:2; /* [Position:222] */
    };
} acc_ras_sd_lxx_ro16_6_t;

typedef volatile union _acc_ras_sd_lxx_ro16_7_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : mac_cdm_ras_des_coef_rtx_g6[8:7]: Level: Current Remote Transmitter Cursor coefficient Gen6 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g6_12_:4;        /* [Position:224] */
        /* : mac_cdm_ras_des_coef_rtx_g6[12:9]: Level: Current Remote Transmitter Cursor coefficient Gen6 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g6_4_:2;        /* [Position:228] */
        /* : mac_cdm_ras_des_coef_rtx_g6[4:3]: Level: Current Remote Transmitter Pre Cursor coefficient Gen6 */
    };
} acc_ras_sd_lxx_ro16_7_t;
/* offset : 0x4e8 */
typedef volatile union _acc_ras_sd_lxx_ro24_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mac_phy_txelec_txburst:1;        /* [Position:0] */
        /* : mac_phy_txelec_txburst: Level: PIPE: TxElecIdle/TxBurst */
        volatile unsigned int phy_mac_rxelec_rxh8exit:1;        /* [Position:1] */
        /* : phy_mac_rxelec_rxh8exit: Level: PIPE: RxElecIdle/RxHibern8ExitType1 */
        volatile unsigned int phy_mac_rxvalid_rxburst:1;        /* [Position:2] */
        /* : phy_mac_rxvalid_rxburst: Level: PIPE: RxValid/RxBurst */
        volatile unsigned int latched_rxdetected:1;        /* [Position:3] */
        /* : latched_rxdetected: Level: PIPE: Detect Lane */
        volatile unsigned int mac_phy_rxpolarity:1;        /* [Position:4] */
        /* : mac_phy_rxpolarity: Level: PIPE: RxPolarity */
        volatile unsigned int rmlh_deskew_fifo_ptr:8;        /* [Position:5] */
        /* : rmlh_deskew_fifo_ptr: Level: Deskew Pointer */
        volatile unsigned int mac_cdm_ras_des_fs:6;        /* [Position:13] */
        /* : mac_cdm_ras_des_fs: Level: Remote Device FS Gen3 */
        volatile unsigned int mac_cdm_ras_des_lf:6;        /* [Position:19] */
        /* : mac_cdm_ras_des_lf: Level: Remote Device LF Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_17_1:6;        /* [Position:25] */
        /* : mac_cdm_ras_des_coef_rtx[17:12]: Level: Current Remote Transmitter Post Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_11_:1; /* [Position:31] */
    };
} acc_ras_sd_lxx_ro24_0_t;

typedef volatile union _acc_ras_sd_lxx_ro24_1_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : mac_cdm_ras_des_coef_rtx[11:6]: Level: Current Remote Transmitter Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_11_:5;        /* [Position:32] */
        /* : mac_cdm_ras_des_coef_rtx[11:6]: Level: Current Remote Transmitter Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_5_:6;        /* [Position:37] */
        /* : mac_cdm_ras_des_coef_rtx[5:0]: Level: Current Remote Transmitter Pre Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_17_1:6;        /* [Position:43] */
        /* : mac_cdm_ras_des_coef_ltx[17:12]: Level: Current Local Transmitter Post Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_11_:6;        /* [Position:49] */
        /* : mac_cdm_ras_des_coef_ltx[11:6]: Level: Current Local Transmitter Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_5_:6;        /* [Position:55] */
        /* : mac_cdm_ras_des_coef_ltx[5:0]: Level: Current Local Transmitter Pre Cursor coefficient Gen3 */
        volatile unsigned int mac_cdm_ras_des_pset_lrx:3; /* [Position:61] */
    };
} acc_ras_sd_lxx_ro24_1_t;

typedef volatile union _acc_ras_sd_lxx_ro24_2_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : mac_cdm_ras_des_pset_lrx: Level: Current Local Receiver Preset Hint Gen3 */
        volatile unsigned int phy_cdm_ras_des_fomfeedback:8;        /* [Position:64] */
        /* : phy_cdm_ras_des_fomfeedback: Level: Current Figure of Merit Gen3 */
        volatile unsigned int mac_cdm_ras_des_reject_rtx:1;        /* [Position:72] */
        /* : mac_cdm_ras_des_reject_rtx: Level: Receive Reject Coefficient Event status Gen3 */
        volatile unsigned int eqpa_violate_rule_1230:1;        /* [Position:73] */
        /* : eqpa_violate_rule_123[0]: Level: Rule A Violation Event Status Gen3 */
        volatile unsigned int eqpa_violate_rule_1231:1;        /* [Position:74] */
        /* : eqpa_violate_rule_123[1]: Level: Rule B Violation Event Status Gen3 */
        volatile unsigned int eqpa_violate_rule_1232:1;        /* [Position:75] */
        /* : eqpa_violate_rule_123[2]: Level: Rule C Violation Event Status Gen3 */
        volatile unsigned int bit77:2;        /* [Position:76] */
        /* : eq_convergence_sts [1:0]: Level: Equalization convergence information Gen3 */
        volatile unsigned int mac_cdm_ras_des_fs_g4:6;        /* [Position:78] */
        /* : mac_cdm_ras_des_fs_g4: Level: Remote Device FS Gen4 */
        volatile unsigned int mac_cdm_ras_des_lf_g4:6;        /* [Position:84] */
        /* : mac_cdm_ras_des_lf_g4: Level: Remote Device LF Gen4 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g4_17_1:6; /* [Position:90] */
    };
} acc_ras_sd_lxx_ro24_2_t;

typedef volatile union _acc_ras_sd_lxx_ro24_3_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : mac_cdm_ras_des_coef_rtx_g4[17:12]: Level: Current Remote Transmitter Post Cursor coefficient Gen4 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g4_11_:6;        /* [Position:96] */
        /* : mac_cdm_ras_des_coef_rtx_g4[11:6]: Level: Current Remote Transmitter Cursor coefficient Gen4 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g4_5_:6;        /* [Position:102] */
        /* : mac_cdm_ras_des_coef_rtx_g4[5:0]: Level: Current Remote Transmitter Pre Cursor coefficient Gen4 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_g4_17_1:6;        /* [Position:108] */
        /* : mac_cdm_ras_des_coef_ltx_g4[17:12]: Level: Current Local Transmitter Post Cursor coefficient Gen4 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_g4_11_:6;        /* [Position:114] */
        /* : mac_cdm_ras_des_coef_ltx_g4[11:6]: Level: Current Local Transmitter Cursor coefficient Gen4 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_g4_5_:6;        /* [Position:120] */
        /* : mac_cdm_ras_des_coef_ltx_g4[5:0]: Level: Current Local Transmitter Pre Cursor coefficient Gen4 */
        volatile unsigned int Reserved_126:2; /* [Position:126] */
    };
} acc_ras_sd_lxx_ro24_3_t;

typedef volatile union _acc_ras_sd_lxx_ro24_4_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : 3'b000: Reserved */
        volatile unsigned int Reserved_128:1;        /* [Position:128] */
        /* : 3'b000: Reserved */
        volatile unsigned int phy_cdm_ras_des_fomfeedback_g4:8;        /* [Position:129] */
        /* : phy_cdm_ras_des_fomfeedback_g4: Level: Current Figure of Merit Gen4 */
        volatile unsigned int mac_cdm_ras_des_reject_rtx_g4:1;        /* [Position:137] */
        /* : mac_cdm_ras_des_reject_rtx_g4: Level: Receive Reject Coefficient Event status Gen4 */
        volatile unsigned int eqpa_violate_rule_123_g40:1;        /* [Position:138] */
        /* : eqpa_violate_rule_123_g4[0]: Level: Rule A Violation Event Status Gen4 */
        volatile unsigned int eqpa_violate_rule_123_g41:1;        /* [Position:139] */
        /* : eqpa_violate_rule_123_g4[1]: Level: Rule B Violation Event Status Gen4 */
        volatile unsigned int eqpa_violate_rule_123_g42:1;        /* [Position:140] */
        /* : eqpa_violate_rule_123_g4[2]: Level: Rule C Violation Event Status Gen4 */
        volatile unsigned int bit142:2;        /* [Position:141] */
        /* : eq_convergence_sts_g4 [1:0]: Level: Equalization convergence information Gen4 */
        volatile unsigned int mac_cdm_ras_des_fs_g5:6;        /* [Position:143] */
        /* : mac_cdm_ras_des_fs_g5: Level: Remote Device FS Gen5 */
        volatile unsigned int mac_cdm_ras_des_lf_g5:6;        /* [Position:149] */
        /* : mac_cdm_ras_des_lf_g5: Level: Remote Device LF Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g5_17_1:5; /* [Position:155] */
    };
} acc_ras_sd_lxx_ro24_4_t;

typedef volatile union _acc_ras_sd_lxx_ro24_5_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : mac_cdm_ras_des_coef_rtx_g5[17:12]: Level: Current Remote Transmitter Post Cursor coefficient Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g5_17_1:1;        /* [Position:160] */
        /* : mac_cdm_ras_des_coef_rtx_g5[17:12]: Level: Current Remote Transmitter Post Cursor coefficient Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g5_11_:6;        /* [Position:161] */
        /* : mac_cdm_ras_des_coef_rtx_g5[11:6]: Level: Current Remote Transmitter Cursor coefficient Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g5_5_:6;        /* [Position:167] */
        /* : mac_cdm_ras_des_coef_rtx_g5[5:0]: Level: Current Remote Transmitter Pre Cursor coefficient Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_g5_17_1:6;        /* [Position:173] */
        /* : mac_cdm_ras_des_coef_ltx_g5[17:12]: Level: Current Local Transmitter Post Cursor coefficient Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_g5_11_:6;        /* [Position:179] */
        /* : mac_cdm_ras_des_coef_ltx_g5[11:6]: Level: Current Local Transmitter Cursor coefficient Gen5 */
        volatile unsigned int mac_cdm_ras_des_coef_ltx_g5_5_:6;        /* [Position:185] */
        /* : mac_cdm_ras_des_coef_ltx_g5[5:0]: Level: Current Local Transmitter Pre Cursor coefficient Gen5 */
        volatile unsigned int phy_cdm_ras_des_fomfeedback_g5:1; /* [Position:191] */
    };
} acc_ras_sd_lxx_ro24_5_t;

typedef volatile union _acc_ras_sd_lxx_ro24_6_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : phy_cdm_ras_des_fomfeedback_g5: Level: Current Figure of Merit Gen5 */
        volatile unsigned int phy_cdm_ras_des_fomfeedback_g5:7;        /* [Position:192] */
        /* : phy_cdm_ras_des_fomfeedback_g5: Level: Current Figure of Merit Gen5 */
        volatile unsigned int mac_cdm_ras_des_reject_rtx_g5:1;        /* [Position:199] */
        /* : mac_cdm_ras_des_reject_rtx_g5: Level: Receive Reject Coefficient Event status Gen5 */
        volatile unsigned int eqpa_violate_rule_123_g50:1;        /* [Position:200] */
        /* : eqpa_violate_rule_123_g5[0]: Level: Rule A Violation Event Status Gen5 */
        volatile unsigned int eqpa_violate_rule_123_g51:1;        /* [Position:201] */
        /* : eqpa_violate_rule_123_g5[1]: Level: Rule B Violation Event Status Gen5 */
        volatile unsigned int eqpa_violate_rule_123_g52:1;        /* [Position:202] */
        /* : eqpa_violate_rule_123_g5[2]: Level: Rule C Violation Event Status Gen5 */
        volatile unsigned int bit204:2;        /* [Position:203] */
        /* : eq_convergence_sts_g5 [1:0]: Level: Equalization convergence information Gen5 */
        volatile unsigned int mac_cdm_ras_des_fs_g6:6;        /* [Position:205] */
        /* : mac_cdm_ras_des_fs_g6: Level: Remote Device FS Gen6 */
        volatile unsigned int mac_cdm_ras_des_lf_g6:6;        /* [Position:211] */
        /* : mac_cdm_ras_des_lf_g6: Level: Remote Device LF Gen6 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g6_17_1:5;        /* [Position:217] */
        /* : mac_cdm_ras_des_coef_rtx_g6[17:13]: Level: Current Remote Transmitter Post Cursor coefficient Gen6 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g6_8_:2; /* [Position:222] */
    };
} acc_ras_sd_lxx_ro24_6_t;

typedef volatile union _acc_ras_sd_lxx_ro24_7_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : mac_cdm_ras_des_coef_rtx_g6[8:7]: Level: Current Remote Transmitter Cursor coefficient Gen6 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g6_12_:4;        /* [Position:224] */
        /* : mac_cdm_ras_des_coef_rtx_g6[12:9]: Level: Current Remote Transmitter Cursor coefficient Gen6 */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g6_4_:2;        /* [Position:228] */
        /* : mac_cdm_ras_des_coef_rtx_g6[4:3]: Level: Current Remote Transmitter Pre Cursor coefficient Gen6 */
    };
} acc_ras_sd_lxx_ro24_7_t;
/* offset : 0x508 */
typedef volatile union _acc_ras_sd_vxx_ro00_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int xadm_fc_cnsmd_ph:8;        /* [Position:0] */
        /* : xadm_fc_cnsmd_ph: Level: Credit Consumed (PH) VCn */
        volatile unsigned int xadm_fc_cnsmd_pd:12;        /* [Position:8] */
        /* : xadm_fc_cnsmd_pd: Level: Credit Consumed (PD) VCn */
        volatile unsigned int xadm_fc_cnsmd_nph:8;        /* [Position:20] */
        /* : xadm_fc_cnsmd_nph: Level: Credit Consumed (NH) VCn */
        volatile unsigned int xadm_fc_cnsmd_npd:4; /* [Position:28] */
    };
} acc_ras_sd_vxx_ro00_0_t;

typedef volatile union _acc_ras_sd_vxx_ro00_1_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : xadm_fc_cnsmd_npd: Level: Credit Consumed (ND) VCn */
        volatile unsigned int xadm_fc_cnsmd_npd:8;        /* [Position:32] */
        /* : xadm_fc_cnsmd_npd: Level: Credit Consumed (ND) VCn */
        volatile unsigned int xadm_fc_cnsmd_cplh:8;        /* [Position:40] */
        /* : xadm_fc_cnsmd_cplh: Level: Credit Consumed (CH) VCn */
        volatile unsigned int xadm_fc_cnsmd_cpld:12;        /* [Position:48] */
        /* : xadm_fc_cnsmd_cpld: Level: Credit Consumed (CD) VCn */
        volatile unsigned int xadm_fc_limit_ph:4; /* [Position:60] */
    };
} acc_ras_sd_vxx_ro00_1_t;

typedef volatile union _acc_ras_sd_vxx_ro00_2_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : xadm_fc_limit_ph: Level: Credit Limit (PH) VCn */
        volatile unsigned int xadm_fc_limit_ph:4;        /* [Position:64] */
        /* : xadm_fc_limit_ph: Level: Credit Limit (PH) VCn */
        volatile unsigned int xadm_fc_limit_pd:12;        /* [Position:68] */
        /* : xadm_fc_limit_pd: Level: Credit Limit (PD) VCn */
        volatile unsigned int xadm_fc_limit_nph:8;        /* [Position:80] */
        /* : xadm_fc_limit_nph: Level: Credit Limit (NH) VCn */
        volatile unsigned int xadm_fc_limit_npd:8; /* [Position:88] */
    };
} acc_ras_sd_vxx_ro00_2_t;

typedef volatile union _acc_ras_sd_vxx_ro00_3_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : xadm_fc_limit_npd: Level: Credit Limit (ND) VCn */
        volatile unsigned int xadm_fc_limit_npd:4;        /* [Position:96] */
        /* : xadm_fc_limit_npd: Level: Credit Limit (ND) VCn */
        volatile unsigned int xadm_fc_limit_cplh:8;        /* [Position:100] */
        /* : xadm_fc_limit_cplh: Level: Credit Limit (CH) VCn */
        volatile unsigned int xadm_fc_limit_cpld:12;        /* [Position:108] */
        /* : xadm_fc_limit_cpld: Level: Credit Limit (CD) VCn */
        volatile unsigned int rtlh_fc_rcvd_ph:8; /* [Position:120] */
    };
} acc_ras_sd_vxx_ro00_3_t;

typedef volatile union _acc_ras_sd_vxx_ro00_4_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : rtlh_fc_rcvd_ph: Level: Credit Received (PH) VCn */
        volatile unsigned int rtlh_fc_rcvd_pd:12;        /* [Position:128] */
        /* : rtlh_fc_rcvd_pd: Level: Credit Received (PD) VCn */
        volatile unsigned int rtlh_fc_rcvd_nph:8;        /* [Position:140] */
        /* : rtlh_fc_rcvd_nph: Level: Credit Received (NH) VCn */
        volatile unsigned int rtlh_fc_rcvd_npd:12; /* [Position:148] */
    };
} acc_ras_sd_vxx_ro00_4_t;

typedef volatile union _acc_ras_sd_vxx_ro00_5_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : rtlh_fc_rcvd_npd: Level: Credit Received (ND) VCn */
        volatile unsigned int rtlh_fc_rcvd_cplh:8;        /* [Position:160] */
        /* : rtlh_fc_rcvd_cplh: Level: Credit Received (CH) VCn */
        volatile unsigned int rtlh_fc_rcvd_cpld:12;        /* [Position:168] */
        /* : rtlh_fc_rcvd_cpld: Level: Credit Received (CD) VCn */
        volatile unsigned int rtlh_fc_allctd_ph:8;        /* [Position:180] */
        /* : rtlh_fc_allctd_ph: Level: Credit Allocated (PH) VCn */
        volatile unsigned int rtlh_fc_allctd_pd:4; /* [Position:188] */
    };
} acc_ras_sd_vxx_ro00_5_t;

typedef volatile union _acc_ras_sd_vxx_ro00_6_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : rtlh_fc_allctd_pd: Level: Credit Allocated (PD) VCn */
        volatile unsigned int rtlh_fc_allctd_pd:8;        /* [Position:192] */
        /* : rtlh_fc_allctd_pd: Level: Credit Allocated (PD) VCn */
        volatile unsigned int rtlh_fc_allctd_nph:8;        /* [Position:200] */
        /* : rtlh_fc_allctd_nph: Level: Credit Allocated (NH) VCn */
        volatile unsigned int rtlh_fc_allctd_npd:12;        /* [Position:208] */
        /* : rtlh_fc_allctd_npd: Level: Credit Allocated (ND) VCn  */
        volatile unsigned int rtlh_fc_allctd_cplh:4; /* [Position:220] */
    };
} acc_ras_sd_vxx_ro00_6_t;

typedef volatile union _acc_ras_sd_vxx_ro00_7_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /* : rtlh_fc_allctd_cplh: Level: Credit Allocated (CH) VCn  */
        volatile unsigned int rtlh_fc_allctd_npd:4;        /* [Position:224] */
        /* : rtlh_fc_allctd_npd: Level: Credit Allocated (CH) VCn */
        volatile unsigned int rtlh_fc_allctd_cpld:12;        /* [Position:228] */
        /* : rtlh_fc_allctd_cpld: Level: Credit Allocated (CD) VCn */
    };
} acc_ras_sd_vxx_ro00_7_t;
/* offset : 0x578 */
typedef volatile union _acc_ras_tba_com_ro00_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int smlh_in_l0s:1;        /* [Position:0] */
        /* : smlh_in_l0s: Tx L0s */
        volatile unsigned int smlh_in_rl0s:1;        /* [Position:1] */
        /* : smlh_in_rl0s: Rx L0s */
        volatile unsigned int smlh_in_l0:1;        /* [Position:2] */
        /* : smlh_in_l0: L0 */
        volatile unsigned int smlh_in_l1:1;        /* [Position:3] */
        /* : smlh_in_l1: L1 */
        volatile unsigned int pm_in_l11:1;        /* [Position:4] */
        /* : pm_in_l11: L1.1 */
        volatile unsigned int pm_in_l12:1;        /* [Position:5] */
        /* : pm_in_l12: L1.2 */
        volatile unsigned int smlh_link_in_training:1;        /* [Position:6] */
        /* : smlh_link_in_training: Config/Recovery */
        volatile unsigned int RESERVED_7:25; /* [Position:7] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_ras_tba_com_ro00_0_t;
/* offset : 0x57c */
typedef volatile union _acc_rbar_ctl_ro00_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int rbar_ctrl_update:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
         Indicates that a resizable BAR control register has been updated: 
 1 bit per Physical function.
        */
        volatile unsigned int RESERVED_1:31; /* [Position:1] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_rbar_ctl_ro00_0_t;
/* offset : 0x580 */
typedef volatile union _acc_rbar_ro00_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_rbar_size00:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        cfg_rbar_size[32*00+:32].
BAR size field from each of the resizable BAR control registers, per 
function. For BARs that are not resizable the corresponding bits in 
cfg_rbar_size are set to 0.
        */
    };
} acc_rbar_ro00_0_t;
/* offset : 0x584 */
typedef volatile union _acc_rbar_ro01_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_rbar_size01:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        cfg_rbar_size[32*01+:32].
BAR size field from each of the resizable BAR control registers, per 
function. For BARs that are not resizable the corresponding bits in 
cfg_rbar_size are set to 0.
        */
    };
} acc_rbar_ro01_0_t;
/* offset : 0x594 */
typedef volatile union _acc_rbar_vf_ctl_ro00_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int vf_rbar_ctrl_update:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that a resizable VF BAR control register has been updated: 
 1 bit per Physical function.
        */
        volatile unsigned int RESERVED_1:31; /* [Position:1] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_rbar_vf_ctl_ro00_0_t;
/* offset : 0x598 */
typedef volatile union _acc_rbar_vf_ro00_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_vf_rbar_size00:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        cfg_vf_rbar_size[32*00+:32].
BAR size field from each of the resizable VF BAR control registers, per 
function. For BARs that are not resizable, the corresponding bits in 
cfg_vf_rbar_size are set to 0.
        */
    };
} acc_rbar_vf_ro00_0_t;
/* offset : 0x59c */
typedef volatile union _acc_rbar_vf_ro01_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_vf_rbar_size01:4; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        cfg_vf_rbar_size[32*01+:04].
BAR size field from each of the resizable VF BAR control registers, per 
function. For BARs that are not resizable, the corresponding bits in 
cfg_vf_rbar_size are set to 0.
        */
        volatile unsigned int RESERVED_4:28; /* [Position:4] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_rbar_vf_ro01_0_t;
/* offset : 0x80c */
typedef volatile union _acc_etc_rw56_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int app_poisoned_tlp_type:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Determines the type of the received poisoned TLP as follows: ▪ 0b: The received poisoned TLP is a write request. ▪ 1b: The received poisoned TLP is a completion TLP. app_poisoned_tlp_type is only valid in case of receving a poisoned TLP error. In case of other types of errors, app_poisoned_tlp_type should be ignored.
        */
        volatile unsigned int reg_pcie0_phy0_phy0_ref_clk_req_n_sel:2; /* [Position:1] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        ref_clk_req_n_sel
        */
        volatile unsigned int RESERVED_3:29; /* [Position:3] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw56_0_t;
/* offset : 0x814 */
typedef volatile union _acc_etc_rw57_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int app_vf_req_retry_en:4; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Provides a per Virtual Function (VF) capability to defer incoming con_x0002_figuration requests until initialization is complete. When app_vf_req_- retry_en is asserted for a certain VF, the controller completes incom_x0002_ing configuration requests with a configuration request retry status oth_x0002_er incoming requests are not affected. When app_req_retry_en is al_x0002_so asserted, the controller completes incoming configuration requests with a configuration request retry status for ALL PFs and VFs, regard_x0002_less of the status of app_pf_req_retry_en/app_vf_req_retry_en. When the Readiness Notification mechanism is supported, DRS messaging is blocked when app_vf_req_retry_en =1 for at least one VF. The same applies when app_vf_req_retry_en =0 for all VFs, but app_req_retry_- en is asserted. When app_vf_req_retry_en =0 for a certain VF, and ap_x0002_p_req_retry_en =0, and app_drs_ready =1, the controller autonomous_x0002_ly transmits a DRS message when the link transitions from DL_Down to DL_Up.
        */
        volatile unsigned int app_vf_frs_ready:4; /* [Position:4] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Defers FRS messaging for a VF when set to '0'.
        */
        volatile unsigned int slv_awmisc_info_func_num:1; /* [Position:8] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Function Number of the AXI Slave Write Transaction. It is used instead of bits [24:22] of slv_awmisc_info when CX_SRIOV_ENABLE =1 or CX_ARI_ENABLE =1. This is an additional signal. It is not a part of the standard AXI interface. For more information, see 'AXI Slave Interface Sideband Signals'. Function numbering starts at '0'.
        */
        volatile unsigned int slv_awmisc_info_vfunc_active:1; /* [Position:9] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        VF Active Indication of the AXI Slave Write Transaction. Indicates the VF that is sending the request. ▪ 0: No VF is active and slv_awmisc_info_vfunc_num is invalid. A PF is valid and identified by slv_awmisc_info_func_num. ▪ 1: A VF is active and is identified by slv_awmisc_info_vfunc_num. This is an additional signal. It is not a part of the standard AXI inter_x0002_face. For more information, see 'AXI Slave Interface Sideband Signals'.
        */
        volatile unsigned int slv_awmisc_info_vfunc_num:2; /* [Position:10] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Virtual Function Number of the AXI Slave Write Transaction. Identi_x0002_fies the VF that is sending the request. The PCIe SR-IOV specification starts numbering VFs at 1. To ease timing during synthesis, the PCIe controller starts numbering VFs at 0. For example, slv_awmisc_info_- vfunc_num =0 refers to the first VF of the PF that is identified by slv_- awmisc_info_func_num. For more information of VF numbering, see 'Programming View'. This is an additional signal. It is not a part of the standard AXI interface. For more information, see 'AXI Slave Interface Sideband Signals'.
        */
        volatile unsigned int slv_armisc_info_func_num:1; /* [Position:12] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Function Number of the AXI Slave Read Transaction. It is used instead of bits [24:22] of slv_armisc_info when CX_SRIOV_ENABLE =1 or CX_ARI_ENABLE =1. This is an additional signal. It is not a part of the standard AXI interface. For more information, see 'AXI Slave Interface Sideband Signals'. Function numbering starts at '0'.
        */
        volatile unsigned int slv_armisc_info_vfunc_active:1; /* [Position:13] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        VF Active Indication of the AXI Slave Read Transaction. Indicates the VF that is sending the request. ▪ 0: No VF is active and slv_armisc_info_vfunc_num is invalid. A PF is valid and identified by slv_armisc_info_func_num. ▪ 1: A VF is active and is identified by slv_armisc_info_vfunc_num. This is an additional signal. It is not a part of the standard AXI inter_x0002_face. For more information, see 'AXI Slave Interface Sideband Signals'.
        */
        volatile unsigned int slv_armisc_info_vfunc_num:2; /* [Position:14] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Virtual Function of the AXI Slave Read Transaction. Identifies the VF that is sending the request. The PCIe SR-IOV specification starts num_x0002_bering VFs at 1. To ease timing during synthesis, the PCIe controller starts numbering VFs at 0. For example, slv_armisc_info_vfunc_num =0 refers to the first VF of the PF that is identified by slv_armisc_info_- func_num. For more information of VF numbering, see 'Programming View'. This is an additional signal. It is not a part of the standard AXI in_x0002_terface. For more information, see 'AXI Slave Interface Sideband Sig_x0002_nals'.
        */
        volatile unsigned int RESERVED_16:16; /* [Position:16] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw57_0_t;
/* offset : 0x824 */
typedef volatile union _acc_etc_rw61_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int ven_msg_vfunc_num:2; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Number of the virtual function accessing the VMI interface. The PCIe SR-IOV specification starts numbering VFs at 1. To ease timing during synthesis, the PCIe controller starts numbering VFs at 0. For example, ven_msg_vfunc_num=0 refers to the first VF of the PF that is identified by ven_msg_func_num. For more information of VF numbering, see 'Programming View'
        */
        volatile unsigned int ven_msg_vfunc_active:1; /* [Position:2] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Indicates that a VF is accessing the VMI. ▪ 0: No VF is active and ven_msg_vfunc_num is invalid. A PF is valid and identified by ven_msg_func_num. ▪ 1: A VF is active and is identified by ven_msg_vfunc_num.
        */
        volatile unsigned int app_err_vfunc_num:2; /* [Position:3] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        The number of the virtual function that is reporting the error indicated app_err_bus, valid when app_hdr_valid is asserted. Uncorrected Inter_x0002_nal errors (app_err_bus[9]) are not recorded for virtual functions. The PCIe SR-IOV specification starts numbering VFs at 1. To ease timing during synthesis, the PCIe controller starts numbering VFs at 0. For example, app_err_vfunc_num=0 refers to the first VF of the PF that is identified by app_err_func_num. For more information of VF number_x0002_ing, see 'Programming View'. When CX_EXTENSIBLE_VFUNC=1, this signal is only for internal VFs.
        */
        volatile unsigned int app_err_vfunc_active:1; /* [Position:5] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Indicates the function number in app_err_vfunc_num is valid.
        */
        volatile unsigned int RESERVED_6:26; /* [Position:6] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw61_0_t;
/* offset : 0x828 */
typedef volatile union _acc_etc_ro165_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int vf_frs_grant:4; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicator of when an FRS message for this VF has been scheduled for transmission.
        */
        volatile unsigned int mstr_awmisc_info_func_num:1; /* [Position:4] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Function Number of the AXI Master Write Transaction. This signal is used instead of bits [13:11] of mstr_awmisc_info when CX_SRIOV_- ENABLE =1 or CX_ARI_ENABLE =1. This is an additional signal. It is not a part of the standard AXI interface. For more information, see 'AXI Master Interface Sideband Signals'. Function numbering starts at '0'.
        */
        volatile unsigned int mstr_awmisc_info_vfunc_num:2; /* [Position:5] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Virtual Function Number of the AXI Master Write Transaction. Indi_x0002_cates which VF has been targeted by the received request.The PCIe SR-IOV specification starts numbering VFs at 1. To ease timing during synthesis, the PCIe controller starts numbering VFs at 0. For example, mstr_awmisc_info_vfunc_num =0 refers to the first VF of the PF that is identified by mstr_awmisc_info_func_num. For more information of VF numbering, see 'Programming View'. This is a signal that the appli_x0002_cation can optionally use. It is not a part of the standard AXI interface. For more information, see 'AXI Master Interface Sideband Signals'.
        */
        volatile unsigned int mstr_awmisc_info_vfunc_active:1; /* [Position:7] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        VF Active Indication of the AXI Master Write Transaction. Indicates that the received request matched a VF BAR. ▪ 0: No VF is active and mstr_awmisc_info_vfunc_num is invalid. A PF is valid and identified by mstr_awmisc_info_func_num. ▪ 1: A VF is active and is identified by mstr_awmisc_info_vfunc_num. This is a signal that the application can optionally use. It is not a part of the standard AXI interface. For more information, see 'AXI Master In_x0002_terface Sideband Signals'
        */
        volatile unsigned int mstr_armisc_info_func_num:1; /* [Position:8] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Function Number of the AXI Master Read Transaction. It is used in_x0002_stead of bits [13:11] of mstr_armisc_info when CX_SRIOV_ENABLE =1 or CX_ARI_ENABLE =1. This is an additional signal. It is not a part of the standard AXI interface. For more information, see 'AXI Master Interface Sideband Signals'. Function numbering starts at '0'.
        */
        volatile unsigned int mstr_armisc_info_vfunc_num:2; /* [Position:9] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Virtual Function Number of the AXI Master Read Transaction. Indicates which VF has been targeted by the received request.The PCIe SR-IOV specification starts numbering VFs at 1. To ease timing during synthe_x0002_sis, the PCIe controller starts numbering VFs at 0. For example, mstr_- armisc_info_vfunc_num =0 refers to the first VF of the PF that is identi_x0002_fied by mstr_armisc_info_func_num. For more information of VF num_x0002_bering, see 'Programming View'. This is an additional signal. It is not a part of the standard AXI interface. For more information, see 'AXI Mas_x0002_ter Interface Sideband Signals'
        */
        volatile unsigned int mstr_armisc_info_vfunc_active:1; /* [Position:11] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        VF Active Indication of the AXI Master Read Transaction. Indicates that the received request matched a VF BAR. ▪ 0: No VF is active and mstr_armisc_info_vfunc_num is invalid. A PF is valid and identified by mstr_armisc_info_func_num. ▪ 1: A VF is active and is identified by mstr_armisc_info_vfunc_num. This is an additional signal. It is not a part of the standard AXI inter_x0002_face. For more information, see 'AXI Master Interface Sideband Sig_x0002_nals
        */
        volatile unsigned int lbc_ext_vfunc_num:2; /* [Position:12] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Identifies the virtual function (VF) inside the controller, which has been targeted. The PCIe SR-IOV specification starts numbering VFs at 1. To ease timing during synthesis, the PCIe controller starts numbering VFs at 0. For example, lbc_ext_vfunc_num=0 refers to the first VF of the PF that is identified by lbc_ext_func_num. For more information of VF number_x0002_ing, see 'Programming View'.
        */
        volatile unsigned int lbc_ext_vfunc_active:1; /* [Position:14] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that the received TLP has been targeted to a virtual function (VF) inside the controller, and is identified by lbc_ext_vfunc_num.
        */
        volatile unsigned int cfg_vf_bme:4; /* [Position:15] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Bus master enable bit from the Control Register in the PCI header of each VF. Each bit field corresponds to one of the NVF virtual functions. You can use the cfg_start_vfi output signal or the VF Index in 'Bus Numbering Overview' index the corresponding bit field for a particular VF.
        */
        volatile unsigned int cfg_vf_en:1; /* [Position:19] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Identifies those physical functions that have virtual functions enabled.
        */
        volatile unsigned int RESERVED_20:12; /* [Position:20] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro165_0_t;
/* offset : 0x82c */
typedef volatile union _acc_etc_ro166_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_num_vf:16; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The number of virtual functions in each physical function
        */
        volatile unsigned int cfg_start_vfi:2; /* [Position:16] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The index of the first VF in every PF. This bus is a concatenation of NF numbers, each of which is log2NVF bits wide. You can use the cfg_s_x0002_tart_vfi output signal or the VF Index in 'Bus Numbering Overview', to index the corresponding bit field for a particular VF in signals such as cfg_vf_bme. For example, if PF0 has 10 VFs, PF1 has 30, PF2 has 50, then cfg_start_vfi would contain the following: ▪ {...., 90, 40, 10, 0} If a PF has no VFs associated with it, then there is no entry in the bus for that PF. For example, if PF0 has 10 VFs, PF1 has 0, PF2 has 50, then cfg_start_vfi would contain the following: ▪ {...., 60, 10, 10, 0} Your application must correctly interpret that index 10 belongs to PF2 and not of PF1. The cfg_vf_en output identifies those physical func_x0002_tions that have virtual functions enabled and the cfg_num_vf output identifies the number of virtual functions in each physical function.
        */
        volatile unsigned int cfg_flr_pf_active:1; /* [Position:18] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Set when the software initiates FLR a physical function by writing to the "Initiate FLR" register bit of that function. This signal is held assert_x0002_ed until reset of both the application and controller logic has been com_x0002_pleted.
        */
        volatile unsigned int cfg_flr_vf_active:4; /* [Position:19] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Set when the software initiates FLR a virtual function by writing to the 'Initiate FLR' register bit of that function. This signal is held asserted until the reset of both the application and internal controller logic has been completed. Each bit position corresponds to one of the NVF virtu_x0002_al functions. You can have multiple VFs in FLR at any given time, and therefore, the controller can asserts multiple bits in output bus. You can use the cfg_start_vfi output signal or the VF Index in 'Bus Numbering Overview' index the corresponding bit field for a particular VF. When CX_EXTENSIBLE_VFUNC=1, bits [INT_NVF-1:0] represent internal VFs, [NVF-1:INT_NVF] represent external VFs.
        */
        volatile unsigned int trgt_timeout_cpl_vfunc_num:2; /* [Position:23] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates which virtual function (VF) timed out. The PCIe SR-IOV spec_x0002_ification starts numbering VFs at 1. To ease timing during synthesis, the PCIe controller starts numbering VFs at 0. For example, trgt_time_x0002_out_cpl_ vfunc_num=0 refers to the first VF of the PF that is identified by trgt_timeout_cpl_func_num.
        */
        volatile unsigned int trgt_timeout_cpl_vfunc_active:1; /* [Position:25] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that the timeout is coming from a VF (Virtual function). ▪ 0: No VF is active and trgt_timeout_cpl_vfunc_num is invalid. A PF is valid and identified by trgt_timeout_cpl_func_num ▪ 1: A VF is active and is identified by trgt_timeout_cpl_vfunc_num
        */
        volatile unsigned int radm_timeout_vfunc_num:2; /* [Position:26] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates which virtual function (VF) had a completion timeout. The PCIe SR-IOV specification starts numbering VFs at '1'. To ease timing during synthesis, the PCIe controller starts numbering VFs at '0'. For example, radm_timeout_vfunc_num =0 refers to the first VF of the PF that is identified by radm_timeout_func_num. For more information of VF numbering, see 'Programming View'.
        */
        volatile unsigned int radm_timeout_vfunc_active:1; /* [Position:28] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that a virtual function (VF) had a completion timeout. ▪ 0: No VF is active and radm_timeout_vfunc_num is invalid. A PF is valid and identified by radm_timeout_func_num. ▪ 1: A VF is active and is identified by radm_timeout_vfunc_num.
        */
        volatile unsigned int RESERVED_29:3; /* [Position:29] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro166_0_t;
/* offset : 0x848 */
typedef volatile union _acc_etc_ro168_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_vf_msi_en:4; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates when the VF MSI Capability structure is enabled for each configured function. Each bit field corresponds to one of the NVF virtual functions. You can use the cfg_start_vfi output signal or the VF In_x0002_dex in 'Bus Numbering Overview' index the corresponding bit field for a particular VF.
        */
        volatile unsigned int cfg_msi_en:1; /* [Position:4] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that MSI is enabled (INTx message is not sent), one bit per configured function.
        */
        volatile unsigned int RESERVED_5:27; /* [Position:5] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro168_0_t;
/* offset : 0x84c */
typedef volatile union _acc_etc_ro169_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_vf_msi_mask0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Contents of the Per Vector Mask register in the VF MSI Capability structure. For each bit that is set, the function is prohibited from sending the associated message. Each bit field corresponds to one of the NVF virtual functions. You can use the cfg_start_vfi output signal or the VF Index in 'Bus Numbering Overview' index the corresponding bit field for a particular VF.
        */
    };
} acc_etc_ro169_0_t;
/* offset : 0x850 */
typedef volatile union _acc_etc_ro170_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_vf_msi_mask1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Contents of the Per Vector Mask register in the VF MSI Capability structure. For each bit that is set, the function is prohibited from sending the associated message. Each bit field corresponds to one of the NVF virtual functions. You can use the cfg_start_vfi output signal or the VF Index in 'Bus Numbering Overview' index the corresponding bit field for a particular VF.
        */
    };
} acc_etc_ro170_0_t;
/* offset : 0x854 */
typedef volatile union _acc_etc_ro171_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_vf_msi_mask2:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Contents of the Per Vector Mask register in the VF MSI Capability structure. For each bit that is set, the function is prohibited from sending the associated message. Each bit field corresponds to one of the NVF virtual functions. You can use the cfg_start_vfi output signal or the VF Index in 'Bus Numbering Overview' index the corresponding bit field for a particular VF.
        */
    };
} acc_etc_ro171_0_t;
/* offset : 0x858 */
typedef volatile union _acc_etc_ro172_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_vf_msi_mask3:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Contents of the Per Vector Mask register in the VF MSI Capability structure. For each bit that is set, the function is prohibited from sending the associated message. Each bit field corresponds to one of the NVF virtual functions. You can use the cfg_start_vfi output signal or the VF Index in 'Bus Numbering Overview' index the corresponding bit field for a particular VF.
        */
    };
} acc_etc_ro172_0_t;
/* offset : 0x85c */
typedef volatile union _acc_etc_rw67_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int reg_app_flr_pf_done:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Indicates that FLR a physical function has been completed, which means the application has completed initializing its data structures for the function and there are no more TLPs associated with this function in the application's transmit queue. This signal must be asserted un_x0002_til the controller de-asserts cfg_flr_pf_active for the function that has been reset.
        */
        volatile unsigned int reg_app_flr_vf_done:4; /* [Position:1] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Indicates that FLR a virtual function has been completed, which means the application has completed initializing its data structures for the function and there are no more TLPs associated with this function in the application's transmit queue. This signal is held asserted until the controller de-asserts cfg_flr_vf_active for the function that has been reset. Each bit position corresponds to one of the NVF virtual functions. You can have multiple VFs in FLR at any given time, and therefore, the controller can asserts multiple bits in output bus. You can use the cfg_- start_vfi output signal or the VF Index in 'Bus Numbering Overview' index the corresponding bit field for a particular VF. When CX_EXTENSIBLE_VFUNC=1, bits [INT_NVF-1:0] represent internal VFs, [NVF-1:INT_NVF] represent external VFs.
        */
        volatile unsigned int flr_sel:1; /* [Position:5] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        flr_select signal.
flr_sel = 1'b0, autoly assert app_flr_**_done. when cfg_flr_**_active assert and clock count exceeds flr_count
flr_sel = 1'b1, you must write reg_app_flr_**_done. when cfg_flr_**_active assert
        */
        volatile unsigned int RESERVED_6:26; /* [Position:6] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw67_0_t;
/* offset : 0x860 */
typedef volatile union _acc_etc_rw68_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int flr_count:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        flr_count. you can set time that autoly assert app_flr_**_done when flr_sel is 1'b0 and cfg_flr_**-active assert
        */
    };
} acc_etc_rw68_0_t;
/* offset : 0x880 */
typedef volatile union _acc_etc_rw69_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int reg_intr_sel:9; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Signal to choose between SW interrupt and npu interrupt
When each bit is "asserted", npu interrupt is selected for that bit.
        */
        volatile unsigned int RESERVED_9:23; /* [Position:9] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw69_0_t;
/* offset : 0x884 */
typedef volatile union _acc_etc_rw70_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int reg_ven_msi_tc0:16; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Traffic class value for each interrupt[15:0]
        */
        volatile unsigned int RESERVED_16:16; /* [Position:16] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw70_0_t;
/* offset : 0x888 */
typedef volatile union _acc_etc_rw71_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int reg_ven_msi_tc1:16; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Traffic class value for each interrupt[31:16]
        */
        volatile unsigned int RESERVED_16:16; /* [Position:16] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw71_0_t;
/* offset : 0x88c */
typedef volatile union _acc_etc_rw72_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int reg_ven_msi_tc2:16; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Traffic class value for each interrupt[47:32]
        */
        volatile unsigned int RESERVED_16:16; /* [Position:16] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw72_0_t;
/* offset : 0x890 */
typedef volatile union _acc_etc_rw73_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int reg_msi_vfunc_num:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Virtual function number for each interrupt
        */
    };
} acc_etc_rw73_0_t;
/* offset : 0x898 */
typedef volatile union _acc_etc_rw75_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int reg_msi_vfunc_active:16; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Virtual function active for each interrupt
        */
        volatile unsigned int RESERVED_16:16; /* [Position:16] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw75_0_t;
/* offset : 0x89c */
typedef volatile union _acc_etc_ro173_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int local_int_array_clr:9; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        npu intrrupt clear 
        */
        volatile unsigned int RESERVED_9:23; /* [Position:9] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro173_0_t;
/* offset : 0x8a0 */
typedef volatile union _acc_etc_ro174_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int msi_int_st:16; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        msi intrrupt status
        */
        volatile unsigned int RESERVED_16:16; /* [Position:16] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro174_0_t;
/* offset : 0x8a4 */
typedef volatile union _acc_etc_ro175_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_msi_pending:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indication from application about which functions have a pending associated message. Used to determine the value of the Vector Interrupt Pending register in the MSI Capability structure
        */
    };
} acc_etc_ro175_0_t;
/* offset : 0x8a8 */
typedef volatile union _acc_etc_ro176_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_vf_msi_pending0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Contents of the Per Vector Pending register in the VF MSI Capabili_x0002_ty structure. For each bit that is set, the function has a pending associated message. Each bit field corresponds to one of the NVF virtual functions. You can use the cfg_start_vfi output signal or the VF Index in 'Bus Numbering Overview' index the corresponding bit field for a particular VF. [31:0]
        */
    };
} acc_etc_ro176_0_t;
/* offset : 0x8ac */
typedef volatile union _acc_etc_ro177_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_vf_msi_pending1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Contents of the Per Vector Pending register in the VF MSI Capabili_x0002_ty structure. For each bit that is set, the function has a pending associated message. Each bit field corresponds to one of the NVF virtual functions. You can use the cfg_start_vfi output signal or the VF Index in 'Bus Numbering Overview' index the corresponding bit field for a particular VF.[63:32]
        */
    };
} acc_etc_ro177_0_t;
/* offset : 0x8b0 */
typedef volatile union _acc_etc_ro178_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_vf_msi_pending2:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Contents of the Per Vector Pending register in the VF MSI Capabili_x0002_ty structure. For each bit that is set, the function has a pending associated message. Each bit field corresponds to one of the NVF virtual functions. You can use the cfg_start_vfi output signal or the VF Index in 'Bus Numbering Overview' index the corresponding bit field for a particular VF.[95:64]
        */
    };
} acc_etc_ro178_0_t;
/* offset : 0x8b4 */
typedef volatile union _acc_etc_ro179_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_vf_msi_pending3:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Contents of the Per Vector Pending register in the VF MSI Capabili_x0002_ty structure. For each bit that is set, the function has a pending asso_x0002_ciated message. Each bit field corresponds to one of the NVF virtual functions. You can use the cfg_start_vfi output signal or the VF Index in 'Bus Numbering Overview' index the corresponding bit field for a par_x0002_ticular VF.[127:96]
        */
    };
} acc_etc_ro179_0_t;
/* offset : 0x8b8 */
typedef volatile union _acc_etc_ro180_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int ven_msi_req:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Request from your application to send an MSI when MSI is enabled. When MSI-X is enabled instead of MSI, assertion of ven_msi_req causes the controller to generate an MSI-X message. Once asserted, ven_msi_req must remain asserted until the controller asserts ven_msi_grant.
        */
        volatile unsigned int ven_msi_vector:5; /* [Position:1] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Used to modulate the lower five bits of the MSI Data register when multiple message mode is enabled.
        */
        volatile unsigned int ven_msi_vfunc_active:1; /* [Position:6] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that the MSI request is coming from a VF. 0: No VF is active and ven_msi_vfunc_num is invalid. A PF is valid and identified by ven_msi_func_num. 1: A VF is active and is identified by ven_msi_vfunc_num.
        */
        volatile unsigned int ven_msi_vfunc_num:2; /* [Position:7] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Identifies the VF which is making the MSI request. The PCIe SR-IOV specification starts numbering VFs at 1. To ease timing during synthe_x0002_sis, the PCIe controller starts numbering VFs at 0. For example, ven_- msi_vfunc_num=0 refers to the first VF of the PF that is identified by ven_msi_func_num. For more information of VF numbering, see 'Programming View'. When the "External Virtual Function Registers" feature is enabled (CX_EXTENSIBLE_VFUNC=1): ▪ In the case of MSI, this I/F cannot be used for external VFs you must use XALI or AXI slave I/F. It can be used for internal VFs. ▪ In the case of MSI-X, this I/F can be used for internal and external VFs.
        */
        volatile unsigned int ven_msi_tc:3; /* [Position:9] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Traffic Class of the MSI request, valid when ven_msi_req is asserted.
        */
        volatile unsigned int RESERVED_12:20; /* [Position:12] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro180_0_t;
/* offset : 0x8c0 */
typedef volatile union _acc_slv_awaddr_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int reg_slv_awaddr_i:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        reg_slv_awaddr_i, ctrl awaddr is assign {8'b, reg_slv_awaddr_i, slv_awaddr_i[23:0]}
        */
    };
} acc_slv_awaddr_0_t;
/* offset : 0x8c4 */
typedef volatile union _acc_slv_araddr_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int reg_slv_araddr_i:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        reg_slv_araddr_i, ctrl araddr is assign {8'b, reg_slv_araddr_i, slv_araddr_i[23:0]}
        */
    };
} acc_slv_araddr_0_t;
/* offset : 0x8c8 */
typedef volatile union _acc_parity_intr_mon0_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int parity_intr_mon0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        parity intrrupt monitor signal [31:0]
        */
    };
} acc_parity_intr_mon0_0_t;
/* offset : 0x8cc */
typedef volatile union _acc_parity_intr_mon1_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int parity_intr_mon1:16; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        parity intrrupt monitor signal [47:32]
        */
        volatile unsigned int RESERVED_16:16; /* [Position:16] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        RESERVED
        */
    };
} acc_parity_intr_mon1_0_t;

typedef volatile struct _pcie_hif_reg_t
{
    acc_ctrl00_0_t acc_ctrl00_0; /* 0x0 */
    acc_ctrl01_0_t acc_ctrl01_0; /* 0x4 */
    volatile unsigned int rsvd_0x8[2];
    acc_intr_en0_0_t acc_intr_en0_0; /* 0x10 */
    acc_intr_cl0_0_t acc_intr_cl0_0; /* 0x14 */
    acc_intr_st_part_0_t acc_intr_st_part_0; /* 0x18 */
    acc_perst_n_intr_en_0_t acc_perst_n_intr_en_0; /* 0x1c */
    acc_perst_n_intr_clr_0_t acc_perst_n_intr_clr_0; /* 0x20 */
    acc_perst_n_intr_st_0_t acc_perst_n_intr_st_0; /* 0x24 */
    acc_edma_wr_intr_en_0_t acc_edma_wr_intr_en_0; /* 0x28 */
    acc_edma_wr_intr_st_0_t acc_edma_wr_intr_st_0; /* 0x2c */
    volatile unsigned int rsvd_0x30[1];
    acc_edma_rd_intr_en_0_t acc_edma_rd_intr_en_0; /* 0x34 */
    acc_edma_rd_intr_st_0_t acc_edma_rd_intr_st_0; /* 0x38 */
    volatile unsigned int rsvd_0x3c[3];
    acc_pipe02_0_t acc_pipe02_0; /* 0x48 */
    volatile unsigned int rsvd_0x4c[1];
    acc_pipe04_0_t acc_pipe04_0; /* 0x50 */
    acc_pipe05_0_t acc_pipe05_0; /* 0x54 */
    acc_pipe06_0_t acc_pipe06_0; /* 0x58 */
    volatile unsigned int rsvd_0x5c[10];
    acc_etc_rw01_0_t acc_etc_rw01_0; /* 0x84 */
    acc_etc_rw02_0_t acc_etc_rw02_0; /* 0x88 */
    volatile unsigned int rsvd_0x8c[2];
    acc_etc_rw05_0_t acc_etc_rw05_0; /* 0x94 */
    acc_etc_rw06_0_t acc_etc_rw06_0; /* 0x98 */
    acc_etc_rw07_0_t acc_etc_rw07_0; /* 0x9c */
    acc_etc_rw08_0_t acc_etc_rw08_0; /* 0xa0 */
    acc_etc_rw09_0_t acc_etc_rw09_0; /* 0xa4 */
    acc_etc_rw10_0_t acc_etc_rw10_0; /* 0xa8 */
    acc_etc_rw11_0_t acc_etc_rw11_0; /* 0xac */
    acc_etc_rw12_0_t acc_etc_rw12_0; /* 0xb0 */
    acc_etc_rw13_0_t acc_etc_rw13_0; /* 0xb4 */
    acc_etc_rw14_0_t acc_etc_rw14_0; /* 0xb8 */
    acc_etc_rw15_0_t acc_etc_rw15_0; /* 0xbc */
    acc_etc_rw16_0_t acc_etc_rw16_0; /* 0xc0 */
    acc_etc_rw17_0_t acc_etc_rw17_0; /* 0xc4 */
    acc_etc_rw18_0_t acc_etc_rw18_0; /* 0xc8 */
    acc_etc_rw19_0_t acc_etc_rw19_0; /* 0xcc */
    acc_etc_rw20_0_t acc_etc_rw20_0; /* 0xd0 */
    acc_etc_rw21_0_t acc_etc_rw21_0; /* 0xd4 */
    acc_etc_rw22_0_t acc_etc_rw22_0; /* 0xd8 */
    acc_etc_rw23_0_t acc_etc_rw23_0; /* 0xdc */
    acc_etc_rw24_0_t acc_etc_rw24_0; /* 0xe0 */
    acc_etc_rw25_0_t acc_etc_rw25_0; /* 0xe4 */
    acc_etc_rw26_0_t acc_etc_rw26_0; /* 0xe8 */
    volatile unsigned int rsvd_0xec[3];
    acc_etc_rw30_0_t acc_etc_rw30_0; /* 0xf8 */
    acc_etc_rw31_0_t acc_etc_rw31_0; /* 0xfc */
    volatile unsigned int rsvd_0x100[1];
    acc_etc_rw33_0_t acc_etc_rw33_0; /* 0x104 */
    volatile unsigned int rsvd_0x108[2];
    acc_etc_rw36_0_t acc_etc_rw36_0; /* 0x110 */
    acc_etc_rw37_0_t acc_etc_rw37_0; /* 0x114 */
    acc_etc_rw38_0_t acc_etc_rw38_0; /* 0x118 */
    acc_etc_rw39_0_t acc_etc_rw39_0; /* 0x11c */
    acc_etc_rw40_0_t acc_etc_rw40_0; /* 0x120 */
    acc_etc_rw41_0_t acc_etc_rw41_0; /* 0x124 */
    acc_etc_rw42_0_t acc_etc_rw42_0; /* 0x128 */
    acc_etc_rw43_0_t acc_etc_rw43_0; /* 0x12c */
    acc_etc_rw44_0_t acc_etc_rw44_0; /* 0x130 */
    acc_etc_rw45_0_t acc_etc_rw45_0; /* 0x134 */
    acc_etc_rw46_0_t acc_etc_rw46_0; /* 0x138 */
    acc_etc_rw47_0_t acc_etc_rw47_0; /* 0x13c */
    acc_etc_rw48_0_t acc_etc_rw48_0; /* 0x140 */
    acc_etc_rw49_0_t acc_etc_rw49_0; /* 0x144 */
    volatile unsigned int rsvd_0x148[2];
    acc_etc_rw52_0_t acc_etc_rw52_0; /* 0x150 */
    acc_etc_ro000_0_t acc_etc_ro000_0; /* 0x154 */
    volatile unsigned int rsvd_0x158[4];
    acc_etc_ro005_0_t acc_etc_ro005_0; /* 0x168 */
    acc_etc_ro006_0_t acc_etc_ro006_0; /* 0x16c */
    acc_etc_ro007_0_t acc_etc_ro007_0; /* 0x170 */
    volatile unsigned int rsvd_0x174[3];
    acc_etc_ro011_0_t acc_etc_ro011_0; /* 0x180 */
    acc_etc_ro012_0_t acc_etc_ro012_0; /* 0x184 */
    acc_etc_ro013_0_t acc_etc_ro013_0; /* 0x188 */
    acc_etc_ro014_0_t acc_etc_ro014_0; /* 0x18c */
    acc_etc_ro015_0_t acc_etc_ro015_0; /* 0x190 */
    acc_etc_ro016_0_t acc_etc_ro016_0; /* 0x194 */
    acc_etc_ro017_0_t acc_etc_ro017_0; /* 0x198 */
    acc_etc_ro018_0_t acc_etc_ro018_0; /* 0x19c */
    acc_etc_ro019_0_t acc_etc_ro019_0; /* 0x1a0 */
    acc_etc_ro020_0_t acc_etc_ro020_0; /* 0x1a4 */
    acc_etc_ro021_0_t acc_etc_ro021_0; /* 0x1a8 */
    acc_etc_ro022_0_t acc_etc_ro022_0; /* 0x1ac */
    acc_etc_ro023_0_t acc_etc_ro023_0; /* 0x1b0 */
    acc_etc_ro024_0_t acc_etc_ro024_0; /* 0x1b4 */
    acc_etc_ro025_0_t acc_etc_ro025_0; /* 0x1b8 */
    acc_etc_ro026_0_t acc_etc_ro026_0; /* 0x1bc */
    acc_etc_ro027_0_t acc_etc_ro027_0; /* 0x1c0 */
    acc_etc_ro028_0_t acc_etc_ro028_0; /* 0x1c4 */
    acc_etc_ro029_0_t acc_etc_ro029_0; /* 0x1c8 */
    acc_etc_ro030_0_t acc_etc_ro030_0; /* 0x1cc */
    acc_etc_ro031_0_t acc_etc_ro031_0; /* 0x1d0 */
    acc_etc_ro032_0_t acc_etc_ro032_0; /* 0x1d4 */
    acc_etc_ro033_0_t acc_etc_ro033_0; /* 0x1d8 */
    acc_etc_ro034_0_t acc_etc_ro034_0; /* 0x1dc */
    acc_etc_ro035_0_t acc_etc_ro035_0; /* 0x1e0 */
    acc_etc_ro036_0_t acc_etc_ro036_0; /* 0x1e4 */
    volatile unsigned int rsvd_0x1e8[1];
    acc_etc_ro038_0_t acc_etc_ro038_0; /* 0x1ec */
    acc_etc_ro039_0_t acc_etc_ro039_0; /* 0x1f0 */
    volatile unsigned int rsvd_0x1f4[4];
    acc_etc_ro044_0_t acc_etc_ro044_0; /* 0x204 */
    acc_etc_ro045_0_t acc_etc_ro045_0; /* 0x208 */
    acc_etc_ro046_0_t acc_etc_ro046_0; /* 0x20c */
    acc_etc_ro047_0_t acc_etc_ro047_0; /* 0x210 */
    volatile unsigned int rsvd_0x214[6];
    acc_etc_ro054_0_t acc_etc_ro054_0; /* 0x22c */
    acc_etc_ro055_0_t acc_etc_ro055_0; /* 0x230 */
    volatile unsigned int rsvd_0x234[6];
    acc_etc_ro062_0_t acc_etc_ro062_0; /* 0x24c */
    volatile unsigned int rsvd_0x250[3];
    acc_etc_ro066_0_t acc_etc_ro066_0; /* 0x25c */
    volatile unsigned int rsvd_0x260[3];
    acc_etc_ro070_0_t acc_etc_ro070_0; /* 0x26c */
    acc_etc_ro071_0_t acc_etc_ro071_0; /* 0x270 */
    volatile unsigned int rsvd_0x274[6];
    acc_etc_ro078_0_t acc_etc_ro078_0; /* 0x28c */
    acc_etc_ro079_0_t acc_etc_ro079_0; /* 0x290 */
    volatile unsigned int rsvd_0x294[6];
    acc_etc_ro086_0_t acc_etc_ro086_0; /* 0x2ac */
    volatile unsigned int rsvd_0x2b0[3];
    acc_etc_ro090_0_t acc_etc_ro090_0; /* 0x2bc */
    volatile unsigned int rsvd_0x2c0[3];
    acc_etc_ro094_0_t acc_etc_ro094_0; /* 0x2cc */
    acc_etc_ro095_0_t acc_etc_ro095_0; /* 0x2d0 */
    volatile unsigned int rsvd_0x2d4[6];
    acc_etc_ro102_0_t acc_etc_ro102_0; /* 0x2ec */
    acc_etc_ro103_0_t acc_etc_ro103_0; /* 0x2f0 */
    volatile unsigned int rsvd_0x2f4[6];
    acc_etc_ro110_0_t acc_etc_ro110_0; /* 0x30c */
    volatile unsigned int rsvd_0x310[3];
    acc_etc_ro114_0_t acc_etc_ro114_0; /* 0x31c */
    volatile unsigned int rsvd_0x320[3];
    acc_etc_ro118_0_t acc_etc_ro118_0; /* 0x32c */
    volatile unsigned int rsvd_0x330[3];
    acc_etc_ro122_0_t acc_etc_ro122_0; /* 0x33c */
    volatile unsigned int rsvd_0x340[3];
    acc_etc_ro126_0_t acc_etc_ro126_0; /* 0x34c */
    volatile unsigned int rsvd_0x350[5];
    acc_etc_ro132_0_t acc_etc_ro132_0; /* 0x364 */
    acc_etc_ro133_0_t acc_etc_ro133_0; /* 0x368 */
    acc_etc_ro134_0_t acc_etc_ro134_0; /* 0x36c */
    acc_etc_ro135_0_t acc_etc_ro135_0; /* 0x370 */
    acc_etc_ro136_0_t acc_etc_ro136_0; /* 0x374 */
    acc_etc_ro137_0_t acc_etc_ro137_0; /* 0x378 */
    acc_etc_ro138_0_t acc_etc_ro138_0; /* 0x37c */
    acc_etc_ro139_0_t acc_etc_ro139_0; /* 0x380 */
    acc_etc_ro140_0_t acc_etc_ro140_0; /* 0x384 */
    acc_etc_ro141_0_t acc_etc_ro141_0; /* 0x388 */
    acc_etc_ro144_0_t acc_etc_ro144_0; /* 0x38c */
    acc_etc_ro145_0_t acc_etc_ro145_0; /* 0x390 */
    volatile unsigned int rsvd_0x394[4];
    acc_etc_ro148_0_t acc_etc_ro148_0; /* 0x3a4 */
    acc_etc_ro149_0_t acc_etc_ro149_0; /* 0x3a8 */
    acc_etc_ro150_0_t acc_etc_ro150_0; /* 0x3ac */
    acc_etc_ro151_0_t acc_etc_ro151_0; /* 0x3b0 */
    volatile unsigned int rsvd_0x3b4[1];
    acc_etc_ro153_0_t acc_etc_ro153_0; /* 0x3b8 */
    acc_etc_ro154_0_t acc_etc_ro154_0; /* 0x3bc */
    acc_etc_ro155_0_t acc_etc_ro155_0; /* 0x3c0 */
    acc_etc_ro156_0_t acc_etc_ro156_0; /* 0x3c4 */
    volatile unsigned int rsvd_0x3c8[1];
    acc_etc_ro158_0_t acc_etc_ro158_0; /* 0x3cc */
    volatile unsigned int rsvd_0x3d0[12];
    acc_ras_rw00_0_t acc_ras_rw00_0; /* 0x400 */
    volatile unsigned int rsvd_0x404[22];
    acc_ras_ec_com_ro00_0_t acc_ras_ec_com_ro00_0; /* 0x45c */
    acc_ras_ec_com_ro00_1_t acc_ras_ec_com_ro00_1; /* 0x460 */
    acc_ras_ec_com_ro00_2_t acc_ras_ec_com_ro00_2; /* 0x464 */
    acc_ras_ec_com_ro00_3_t acc_ras_ec_com_ro00_3; /* 0x468 */
    acc_ras_ec_com_ro00_4_t acc_ras_ec_com_ro00_4; /* 0x46c */
    acc_ras_ec_com_ro00_5_t acc_ras_ec_com_ro00_5; /* 0x470 */
    acc_ras_ec_lxx_ro00_0_t acc_ras_ec_lxx_ro00_0; /* 0x474 */
    acc_ras_ec_lxx_ro01_0_t acc_ras_ec_lxx_ro01_0; /* 0x478 */
    acc_ras_sd_com_ro00_0_t acc_ras_sd_com_ro00_0; /* 0x47c */
    acc_ras_sd_com_ro00_1_t acc_ras_sd_com_ro00_1; /* 0x480 */
    acc_ras_sd_com_ro00_2_t acc_ras_sd_com_ro00_2; /* 0x484 */
    acc_ras_sd_lxx_ro00_0_t acc_ras_sd_lxx_ro00_0; /* 0x488 */
    acc_ras_sd_lxx_ro00_1_t acc_ras_sd_lxx_ro00_1; /* 0x48c */
    acc_ras_sd_lxx_ro00_2_t acc_ras_sd_lxx_ro00_2; /* 0x490 */
    acc_ras_sd_lxx_ro00_3_t acc_ras_sd_lxx_ro00_3; /* 0x494 */
    acc_ras_sd_lxx_ro00_4_t acc_ras_sd_lxx_ro00_4; /* 0x498 */
    acc_ras_sd_lxx_ro00_5_t acc_ras_sd_lxx_ro00_5; /* 0x49c */
    acc_ras_sd_lxx_ro00_6_t acc_ras_sd_lxx_ro00_6; /* 0x4a0 */
    acc_ras_sd_lxx_ro00_7_t acc_ras_sd_lxx_ro00_7; /* 0x4a4 */
    acc_ras_sd_lxx_ro08_0_t acc_ras_sd_lxx_ro08_0; /* 0x4a8 */
    acc_ras_sd_lxx_ro08_1_t acc_ras_sd_lxx_ro08_1; /* 0x4ac */
    acc_ras_sd_lxx_ro08_2_t acc_ras_sd_lxx_ro08_2; /* 0x4b0 */
    acc_ras_sd_lxx_ro08_3_t acc_ras_sd_lxx_ro08_3; /* 0x4b4 */
    acc_ras_sd_lxx_ro08_4_t acc_ras_sd_lxx_ro08_4; /* 0x4b8 */
    acc_ras_sd_lxx_ro08_5_t acc_ras_sd_lxx_ro08_5; /* 0x4bc */
    acc_ras_sd_lxx_ro08_6_t acc_ras_sd_lxx_ro08_6; /* 0x4c0 */
    acc_ras_sd_lxx_ro08_7_t acc_ras_sd_lxx_ro08_7; /* 0x4c4 */
    acc_ras_sd_lxx_ro16_0_t acc_ras_sd_lxx_ro16_0; /* 0x4c8 */
    acc_ras_sd_lxx_ro16_1_t acc_ras_sd_lxx_ro16_1; /* 0x4cc */
    acc_ras_sd_lxx_ro16_2_t acc_ras_sd_lxx_ro16_2; /* 0x4d0 */
    acc_ras_sd_lxx_ro16_3_t acc_ras_sd_lxx_ro16_3; /* 0x4d4 */
    acc_ras_sd_lxx_ro16_4_t acc_ras_sd_lxx_ro16_4; /* 0x4d8 */
    acc_ras_sd_lxx_ro16_5_t acc_ras_sd_lxx_ro16_5; /* 0x4dc */
    acc_ras_sd_lxx_ro16_6_t acc_ras_sd_lxx_ro16_6; /* 0x4e0 */
    acc_ras_sd_lxx_ro16_7_t acc_ras_sd_lxx_ro16_7; /* 0x4e4 */
    acc_ras_sd_lxx_ro24_0_t acc_ras_sd_lxx_ro24_0; /* 0x4e8 */
    acc_ras_sd_lxx_ro24_1_t acc_ras_sd_lxx_ro24_1; /* 0x4ec */
    acc_ras_sd_lxx_ro24_2_t acc_ras_sd_lxx_ro24_2; /* 0x4f0 */
    acc_ras_sd_lxx_ro24_3_t acc_ras_sd_lxx_ro24_3; /* 0x4f4 */
    acc_ras_sd_lxx_ro24_4_t acc_ras_sd_lxx_ro24_4; /* 0x4f8 */
    acc_ras_sd_lxx_ro24_5_t acc_ras_sd_lxx_ro24_5; /* 0x4fc */
    acc_ras_sd_lxx_ro24_6_t acc_ras_sd_lxx_ro24_6; /* 0x500 */
    acc_ras_sd_lxx_ro24_7_t acc_ras_sd_lxx_ro24_7; /* 0x504 */
    acc_ras_sd_vxx_ro00_0_t acc_ras_sd_vxx_ro00_0; /* 0x508 */
    acc_ras_sd_vxx_ro00_1_t acc_ras_sd_vxx_ro00_1; /* 0x50c */
    acc_ras_sd_vxx_ro00_2_t acc_ras_sd_vxx_ro00_2; /* 0x510 */
    acc_ras_sd_vxx_ro00_3_t acc_ras_sd_vxx_ro00_3; /* 0x514 */
    acc_ras_sd_vxx_ro00_4_t acc_ras_sd_vxx_ro00_4; /* 0x518 */
    acc_ras_sd_vxx_ro00_5_t acc_ras_sd_vxx_ro00_5; /* 0x51c */
    acc_ras_sd_vxx_ro00_6_t acc_ras_sd_vxx_ro00_6; /* 0x520 */
    acc_ras_sd_vxx_ro00_7_t acc_ras_sd_vxx_ro00_7; /* 0x524 */
    volatile unsigned int rsvd_0x528[20];
    acc_ras_tba_com_ro00_0_t acc_ras_tba_com_ro00_0; /* 0x578 */
    acc_rbar_ctl_ro00_0_t acc_rbar_ctl_ro00_0; /* 0x57c */
    acc_rbar_ro00_0_t acc_rbar_ro00_0; /* 0x580 */
    acc_rbar_ro01_0_t acc_rbar_ro01_0; /* 0x584 */
    volatile unsigned int rsvd_0x588[3];
    acc_rbar_vf_ctl_ro00_0_t acc_rbar_vf_ctl_ro00_0; /* 0x594 */
    acc_rbar_vf_ro00_0_t acc_rbar_vf_ro00_0; /* 0x598 */
    acc_rbar_vf_ro01_0_t acc_rbar_vf_ro01_0; /* 0x59c */
    volatile unsigned int rsvd_0x5a0[155];
    acc_etc_rw56_0_t acc_etc_rw56_0; /* 0x80c */
    volatile unsigned int rsvd_0x810[1];
    acc_etc_rw57_0_t acc_etc_rw57_0; /* 0x814 */
    volatile unsigned int rsvd_0x818[3];
    acc_etc_rw61_0_t acc_etc_rw61_0; /* 0x824 */
    acc_etc_ro165_0_t acc_etc_ro165_0; /* 0x828 */
    acc_etc_ro166_0_t acc_etc_ro166_0; /* 0x82c */
    volatile unsigned int rsvd_0x830[6];
    acc_etc_ro168_0_t acc_etc_ro168_0; /* 0x848 */
    acc_etc_ro169_0_t acc_etc_ro169_0; /* 0x84c */
    acc_etc_ro170_0_t acc_etc_ro170_0; /* 0x850 */
    acc_etc_ro171_0_t acc_etc_ro171_0; /* 0x854 */
    acc_etc_ro172_0_t acc_etc_ro172_0; /* 0x858 */
    acc_etc_rw67_0_t acc_etc_rw67_0; /* 0x85c */
    acc_etc_rw68_0_t acc_etc_rw68_0; /* 0x860 */
    volatile unsigned int rsvd_0x864[7];
    acc_etc_rw69_0_t acc_etc_rw69_0; /* 0x880 */
    acc_etc_rw70_0_t acc_etc_rw70_0; /* 0x884 */
    acc_etc_rw71_0_t acc_etc_rw71_0; /* 0x888 */
    acc_etc_rw72_0_t acc_etc_rw72_0; /* 0x88c */
    acc_etc_rw73_0_t acc_etc_rw73_0; /* 0x890 */
    volatile unsigned int rsvd_0x894[1];
    acc_etc_rw75_0_t acc_etc_rw75_0; /* 0x898 */
    acc_etc_ro173_0_t acc_etc_ro173_0; /* 0x89c */
    acc_etc_ro174_0_t acc_etc_ro174_0; /* 0x8a0 */
    acc_etc_ro175_0_t acc_etc_ro175_0; /* 0x8a4 */
    acc_etc_ro176_0_t acc_etc_ro176_0; /* 0x8a8 */
    acc_etc_ro177_0_t acc_etc_ro177_0; /* 0x8ac */
    acc_etc_ro178_0_t acc_etc_ro178_0; /* 0x8b0 */
    acc_etc_ro179_0_t acc_etc_ro179_0; /* 0x8b4 */
    acc_etc_ro180_0_t acc_etc_ro180_0; /* 0x8b8 */
    volatile unsigned int rsvd_0x8bc[1];
    acc_slv_awaddr_0_t acc_slv_awaddr_0; /* 0x8c0 */
    acc_slv_araddr_0_t acc_slv_araddr_0; /* 0x8c4 */
    acc_parity_intr_mon0_0_t acc_parity_intr_mon0_0; /* 0x8c8 */
    acc_parity_intr_mon1_0_t acc_parity_intr_mon1_0; /* 0x8cc */
    volatile unsigned int rsvd_0x8d0[204];
    acc_ras_st_bus_ro00_0_t acc_ras_st_bus_ro00_0; /* 0xc00 */
    acc_ras_st_bus_ro00_1_t acc_ras_st_bus_ro00_1; /* 0xc04 */
    acc_ras_st_bus_ro00_2_t acc_ras_st_bus_ro00_2; /* 0xc08 */
    acc_ras_st_bus_ro00_3_t acc_ras_st_bus_ro00_3; /* 0xc0c */
    acc_ras_st_bus_ro00_4_t acc_ras_st_bus_ro00_4; /* 0xc10 */
    acc_ras_st_bus_ro00_5_t acc_ras_st_bus_ro00_5; /* 0xc14 */
    acc_ras_st_bus_ro00_6_t acc_ras_st_bus_ro00_6; /* 0xc18 */
    acc_ras_st_bus_ro00_7_t acc_ras_st_bus_ro00_7; /* 0xc1c */
    acc_ras_st_bus_ro00_8_t acc_ras_st_bus_ro00_8; /* 0xc20 */
    acc_ras_st_bus_ro00_9_t acc_ras_st_bus_ro00_9; /* 0xc24 */
    acc_ras_st_bus_ro00_10_t acc_ras_st_bus_ro00_10; /* 0xc28 */
    acc_ras_st_bus_ro00_11_t acc_ras_st_bus_ro00_11; /* 0xc2c */
    acc_ras_st_bus_ro00_12_t acc_ras_st_bus_ro00_12; /* 0xc30 */
    acc_ras_st_bus_ro00_13_t acc_ras_st_bus_ro00_13; /* 0xc34 */
    acc_ras_st_bus_ro00_14_t acc_ras_st_bus_ro00_14; /* 0xc38 */
    acc_ras_st_bus_ro00_15_t acc_ras_st_bus_ro00_15; /* 0xc3c */
    acc_ras_st_bus_ro00_16_t acc_ras_st_bus_ro00_16; /* 0xc40 */
    acc_ras_st_bus_ro00_17_t acc_ras_st_bus_ro00_17; /* 0xc44 */
    acc_ras_st_bus_ro00_18_t acc_ras_st_bus_ro00_18; /* 0xc48 */
    acc_ras_st_bus_ro00_19_t acc_ras_st_bus_ro00_19; /* 0xc4c */
    acc_ras_st_bus_ro00_20_t acc_ras_st_bus_ro00_20; /* 0xc50 */
    acc_ras_st_bus_ro00_21_t acc_ras_st_bus_ro00_21; /* 0xc54 */
    acc_ras_st_bus_ro00_22_t acc_ras_st_bus_ro00_22; /* 0xc58 */
    acc_ras_st_bus_ro00_23_t acc_ras_st_bus_ro00_23; /* 0xc5c */
    acc_ras_st_bus_ro00_24_t acc_ras_st_bus_ro00_24; /* 0xc60 */
    acc_ras_st_bus_ro00_25_t acc_ras_st_bus_ro00_25; /* 0xc64 */
    acc_ras_st_bus_ro00_26_t acc_ras_st_bus_ro00_26; /* 0xc68 */
    acc_ras_st_bus_ro00_27_t acc_ras_st_bus_ro00_27; /* 0xc6c */
    acc_ras_st_bus_ro00_28_t acc_ras_st_bus_ro00_28; /* 0xc70 */
    acc_ras_st_bus_ro00_29_t acc_ras_st_bus_ro00_29; /* 0xc74 */
    acc_ras_st_bus_ro00_30_t acc_ras_st_bus_ro00_30; /* 0xc78 */
    acc_ras_st_bus_ro00_31_t acc_ras_st_bus_ro00_31; /* 0xc7c */
    acc_ras_st_bus_ro00_32_t acc_ras_st_bus_ro00_32; /* 0xc80 */
    acc_ras_st_bus_ro00_33_t acc_ras_st_bus_ro00_33; /* 0xc84 */
    acc_ras_st_bus_ro00_34_t acc_ras_st_bus_ro00_34; /* 0xc88 */
    acc_ras_st_bus_ro00_35_t acc_ras_st_bus_ro00_35; /* 0xc8c */
    acc_ras_st_bus_ro00_36_t acc_ras_st_bus_ro00_36; /* 0xc90 */
    acc_ras_st_bus_ro00_37_t acc_ras_st_bus_ro00_37; /* 0xc94 */
    acc_ras_st_bus_ro00_38_t acc_ras_st_bus_ro00_38; /* 0xc98 */
    acc_ras_st_bus_ro00_39_t acc_ras_st_bus_ro00_39; /* 0xc9c */
    acc_ras_st_bus_ro00_40_t acc_ras_st_bus_ro00_40; /* 0xca0 */
    acc_ras_st_bus_ro00_41_t acc_ras_st_bus_ro00_41; /* 0xca4 */
    acc_ras_st_bus_ro00_42_t acc_ras_st_bus_ro00_42; /* 0xca8 */
    acc_ras_st_bus_ro00_43_t acc_ras_st_bus_ro00_43; /* 0xcac */
    acc_ras_st_bus_ro00_44_t acc_ras_st_bus_ro00_44; /* 0xcb0 */
    acc_ras_st_bus_ro00_45_t acc_ras_st_bus_ro00_45; /* 0xcb4 */
    acc_ras_st_bus_ro00_46_t acc_ras_st_bus_ro00_46; /* 0xcb8 */
    acc_ras_st_bus_ro00_47_t acc_ras_st_bus_ro00_47; /* 0xcbc */
    acc_ras_st_bus_ro00_48_t acc_ras_st_bus_ro00_48; /* 0xcc0 */
    acc_ras_st_bus_ro00_49_t acc_ras_st_bus_ro00_49; /* 0xcc4 */
    acc_ras_st_bus_ro00_50_t acc_ras_st_bus_ro00_50; /* 0xcc8 */
    acc_ras_st_bus_ro00_51_t acc_ras_st_bus_ro00_51; /* 0xccc */
    acc_ras_st_bus_ro00_52_t acc_ras_st_bus_ro00_52; /* 0xcd0 */
    acc_ras_st_bus_ro00_53_t acc_ras_st_bus_ro00_53; /* 0xcd4 */
    acc_ras_st_bus_ro00_54_t acc_ras_st_bus_ro00_54; /* 0xcd8 */
    acc_ras_st_bus_ro00_55_t acc_ras_st_bus_ro00_55; /* 0xcdc */
    acc_ras_st_bus_ro00_56_t acc_ras_st_bus_ro00_56; /* 0xce0 */
    acc_ras_st_bus_ro00_57_t acc_ras_st_bus_ro00_57; /* 0xce4 */
    acc_ras_st_bus_ro00_58_t acc_ras_st_bus_ro00_58; /* 0xce8 */
    acc_ras_st_bus_ro00_59_t acc_ras_st_bus_ro00_59; /* 0xcec */
    acc_ras_st_bus_ro00_60_t acc_ras_st_bus_ro00_60; /* 0xcf0 */
} pcie_hif_reg_t;

#endif /* __DX_PCIE_HIF_H */