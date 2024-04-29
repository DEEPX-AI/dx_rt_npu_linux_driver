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
        volatile unsigned int sw_perst_en:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        
        */
        volatile unsigned int sw_power_up_rst_en:1; /* [Position:1] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        
        */
        volatile unsigned int sw_button_rst_en:1; /* [Position:2] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        
        */
        volatile unsigned int lane4_sel:1; /* [Position:3] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        
        */
        volatile unsigned int sys_aux_pwr_det:1; /* [Position:4] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Auxiliary Power Detected. Used to report to the host software that auxiliary power (Vaux) is present.
        */
        volatile unsigned int RESERVED_5:3; /* [Position:5] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
        volatile unsigned int dbi_area_use:1; /* [Position:8] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        
        */
        volatile unsigned int dbi_area:2; /* [Position:9] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        
        */
        volatile unsigned int RESERVED_11:21; /* [Position:11] */
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
        volatile unsigned int clkreq_usp_dspn_sel:1; /* [Position:2] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        ctl_ref_clk_req_b selection bit. 
When this bit is 1'b1, ctl_ref_clk_req_n = local_ref_clk_req_n.
        */
        volatile unsigned int RESERVED_3:29; /* [Position:3] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_ctrl01_0_t;
/* offset : 0x48 */
typedef volatile union _acc_pipe02_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int phy_cfg_status:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Input bus that can optionally be used to read PHY status. The phy_cfg_status bus maps to the PHY Status register
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
/* offset : 0x80 */
typedef volatile union _acc_etc_rw00_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int device_type:4; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Device/port type. Indicates the specific type of this PCI Express function. 
4'b0000: PCI Express Endpoint
4'b0001: Legacy PCI Express Endpoint
4'b0100: Root Port of PCI Express Root Complex
        */
        volatile unsigned int RESERVED_4:28; /* [Position:4] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw00_0_t;
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
        Provides a per Physical Function (PF) capability to defer incoming configuration requests until initialization is complete. When app_pf_req_retry_en is asserted for a certain PF, the controller completes incoming configuration requests with a configuration request retry status; other incoming requests are not affected.
        */
        volatile unsigned int app_drs_ready:1; /* [Position:15] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Defers DRS messaging when set to '0'.
        */
        volatile unsigned int app_pf_frs_ready:4; /* [Position:16] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Defers FRS messaging when set to '0'.
        */
        volatile unsigned int ptm_auto_update_signal:1; /* [Position:20] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Indicates that the controller should update the PTM Requester Context and Clock automatically every 10ms.
        */
        volatile unsigned int ptm_manual_update_pulse:1; /* [Position:21] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Indicates that the controller should update the PTM Requester Context and Clock now.
        */
        volatile unsigned int ptm_external_master_strobe:1; /* [Position:22] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        PTM External Master Time Strobe.
        */
        volatile unsigned int exp_rom_validation_status_strobe:4; /* [Position:23] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        This strobe sets the Expansion ROM Validation Status field (ROM_BAR_VALIDATION_STATUS) of the Expansion ROM BAR register as specified on the exp_rom_validation_status input signal.
        */
        volatile unsigned int exp_rom_validation_details_strobe:4; /* [Position:27] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        This strobe sets the Expansion ROM Validation Details field (ROM_BAR_VALIDATION_DETAILS) of the Expansion ROM BAR register as specified on the exp_rom_validation_details input signal.
        */
        volatile unsigned int RESERVED_31:1; /* [Position:31] */
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
        volatile unsigned int exp_rom_validation_status:12; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        You can use this signal to notify the controller about the expansion ROM validation status. On assertion of exp_rom_validation_status_strobe, the controller stores the value of this signal in Expansion ROM Validation Status field (ROM_BAR_VALIDATION_STATUS) of the Expansion ROM BAR register.
        */
        volatile unsigned int exp_rom_validation_details:16; /* [Position:12] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        You can use this signal to notify the controller about the expansion ROM validation details. On assertion of exp_rom_validation_details_strobe, the controller stores the value of this signal in Expansion ROM Validation Details field (ROM_BAR_VALIDATION_DETAILS) of the Expansion ROM BAR register.
        */
        volatile unsigned int RESERVED_28:4; /* [Position:28] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw02_0_t;
/* offset : 0x8c */
typedef volatile union _acc_etc_rw03_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int ptm_external_master_time0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        PTM External Master Time.[LSB 32bit]
        */
    };
} acc_etc_rw03_0_t;
/* offset : 0x90 */
typedef volatile union _acc_etc_rw04_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int ptm_external_master_time1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        PTM External Master Time.[MSB 32bit]
        */
    };
} acc_etc_rw04_0_t;
/* offset : 0x94 */
typedef volatile union _acc_etc_rw05_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int prs_res_failure:4; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Pulse per function(PF) to indicate that the function (PF or associated VF) has received a PRG Response Failure.
        */
        volatile unsigned int prs_uprgi:4; /* [Position:4] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Pulse per function(PF) to indicate that the function (PF or associated VF) has received a response with Unexpected Page Request Group Index.
        */
        volatile unsigned int prs_stopped:4; /* [Position:8] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Pulse per function(PF) to indicate that the function (PF or associated VF) has completed all previously issued Page Requests and that it has stopped requests for additional pages. Only valid when the PRS enable bit is clear.
        */
        volatile unsigned int clkreq_in_n:1; /* [Position:12] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Status of the CLKREQ# bidirectional CMOS board-level signal. Used by the controller to determine when to enter and exit L1 Substates when using the CLKREQ#-based mechanism.
        */
        volatile unsigned int RESERVED_13:19; /* [Position:13] */
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
000: SC (Successful Completion)
001: UR (Unsupported Request)
010: RRS (Request Retry Status)
100: CA (Completer Abort) All others: Reserved
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
        volatile unsigned int slv_awmisc_info:25; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave Write Transaction Associated Misc Information.
[4:0]: TYPE bits of the TLP. For more information, see 'I/O and CFG Transaction Handling'
[5]: Serialize NP Requests [6]: EP bit of the TLP
[7]: Reserved [8]: NS bit of the TLP [9]: RO bit of the TLP
[12:10]: TC bits of the TLP [20:13]: MSG code of the TLP
[21]: AXI transaction is a DBI access. This is for SHARED DBI mode only.
[24:22]: Function number of the TLP. Function numbering starts at 
'0'. Not used when CX_SRIOV_ENABLE =1 or CX_ARI_ENABLE =1. 
        */
        volatile unsigned int RESERVED_25:7; /* [Position:25] */
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
        volatile unsigned int slv_armisc_info:25; /* [Position:0] */
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
        volatile unsigned int RESERVED_25:1; /* [Position:25] */
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
} acc_etc_rw16_0_t;
/* offset : 0xc4 */
typedef volatile union _acc_etc_rw17_0_t
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
} acc_etc_rw17_0_t;
/* offset : 0xc8 */
typedef volatile union _acc_etc_rw18_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int slv_armisc_info_tlpprfx0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave Read Request TLP Prefixes. The field [31:0] represents the first prefix to be transmitted. [31:0]
        */
    };
} acc_etc_rw18_0_t;
/* offset : 0xcc */
typedef volatile union _acc_etc_rw19_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int slv_armisc_info_tlpprfx1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave Read Request TLP Prefixes. The field [31:0] represents the first prefix to be transmitted. [63:32]
        */
    };
} acc_etc_rw19_0_t;
/* offset : 0xd0 */
typedef volatile union _acc_etc_rw20_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int slv_armisc_info_tlpprfx2:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave Read Request TLP Prefixes. The field [31:0] represents the first prefix to be transmitted. [95:64]
        */
    };
} acc_etc_rw20_0_t;
/* offset : 0xd4 */
typedef volatile union _acc_etc_rw21_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int slv_armisc_info_tlpprfx3:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        AXI Slave Read Request TLP Prefixes. The field [31:0] represents the first prefix to be transmitted. [127:96]
        */
    };
} acc_etc_rw21_0_t;
/* offset : 0xd8 */
typedef volatile union _acc_etc_rw22_0_t
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
} acc_etc_rw22_0_t;
/* offset : 0xdc */
typedef volatile union _acc_etc_rw23_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int ext_lbc_din0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Data bus from the external register block. Depending on value of CX_LBC_NW, there are 32/64/128 bits of ext_lbc_din for each function in your controller configuration. [31:0]
        */
    };
} acc_etc_rw23_0_t;
/* offset : 0xe0 */
typedef volatile union _acc_etc_rw24_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int ext_lbc_din1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Data bus from the external register block. Depending on value of CX_LBC_NW, there are 32/64/128 bits of ext_lbc_din for each function in your controller configuration. [63:32]
        */
    };
} acc_etc_rw24_0_t;
/* offset : 0xe4 */
typedef volatile union _acc_etc_rw25_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int ext_lbc_din2:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Data bus from the external register block. Depending on value of CX_LBC_NW, there are 32/64/128 bits of ext_lbc_din for each function in your controller configuration. [95:64]
        */
    };
} acc_etc_rw25_0_t;
/* offset : 0xe8 */
typedef volatile union _acc_etc_rw26_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int ext_lbc_din3:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Data bus from the external register block. Depending on value of CX_LBC_NW, there are 32/64/128 bits of ext_lbc_din for each function in your controller configuration. [127:96]
        */
    };
} acc_etc_rw26_0_t;
/* offset : 0xec */
typedef volatile union _acc_etc_rw27_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int ven_msi_req:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Request from your application to send an MSI when MSI is enabled. When MSI-X is enabled instead of MSI, assertion of ven_msi_req causes the controller to generate an MSI-X message.
        */
        volatile unsigned int ven_msi_tc:3; /* [Position:1] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Traffic Class of the MSI request, valid when ven_msi_req is asserted.
        */
        volatile unsigned int ven_msi_vector:5; /* [Position:4] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Used to modulate the lower five bits of the MSI Data register when multiple message mode is enabled.
        */
        volatile unsigned int ven_msi_func_num:3; /* [Position:9] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        The function number of the MSI request. Function numbering starts at '0'.
        */
        volatile unsigned int RESERVED_12:20; /* [Position:12] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_rw27_0_t;
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
        volatile unsigned int app_ltr_msg_func_num:3; /* [Position:1] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Function number in your application that is requesting to send an LTR message. Function numbering starts at '0'.
        */
        volatile unsigned int RESERVED_4:28; /* [Position:4] */
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
        volatile unsigned int ven_msg_func_num:3; /* [Position:26] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Function Number for the vendor-defined Message TLP. Function numbering starts at '0'.
        */
        volatile unsigned int RESERVED_29:3; /* [Position:29] */
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
        volatile unsigned int sys_int:4; /* [Position:2] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        When sys_int goes from low to high, the controller generates an Assert_INTx Message. When sys_int goes from high to low, the controller generates a Deassert_INTx Message.
        */
        volatile unsigned int apps_pm_xmt_pme:4; /* [Position:6] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Wake Up. If PME is enabled and PME support is configured for current PMCSR D-state asserting this signal causes the controller to wake from either L1 or L2 state.
        */
        volatile unsigned int sys_atten_button_pressed:4; /* [Position:10] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Attention Button Pressed. Indicates that the system attention button has been pressed, sets the Attention Button Pressed bit in the Slot Status Register.
        */
        volatile unsigned int sys_pre_det_state:4; /* [Position:14] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Presence Detect State. Indicates whether or not a card is present in the slot: 
0: Slot is empty 1: Card is present in the slot
        */
        volatile unsigned int sys_mrl_sensor_state:4; /* [Position:18] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        MRL Sensor State. Indicates the state of the manually-operated retention latch (MRL) sensor: 0: MRL is closed 1: MRL is open
        */
        volatile unsigned int sys_pwr_fault_det:4; /* [Position:22] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Power Fault Detected. Indicates the power controller detected a power fault at this slot. There is a separate sys_pwr_fault_det input bit for each function in your controller configuration.
        */
        volatile unsigned int sys_mrl_sensor_chged:4; /* [Position:26] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        MRL Sensor Changed. Indicates that the state of MRL sensor has changed. There is a separate sys_mrl_sensor_chged input bit for each function in your controller configuration.
        */
        volatile unsigned int RESERVED_30:2; /* [Position:30] */
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
        volatile unsigned int sys_pre_det_chged:4; /* [Position:0] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Presence Detect Changed. Indicates that the state of card present detector has changed.
        */
        volatile unsigned int sys_cmd_cpled_int:4; /* [Position:4] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Command completed Interrupt. Indicates that the Hot-Plug controller completed a command. There is a separate sys_cmd_cpled_int input bit for each function in your controller configuration.
        */
        volatile unsigned int sys_eml_interlock_engaged:4; /* [Position:8] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        System Electromechanical Interlock Engaged. Indicates whether the system electromechanical interlock is engaged and controls the state of the Electromechanical Interlock Status bit in the Slot Status register.
        */
        volatile unsigned int app_unlock_msg:1; /* [Position:12] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Request from your application to generate an Unlock message. You must assert this signal for one clock cycle.
        */
        volatile unsigned int apps_pm_xmt_turnoff:1; /* [Position:13] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Request from your application to generate a PM_Turn_Off message. You must assert this signal for one clock cycle.
        */
        volatile unsigned int outband_pwrup_cmd:4; /* [Position:14] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Wake Up. If PME is enabled and PME support is configured for current PMCSR D-state asserting this signal causes the controller to wake from either L1 or L2 state.
        */
        volatile unsigned int app_obff_idle_msg_req:1; /* [Position:18] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Request from your application to generate an 'IDLE' OBFF message. Only usable in a downstream port.
        */
        volatile unsigned int app_obff_obff_msg_req:1; /* [Position:19] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Request from your application to generate an 'OBFF' OBFF message. Only usable in a downstream port.
        */
        volatile unsigned int app_obff_cpu_active_msg_req:1; /* [Position:20] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Request from your application to generate a 'CPU Active' OBFF message. Only usable in a downstream port.
        */
        volatile unsigned int app_hdr_valid:1; /* [Position:21] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        One-clock-cycle pulse indicating that the data app_hdr_log, app_err_bus, app_err_func_num, and app_tlp_prfx_log is valid.
        */
        volatile unsigned int RESERVED_22:10; /* [Position:22] */
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
        volatile unsigned int app_err_func_num:3; /* [Position:28] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        The number of the function that is reporting the error indicated app_err_bus, valid when app_hdr_valid is asserted.
        */
        volatile unsigned int RESERVED_31:1; /* [Position:31] */
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
        volatile unsigned int app_dpc_rp_busy:1; /* [Position:1] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        DPC Root Port Busy. When Set, it indicates that the Root Port is busy with internal activity. DPC is event triggered. Used to update the corresponding DPC Status register field.
        */
        volatile unsigned int app_dev_num:5; /* [Position:2] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Device number. Your application must drive this signal to set the device number in the Requester ID for Root Port and Switch DSP port.
        */
        volatile unsigned int app_bus_num:8; /* [Position:7] */
        /* Default Value : 0, TYPE : RW */
        /* Description :
        Bus number. Your application must drive this signal to set the bus number in the Requester ID for Root Port and Switch DSP port.
        */
        volatile unsigned int RESERVED_15:17; /* [Position:15] */
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
        volatile unsigned int pf_frs_grant:4; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicator of when an FRS message for this function has been scheduled for transmission.
        */
        volatile unsigned int ptm_context_valid:1; /* [Position:4] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Context Valid.
        */
        volatile unsigned int ptm_responder_rdy_to_validate:1; /* [Position:5] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        PTM Responder Ready to Validate.
        */
        volatile unsigned int ptm_trigger_allowed:1; /* [Position:6] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that a PTM Requester manual update trigger is allowed.
        */
        volatile unsigned int ptm_updating:1; /* [Position:7] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that a PTM update is in progress.
        */
        volatile unsigned int ptm_clock_updated:1; /* [Position:8] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        ndicates that the controller has updated the Local Clock.
        */
        volatile unsigned int ptm_req_response_timeout:1; /* [Position:9] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        PTM Requester Response Timeout. Single-cycle pulse indicating 100us timeout occurred while waiting for a PTM Response or PTM ResponseD message.
        */
        volatile unsigned int ptm_req_dup_rx:1; /* [Position:10] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        PTM Requester Duplicate Received. Single-cycle pulse indicating PTM Requester received a duplicate TLP while, 
- PTM Requester is in the process of updating PTM local clock, or 
- Following calculation of the update, when the PTM context is valid
        */
        volatile unsigned int ptm_req_replay_tx:1; /* [Position:11] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        PTM Requester Replay Sent. Single-cycle pulse indicating PTM Requester detected a TLP replay being sent when ResponseD messages are in use while, 
- PTM Requester is in the process of updating PTM local clock, or 
- Following calculation of the update, when the PTM context is valid
        */
        volatile unsigned int RESERVED_12:20; /* [Position:12] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro000_0_t;
/* offset : 0x158 */
typedef volatile union _acc_etc_ro001_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int ptm_local_clock0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Local Clock value. [31:0]
        */
    };
} acc_etc_ro001_0_t;
/* offset : 0x15c */
typedef volatile union _acc_etc_ro002_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int ptm_local_clock1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Local Clock value. [63:32]
        */
    };
} acc_etc_ro002_0_t;
/* offset : 0x160 */
typedef volatile union _acc_etc_ro003_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int ptm_clock_correction0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Amount by which Local Clock has been corrected. [31:0]
        */
    };
} acc_etc_ro003_0_t;
/* offset : 0x164 */
typedef volatile union _acc_etc_ro004_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int ptm_clock_correction1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Amount by which Local Clock has been corrected. [63:32]
        */
    };
} acc_etc_ro004_0_t;
/* offset : 0x168 */
typedef volatile union _acc_etc_ro005_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_ats_stu:20; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Contents of the "Smallest Translation Unit" field (ATU) in the ATS_CAPABILITIES_CTRL_REG register.
        */
        volatile unsigned int cfg_ats_cache_en:4; /* [Position:20] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Contents of the "Cache Enable" (ENABLE) field in the ATS_CAPABILITIES_CTRL_REG register.
        */
        volatile unsigned int cfg_prs_enable:4; /* [Position:24] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that the Page Request Interface is allowed to make page requests. This output pin reflects the value of the PRS_ENABLE field in the PRS_CONTROL_STATUS_REG register.
        */
        volatile unsigned int cfg_prs_reset:4; /* [Position:28] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Pulse to indicate to the application that the associated page request credit counter and pending request state must be cleared. This output pin reflects the value of the PRS_RESET field in the PRS_CONTROL_STATUS_REG register.
        */
    };
} acc_etc_ro005_0_t;
/* offset : 0x16c */
typedef volatile union _acc_etc_ro006_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_prs_stopped:4; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        This output pin reflects the value of the PRS_STOPPED field in the Page Request Status Register (PRS_CONTROL_STATUS_REG) register.
        */
        volatile unsigned int cfg_prs_uprgi:4; /* [Position:4] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        This output pin reflects the value of the PRS_UPRGI field in the Page Request Status (PRS_CONTROL_STATUS_REG) register.
        */
        volatile unsigned int cfg_prs_response_failure:4; /* [Position:8] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        This output pin reflects the value of the PRS_RESP_FAILURE field in the Page Request Status (PRS_CONTROL_STATUS_REG) register.
        */
        volatile unsigned int cfg_hp_slot_ctrl_access:4; /* [Position:12] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Slot Control Accessed.
        */
        volatile unsigned int cfg_dll_state_chged_en:4; /* [Position:16] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Slot Control DLL State Change Enable.
        */
        volatile unsigned int cfg_cmd_cpled_int_en:4; /* [Position:20] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Slot Control Command Completed Interrupt Enable.
        */
        volatile unsigned int cfg_hp_int_en:4; /* [Position:24] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Slot Control Hot Plug Interrupt Enable.
        */
        volatile unsigned int cfg_pre_det_chged_en:4; /* [Position:28] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Slot Control Presence Detect Changed Enable.
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
/* offset : 0x174 */
typedef volatile union _acc_etc_ro008_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_prs_outstanding_allocation1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Signal to indicate the number of outstanding page request messages the associated Page Request Interface is allowed to issue. This output pin reflects the value of the PRS_OUTSTANDING_ALLOCATION field in the PRS_REQ_ALLOCATION_REG register. [63:32]
        */
    };
} acc_etc_ro008_0_t;
/* offset : 0x178 */
typedef volatile union _acc_etc_ro009_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_prs_outstanding_allocation2:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Signal to indicate the number of outstanding page request messages the associated Page Request Interface is allowed to issue. This output pin reflects the value of the PRS_OUTSTANDING_ALLOCATION field in the PRS_REQ_ALLOCATION_REG register. [95:64]
        */
    };
} acc_etc_ro009_0_t;
/* offset : 0x17c */
typedef volatile union _acc_etc_ro010_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_prs_outstanding_allocation3:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Signal to indicate the number of outstanding page request messages the associated Page Request Interface is allowed to issue. This output pin reflects the value of the PRS_OUTSTANDING_ALLOCATION field in the PRS_REQ_ALLOCATION_REG register. [127:96]
        */
    };
} acc_etc_ro010_0_t;
/* offset : 0x180 */
typedef volatile union _acc_etc_ro011_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_mrl_sensor_chged_en:4; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Slot Control MRL Sensor Changed Enable.
        */
        volatile unsigned int cfg_pwr_fault_det_en:4; /* [Position:4] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Slot Control Power Fault Detect Enable.
        */
        volatile unsigned int cfg_atten_button_pressed_en:4; /* [Position:8] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Slot Control Attention Button Pressed Enable.
        */
        volatile unsigned int local_ref_clk_req_n:1; /* [Position:12] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        This signal may be connected to the CLKREQ# driver to negotiate entry into L1 sub-states. When this signal is set to 1 the controller is requesting entry into L1 sub-states and CLKREQ# may be de-asserted.
        */
        volatile unsigned int cfg_hw_auto_sp_dis:1; /* [Position:13] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Autonomous speed disable. Used in downstream ports only.
        */
        volatile unsigned int cfg_end2end_tlp_pfx_blck:4; /* [Position:14] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The value of the End-End TLP Prefix Blocking field in the Device Control 2 register.
        */
        volatile unsigned int RESERVED_18:14; /* [Position:18] */
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
        volatile unsigned int mstr_armisc_info0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Read Transaction Associated Misc Information (from the TLP received by the native PCIe controller). [31:0]
        */
    };
} acc_etc_ro021_0_t;
/* offset : 0x1ac */
typedef volatile union _acc_etc_ro022_0_t
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
} acc_etc_ro022_0_t;
/* offset : 0x1b0 */
typedef volatile union _acc_etc_ro023_0_t
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
} acc_etc_ro023_0_t;
/* offset : 0x1b4 */
typedef volatile union _acc_etc_ro024_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mstr_armisc_info_tlpprfx0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Read Request TLP Prefixes. The field [31:0] represents the first prefix received. [31:0]
        */
    };
} acc_etc_ro024_0_t;
/* offset : 0x1b8 */
typedef volatile union _acc_etc_ro025_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mstr_armisc_info_tlpprfx1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Read Request TLP Prefixes. The field [31:0] represents the first prefix received. [63:32]
        */
    };
} acc_etc_ro025_0_t;
/* offset : 0x1bc */
typedef volatile union _acc_etc_ro026_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mstr_armisc_info_tlpprfx2:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Read Request TLP Prefixes. The field [31:0] represents the first prefix received. [95:64]
        */
    };
} acc_etc_ro026_0_t;
/* offset : 0x1c0 */
typedef volatile union _acc_etc_ro027_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mstr_armisc_info_tlpprfx3:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        AXI Master Read Request TLP Prefixes. The field [31:0] represents the first prefix received. [127:96]
        */
    };
} acc_etc_ro027_0_t;
/* offset : 0x1c4 */
typedef volatile union _acc_etc_ro028_0_t
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
        volatile unsigned int slv_bmisc_info:14; /* [Position:4] */
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
        volatile unsigned int slv_rmisc_info:14; /* [Position:18] */
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
    };
} acc_etc_ro028_0_t;
/* offset : 0x1c8 */
typedef volatile union _acc_etc_ro029_0_t
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
} acc_etc_ro029_0_t;
/* offset : 0x1cc */
typedef volatile union _acc_etc_ro030_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int lbc_ext_addr:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Address bus to the external register block. The width of the address bus is the value you select for the CX_LBC_EXT_AW parameter.
        */
    };
} acc_etc_ro030_0_t;
/* offset : 0x1d0 */
typedef volatile union _acc_etc_ro031_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int lbc_ext_dout:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Write data bus to the external register block, driven to all functions in a multi-function configuration.
        */
    };
} acc_etc_ro031_0_t;
/* offset : 0x1d4 */
typedef volatile union _acc_etc_ro032_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int lbc_ext_cs:4; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The controller asserts lbc_ext_cs when a received TLP for a read or write request has an address in the range of your application device, as determined by the BAR configuration.
        */
        volatile unsigned int lbc_ext_wr:4; /* [Position:4] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates when the external register access is a read or a write. For writes, lbc_ext_wr also indicates the byte enables.
        */
        volatile unsigned int lbc_ext_dbi_access:1; /* [Position:8] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that the current ELBI access is initiated using DBI.
        */
        volatile unsigned int lbc_ext_rom_access:1; /* [Position:9] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that the current ELBI access is for expansion ROM.
        */
        volatile unsigned int lbc_ext_io_access:1; /* [Position:10] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that the current ELBI access is an I/O access.
        */
        volatile unsigned int lbc_ext_bar_num:3; /* [Position:11] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The BAR number of the current ELBI access: 
000b: BAR 0, 001b: BAR 1, 010b: BAR 2, 011b: BAR 3 
100b: BAR 4, 101b: BAR 5, 110b is not used. 
111b is used to indicate a configuration access. This is designed for an application with application-specific configuration registers such as vendor capability. lbc_ext_bar_num is set to 3'b111 for all DBI accesses.
        */
        volatile unsigned int ven_msi_grant:1; /* [Position:14] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        One-cycle pulse that indicates that the controller has accepted the request to send an MSI. After asserting ven_msi_grant for one cycle, the controller does not wait for ven_msi_req to be de-asserted then reasserted to generate another MSI.
        */
        volatile unsigned int RESERVED_15:17; /* [Position:15] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro032_0_t;
/* offset : 0x1d8 */
typedef volatile union _acc_etc_ro033_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_msi_en:4; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that MSI is enabled (INTx message is not sent), one bit per configured function.
        */
        volatile unsigned int cfg_tph_req_en:8; /* [Position:4] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The 2-bit TPH Requester Enabled field of each TPH Requester Control register.
        */
        volatile unsigned int cfg_pf_pasid_en:4; /* [Position:12] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The value of the PASID Enable field in each PF PASID Control Register.
        */
        volatile unsigned int cfg_pf_pasid_execute_perm_en:4; /* [Position:16] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The value of the Execute Permission Enable field in each PF PASID Control Register.
        */
        volatile unsigned int cfg_pf_pasid_priv_mode_en:4; /* [Position:20] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The value of the Privileged Mode Enable field in each PF PASID Control Register.
        */
        volatile unsigned int dpa_substate_update:4; /* [Position:24] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        DPA Substate-Updated Indication Signal Bus. The controller asserts it for one core_clk cycle when the Substate Control field of the DPA Control Register has been updated with a new value for the associated function. The controller asserts it when the updated value of the Substate Control field does not match the Substate Status field of the Status Register when the Substate Control Enable bit field of the Status Register is set to "1".
        */
        volatile unsigned int cfg_ltr_m_en:1; /* [Position:28] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The LTR Mechanism Enable field of the Device Control 2 register of function 0.
        */
        volatile unsigned int app_ltr_msg_grant:1; /* [Position:29] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that the controller has accepted your request to send an LTR message.
        */
        volatile unsigned int RESERVED_30:2; /* [Position:30] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro033_0_t;
/* offset : 0x1dc */
typedef volatile union _acc_etc_ro034_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_ltr_max_latency0:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The concatenated contents of 'LTR Max No-Snoop Latency Register' and 'LTR Max Snoop Latency Register'. The 'LTR Max Snoop Latency ' of function 0 occupies the lower 16-bits. [31:0]
        */
    };
} acc_etc_ro034_0_t;
/* offset : 0x1e0 */
typedef volatile union _acc_etc_ro035_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_ltr_max_latency1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The concatenated contents of 'LTR Max No-Snoop Latency Register' and 'LTR Max Snoop Latency Register'. The 'LTR Max Snoop Latency ' of function 0 occupies the lower 16-bits. [63:32]
        */
    };
} acc_etc_ro035_0_t;
/* offset : 0x1e4 */
typedef volatile union _acc_etc_ro036_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_ltr_max_latency2:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The concatenated contents of 'LTR Max No-Snoop Latency Register' and 'LTR Max Snoop Latency Register'. The 'LTR Max Snoop Latency ' of function 0 occupies the lower 16-bits. [95:64]
        */
    };
} acc_etc_ro036_0_t;
/* offset : 0x1e8 */
typedef volatile union _acc_etc_ro037_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_ltr_max_latency3:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The concatenated contents of 'LTR Max No-Snoop Latency Register' and 'LTR Max Snoop Latency Register'. The 'LTR Max Snoop Latency ' of function 0 occupies the lower 16-bits. [127:96]
        */
    };
} acc_etc_ro037_0_t;
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
        volatile unsigned int radm_msg_ltr:1; /* [Position:1] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        One-clock-cycle pulse that indicates that the controller received an LTR message. The controller makes the message header available the radm_msg_payload output.
        */
        volatile unsigned int ven_msg_grant:1; /* [Position:2] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        One-cycle pulse that indicates that the controller has accepted the request to send the vendor-defined Message.
        */
        volatile unsigned int rtlh_rfc_upd:1; /* [Position:3] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that the controller received a flow control update DLLP. Used for applications that implement flow Control outside the controller.
        */
        volatile unsigned int RESERVED_4:28; /* [Position:4] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro039_0_t;
/* offset : 0x1f4 */
typedef volatile union _acc_etc_ro040_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_pwr_ind:8; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Controls the system power indicator (from bits [9:8] of the Slot Control register), per function: 00b: Reserved 01b: On 10b: Blink 11b: Off
        */
        volatile unsigned int cfg_atten_ind:8; /* [Position:8] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Controls the system attention indicator (from bits [7:6] of the Slot Control register), per function: 00b: Reserved 01b: On 10b: Blink 11b: Off
        */
        volatile unsigned int cfg_pwr_ctrler_ctrl:4; /* [Position:16] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Controls the system power controller (from bit 10 of the Slot Control register), per function:  0: Power On 1: Power Off
        */
        volatile unsigned int cfg_sys_err_rc:4; /* [Position:20] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        System error detected. A one-clock-cycle pulse that indicates if any device in the hierarchy reports any of the following errors and the associated enable bit is set in the Root Control register: ERR_COR, ERR_FATAL, ERR_NONFATAL.
        */
        volatile unsigned int cfg_aer_rc_err_int:4; /* [Position:24] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Asserted when a reported error condition causes a bit to be set in the Root Error Status register and the associated error message reporting enable bit is set in the Root Error Command register.
        */
        volatile unsigned int cfg_aer_rc_err_msi:4; /* [Position:28] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The controller asserts cfg_aer_rc_err_msi for one clock cycle when all of the following conditions are true: 
-MSI or MSI-X is enabled. 
-A reported error condition causes a bit to be set in the Root Error Status register. 
-The associated error message reporting enable bit is set in the Root Error Command register.
        */
    };
} acc_etc_ro040_0_t;
/* offset : 0x1f8 */
typedef volatile union _acc_etc_ro041_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_aer_int_msg_num:20; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        From bits [31:27] of the Root Error Status register, used when MSI or MSI-X is enabled. Assertion of cfg_aer_rc_err_msi along with a value cfg_aer_int_msg_num is equivalent to the controller receiving an MSI with the cfg_aer_int_msg_num value as the MSI vector.
        */
        volatile unsigned int cfg_pme_int:4; /* [Position:20] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The controller asserts cfg_pme_int when all of the following conditions are true: 
-The INTx Assertion Disable bit in the Command register is 0. 
-The PME Interrupt Enable bit in the Root Control register is set to 1. 
-The PME Status bit in the Root Status register is set to 1.
        */
        volatile unsigned int cfg_pme_msi:4; /* [Position:24] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The controller asserts cfg_pme_msi (as a one-cycle pulse) when all of the following conditions are true: 
-MSI or MSI-X is enabled. 
-The PME Interrupt Enable bit in the Root Control register is set to 1. 
-The PME Status bit in the Root Status register is set to 1.
        */
        volatile unsigned int cfg_crs_sw_vis_en:4; /* [Position:28] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates the value of the CRS Software Visibility enable bit in the Root Control register. Applicable only for Root Ports.
        */
    };
} acc_etc_ro041_0_t;
/* offset : 0x1fc */
typedef volatile union _acc_etc_ro042_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_pcie_cap_int_msg_num:20; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        From bits [13:9] of the PCI Express Capabilities register, used when MSI or MSI-X is enabled. Assertion of hp_msi or cfg_pme_msi along with a value cfg_pcie_cap_int_msg_num is equivalent to the controller receiving an MSI with the cfg_pcie_cap_int_msg_num value as the MSI vector.
        */
        volatile unsigned int cfg_eml_control:4; /* [Position:20] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Electromechanical Interlock Control. The state of the Electromechanical Interlock Control bit in the Slot Control register.
        */
        volatile unsigned int RESERVED_24:8; /* [Position:24] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro042_0_t;
/* offset : 0x200 */
typedef volatile union _acc_etc_ro043_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int rtlh_rfc_data:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The data from a received flow control update DLLP. The width and contents of rtlh_rfc_data depend the value of RX_NDLLP which is defined in the description of rtlh_rfc_upd.
        */
    };
} acc_etc_ro043_0_t;
/* offset : 0x204 */
typedef volatile union _acc_etc_ro044_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int radm_inta_asserted:1; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        One-clock-cycle pulse that indicates that the controller received an Assert_INTA Message from the downstream device.
        */
        volatile unsigned int radm_intb_asserted:1; /* [Position:1] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        One-clock-cycle pulse that indicates that the controller received an Assert_INTB Message from the downstream device.
        */
        volatile unsigned int radm_intc_asserted:1; /* [Position:2] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        One-clock-cycle pulse that indicates that the controller received an Assert_INTC Message from the downstream device.
        */
        volatile unsigned int radm_intd_asserted:1; /* [Position:3] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        One-clock-cycle pulse that indicates that the controller received an Assert_INTD Message from the downstream device.
        */
        volatile unsigned int radm_inta_deasserted:1; /* [Position:4] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        One-clock-cycle pulse that indicates that the controller received a Deassert_INTA Message from the downstream device.
        */
        volatile unsigned int radm_intb_deasserted:1; /* [Position:5] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        One-clock-cycle pulse that indicates that the controller received a Deassert_INTB Message from the downstream device.
        */
        volatile unsigned int radm_intc_deasserted:1; /* [Position:6] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        One-clock-cycle pulse that indicates that the controller received a Deassert_INTC Message from the downstream device.
        */
        volatile unsigned int radm_intd_deasserted:1; /* [Position:7] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        One-clock-cycle pulse that indicates that the controller received a Deassert_INTD Message from the downstream device.
        */
        volatile unsigned int radm_correctable_err:1; /* [Position:8] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        One-clock-cycle pulse that indicates that the controller received an ERR_COR message. The controller makes the message header available the radm_msg_payload output.
        */
        volatile unsigned int radm_nonfatal_err:1; /* [Position:9] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        One-clock-cycle pulse that indicates that the controller received an ERR_NONFATAL message.
        */
        volatile unsigned int radm_fatal_err:1; /* [Position:10] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        One-clock-cycle pulse that indicates that the controller received an ERR_FATAL message.
        */
        volatile unsigned int radm_pm_pme:1; /* [Position:11] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        One-clock-cycle pulse that indicates that the controller received a PM_PME message.
        */
        volatile unsigned int radm_pm_to_ack:1; /* [Position:12] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        One-clock-cycle pulse that indicates that the controller received a PME_TO_Ack message. Upstream port: Reserved.
        */
        volatile unsigned int pm_xtlh_block_tlp:1; /* [Position:13] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that your application must stop generating new outgoing request TLPs due to the current power management state.
        */
        volatile unsigned int hp_pme:4; /* [Position:14] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The controller asserts hp_pme when all of the following conditions are true: 
-The PME Enable bit in the Power Management Control and Status register is set to 1. 
-Any bit in the Slot Status register transitions from 0 to 1 and the associated event notification is enabled in the Slot Control register.
        */
        volatile unsigned int hp_int:4; /* [Position:18] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The controller asserts hp_int when all of the following conditions are true: 
-The INTx Assertion Disable bit in the Command register is 0. 
-Hot-Plug interrupts are enabled in the Slot Control register. 
-Any bit in the Slot Status register is equal to 1, and the associated event notification is enabled in the Slot Control register.
        */
        volatile unsigned int hp_msi:4; /* [Position:22] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The controller asserts hp_msi (as a one-cycle pulse) when the logical AND of the following conditions transitions from false to true: 
-MSI or MSI-X is enabled. 
-Hot-Plug interrupts are enabled in the Slot Control register. 
-Any bit in the Slot Status register transitions from 0 to 1 and the asso_x0002_ciated event notification is enabled in the Slot Control register.
        */
        volatile unsigned int radm_q_not_empty:4; /* [Position:26] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Level indicating that the receive queues contain TLP header/data.
        */
        volatile unsigned int RESERVED_30:2; /* [Position:30] */
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
        volatile unsigned int radm_qoverflow:4; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Pulse indicating that one or more of the P/NP/CPL receive queues have overflowed. There is a 1-bit indication for each configured virtual channel.
        */
        volatile unsigned int RESERVED_4:28; /* [Position:4] */
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
/* offset : 0x214 */
typedef volatile union _acc_etc_ro048_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar0_start2:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 0 (a memory or I/O BAR). There are 64 bits of cfg_bar0_start assigned to each configured function. The actual BAR start address present any 64-bit segment can be either a 64-bit or 32-bit BAR start address. [95:64]
        */
    };
} acc_etc_ro048_0_t;
/* offset : 0x218 */
typedef volatile union _acc_etc_ro049_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar0_start3:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 0 (a memory or I/O BAR). There are 64 bits of cfg_bar0_start assigned to each configured function. The actual BAR start address present any 64-bit segment can be either a 64-bit or 32-bit BAR start address. [127:96]
        */
    };
} acc_etc_ro049_0_t;
/* offset : 0x21c */
typedef volatile union _acc_etc_ro050_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar0_start4:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 0 (a memory or I/O BAR). There are 64 bits of cfg_bar0_start assigned to each configured function. The actual BAR start address present any 64-bit segment can be either a 64-bit or 32-bit BAR start address. [159:128]
        */
    };
} acc_etc_ro050_0_t;
/* offset : 0x220 */
typedef volatile union _acc_etc_ro051_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar0_start5:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 0 (a memory or I/O BAR). There are 64 bits of cfg_bar0_start assigned to each configured function. The actual BAR start address present any 64-bit segment can be either a 64-bit or 32-bit BAR start address. [191:160]
        */
    };
} acc_etc_ro051_0_t;
/* offset : 0x224 */
typedef volatile union _acc_etc_ro052_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar0_start6:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 0 (a memory or I/O BAR). There are 64 bits of cfg_bar0_start assigned to each configured function. The actual BAR start address present any 64-bit segment can be either a 64-bit or 32-bit BAR start address. [223:192]
        */
    };
} acc_etc_ro052_0_t;
/* offset : 0x228 */
typedef volatile union _acc_etc_ro053_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar0_start7:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 0 (a memory or I/O BAR). There are 64 bits of cfg_bar0_start assigned to each configured function. The actual BAR start address present any 64-bit segment can be either a 64-bit or 32-bit BAR start address. [255:224]
        */
    };
} acc_etc_ro053_0_t;
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
/* offset : 0x234 */
typedef volatile union _acc_etc_ro056_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar0_limit2:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 0 (a memory or I/O BAR). There are 64 bits of cfg_bar0_limit assigned to each configured function. The actual BAR end address present any 64-bit segment can be either a 64-bit or 32-bit BAR end address. [95:64]
        */
    };
} acc_etc_ro056_0_t;
/* offset : 0x238 */
typedef volatile union _acc_etc_ro057_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar0_limit3:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 0 (a memory or I/O BAR). There are 64 bits of cfg_bar0_limit assigned to each configured function. The actual BAR end address present any 64-bit segment can be either a 64-bit or 32-bit BAR end address. [127:96]
        */
    };
} acc_etc_ro057_0_t;
/* offset : 0x23c */
typedef volatile union _acc_etc_ro058_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar0_limit4:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 0 (a memory or I/O BAR). There are 64 bits of cfg_bar0_limit assigned to each configured function. The actual BAR end address present any 64-bit segment can be either a 64-bit or 32-bit BAR end address. [159:128]
        */
    };
} acc_etc_ro058_0_t;
/* offset : 0x240 */
typedef volatile union _acc_etc_ro059_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar0_limit5:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 0 (a memory or I/O BAR). There are 64 bits of cfg_bar0_limit assigned to each configured function. The actual BAR end address present any 64-bit segment can be either a 64-bit or 32-bit BAR end address. [191:160]
        */
    };
} acc_etc_ro059_0_t;
/* offset : 0x244 */
typedef volatile union _acc_etc_ro060_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar0_limit6:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 0 (a memory or I/O BAR). There are 64 bits of cfg_bar0_limit assigned to each configured function. The actual BAR end address present any 64-bit segment can be either a 64-bit or 32-bit BAR end address. [223:192]
        */
    };
} acc_etc_ro060_0_t;
/* offset : 0x248 */
typedef volatile union _acc_etc_ro061_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar0_limit7:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 0 (a memory or I/O BAR). There are 64 bits of cfg_bar0_limit assigned to each configured function. The actual BAR end address present any 64-bit segment can be either a 64-bit or 32-bit BAR end address. [255:224]
        */
    };
} acc_etc_ro061_0_t;
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
/* offset : 0x250 */
typedef volatile union _acc_etc_ro063_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar1_start1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 1 (a memory or I/O BAR). There are 32 bits of cfg_bar1_start assigned to each configured function. [63:32]
        */
    };
} acc_etc_ro063_0_t;
/* offset : 0x254 */
typedef volatile union _acc_etc_ro064_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar1_start2:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 1 (a memory or I/O BAR). There are 32 bits of cfg_bar1_start assigned to each configured function. [95:64]
        */
    };
} acc_etc_ro064_0_t;
/* offset : 0x258 */
typedef volatile union _acc_etc_ro065_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar1_start3:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 1 (a memory or I/O BAR). There are 32 bits of cfg_bar1_start assigned to each configured function. [127:96]
        */
    };
} acc_etc_ro065_0_t;
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
/* offset : 0x260 */
typedef volatile union _acc_etc_ro067_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar1_limit5:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 1 (a memory or I/O BAR). There are 32 bits of cfg_bar1_start assigned to each configured function. [191:160]
        */
    };
} acc_etc_ro067_0_t;
/* offset : 0x264 */
typedef volatile union _acc_etc_ro068_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar1_limit6:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 1 (a memory or I/O BAR). There are 32 bits of cfg_bar1_start assigned to each configured function. [223:192]
        */
    };
} acc_etc_ro068_0_t;
/* offset : 0x268 */
typedef volatile union _acc_etc_ro069_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar1_limit7:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 1 (a memory or I/O BAR). There are 32 bits of cfg_bar1_start assigned to each configured function. [255:224]
        */
    };
} acc_etc_ro069_0_t;
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
/* offset : 0x274 */
typedef volatile union _acc_etc_ro072_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar2_start2:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 2 (a memory or I/O BAR). There are 64 bits of cfg_bar2_start assigned to each configured function. The actual BAR start address present any 64-bit segment can be either a 64-bit or 32-bit BAR start address. [95:64]
        */
    };
} acc_etc_ro072_0_t;
/* offset : 0x278 */
typedef volatile union _acc_etc_ro073_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar2_start3:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 2 (a memory or I/O BAR). There are 64 bits of cfg_bar2_start assigned to each configured function. The actual BAR start address present any 64-bit segment can be either a 64-bit or 32-bit BAR start address. [127:96]
        */
    };
} acc_etc_ro073_0_t;
/* offset : 0x27c */
typedef volatile union _acc_etc_ro074_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar2_start4:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 2 (a memory or I/O BAR). There are 64 bits of cfg_bar2_start assigned to each configured function. The actual BAR start address present any 64-bit segment can be either a 64-bit or 32-bit BAR start address. [159:128]
        */
    };
} acc_etc_ro074_0_t;
/* offset : 0x280 */
typedef volatile union _acc_etc_ro075_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar2_start5:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 2 (a memory or I/O BAR). There are 64 bits of cfg_bar2_start assigned to each configured function. The actual BAR start address present any 64-bit segment can be either a 64-bit or 32-bit BAR start address. [191:160]
        */
    };
} acc_etc_ro075_0_t;
/* offset : 0x284 */
typedef volatile union _acc_etc_ro076_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar2_start6:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 2 (a memory or I/O BAR). There are 64 bits of cfg_bar2_start assigned to each configured function. The actual BAR start address present any 64-bit segment can be either a 64-bit or 32-bit BAR start address. [223:192]
        */
    };
} acc_etc_ro076_0_t;
/* offset : 0x288 */
typedef volatile union _acc_etc_ro077_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar2_start7:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 2 (a memory or I/O BAR). There are 64 bits of cfg_bar2_start assigned to each configured function. The actual BAR start address present any 64-bit segment can be either a 64-bit or 32-bit BAR start address. [255:224]
        */
    };
} acc_etc_ro077_0_t;
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
/* offset : 0x294 */
typedef volatile union _acc_etc_ro080_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar2_limit2:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 2 (a memory or I/O BAR). There are 64 bits of cfg_bar2_limit assigned to each configured function. The actual BAR end address present any 64-bit segment can be either a 64-bit or 32-bit BAR end address. [95:64]
        */
    };
} acc_etc_ro080_0_t;
/* offset : 0x298 */
typedef volatile union _acc_etc_ro081_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar2_limit3:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 2 (a memory or I/O BAR). There are 64 bits of cfg_bar2_limit assigned to each configured function. The actual BAR end address present any 64-bit segment can be either a 64-bit or 32-bit BAR end address. [127:96]
        */
    };
} acc_etc_ro081_0_t;
/* offset : 0x29c */
typedef volatile union _acc_etc_ro082_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar2_limit4:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 2 (a memory or I/O BAR). There are 64 bits of cfg_bar2_limit assigned to each configured function. The actual BAR end address present any 64-bit segment can be either a 64-bit or 32-bit BAR end address. [159:128]
        */
    };
} acc_etc_ro082_0_t;
/* offset : 0x2a0 */
typedef volatile union _acc_etc_ro083_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar2_limit5:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 2 (a memory or I/O BAR). There are 64 bits of cfg_bar2_limit assigned to each configured function. The actual BAR end address present any 64-bit segment can be either a 64-bit or 32-bit BAR end address. [191:160]
        */
    };
} acc_etc_ro083_0_t;
/* offset : 0x2a4 */
typedef volatile union _acc_etc_ro084_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar2_limit6:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 2 (a memory or I/O BAR). There are 64 bits of cfg_bar2_limit assigned to each configured function. The actual BAR end address present any 64-bit segment can be either a 64-bit or 32-bit BAR end address. [223:192]
        */
    };
} acc_etc_ro084_0_t;
/* offset : 0x2a8 */
typedef volatile union _acc_etc_ro085_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar2_limit7:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 2 (a memory or I/O BAR). There are 64 bits of cfg_bar2_limit assigned to each configured function. The actual BAR end address present any 64-bit segment can be either a 64-bit or 32-bit BAR end address. [255:224]
        */
    };
} acc_etc_ro085_0_t;
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
/* offset : 0x2b0 */
typedef volatile union _acc_etc_ro087_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar3_start1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 3 (a memory or I/O BAR). There are 32 bits of cfg_bar3_start assigned to each configured function.[63:32]
        */
    };
} acc_etc_ro087_0_t;
/* offset : 0x2b4 */
typedef volatile union _acc_etc_ro088_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar3_start2:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 3 (a memory or I/O BAR). There are 32 bits of cfg_bar3_start assigned to each configured function.[95:64]
        */
    };
} acc_etc_ro088_0_t;
/* offset : 0x2b8 */
typedef volatile union _acc_etc_ro089_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar3_start3:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 3 (a memory or I/O BAR). There are 32 bits of cfg_bar3_start assigned to each configured function.[127:96]
        */
    };
} acc_etc_ro089_0_t;
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
/* offset : 0x2c0 */
typedef volatile union _acc_etc_ro091_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar3_limit1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 3 (a memory or I/O BAR). There are 32 bits of cfg_bar3_limit assigned to each configured function.[63:32]
        */
    };
} acc_etc_ro091_0_t;
/* offset : 0x2c4 */
typedef volatile union _acc_etc_ro092_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar3_limit2:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 3 (a memory or I/O BAR). There are 32 bits of cfg_bar3_limit assigned to each configured function.[95:64]
        */
    };
} acc_etc_ro092_0_t;
/* offset : 0x2c8 */
typedef volatile union _acc_etc_ro093_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar3_limit3:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 3 (a memory or I/O BAR). There are 32 bits of cfg_bar3_limit assigned to each configured function.[127:96]
        */
    };
} acc_etc_ro093_0_t;
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
/* offset : 0x2d4 */
typedef volatile union _acc_etc_ro096_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar4_start2:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 4 (a memory or I/O BAR). There are 64 bits of cfg_bar4_start assigned to each configured function. The actual BAR start address present any 64-bit segment can be either a 64-bit or 32-bit BAR start address. [95:64]
        */
    };
} acc_etc_ro096_0_t;
/* offset : 0x2d8 */
typedef volatile union _acc_etc_ro097_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar4_start3:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 4 (a memory or I/O BAR). There are 64 bits of cfg_bar4_start assigned to each configured function. The actual BAR start address present any 64-bit segment can be either a 64-bit or 32-bit BAR start address. [127:96]
        */
    };
} acc_etc_ro097_0_t;
/* offset : 0x2dc */
typedef volatile union _acc_etc_ro098_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar4_start4:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 4 (a memory or I/O BAR). There are 64 bits of cfg_bar4_start assigned to each configured function. The actual BAR start address present any 64-bit segment can be either a 64-bit or 32-bit BAR start address. [159:128]
        */
    };
} acc_etc_ro098_0_t;
/* offset : 0x2e0 */
typedef volatile union _acc_etc_ro099_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar4_start5:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 4 (a memory or I/O BAR). There are 64 bits of cfg_bar4_start assigned to each configured function. The actual BAR start address present any 64-bit segment can be either a 64-bit or 32-bit BAR start address. [191:160]
        */
    };
} acc_etc_ro099_0_t;
/* offset : 0x2e4 */
typedef volatile union _acc_etc_ro100_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar4_start6:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 4 (a memory or I/O BAR). There are 64 bits of cfg_bar4_start assigned to each configured function. The actual BAR start address present any 64-bit segment can be either a 64-bit or 32-bit BAR start address. [223:192]
        */
    };
} acc_etc_ro100_0_t;
/* offset : 0x2e8 */
typedef volatile union _acc_etc_ro101_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar4_start7:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 4 (a memory or I/O BAR). There are 64 bits of cfg_bar4_start assigned to each configured function. The actual BAR start address present any 64-bit segment can be either a 64-bit or 32-bit BAR start address. [255:224]
        */
    };
} acc_etc_ro101_0_t;
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
/* offset : 0x2f4 */
typedef volatile union _acc_etc_ro104_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar4_limit2:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 4 (a memory or I/O BAR). There are 64 bits of cfg_bar4_limit assigned to each configured function. The actual BAR end address present any 64-bit segment can be either a 64-bit or 32-bit BAR end address. [95:64]
        */
    };
} acc_etc_ro104_0_t;
/* offset : 0x2f8 */
typedef volatile union _acc_etc_ro105_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar4_limit3:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 4 (a memory or I/O BAR). There are 64 bits of cfg_bar4_limit assigned to each configured function. The actual BAR end address present any 64-bit segment can be either a 64-bit or 32-bit BAR end address. [127:96]
        */
    };
} acc_etc_ro105_0_t;
/* offset : 0x2fc */
typedef volatile union _acc_etc_ro106_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar4_limit4:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 4 (a memory or I/O BAR). There are 64 bits of cfg_bar4_limit assigned to each configured function. The actual BAR end address present any 64-bit segment can be either a 64-bit or 32-bit BAR end address. [159:128]
        */
    };
} acc_etc_ro106_0_t;
/* offset : 0x300 */
typedef volatile union _acc_etc_ro107_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar4_limit5:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 4 (a memory or I/O BAR). There are 64 bits of cfg_bar4_limit assigned to each configured function. The actual BAR end address present any 64-bit segment can be either a 64-bit or 32-bit BAR end address. [191:160]
        */
    };
} acc_etc_ro107_0_t;
/* offset : 0x304 */
typedef volatile union _acc_etc_ro108_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar4_limit6:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 4 (a memory or I/O BAR). There are 64 bits of cfg_bar4_limit assigned to each configured function. The actual BAR end address present any 64-bit segment can be either a 64-bit or 32-bit BAR end address. [223:192]
        */
    };
} acc_etc_ro108_0_t;
/* offset : 0x308 */
typedef volatile union _acc_etc_ro109_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar4_limit7:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 4 (a memory or I/O BAR). There are 64 bits of cfg_bar4_limit assigned to each configured function. The actual BAR end address present any 64-bit segment can be either a 64-bit or 32-bit BAR end address. [255:224]
        */
    };
} acc_etc_ro109_0_t;
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
/* offset : 0x310 */
typedef volatile union _acc_etc_ro111_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar5_start1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 5 (a memory or I/O BAR). There are 32 bits of cfg_bar5_start assigned to each configured function.[63:32]
        */
    };
} acc_etc_ro111_0_t;
/* offset : 0x314 */
typedef volatile union _acc_etc_ro112_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar5_start2:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 5 (a memory or I/O BAR). There are 32 bits of cfg_bar5_start assigned to each configured function.[95:64]
        */
    };
} acc_etc_ro112_0_t;
/* offset : 0x318 */
typedef volatile union _acc_etc_ro113_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar5_start3:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of BAR 5 (a memory or I/O BAR). There are 32 bits of cfg_bar5_start assigned to each configured function.[127:96]
        */
    };
} acc_etc_ro113_0_t;
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
/* offset : 0x320 */
typedef volatile union _acc_etc_ro115_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar5_limit1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 5 (a memory or I/O BAR). There are 32 bits of cfg_bar5_limit assigned to each configured function.[63:32]
        */
    };
} acc_etc_ro115_0_t;
/* offset : 0x324 */
typedef volatile union _acc_etc_ro116_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar5_limit2:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 5 (a memory or I/O BAR). There are 32 bits of cfg_bar5_limit assigned to each configured function.[95:64]
        */
    };
} acc_etc_ro116_0_t;
/* offset : 0x328 */
typedef volatile union _acc_etc_ro117_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bar5_limit3:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of BAR 5 (a memory or I/O BAR). There are 32 bits of cfg_bar5_limit assigned to each configured function.[127:96]
        */
    };
} acc_etc_ro117_0_t;
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
/* offset : 0x330 */
typedef volatile union _acc_etc_ro119_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_exp_rom_start1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of expansion ROM. There are 32 bits of cfg_exp_rom_start assigned to each configured function.[63:32]
        */
    };
} acc_etc_ro119_0_t;
/* offset : 0x334 */
typedef volatile union _acc_etc_ro120_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_exp_rom_start2:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of expansion ROM. There are 32 bits of cfg_exp_rom_start assigned to each configured function.[95:64]
        */
    };
} acc_etc_ro120_0_t;
/* offset : 0x338 */
typedef volatile union _acc_etc_ro121_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_exp_rom_start3:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The starting address of expansion ROM. There are 32 bits of cfg_exp_rom_start assigned to each configured function.[127:96]
        */
    };
} acc_etc_ro121_0_t;
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
/* offset : 0x340 */
typedef volatile union _acc_etc_ro123_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_exp_rom_limit1:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of expansion ROM. There are 32 bits of cfg_exp_rom_limit assigned to each configured function.[63:32]
        */
    };
} acc_etc_ro123_0_t;
/* offset : 0x344 */
typedef volatile union _acc_etc_ro124_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_exp_rom_limit2:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of expansion ROM. There are 32 bits of cfg_exp_rom_limit assigned to each configured function.[95:64]
        */
    };
} acc_etc_ro124_0_t;
/* offset : 0x348 */
typedef volatile union _acc_etc_ro125_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_exp_rom_limit3:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The end address of expansion ROM. There are 32 bits of cfg_exp_rom_limit assigned to each configured function.[127:96]
        */
    };
} acc_etc_ro125_0_t;
/* offset : 0x34c */
typedef volatile union _acc_etc_ro126_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_bus_master_en:4; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The state of the bus master enable bit in the PCI-compatible Command register. There is 1 bit of cfg_bus_master_en assigned to each configured function.
        */
        volatile unsigned int RESERVED_4:1; /* [Position:4] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
        volatile unsigned int cfg_2nd_reset:1; /* [Position:5] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Secondary Bus Reset. Indicates that your application has requested this downstream port to start Link hot reset. This signal is asserted when your application writes to the Secondary Bus Reset field (bit 6) of the Bridge Control Register (Offset 0x3E) in the Type 1 Configuration Space header.
        */
        volatile unsigned int cfg_rcb:4; /* [Position:6] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The value of the RCB bit in the Link Control register. There is 1 bit of cfg_rcb assigned to each configured function.
        */
        volatile unsigned int cfg_max_payload_size:12; /* [Position:10] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The value of the Max_Payload_Size field in the Device Control register. There are 3 bits of cfg_max_payload_size assigned to each configured function.
        */
        volatile unsigned int RESERVED_22:10; /* [Position:22] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro126_0_t;
/* offset : 0x350 */
typedef volatile union _acc_etc_ro127_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_2ndbus_num:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Configured Secondary Bus Number. The secondary bus number assigned to the device. When the host software detects the upstream switch port, it assigns a secondary bus number.
        */
    };
} acc_etc_ro127_0_t;
/* offset : 0x354 */
typedef volatile union _acc_etc_ro128_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_subbus_num:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Configured Subordinate Bus Number. When the switch controller receives a configuration request from the upstream port, it must see the secondary and subordinate bus number fields from the downstream ports to route the request.
        */
    };
} acc_etc_ro128_0_t;
/* offset : 0x358 */
typedef volatile union _acc_etc_ro129_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_vc_enable:4; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Configured VC Enable. Indicates the VC enable state for each supported VC. NVC represents the number of supported VCs. NoteVC0 is always supported and enabled.
        */
        volatile unsigned int cfg_vc_struc_vc_id_map:12; /* [Position:4] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Configured Structural VC to VC Identifier Map. This bus provides the mapping from the structural (physical) VC numbering scheme, that is, the order in which the VCs are declared in the configuration space, to the logical VC identifiers that are assigned by system software.
- Bits [2:0] return the logical ID for structural VC 0 and always equals to 0.
- Bits [5:3] return the logical ID for structural VC 1.
- and so on.
        */
        volatile unsigned int RESERVED_16:16; /* [Position:16] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro129_0_t;
/* offset : 0x35c */
typedef volatile union _acc_etc_ro130_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_vc_id_vc_struc_map:24; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Configured VC Identifier to Structural VC Map. This bus provides the mapping of the logical VC identifiers that are defined by the system, to the structural (physical) VC numbering scheme. To obtain the structural ID, index this bus with the logical ID. For example, 
- Bits [2:0] return the structural ID (position) for the VC assigned to VC 0 and always equals to 0. 
- Bits [5:3] return the structural ID (position) for the VC assigned to VC 1. 
- and so on. When a given ID is not assigned, the bus returns 0.
        */
        volatile unsigned int RESERVED_24:8; /* [Position:24] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro130_0_t;
/* offset : 0x360 */
typedef volatile union _acc_etc_ro131_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_tc_enable:8; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates the enabled TCs. TCn is enabled when bit[n] is 1.
        */
        volatile unsigned int cfg_tc_struc_vc_map:24; /* [Position:8] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        TC to Structural VC map. 
- Bits [2:0] return the structural VC number for the TC0 and always equals to 0. 
- Bits [5:3] return the structural VC number for the TC1. 
- and so on. Each structural VC[m] that this signal indicates is enabled by cfg_vc_enable[m].
        */
    };
} acc_etc_ro131_0_t;
/* offset : 0x364 */
typedef volatile union _acc_etc_ro132_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_mem_space_en:4; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The state of the Memory Space Enable bit in the PCI-compatible Command register. There is 1 bit of cfg_mem_space_en assigned to each configured function.
        */
        volatile unsigned int cfg_max_rd_req_size:12; /* [Position:4] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The value of the Max_Read_Request_Size field in the Device Control register. There are 3 bits of cfg_max_rd_req_size assigned to each configured function.
        */
        volatile unsigned int cfg_reg_serren:4; /* [Position:16] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        PF's SERR# Enable registers value in Command Register of Type0 Header, for setting error status registers of external VFs.
        */
        volatile unsigned int cfg_cor_err_rpt_en:4; /* [Position:20] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        PF's Correctable Error Reporting Enable registers value in Device Control Register of PCIe Capability, for sending ERR_MSG of external VFs.
        */
        volatile unsigned int cfg_nf_err_rpt_en:4; /* [Position:24] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        PF's Non-Fatal Error Reporting Enable registers value in Device Control Register of PCIe Capability, for sending ERR_MSG of external VFs.
        */
        volatile unsigned int cfg_f_err_rpt_en:4; /* [Position:28] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        PF's Fatal Error Reporting Enable registers value in Device Control Register of PCIe Capability, for sending ERR_MSG of external VFs.
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
        volatile unsigned int pm_dstate:12; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The current power management D-state of the function: 
000b: D0, 001b: D1, 010b: D2, 
011b: D3, 100b: Uninitialized, Other values: Not applicable
        */
        volatile unsigned int aux_pm_en:4; /* [Position:12] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Auxiliary Power Enable bit in the Device Control register. There is 1 bit of aux_pm_en for each configured function.
        */
        volatile unsigned int pm_pme_en:4; /* [Position:16] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        PME Enable bit in the PMCSR. There is 1 bit of pm_pme_en for each configured function.
        */
        volatile unsigned int pm_linkst_in_l0s:1; /* [Position:20] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Power management is in L0s state.
        */
        volatile unsigned int pm_linkst_in_l1:1; /* [Position:21] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Power management is in L1 state.
        */
        volatile unsigned int pm_l1_entry_started:1; /* [Position:22] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        L1 entry process is in progress.
        */
        volatile unsigned int pm_linkst_in_l2:1; /* [Position:23] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Power management is in L2 state
        */
        volatile unsigned int pm_linkst_l2_exit:1; /* [Position:24] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Power management is exiting L2 state. Not applicable for downstream port.
        */
        volatile unsigned int pm_linkst_in_l1sub:1; /* [Position:25] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Power management is in L1 substate. Indicates when the link has entered L1 substates. It is used in DWC_pcie_clkrst.v (see 'Clock Generation and Gating Design Example') to ensure that the switching back of aux_clk from AUXCLK to PCLK occurs only after L1 substates have been exited.
        */
        volatile unsigned int cfg_l1sub_en:1; /* [Position:26] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that any of the L1 Substates are enabled in the L1 Substates Control 1 Register. Could be used by your application in a downstream port to determine when not to drive CLKREQ# such as when L1 Substates are not enabled.
        */
        volatile unsigned int pm_status:4; /* [Position:27] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        PME Status bit from the PMCSR. There is 1 bit of pm_status for each configured function.
        */
        volatile unsigned int RESERVED_31:1; /* [Position:31] */
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
        volatile unsigned int app_obff_msg_grant:1; /* [Position:18] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates that the controller has accepted your request to generate an OBFF message. Only usable in a downstream port
        */
        volatile unsigned int radm_vendor_msg:1; /* [Position:19] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        One-cycle pulse that indicates the controller received a vendor-defined message. The controller makes the message header available the radm_msg_payload output. When FX_TLP > 1 and when two messages of the same type are received in the same clock cycle (back-to-back), then both bits are asserted.
        */
        volatile unsigned int RESERVED_20:12; /* [Position:20] */
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
        volatile unsigned int trgt_timeout_cpl_func_num:3; /* [Position:18] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The function number of the timed out completion. Function numbering starts at '0'.
        */
        volatile unsigned int RESERVED_21:11; /* [Position:21] */
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
        volatile unsigned int radm_timeout_func_num:3; /* [Position:10] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The function Number of the timed out completion. Function numbering starts at '0'.
        */
        volatile unsigned int RESERVED_13:19; /* [Position:13] */
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
        volatile unsigned int cfg_int_pin:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The cfg_int_pin indicates the configured value for the Interrupt Pin Register field in the BRIDGE_CTRL_INT_PIN_INT_LINE register.
        */
    };
} acc_etc_ro150_0_t;
/* offset : 0x3b0 */
typedef volatile union _acc_etc_ro151_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_send_cor_err:4; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Sent Correctable Error. Controller has sent a message towards the Root Complex indicating that an Rx TLP that contained an error, and that can be corrected, has been received by the Endpoint
        */
        volatile unsigned int cfg_send_nf_err:4; /* [Position:4] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Sent Non-Fatal Error. Controller has sent a message towards the Root Complex indicating that an Rx TLP that contained an non-fatal error, and that can not be corrected, has been received by the Endpoint
        */
        volatile unsigned int cfg_send_f_err:4; /* [Position:8] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Sent Fatal Error. Controller has sent a message towards the Root Complex indicating that an Rx TLP that contained a fatal error, and that can not be corrected, has been received by the Endpoint
        */
        volatile unsigned int cfg_int_disable:4; /* [Position:12] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        When high a functions ability to generate INTx messages is Disabled
        */
        volatile unsigned int cfg_no_snoop_en:4; /* [Position:16] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Contents of the "Enable No Snoop" field (PCIE_CAP_EN_NO_SNOOP) in the "Device Control and Status" register (DEVICE_CONTROL_DEVICE_STATUS) register.
        */
        volatile unsigned int cfg_relax_order_en:4; /* [Position:20] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Contents of the "Enable Relaxed Ordering" field (PCIE_CAP_EN_REL_ORDER) in the "Device Control and Status" register (DEVICE_CONTROL_DEVICE_STATUS) register.
        */
        volatile unsigned int cfg_link_auto_bw_int:1; /* [Position:24] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The controller asserts cfg_link_auto_bw_int when all of the following conditions are true: 
- The INTx assertion disable bit in the Command register is 0, and 
- The Link Autonomous Bandwidth Interrupt Enable bit in the Link Control register is set to 1, and 
- The Link Autonomous Bandwidth Interrupt Status bit in the Link Status register is set to 1. The cfg_link_auto_bw_msi output is a pulse signal (only asserted for one clock cycle); but cfg_link_auto_bw_int is a level signal.
        */
        volatile unsigned int cfg_link_auto_bw_msi:1; /* [Position:25] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The controller sets this pin when following conditions are true: 
- MSI or MSI-X is enabled. 
- The Link Autonomous Bandwidth Status register (Link Status register bit 15) is updated. 
- The Link Autonomous Bandwidth Interrupt Enable (Link Control register bit 11) is set. The controller does not check if the associated MSI vector (asserted cfg_pcie_cap_int_msg_num) is unmasked. It is up to the application to check whether the vector is masked or unmasked.
        */
        volatile unsigned int cfg_bw_mgt_int:1; /* [Position:26] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The controller asserts cfg_bw_mgt_int when all of the following conditions are true: 
- The INTx Assertion Disable bit in the Command register is 0, and 
- The Bandwidth Management Interrupt Enable bit in the Link Control register is set to 1, and 
- The Bandwidth Management Interrupt Status bit in the Link Status register is set to 1. The cfg_bw_mgt_msi output is a pulse signal (only asserted for one clock cycle); but cfg_bw_mgt_int is a level signal.
        */
        volatile unsigned int cfg_bw_mgt_msi:1; /* [Position:27] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        The controller sets this pin when following conditions are true: 
- MSI or MSI-X is enabled. 
- The Link Bandwidth Management Status register (Link Control Status register bit 14) is updated 
- The Link Bandwidth Management Interrupt Enable (Link Control register bit 10) is set.
        */
        volatile unsigned int cfg_link_eq_req_int:1; /* [Position:28] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Interrupt indicating to your application that the Link Equalization Request bit in the Link Status 2 Register has been set and the Link Equalization Request Interrupt Enable (Link Control 3 Register bit 1) is set.
        */
        volatile unsigned int usp_eq_redo_executed_int:1; /* [Position:29] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Interrupt indicating the EQ redo is executed by USP. You can use mac_phy_rate to know the data rate at which the interrupt occured. You can leave this pin unconnected.
        */
        volatile unsigned int RESERVED_30:2; /* [Position:30] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro151_0_t;
/* offset : 0x3b4 */
typedef volatile union _acc_etc_ro152_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int frsq_msi:4; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        FRSQ Interrupt Pulse. The RC asserts this output when it receives an FRQ message or when the FRS queue overflows.
        */
        volatile unsigned int frsq_int_msg_num:20; /* [Position:4] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        FRSQ Interrupt Message Number. This reflects the FRS_INT_MESSAGE_NUMBER field in the FRSQ_CAP_OFF register.
        */
        volatile unsigned int cfg_drs_msi:4; /* [Position:24] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        DRS Message Received Interrupt Pulse. The DSP controller asserts the cfg_drs_msi output when all of the following are true: 
- It receives a DRS message 
- PCIE_CAP_DRS_SIGNALING_CONTROL in LINK_CON_x0002_TROL_LINK_STATUS_REG is 2'b01 
- MSI or MSI-X is enabled
        */
        volatile unsigned int cfg_up_drs_to_frs:4; /* [Position:28] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        DRS to FRS Pulse. The DSP controller asserts the cfg_up_drs_to_frs output and sends an FRS message with the reason code set to 'DRS Message Received' when: 
- It receives a DRS message, and 
- PCIE_CAP_DRS_SIGNALING_CONTROL in LINK_CONTROL_LINK_STATUS_REG is 2'b10
        */
    };
} acc_etc_ro152_0_t;
/* offset : 0x3b8 */
typedef volatile union _acc_etc_ro153_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_br_ctrl_serren:4; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        PF's SERR# Enable registers value in Bridge Control Register of Type1 Header. You can use this value to control forwarding of ERR_COR, ERR_NONFATAL, and ERR_FATAL from secondary to primary.
        */
        volatile unsigned int pm_l1sub_state:3; /* [Position:4] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Power management L1 sub-states FSM state
        */
        volatile unsigned int rppio_exception_error:4; /* [Position:7] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Exception Error when RP PIO error occurrs
        */
        volatile unsigned int core_in_dpc:1; /* [Position:11] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        PCIe Controller is in Downstream Port Containment mode.
        */
        volatile unsigned int cfg_dpc_trig_en:2; /* [Position:12] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        DPC Trigger Enable.
        */
        volatile unsigned int cfg_dpc_cpl_cntrl:1; /* [Position:14] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        DPC Completion Control.
        */
        volatile unsigned int cfg_dpc_int_en:1; /* [Position:15] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        DPC Interrupt Enable.
        */
        volatile unsigned int dpc_int:1; /* [Position:16] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        DPC Interrupt Status for INTx signalling. If the Port is enabled for level-triggered interrupt signaling using INTx messages, dpc_int is asserted whenever and as long as the following conditions are satisfied: 
- The value of the Interrupt Disable bit in the Command register is 0b. 
- The value of the DPC Interrupt Enable bit is 1b. 
- The value of the DPC Interrupt Status bit is 1b.
        */
        volatile unsigned int dpc_msi:1; /* [Position:17] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        DPC Interrupt pulse for MSI signalling. If the Port is enabled for edge-triggered interrupt signaling using MSI or MSI-X, dpc_msi is asserted every time the logical AND of the following conditions transitions from FALSE to TRUE: 
- MSI or MSI-X is enabled. 
- The value of the DPC Interrupt Enable bit is 1b. 
- The value of the DPC Interrupt Status bit is 1b
        */
        volatile unsigned int cfg_dpc_err_cor_en:1; /* [Position:18] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        DPC ERR_COR Enable
        */
        volatile unsigned int cfg_dpc_poison_tlp_egr_block_en:1; /* [Position:19] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        DPC Poisoned TLP Egress Blocking Enable.
        */
        volatile unsigned int cfg_dpc_sw_trigger:1; /* [Position:20] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        DPC Software Trigger
        */
        volatile unsigned int cfg_dpc_dl_active_err_cor_en:1; /* [Position:21] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        DPC DL_Active ERR_COR Enable
        */
        volatile unsigned int pm_master_state:5; /* [Position:22] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Power management master FSM state.
        */
        volatile unsigned int pm_slave_state:5; /* [Position:27] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Power management slave FSM state
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
        volatile unsigned int radm_trgt1_atu_sloc_match:2; /* [Position:13] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        ATU Single Location match indication per ATU region. Set for 1 core_clk period pulse when the ATU region matched a received VDM Single Location Address translation.
        */
        volatile unsigned int radm_trgt1_atu_cbuf_err:2; /* [Position:15] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        ATU Error indication per ATU region. Set for 1 core_clk period pulse when the ATU region matched received VDM size (payload + 3rd and 4th DW of Header) is greater than the programmed Circular Buffer Increment size (CBUF_INCR) for Single Location Address translation.
        */
        volatile unsigned int pm_aspm_l1_enter_ready:1; /* [Position:17] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Indicates the controller is idle in L0 or L0s, and it is ready to enter L1 as soon as application releases app_xfer_pending AND the L1 ASPM timer expires.
        */
        volatile unsigned int radm_slot_pwr_limit:1; /* [Position:18] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        One-Clock-Cycle pulse that indicates the controller received a Set_Slot_Power_Limit message.
        */
        volatile unsigned int RESERVED_19:13; /* [Position:19] */
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
        volatile unsigned int cfg_ido_req_en:4; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        ID-Based Ordering Requests Enabled. Independent ordering based on Requester/Completer ID
        */
        volatile unsigned int cfg_ido_cpl_en:4; /* [Position:4] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        ID-Based Ordering Completions Enabled. Independent ordering based on Requester/Completer ID
        */
        volatile unsigned int cfg_pf_tph_st_mode:12; /* [Position:8] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        Steering Tag Mode of Operation for Physical Function. Steering Tags are system-specific values used to identify a processing resource that a Requester explicitly targets
        */
        volatile unsigned int cfg_ln_enable:4; /* [Position:20] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        This signal reflects the contents of the LNR_ENABLE field in the LN Control and Capabilities register in a EP device.
        */
        volatile unsigned int radm_idle:1; /* [Position:24] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        RADM activity status signal. The controller creates the en_radm_clk_g output by gating this signal with the output of the RADM_CLK_GATING_EN field in the CLOCK_GATING_CTRL_OFF register. For debug purposes only.
        */
        volatile unsigned int cfg_pm_no_soft_rst:4; /* [Position:25] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        This is the value of the No Soft Reset bit in the Power Management Control and Status Register. When set, you should not reset any controller registers when transitioning from D3hot to D0. Therefore, you should not assert the non_sticky_rst_n and sticky_rst_n inputs.
        */
        volatile unsigned int RESERVED_29:3; /* [Position:29] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_etc_ro156_0_t;
/* offset : 0x400 */
typedef volatile union _acc_ras_rw00_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int diag_ctrl_bus:3; /* [Position:0] */
        /* Default Value : , TYPE : RW */
        /* Description :
        
        */
        volatile unsigned int app_ras_des_tba_ctrl:2; /* [Position:3] */
        /* Default Value : , TYPE : RW */
        /* Description :
        
        */
        volatile unsigned int RESERVED_5:3; /* [Position:5] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
        volatile unsigned int app_ras_des_sd_hold_ltssm:1; /* [Position:8] */
        /* Default Value : , TYPE : RO */
        /* Description :
        Hold and release LTSSM. For as long as this signal is '1', the controller stays in the current LTSSM
        */
        volatile unsigned int app_ras_des_sd_hold_ltssm_start:1; /* [Position:9] */
        /* Default Value : , TYPE : RO */
        /* Description :
        Start LTSSM signal
        */
        volatile unsigned int RESERVED_10:6; /* [Position:10] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
        volatile unsigned int app_ras_des_sd_hold_ltssm_time:16; /* [Position:16] */
        /* Default Value : , TYPE : RO */
        /* Description :
        Measure LTSSM time
        */
    };
} acc_ras_rw00_0_t;
/* offset : 0x404 */
typedef volatile union _acc_ras_st_bus_ro00_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int rtfcgen_incr_amt:9;        /* [Position:0] */
        /*  = Payload flow control credits consumed. NOTE: hdr credits consumed is always 1 */
        volatile unsigned int rtfcgen_incr_enable:1;        /* [Position:9] */
        /*  = Header flow control credit consumed */
        volatile unsigned int rtfcgen_fctype:2;        /* [Position:10] */
        /*  = Flow control type consumed (P=0, NP=1, CPL=2) */
        volatile unsigned int rtcheck_rtfcgen_vc:3;        /* [Position:12] */
        /*  = Virtual channel of received TLP */
        volatile unsigned int xdlh_xtlh_halt:1;        /* [Position:15] */
        /* = Layer2 is not accepting data to transmit this cycle  */
        volatile unsigned int xtlh_xdlh_data:16; /* [Position:16] */
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
        volatile unsigned int xtlh_xdlh_data:16;        /* [Position:64] */
        /*  = Transmit data at the interface between Layer3 and Layer2. If RASDP is enabled, includes the ECC or parity protection code bits. */
        volatile unsigned int xtlh_xdlh_badeot:1;        /* [Position:80] */
        /*  = Nullify this transmit TLP (invert CRC, append EDB) */
        volatile unsigned int xtlh_xdlh_eot:1;        /* [Position:81] */
        /*  = Transmit End of TLP this cycle */
        volatile unsigned int xtlh_xdlh_sot:1;        /* [Position:82] */
        /*  = Transmit Start of TLP this cycle */
        volatile unsigned int ecrc_err_asserted:1;        /* [Position:83] */
        /* = End-to-end CRC corrupted for this packet */
        volatile unsigned int lcrc_err_asserted:1;        /* [Position:84] */
        /* = Link CRC corrupted for this packet */
        volatile unsigned int xplh_xdlh_halt:1;        /* [Position:85] */
        /* = PHY Layer not accepting data this cycle */
        volatile unsigned int xdlh_xplh_data:10; /* [Position:86] */
    };
} acc_ras_st_bus_ro00_2_t;

typedef volatile union _acc_ras_st_bus_ro00_3_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Transmit packet payload (completely framed) */
        volatile unsigned int xdlh_xplh_data:32; /* [Position:96] */
    };
} acc_ras_st_bus_ro00_3_t;

typedef volatile union _acc_ras_st_bus_ro00_4_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Transmit packet payload (completely framed) */
        volatile unsigned int xdlh_xplh_data:22;        /* [Position:128] */
        /*  = Transmit packet payload (completely framed) */
        volatile unsigned int xdlh_xplh_sdp:2;        /* [Position:150] */
        /*  = Transmit Start of DLLP (per dword) */
        volatile unsigned int xdlh_xplh_stp:2;        /* [Position:152] */
        /*  = Transmit Start of TLP (per dword) */
        volatile unsigned int xdlh_xplh_eot:2;        /* [Position:154] */
        /*  = Transmit end of TLP/DLLP (per dword) */
        volatile unsigned int rdlh_xdlh_req_acknack_seqnum:4; /* [Position:156] */
    };
} acc_ras_st_bus_ro00_4_t;

typedef volatile union _acc_ras_st_bus_ro00_5_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int rdlh_xdlh_req_acknack_seqnum:8;       /* [Position:156] */
        /*  = Sequence Number for ACK/NAK DLLP */
        volatile unsigned int rdlh_xdlh_req2send_nack:1;        /* [Position:168] */
        /* = DataLink Layer request to send NAK */
        volatile unsigned int rdlh_xdlh_req2send_ack_due2dup:1;        /* [Position:169] */
        /* = Request to send ACK due to duplicate TLP */
        volatile unsigned int rdlh_xdlh_req2send_ack:1;        /* [Position:170] */
        /* = DataLink Layer request to send ACK */
        volatile unsigned int rdlh_xdlh_rcvd_acknack_seqnum:12;        /* [Position:171] */
        /*  = Sequence number corresponding to NAK/ACK */
        volatile unsigned int rdlh_xdlh_rcvd_ack:1;        /* [Position:183] */
        /* = DataLink Layer received ACK DLLP */
        volatile unsigned int rdlh_xdlh_rcvd_nack:1;        /* [Position:184] */
        /* = DataLink Layer received NAK DLLP */
        volatile unsigned int cfg_link_retrain:1;        /* [Position:185] */
        /* = Software programmed link retrain request */
        volatile unsigned int rtlh_req_link_retrain:1;        /* [Position:186] */
        /* = Receive watchdog timer expired, retrain link */
        volatile unsigned int xdlh_smlh_start_link_retrain:1;        /* [Position:187] */
        /* = vMax retries attempted, request to retrain link */
        volatile unsigned int rdlh_rtlh_tlp_dv:1;        /* [Position:188] */
        /* = Receive data interface from DLL to Transaction Layer valid this cycle (rdlh_rtlh_*) */
        volatile unsigned int rdlh_rtlh_tlp_eot:2;        /* [Position:189] */
        /*  = End of TLP (per dword) */
        volatile unsigned int rdlh_rtlh_tlp_sot:1; /* [Position:191] */
    };
} acc_ras_st_bus_ro00_5_t;

typedef volatile union _acc_ras_st_bus_ro00_6_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int rdlh_rtlh_tlp_sot:1;       /* [Position:191] */
        /*  = Start of TLP (per dword)  */
        volatile unsigned int rplh_rdlh_pkt_data:31; /* [Position:193] */
    };
} acc_ras_st_bus_ro00_6_t;

typedef volatile union _acc_ras_st_bus_ro00_7_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Received packet payload */
        volatile unsigned int rplh_rdlh_pkt_data:32; /* [Position:224] */
    };
} acc_ras_st_bus_ro00_7_t;

typedef volatile union _acc_ras_st_bus_ro00_8_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Received packet payload */
        volatile unsigned int rplh_rdlh_pkt_data:1;        /* [Position:256] */
        /*  = Received packet payload */
        volatile unsigned int rplh_rdlh_pkt_err:2;        /* [Position:257] */
        /*  = Physical Error detected (per dword) */
        volatile unsigned int rplh_rdlh_pkt_dv:1;        /* [Position:259] */
        /* = Receive data interface from PHY to DLL valid this cycle (rplh_rdlh_*) */
        volatile unsigned int rplh_rdlh_pkt_edb:2;        /* [Position:260] */
        /*  = Packet terminated with EDB (per dword) */
        volatile unsigned int rplh_rdlh_pkt_end:2;        /* [Position:262] */
        /*  = DLLP/TLP ending (per dword) */
        volatile unsigned int rplh_rdlh_tlp_start:2;        /* [Position:264] */
        /*  = DLLP starting (per dword) */
        volatile unsigned int rplh_rdlh_dllp_start:2;        /* [Position:266] */
        /*  = MAC layer detected runt STP (per dword) */
        volatile unsigned int rplh_rdlh_nak:2;        /* [Position:268] */
        /*  = MAC layer detected runt STP (per dword) */
        volatile unsigned int rmlh_lanes_rcving:4;        /* [Position:270] */
        /*  = lanes active in link training */
        volatile unsigned int rmlh_rcvd_eidle_set:1;        /* [Position:274] */
        /* = Received EIDLE ordered set, any active lane */
        volatile unsigned int rplh_rcvd_idle1:1;        /* [Position:275] */
        /* = Logical Idle seen for 1+ symbols on all active lanes */
        volatile unsigned int rplh_rcvd_idle0:1;        /* [Position:276] */
        /* = Logical Idle seen for 8+ symbols on all active lanes */
        volatile unsigned int smlh_rcvd_lane_rev:1;        /* [Position:277] */
        /* = Receive logic detected logical lane reversal */
        volatile unsigned int rmlh_ts_link_num_is_k237:1;        /* [Position:278] */
        /* = Received Link number (lane 0) is PAD (k237) */
        volatile unsigned int rmlh_deskew_alignment_err:1;        /* [Position:279] */
        /* = Deskew logic overflow. Unable to align lanes [level] */
        volatile unsigned int rmlh_ts_lane_num_is_k237:1;        /* [Position:280] */
        /* = Received lane number (lane 0) is PAD (k237) */
        volatile unsigned int rmlh_ts2_rcvd:1;        /* [Position:281] */
        /* = At least one active lane received TS2 this cycle */
        volatile unsigned int rmlh_ts1_rcvd:1;        /* [Position:282] */
        /* = At least one active lane received TS1 this cycle */
        volatile unsigned int rmlh_ts_rcv_err:1;        /* [Position:283] */
        /* = Assert when the received data is not an expected */
        volatile unsigned int rmlh_inskip_rcv:1;        /* [Position:284] */
        /* = Skip character is received this cyclePM Diagnostic Bus */
        volatile unsigned int xadm_no_fc_credit:3; /* [Position:285] */
    };
} acc_ras_st_bus_ro00_8_t;

typedef volatile union _acc_ras_st_bus_ro00_9_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int xadm_no_fc_credit:1;       /* [Position:285] */
        /*  = No credits of any type are available (per VC). */
        volatile unsigned int xadm_tlp_pending:1;        /* [Position:289] */
        /* = No TLP transmit requests currently pending (from internal or external clients). */
        volatile unsigned int xadm_had_enough_credit:4;        /* [Position:290] */
        /*  = The controller has enough transmit credits of each type (per VC) to meet the PM entry criteria. */
        volatile unsigned int xdlh_not_expecting_ack:1;        /* [Position:294] */
        /* = All transmitted TLPs have been acknowledged by the link partner. */
        volatile unsigned int xdlh_xmt_pme_ack:1;        /* [Position:295] */
        /* = DataLink layer just transmitted a PME_TO_ACK msg. */
        volatile unsigned int xdlh_nodllp_pending:1;        /* [Position:296] */
        /* = There are no pending or in-progress packets going to the PHY. */
        volatile unsigned int l1sub_state:3;        /* [Position:297] */
        /*  = L1 FSM sub-state (Tied to 3'b000 when !CX_L1_SUBSTATES_ENABLE) */
        volatile unsigned int unexpected_cpl_err:1;        /* [Position:300] */
        /* = Received a completion that was unexpected */
        volatile unsigned int cpl_ca_err:1;        /* [Position:301] */
        /* = Received completion with CA status */
        volatile unsigned int cpl_ur_err:1;        /* [Position:302] */
        /* = Received completion with UR status */
        volatile unsigned int flt_q_cpl_last:1;        /* [Position:303] */
        /* = Final completion for the transaction */
        volatile unsigned int flt_q_cpl_abort:1;        /* [Position:304] */
        /* = Current completion is being aborted (trashed) */
        volatile unsigned int cpl_mlf_err:1;        /* [Position:305] */
        /* = Malformed completion */
        volatile unsigned int flt_q_header_cpl_status:3;        /* [Position:306] */
        /*  = Indicates the filtering result for the current RX request or the completion status for the current RX received completion. */
        volatile unsigned int flt_q_header_destination:2;        /* [Position:309] */
        /*  = Destination interface. */
        volatile unsigned int form_filt_ecrc_err:1;        /* [Position:311] */
        /* = This TLP had an ECRC error */
        volatile unsigned int form_filt_malform_tlp_err:1;        /* [Position:312] */
        /* = This TLP is malformed */
        volatile unsigned int form_filt_dllp_err:1;        /* [Position:313] */
        /* = This TLP has a DataLink Layer Error (for example, LCRC) */
        volatile unsigned int form_filt_eot:1;        /* [Position:314] */
        /* = End of TLP received this cycle */
        volatile unsigned int form_filt_dwen:2;        /* [Position:315] */
        /*  = Dword enables */
        volatile unsigned int form_filt_data:3; /* [Position:317] */
    };
} acc_ras_st_bus_ro00_9_t;

typedef volatile union _acc_ras_st_bus_ro00_10_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Packet data from receive Transaction Layer */
        volatile unsigned int form_filt_data:32; /* [Position:320] */
    };
} acc_ras_st_bus_ro00_10_t;

typedef volatile union _acc_ras_st_bus_ro00_11_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Packet data from receive Transaction Layer */
        volatile unsigned int form_filt_data:29;        /* [Position:352] */
        /*  = Packet data from receive Transaction Layer */
        volatile unsigned int form_filt_dv:1;        /* [Position:381] */
        /* = Packet is in payload stage */
        volatile unsigned int form_filt_hdr:2; /* [Position:382] */
    };
} acc_ras_st_bus_ro00_11_t;

typedef volatile union _acc_ras_st_bus_ro00_12_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Header data (size of stored header is configurable) */
        volatile unsigned int form_filt_hdr:32; /* [Position:384] */
    };
} acc_ras_st_bus_ro00_12_t;

typedef volatile union _acc_ras_st_bus_ro00_13_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Header data (size of stored header is configurable) */
        volatile unsigned int form_filt_hdr:32; /* [Position:416] */
    };
} acc_ras_st_bus_ro00_13_t;

typedef volatile union _acc_ras_st_bus_ro00_14_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Header data (size of stored header is configurable) */
        volatile unsigned int form_filt_hdr:32; /* [Position:448] */
    };
} acc_ras_st_bus_ro00_14_t;

typedef volatile union _acc_ras_st_bus_ro00_15_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Header data (size of stored header is configurable) */
        volatile unsigned int form_filt_hdr:30;        /* [Position:480] */
        /*  = Header data (size of stored header is configurable) */
        volatile unsigned int form_filt_hv:1;        /* [Position:510] */
        /* = Information to the receive packet filter block is valid */
        volatile unsigned int active_grant:1; /* [Position:511] */
    };
} acc_ras_st_bus_ro00_15_t;

typedef volatile union _acc_ras_st_bus_ro00_16_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int active_grant:4;       /* [Position:511] */
        /*  = Currently granted client (1-hot) */
        volatile unsigned int grant_ack:5;        /* [Position:516] */
        /*  = This client has completed his request (1-hot) */
        volatile unsigned int fc_cds_pass:20;        /* [Position:521] */
        /*  = Credit check passed for each request */
        volatile unsigned int arb_reqs:3; /* [Position:541] */
    };
} acc_ras_st_bus_ro00_16_t;

typedef volatile union _acc_ras_st_bus_ro00_17_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int arb_reqs:2;       /* [Position:541] */
        /*  = Transmit requests from each client  */
        volatile unsigned int cfg_pbus_num:8;        /* [Position:546] */
        /*  = Bus number; 8 bits per function when MULTI_DEVICE_AND_BUS_PER_FUNC_EN=1, else 8 bits in total. */
        volatile unsigned int cfg_pbus_dev_num:5;        /* [Position:554] */
        /*  = Device number; 5 bits per function when MULTI_DEVICE_AND_BUS_PER_FUNC_EN=1, else 5 bits in total. */
        volatile unsigned int xdlh_replay_timeout_err:1;        /* [Position:559] */
        /* = Retry timer expired [pulse] */
        volatile unsigned int xdlh_replay_num_rlover_err:1;        /* [Position:560] */
        /* = Max retries exceeded [pulse] */
        volatile unsigned int rdlh_bad_dllp_err:1;        /* [Position:561] */
        /* = Received DLLP with DataLink Layer error [pulse] */
        volatile unsigned int rdlh_bad_tlp_err:1;        /* [Position:562] */
        /* = Received TLP with DataLink Layer error [pulse] */
        volatile unsigned int rdlh_prot_err:1;        /* [Position:563] */
        /* = DLLP protocol error (out of sequence DLLP) [pulse] */
        volatile unsigned int rtlh_fc_prot_err:1;        /* [Position:564] */
        /* = Flow control protocol violation (watchdog timer) [pulse] */
        volatile unsigned int rmlh_rcvd_err:1;        /* [Position:565] */
        /* = Received PHY error this cycle [pulse] */
        volatile unsigned int int_xadm_fc_prot_err:1;        /* [Position:566] */
        /* = Flow control update protocol violation (opt.checks) */
        volatile unsigned int radm_cpl_timeout:1;        /* [Position:567] */
        /* = An request failed to complete in the allotted time */
        volatile unsigned int radm_qoverflow:4;        /* [Position:568] */
        /*  = Receive queue overflow. Normally happens only when flow control advertisements are ignored */
        volatile unsigned int radm_unexp_cpl_err:4; /* [Position:572] */
    };
} acc_ras_st_bus_ro00_17_t;

typedef volatile union _acc_ras_st_bus_ro00_18_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Received an unexpected completion [pulse] */
        volatile unsigned int radm_rcvd_cpl_ur:4;        /* [Position:576] */
        /*  = Received a completion with UR status */
        volatile unsigned int radm_rcvd_cpl_ca:4;        /* [Position:580] */
        /*  = Received a completion with CA status */
        volatile unsigned int radm_rcvd_req_ca:4;        /* [Position:584] */
        /*  = Completer aborted request */
        volatile unsigned int radm_rcvd_req_ur:4;        /* [Position:588] */
        /*  = Received a request which device does not support */
        volatile unsigned int radm_ecrc_err:4;        /* [Position:592] */
        /*  = Received a TLP with ECRC error */
        volatile unsigned int radm_mlf_tlp_err:4;        /* [Position:596] */
        /*  = Received a malformed TLP [pulse] */
        volatile unsigned int radm_rcvd_cpl_poisoned:4;        /* [Position:600] */
        /*  = Received a completion with poisoned payload */
        volatile unsigned int radm_rcvd_wreq_poisoned:4; /* [Position:604] */
    };
} acc_ras_st_bus_ro00_18_t;

typedef volatile union _acc_ras_st_bus_ro00_19_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        /*  = Received a write with poisoned payload */
        volatile unsigned int cdm_lbc_ack:4;        /* [Position:608] */
        /*  = Local bus Acknowledge */
        volatile unsigned int lbc_cdm_wr:4;        /* [Position:612] */
        /*  = Local bus Write enable (per byte) */
        volatile unsigned int lbc_cdm_cs:4;        /* [Position:616] */
        /*  = Local bus chip select */
        volatile unsigned int lbc_cdm_data:20; /* [Position:620] */
    };
} acc_ras_st_bus_ro00_19_t;

typedef volatile union _acc_ras_st_bus_ro00_20_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int lbc_cdm_data:12;       /* [Position:620] */
        /*  = Local bus Data */
        volatile unsigned int lbc_cdm_addr:20; /* [Position:652] */
    };
} acc_ras_st_bus_ro00_20_t;

typedef volatile union _acc_ras_st_bus_ro00_21_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int lbc_cdm_addr:12;       /* [Position:652] */
        /*  = Local bus Address */
        volatile unsigned int cfg_sys_err_rc_cor:1;        /* [Position:684] */
        /* = System Error caused by Correctable  */
        volatile unsigned int cfg_sys_err_rc_nf:1;        /* [Position:685] */
        /* = System Error caused by Non-Fatal */
        volatile unsigned int cfg_sys_err_rc_f:1;        /* [Position:686] */
        /* = System Error caused by Fatal */
        volatile unsigned int radm_ide_pcrc_err:1;        /* [Position:687] */
        /* = Received a TLP with PCRC error (Tied 0 if PCIe IDE is not enabled) */
        volatile unsigned int radm_ide_misrouted:1;        /* [Position:688] */
        /* = Received an IDE Misrouted TLP (Tied 0 if PCIe IDE is not enabled) */
        volatile unsigned int radm_ide_check_failed:1;        /* [Position:689] */
        /* = Received an IDE TLP with an IDE Check */
    };
} acc_ras_st_bus_ro00_21_t;
/* offset : 0x45c */
typedef volatile union _acc_ras_ec_com_ro00_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int bit0:1;        /* [Position:0] */
        /* : Reserved (fixed "0") */
        volatile unsigned int bit1:1;        /* [Position:1] */
        /* : Reserved (fixed "0") */
        volatile unsigned int bit2:1;        /* [Position:2] */
        /* : Reserved (fixed "0") */
        volatile unsigned int bit3:1;        /* [Position:3] */
        /* : Reserved (fixed "0") */
        volatile unsigned int bit4:1;        /* [Position:4] */
        /* : Reserved (fixed "0") */
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
        volatile unsigned int rdlh_rcvd_nack_perdllp:5;       /* [Position:29] */
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
        /* : pm_in_l11: Level: L1.1 Entry */
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
        volatile unsigned int rdlh_duplicate_tlp_err_pertlp:3;       /* [Position:95] */
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
        volatile unsigned int rtlh_rx_memrd_evt:1;       /* [Position:125] */
        /* : rtlh_rx_memrd_evt: Pulse: Rx Memory Read */
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
        volatile unsigned int rtlh_rx_atmcop_evt:1;       /* [Position:157] */
        /* : rtlh_rx_atmcop_evt: Pulse: Rx Atomic */
        volatile unsigned int rtlh_rx_tlpwprefix_evt:4;        /* [Position:161] */
        /* : rtlh_rx_tlpwprefix_evt: Pulse: Rx TLP with Prefix */
        volatile unsigned int xtlh_tx_ccix_tlp_evt:2;        /* [Position:165] */
        /* : xtlh_tx_ccix_tlp_evt: Pulse: Tx CCIX TLP */
        volatile unsigned int rtlh_rx_ccix_tlp_evt:4;        /* [Position:167] */
        /* : rtlh_rx_ccix_tlp_evt: Pulse: Rx CCIX TLP */
        volatile unsigned int xtlh_tx_defmemwr_evt:2;        /* [Position:171] */
        /* : xtlh_tx_defmemwr_evt: Pulse: Tx Deferrable Memory Write */
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
        /* : l0_|rmlh_deskew_ctlskp_err[i*5+4:i*5+3]: Pulse: Margin CRC and Parity Error */
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
        /* : l1_|rmlh_deskew_ctlskp_err[i*5+4:i*5+3]: Pulse: Margin CRC and Parity Error */
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
        /* : l2_|rmlh_deskew_ctlskp_err[i*5+4:i*5+3]: Pulse: Margin CRC and Parity Error */
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
        /* : l3_|rmlh_deskew_ctlskp_err[i*5+4:i*5+3]: Pulse: Margin CRC and Parity Error */
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
        /* : pm_master_state[4:0]: Level: PM Internal State (Master) */
        volatile unsigned int pm_slave_state_3_:4;        /* [Position:5] */
        /* : pm_slave_state[3:0]: Level: PM Internal State (Slave) */
        volatile unsigned int rmlh_framing_err_ptr_6_:7;        /* [Position:9] */
        /* : rmlh_framing_err_ptr[6:0]: Pulse: 1st Framing Error Pointer */
        volatile unsigned int smlh_lane_reversed:1;        /* [Position:16] */
        /* : smlh_lane_reversed: Level: Lane Reversal Operation */
        volatile unsigned int pm_pme_resend_flag:1;        /* [Position:17] */
        /* : pm_pme_resend_flag: Pulse: PME Re-Send flag */
        volatile unsigned int bit33:14; /* [Position:18] */
    };
} acc_ras_sd_com_ro00_0_t;

typedef volatile union _acc_ras_sd_com_ro00_1_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int bit33:2;       /* [Position:18] */
        /* : smlh_ltssm_variable [15:0]: Level: LTSSM Variable */
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
        volatile unsigned int bit72:3; /* [Position:61] */
    };
} acc_ras_sd_com_ro00_1_t;

typedef volatile union _acc_ras_sd_com_ro00_2_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int bit72:9;       /* [Position:61] */
        /* : xdlh_curnt_seqnum [11:0]: Level: Tx TLP SEQ# */
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
        volatile unsigned int mac_cdm_ras_des_coef_rtx_11_:5;       /* [Position:31] */
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
        volatile unsigned int Reserved_126:1;       /* [Position:126] */
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
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g5_17_1:1;       /* [Position:155] */
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
        volatile unsigned int phy_cdm_ras_des_fomfeedback_g5:7;       /* [Position:191] */
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
    };
} acc_ras_sd_lxx_ro00_6_t;
/* offset : 0x4a4 */
typedef volatile union _acc_ras_sd_lxx_ro07_0_t
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
} acc_ras_sd_lxx_ro07_0_t;

typedef volatile union _acc_ras_sd_lxx_ro07_1_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_11_:5;       /* [Position:31] */
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
} acc_ras_sd_lxx_ro07_1_t;

typedef volatile union _acc_ras_sd_lxx_ro07_2_t
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
} acc_ras_sd_lxx_ro07_2_t;

typedef volatile union _acc_ras_sd_lxx_ro07_3_t
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
} acc_ras_sd_lxx_ro07_3_t;

typedef volatile union _acc_ras_sd_lxx_ro07_4_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int Reserved_126:1;       /* [Position:126] */
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
} acc_ras_sd_lxx_ro07_4_t;

typedef volatile union _acc_ras_sd_lxx_ro07_5_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g5_17_1:1;       /* [Position:155] */
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
} acc_ras_sd_lxx_ro07_5_t;

typedef volatile union _acc_ras_sd_lxx_ro07_6_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int phy_cdm_ras_des_fomfeedback_g5:7;       /* [Position:191] */
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
    };
} acc_ras_sd_lxx_ro07_6_t;
/* offset : 0x4c0 */
typedef volatile union _acc_ras_sd_lxx_ro14_0_t
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
} acc_ras_sd_lxx_ro14_0_t;

typedef volatile union _acc_ras_sd_lxx_ro14_1_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_11_:5;       /* [Position:31] */
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
} acc_ras_sd_lxx_ro14_1_t;

typedef volatile union _acc_ras_sd_lxx_ro14_2_t
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
} acc_ras_sd_lxx_ro14_2_t;

typedef volatile union _acc_ras_sd_lxx_ro14_3_t
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
} acc_ras_sd_lxx_ro14_3_t;

typedef volatile union _acc_ras_sd_lxx_ro14_4_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int Reserved_126:1;       /* [Position:126] */
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
} acc_ras_sd_lxx_ro14_4_t;

typedef volatile union _acc_ras_sd_lxx_ro14_5_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g5_17_1:1;       /* [Position:155] */
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
} acc_ras_sd_lxx_ro14_5_t;

typedef volatile union _acc_ras_sd_lxx_ro14_6_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int phy_cdm_ras_des_fomfeedback_g5:7;       /* [Position:191] */
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
    };
} acc_ras_sd_lxx_ro14_6_t;
/* offset : 0x4dc */
typedef volatile union _acc_ras_sd_lxx_ro21_0_t
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
} acc_ras_sd_lxx_ro21_0_t;

typedef volatile union _acc_ras_sd_lxx_ro21_1_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_11_:5;       /* [Position:31] */
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
} acc_ras_sd_lxx_ro21_1_t;

typedef volatile union _acc_ras_sd_lxx_ro21_2_t
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
} acc_ras_sd_lxx_ro21_2_t;

typedef volatile union _acc_ras_sd_lxx_ro21_3_t
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
} acc_ras_sd_lxx_ro21_3_t;

typedef volatile union _acc_ras_sd_lxx_ro21_4_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int Reserved_126:1;       /* [Position:126] */
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
} acc_ras_sd_lxx_ro21_4_t;

typedef volatile union _acc_ras_sd_lxx_ro21_5_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int mac_cdm_ras_des_coef_rtx_g5_17_1:1;       /* [Position:155] */
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
} acc_ras_sd_lxx_ro21_5_t;

typedef volatile union _acc_ras_sd_lxx_ro21_6_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int phy_cdm_ras_des_fomfeedback_g5:7;       /* [Position:191] */
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
    };
} acc_ras_sd_lxx_ro21_6_t;
/* offset : 0x4f8 */
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
        volatile unsigned int xadm_fc_cnsmd_npd:8;       /* [Position:28] */
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
        volatile unsigned int xadm_fc_limit_ph:4;       /* [Position:60] */
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
        volatile unsigned int xadm_fc_limit_npd:4;       /* [Position:88] */
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
        volatile unsigned int rtlh_fc_allctd_pd:8;       /* [Position:188] */
        /* : rtlh_fc_allctd_pd: Level: Credit Allocated (PD) VCn */
        volatile unsigned int rtlh_fc_allctd_nph:8;        /* [Position:200] */
        /* : rtlh_fc_allctd_nph: Level: Credit Allocated (NH) VCn */
        volatile unsigned int rtlh_fc_allctd_npd:12;        /* [Position:208] */
        /* : rtlh_fc_allctd_npd: Level: Credit Allocated (ND) VCn */
        volatile unsigned int rtlh_fc_allctd_cplh:4; /* [Position:220] */
    };
} acc_ras_sd_vxx_ro00_6_t;

typedef volatile union _acc_ras_sd_vxx_ro00_7_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int rtlh_fc_allctd_cplh:4;       /* [Position:220] */
        /* : rtlh_fc_allctd_cplh: Level: Credit Allocated (CH) VCn */
        volatile unsigned int rtlh_fc_allctd_cpld:12;        /* [Position:228] */
        /* : rtlh_fc_allctd_cpld: Level: Credit Allocated (CD) VCn */
    };
} acc_ras_sd_vxx_ro00_7_t;
/* offset : 0x518 */
typedef volatile union _acc_ras_sd_vxx_ro08_0_t
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
} acc_ras_sd_vxx_ro08_0_t;

typedef volatile union _acc_ras_sd_vxx_ro08_1_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int xadm_fc_cnsmd_npd:8;       /* [Position:28] */
        /* : xadm_fc_cnsmd_npd: Level: Credit Consumed (ND) VCn */
        volatile unsigned int xadm_fc_cnsmd_cplh:8;        /* [Position:40] */
        /* : xadm_fc_cnsmd_cplh: Level: Credit Consumed (CH) VCn */
        volatile unsigned int xadm_fc_cnsmd_cpld:12;        /* [Position:48] */
        /* : xadm_fc_cnsmd_cpld: Level: Credit Consumed (CD) VCn */
        volatile unsigned int xadm_fc_limit_ph:4; /* [Position:60] */
    };
} acc_ras_sd_vxx_ro08_1_t;

typedef volatile union _acc_ras_sd_vxx_ro08_2_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int xadm_fc_limit_ph:4;       /* [Position:60] */
        /* : xadm_fc_limit_ph: Level: Credit Limit (PH) VCn */
        volatile unsigned int xadm_fc_limit_pd:12;        /* [Position:68] */
        /* : xadm_fc_limit_pd: Level: Credit Limit (PD) VCn */
        volatile unsigned int xadm_fc_limit_nph:8;        /* [Position:80] */
        /* : xadm_fc_limit_nph: Level: Credit Limit (NH) VCn */
        volatile unsigned int xadm_fc_limit_npd:8; /* [Position:88] */
    };
} acc_ras_sd_vxx_ro08_2_t;

typedef volatile union _acc_ras_sd_vxx_ro08_3_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int xadm_fc_limit_npd:4;       /* [Position:88] */
        /* : xadm_fc_limit_npd: Level: Credit Limit (ND) VCn */
        volatile unsigned int xadm_fc_limit_cplh:8;        /* [Position:100] */
        /* : xadm_fc_limit_cplh: Level: Credit Limit (CH) VCn */
        volatile unsigned int xadm_fc_limit_cpld:12;        /* [Position:108] */
        /* : xadm_fc_limit_cpld: Level: Credit Limit (CD) VCn */
        volatile unsigned int rtlh_fc_rcvd_ph:8; /* [Position:120] */
    };
} acc_ras_sd_vxx_ro08_3_t;

typedef volatile union _acc_ras_sd_vxx_ro08_4_t
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
} acc_ras_sd_vxx_ro08_4_t;

typedef volatile union _acc_ras_sd_vxx_ro08_5_t
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
} acc_ras_sd_vxx_ro08_5_t;

typedef volatile union _acc_ras_sd_vxx_ro08_6_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int rtlh_fc_allctd_pd:8;       /* [Position:188] */
        /* : rtlh_fc_allctd_pd: Level: Credit Allocated (PD) VCn */
        volatile unsigned int rtlh_fc_allctd_nph:8;        /* [Position:200] */
        /* : rtlh_fc_allctd_nph: Level: Credit Allocated (NH) VCn */
        volatile unsigned int rtlh_fc_allctd_npd:12;        /* [Position:208] */
        /* : rtlh_fc_allctd_npd: Level: Credit Allocated (ND) VCn */
        volatile unsigned int rtlh_fc_allctd_cplh:4; /* [Position:220] */
    };
} acc_ras_sd_vxx_ro08_6_t;

typedef volatile union _acc_ras_sd_vxx_ro08_7_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int rtlh_fc_allctd_cplh:4;       /* [Position:220] */
        /* : rtlh_fc_allctd_cplh: Level: Credit Allocated (CH) VCn */
        volatile unsigned int rtlh_fc_allctd_cpld:12;        /* [Position:228] */
        /* : rtlh_fc_allctd_cpld: Level: Credit Allocated (CD) VCn */
    };
} acc_ras_sd_vxx_ro08_7_t;
/* offset : 0x538 */
typedef volatile union _acc_ras_sd_vxx_ro16_0_t
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
} acc_ras_sd_vxx_ro16_0_t;

typedef volatile union _acc_ras_sd_vxx_ro16_1_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int xadm_fc_cnsmd_npd:8;       /* [Position:28] */
        /* : xadm_fc_cnsmd_npd: Level: Credit Consumed (ND) VCn */
        volatile unsigned int xadm_fc_cnsmd_cplh:8;        /* [Position:40] */
        /* : xadm_fc_cnsmd_cplh: Level: Credit Consumed (CH) VCn */
        volatile unsigned int xadm_fc_cnsmd_cpld:12;        /* [Position:48] */
        /* : xadm_fc_cnsmd_cpld: Level: Credit Consumed (CD) VCn */
        volatile unsigned int xadm_fc_limit_ph:4; /* [Position:60] */
    };
} acc_ras_sd_vxx_ro16_1_t;

typedef volatile union _acc_ras_sd_vxx_ro16_2_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int xadm_fc_limit_ph:4;       /* [Position:60] */
        /* : xadm_fc_limit_ph: Level: Credit Limit (PH) VCn */
        volatile unsigned int xadm_fc_limit_pd:12;        /* [Position:68] */
        /* : xadm_fc_limit_pd: Level: Credit Limit (PD) VCn */
        volatile unsigned int xadm_fc_limit_nph:8;        /* [Position:80] */
        /* : xadm_fc_limit_nph: Level: Credit Limit (NH) VCn */
        volatile unsigned int xadm_fc_limit_npd:8; /* [Position:88] */
    };
} acc_ras_sd_vxx_ro16_2_t;

typedef volatile union _acc_ras_sd_vxx_ro16_3_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int xadm_fc_limit_npd:4;       /* [Position:88] */
        /* : xadm_fc_limit_npd: Level: Credit Limit (ND) VCn */
        volatile unsigned int xadm_fc_limit_cplh:8;        /* [Position:100] */
        /* : xadm_fc_limit_cplh: Level: Credit Limit (CH) VCn */
        volatile unsigned int xadm_fc_limit_cpld:12;        /* [Position:108] */
        /* : xadm_fc_limit_cpld: Level: Credit Limit (CD) VCn */
        volatile unsigned int rtlh_fc_rcvd_ph:8; /* [Position:120] */
    };
} acc_ras_sd_vxx_ro16_3_t;

typedef volatile union _acc_ras_sd_vxx_ro16_4_t
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
} acc_ras_sd_vxx_ro16_4_t;

typedef volatile union _acc_ras_sd_vxx_ro16_5_t
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
} acc_ras_sd_vxx_ro16_5_t;

typedef volatile union _acc_ras_sd_vxx_ro16_6_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int rtlh_fc_allctd_pd:8;       /* [Position:188] */
        /* : rtlh_fc_allctd_pd: Level: Credit Allocated (PD) VCn */
        volatile unsigned int rtlh_fc_allctd_nph:8;        /* [Position:200] */
        /* : rtlh_fc_allctd_nph: Level: Credit Allocated (NH) VCn */
        volatile unsigned int rtlh_fc_allctd_npd:12;        /* [Position:208] */
        /* : rtlh_fc_allctd_npd: Level: Credit Allocated (ND) VCn */
        volatile unsigned int rtlh_fc_allctd_cplh:4; /* [Position:220] */
    };
} acc_ras_sd_vxx_ro16_6_t;

typedef volatile union _acc_ras_sd_vxx_ro16_7_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int rtlh_fc_allctd_cplh:4;       /* [Position:220] */
        /* : rtlh_fc_allctd_cplh: Level: Credit Allocated (CH) VCn */
        volatile unsigned int rtlh_fc_allctd_cpld:12;        /* [Position:228] */
        /* : rtlh_fc_allctd_cpld: Level: Credit Allocated (CD) VCn */
    };
} acc_ras_sd_vxx_ro16_7_t;
/* offset : 0x558 */
typedef volatile union _acc_ras_sd_vxx_ro24_0_t
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
} acc_ras_sd_vxx_ro24_0_t;

typedef volatile union _acc_ras_sd_vxx_ro24_1_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int xadm_fc_cnsmd_npd:8;       /* [Position:28] */
        /* : xadm_fc_cnsmd_npd: Level: Credit Consumed (ND) VCn */
        volatile unsigned int xadm_fc_cnsmd_cplh:8;        /* [Position:40] */
        /* : xadm_fc_cnsmd_cplh: Level: Credit Consumed (CH) VCn */
        volatile unsigned int xadm_fc_cnsmd_cpld:12;        /* [Position:48] */
        /* : xadm_fc_cnsmd_cpld: Level: Credit Consumed (CD) VCn */
        volatile unsigned int xadm_fc_limit_ph:4; /* [Position:60] */
    };
} acc_ras_sd_vxx_ro24_1_t;

typedef volatile union _acc_ras_sd_vxx_ro24_2_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int xadm_fc_limit_ph:4;       /* [Position:60] */
        /* : xadm_fc_limit_ph: Level: Credit Limit (PH) VCn */
        volatile unsigned int xadm_fc_limit_pd:12;        /* [Position:68] */
        /* : xadm_fc_limit_pd: Level: Credit Limit (PD) VCn */
        volatile unsigned int xadm_fc_limit_nph:8;        /* [Position:80] */
        /* : xadm_fc_limit_nph: Level: Credit Limit (NH) VCn */
        volatile unsigned int xadm_fc_limit_npd:8; /* [Position:88] */
    };
} acc_ras_sd_vxx_ro24_2_t;

typedef volatile union _acc_ras_sd_vxx_ro24_3_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int xadm_fc_limit_npd:4;       /* [Position:88] */
        /* : xadm_fc_limit_npd: Level: Credit Limit (ND) VCn */
        volatile unsigned int xadm_fc_limit_cplh:8;        /* [Position:100] */
        /* : xadm_fc_limit_cplh: Level: Credit Limit (CH) VCn */
        volatile unsigned int xadm_fc_limit_cpld:12;        /* [Position:108] */
        /* : xadm_fc_limit_cpld: Level: Credit Limit (CD) VCn */
        volatile unsigned int rtlh_fc_rcvd_ph:8; /* [Position:120] */
    };
} acc_ras_sd_vxx_ro24_3_t;

typedef volatile union _acc_ras_sd_vxx_ro24_4_t
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
} acc_ras_sd_vxx_ro24_4_t;

typedef volatile union _acc_ras_sd_vxx_ro24_5_t
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
} acc_ras_sd_vxx_ro24_5_t;

typedef volatile union _acc_ras_sd_vxx_ro24_6_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int rtlh_fc_allctd_pd:8;       /* [Position:188] */
        /* : rtlh_fc_allctd_pd: Level: Credit Allocated (PD) VCn */
        volatile unsigned int rtlh_fc_allctd_nph:8;        /* [Position:200] */
        /* : rtlh_fc_allctd_nph: Level: Credit Allocated (NH) VCn */
        volatile unsigned int rtlh_fc_allctd_npd:12;        /* [Position:208] */
        /* : rtlh_fc_allctd_npd: Level: Credit Allocated (ND) VCn */
        volatile unsigned int rtlh_fc_allctd_cplh:4; /* [Position:220] */
    };
} acc_ras_sd_vxx_ro24_6_t;

typedef volatile union _acc_ras_sd_vxx_ro24_7_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int rtlh_fc_allctd_cplh:4;       /* [Position:220] */
        /* : rtlh_fc_allctd_cplh: Level: Credit Allocated (CH) VCn */
        volatile unsigned int rtlh_fc_allctd_cpld:12;        /* [Position:228] */
        /* : rtlh_fc_allctd_cpld: Level: Credit Allocated (CD) VCn */
    };
} acc_ras_sd_vxx_ro24_7_t;
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
        volatile unsigned int rbar_ctrl_update:4; /* [Position:0] */
        /* Default Value : , TYPE : RO */
        /* Description :
        Indicates that a resizable BAR control register has been updated: 1 bit per Physical function.
        */
        volatile unsigned int RESERVED_4:28; /* [Position:4] */
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
        volatile unsigned int cfg_rbar_size:32; /* [Position:0] */
        /* Default Value : , TYPE : RO */
        /* Description :
        BAR size field from each of the resizable BAR control registers, per function. For BARs that are not resizable the corresponding bits in cfg_rbar_size are set to 0.
        */
    };
} acc_rbar_ro00_0_t;
/* offset : 0x584 */
typedef volatile union _acc_rbar_ro01_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_rbar_size:32; /* [Position:0] */
        /* Default Value : , TYPE : RO */
        /* Description :
        BAR size field from each of the resizable BAR control registers, per function. For BARs that are not resizable the corresponding bits in cfg_rbar_size are set to 0.
        */
    };
} acc_rbar_ro01_0_t;
/* offset : 0x588 */
typedef volatile union _acc_rbar_ro02_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_rbar_size:32; /* [Position:0] */
        /* Default Value : , TYPE : RO */
        /* Description :
        BAR size field from each of the resizable BAR control registers, per function. For BARs that are not resizable the corresponding bits in cfg_rbar_size are set to 0.
        */
    };
} acc_rbar_ro02_0_t;
/* offset : 0x58c */
typedef volatile union _acc_rbar_ro04_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_rbar_size:16; /* [Position:0] */
        /* Default Value : , TYPE : RO */
        /* Description :
        BAR size field from each of the resizable BAR control registers, per function. For BARs that are not resizable the corresponding bits in cfg_rbar_size are set to 0.
        */
        volatile unsigned int RESERVED_16:16; /* [Position:16] */
        /* Default Value : x, TYPE : U */
        /* Description :
        BAR size field from each of the resizable BAR control registers, per function. For BARs that are not resizable the corresponding bits in cfg_rbar_size are set to 0.
        */
    };
} acc_rbar_ro04_0_t;
/* offset : 0x590 */
typedef volatile union _acc_rbar_vf_ctl_ro00_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int vf_rbar_ctrl_update:4; /* [Position:0] */
        /* Default Value : , TYPE : RO */
        /* Description :
        Indicates that a resizable VF BAR control register has been updated: 1 bit per Physical function.
        */
        volatile unsigned int RESERVED_4:28; /* [Position:4] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_rbar_vf_ctl_ro00_0_t;
/* offset : 0x594 */
typedef volatile union _acc_rbar_vf_ro00_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_vf_rbar_size:32; /* [Position:0] */
        /* Default Value : , TYPE : RO */
        /* Description :
        BAR size field from each of the resizable VF BAR control registers, per function. For BARs that are not resizable, the corresponding bits in cfg_vf_rbar_size are set to 0.
        */
    };
} acc_rbar_vf_ro00_0_t;
/* offset : 0x598 */
typedef volatile union _acc_rbar_vf_ro01_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_vf_rbar_size:32; /* [Position:0] */
        /* Default Value : , TYPE : RO */
        /* Description :
        BAR size field from each of the resizable VF BAR control registers, per function. For BARs that are not resizable, the corresponding bits in cfg_vf_rbar_size are set to 0.
        */
    };
} acc_rbar_vf_ro01_0_t;
/* offset : 0x59c */
typedef volatile union _acc_rbar_vf_ro02_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_vf_rbar_size:32; /* [Position:0] */
        /* Default Value : , TYPE : RO */
        /* Description :
        BAR size field from each of the resizable VF BAR control registers, per function. For BARs that are not resizable, the corresponding bits in cfg_vf_rbar_size are set to 0.
        */
    };
} acc_rbar_vf_ro02_0_t;
/* offset : 0x5a0 */
typedef volatile union _acc_rbar_vf_ro03_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_vf_rbar_size:32; /* [Position:0] */
        /* Default Value : , TYPE : RO */
        /* Description :
        BAR size field from each of the resizable VF BAR control registers, per function. For BARs that are not resizable, the corresponding bits in cfg_vf_rbar_size are set to 0.
        */
    };
} acc_rbar_vf_ro03_0_t;
/* offset : 0x5a4 */
typedef volatile union _acc_rbar_vf_ro04_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_vf_rbar_size:16; /* [Position:0] */
        /* Default Value : , TYPE : RO */
        /* Description :
        BAR size field from each of the resizable VF BAR control registers, per function. For BARs that are not resizable, the corresponding bits in cfg_vf_rbar_size are set to 0.
        */
        volatile unsigned int RESERVED_16:16; /* [Position:16] */
        /* Default Value : x, TYPE : U */
        /* Description :
        BAR size field from each of the resizable VF BAR control registers, per function. For BARs that are not resizable, the corresponding bits in cfg_vf_rbar_size are set to 0.
        */
    };
} acc_rbar_vf_ro04_0_t;
/* offset : 0x5a8 */
typedef volatile union _acc_rbar_ro03_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int cfg_rbar_size:32; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        cfg_rbar_size[32*03+:32].
BAR size field from each of the resizable BAR control registers, per 
function. For BARs that are not resizable the corresponding bits in 
cfg_rbar_size are set to 0.
        */
    };
} acc_rbar_ro03_0_t;
/* offset : 0x5c0 */
typedef volatile union _acc_edge_intr_en0_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int edge_intr_en:19; /* [Position:0] */
        /* Default Value : , TYPE : RO */
        /* Description :
        edge_intr_en[18*0+:18].
edge_intr_en[00] = cfg_aer_rc_err_int[0] 
edge_intr_en[01] = cfg_aer_rc_err_int[1] 
edge_intr_en[02] = cfg_aer_rc_err_int[2] 
edge_intr_en[03] = cfg_aer_rc_err_int[3] 
edge_intr_en[04] = cfg_aer_rc_err_msi[0] 
edge_intr_en[05] = cfg_aer_rc_err_msi[1] 
edge_intr_en[06] = cfg_aer_rc_err_msi[2] 
edge_intr_en[07] = cfg_aer_rc_err_msi[3] 
edge_intr_en[08] = cfg_pme_int[0] 
edge_intr_en[09] = cfg_pme_int[1] 
edge_intr_en[10] = cfg_pme_int[2] 
edge_intr_en[11] = cfg_pme_int[3] 
edge_intr_en[12] = cfg_pme_msi[0] 
edge_intr_en[13] = cfg_pme_msi[1] 
edge_intr_en[14] = cfg_pme_msi[2] 
edge_intr_en[15] = cfg_pme_msi[3] 
edge_intr_en[16] = hp_pme[0] 
edge_intr_en[17] = hp_pme[1] 
        */
        volatile unsigned int RESERVED_19:13; /* [Position:19] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_edge_intr_en0_0_t;
/* offset : 0x5c4 */
typedef volatile union _acc_edge_intr_en1_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int edge_intr_en:19; /* [Position:0] */
        /* Default Value : , TYPE : RO */
        /* Description :
        edge_intr_en[18*1+:18].
edge_intr_en[18] = hp_pme[2] 
edge_intr_en[19] = hp_pme[3] 
edge_intr_en[20] = hp_int[0] 
edge_intr_en[21] = hp_int[1] 
edge_intr_en[22] = hp_int[2] 
edge_intr_en[23] = hp_int[3] 
edge_intr_en[24] = hp_msi[0] 
edge_intr_en[25] = hp_msi[1] 
edge_intr_en[26] = hp_msi[2] 
edge_intr_en[27] = hp_msi[3] 
edge_intr_en[28] = edma_int[0] 
edge_intr_en[29] = edma_int[1] 
edge_intr_en[30] = edma_int[2] 
edge_intr_en[31] = edma_int[3] 
edge_intr_en[32] = edma_int[4] 
edge_intr_en[33] = edma_int[5] 
edge_intr_en[34] = edma_int[6] 
edge_intr_en[35] = edma_int[7] 
        */
        volatile unsigned int RESERVED_19:13; /* [Position:19] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_edge_intr_en1_0_t;
/* offset : 0x5c8 */
typedef volatile union _acc_edge_intr_en2_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int edge_intr_en:19; /* [Position:0] */
        /* Default Value : , TYPE : RO */
        /* Description :
        edge_intr_en[18*2+:18].
edge_intr_en[36] = dpc_int 
edge_intr_en[37] = dpc_msi 
edge_intr_en[38] = cfg_send_cor_err[0] 
edge_intr_en[39] = cfg_send_cor_err[1] 
edge_intr_en[40] = cfg_send_cor_err[2] 
edge_intr_en[41] = cfg_send_cor_err[3] 
edge_intr_en[42] = cfg_send_nf_err[0] 
edge_intr_en[43] = cfg_send_nf_err[1] 
edge_intr_en[44] = cfg_send_nf_err[2] 
edge_intr_en[45] = cfg_send_nf_err[3] 
edge_intr_en[46] = cfg_send_f_err[0] 
edge_intr_en[47] = cfg_send_f_err[1] 
edge_intr_en[48] = cfg_send_f_err[2] 
edge_intr_en[49] = cfg_send_f_err[3] 
edge_intr_en[50] = radm_inta_asserted 
edge_intr_en[51] = radm_intb_asserted 
edge_intr_en[52] = radm_intc_asserted 
edge_intr_en[53] = radm_intd_asserted 
        */
        volatile unsigned int RESERVED_19:13; /* [Position:19] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_edge_intr_en2_0_t;
/* offset : 0x5cc */
typedef volatile union _acc_edge_intr_en3_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int edge_intr_en:16; /* [Position:0] */
        /* Default Value : , TYPE : RO */
        /* Description :
        edge_intr_en[18*3+:18].
edge_intr_en[54] = radm_inta_deasserted 
edge_intr_en[55] = radm_intb_deasserted 
edge_intr_en[56] = radm_intc_deasserted 
edge_intr_en[57] = radm_intd_deasserted 
edge_intr_en[58] = cfg_link_auto_bw_int 
edge_intr_en[59] = cfg_link_auto_bw_msi 
edge_intr_en[60] = cfg_bw_mgt_int 
edge_intr_en[61] = cfg_bw_mgt_msi 
edge_intr_en[62] = cfg_link_eq_req_int 
edge_intr_en[63] = usp_eq_redo_executed_int 
edge_intr_en[64] = assert_inta_grt 
edge_intr_en[65] = assert_intb_grt 
edge_intr_en[66] = assert_intc_grt 
edge_intr_en[67] = assert_intd_grt 
edge_intr_en[68] = deassert_inta_grt 
edge_intr_en[69] = deassert_intb_grt 
edge_intr_en[70] = deassert_intc_grt 
edge_intr_en[71] = deassert_intd_grt 
        */
        volatile unsigned int RESERVED_16:16; /* [Position:16] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_edge_intr_en3_0_t;
/* offset : 0x5d0 */
typedef volatile union _acc_trig_intr_en0_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int trig_intr_en:19; /* [Position:0] */
        /* Default Value : , TYPE : RO */
        /* Description :
        trig_intr_en[18*0+:18].
trig_intr_en[00] = cfg_aer_rc_err_int[0] 
trig_intr_en[01] = cfg_aer_rc_err_int[1] 
trig_intr_en[02] = cfg_aer_rc_err_int[2] 
trig_intr_en[03] = cfg_aer_rc_err_int[3] 
trig_intr_en[04] = cfg_aer_rc_err_msi[0] 
trig_intr_en[05] = cfg_aer_rc_err_msi[1] 
trig_intr_en[06] = cfg_aer_rc_err_msi[2] 
trig_intr_en[07] = cfg_aer_rc_err_msi[3] 
trig_intr_en[08] = cfg_pme_int[0] 
trig_intr_en[09] = cfg_pme_int[1] 
trig_intr_en[10] = cfg_pme_int[2] 
trig_intr_en[11] = cfg_pme_int[3] 
trig_intr_en[12] = cfg_pme_msi[0] 
trig_intr_en[13] = cfg_pme_msi[1] 
trig_intr_en[14] = cfg_pme_msi[2] 
trig_intr_en[15] = cfg_pme_msi[3] 
trig_intr_en[16] = hp_pme[0] 
trig_intr_en[17] = hp_pme[1] 
        */
        volatile unsigned int RESERVED_19:13; /* [Position:19] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_trig_intr_en0_0_t;
/* offset : 0x5d4 */
typedef volatile union _acc_trig_intr_en1_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int trig_intr_en:19; /* [Position:0] */
        /* Default Value : , TYPE : RO */
        /* Description :
        trig_intr_en[18*1+:18].
trig_intr_en[18] = hp_pme[2] 
trig_intr_en[19] = hp_pme[3] 
trig_intr_en[20] = hp_int[0] 
trig_intr_en[21] = hp_int[1] 
trig_intr_en[22] = hp_int[2] 
trig_intr_en[23] = hp_int[3] 
trig_intr_en[24] = hp_msi[0] 
trig_intr_en[25] = hp_msi[1] 
trig_intr_en[26] = hp_msi[2] 
trig_intr_en[27] = hp_msi[3] 
trig_intr_en[28] = edma_int[0] 
trig_intr_en[29] = edma_int[1] 
trig_intr_en[30] = edma_int[2] 
trig_intr_en[31] = edma_int[3] 
trig_intr_en[32] = edma_int[4] 
trig_intr_en[33] = edma_int[5] 
trig_intr_en[34] = edma_int[6] 
trig_intr_en[35] = edma_int[7] 
        */
        volatile unsigned int RESERVED_19:13; /* [Position:19] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_trig_intr_en1_0_t;
/* offset : 0x5d8 */
typedef volatile union _acc_trig_intr_en2_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int trig_intr_en:19; /* [Position:0] */
        /* Default Value : , TYPE : RO */
        /* Description :
        trig_intr_en[18*2+:18].
trig_intr_en[36] = dpc_int 
trig_intr_en[37] = dpc_msi 
trig_intr_en[38] = cfg_send_cor_err[0] 
trig_intr_en[39] = cfg_send_cor_err[1] 
trig_intr_en[40] = cfg_send_cor_err[2] 
trig_intr_en[41] = cfg_send_cor_err[3] 
trig_intr_en[42] = cfg_send_nf_err[0] 
trig_intr_en[43] = cfg_send_nf_err[1] 
trig_intr_en[44] = cfg_send_nf_err[2] 
trig_intr_en[45] = cfg_send_nf_err[3] 
trig_intr_en[46] = cfg_send_f_err[0] 
trig_intr_en[47] = cfg_send_f_err[1] 
trig_intr_en[48] = cfg_send_f_err[2] 
trig_intr_en[49] = cfg_send_f_err[3] 
trig_intr_en[50] = radm_inta_asserted 
trig_intr_en[51] = radm_intb_asserted 
trig_intr_en[52] = radm_intc_asserted 
trig_intr_en[53] = radm_intd_asserted 
        */
        volatile unsigned int RESERVED_19:13; /* [Position:19] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_trig_intr_en2_0_t;
/* offset : 0x5dc */
typedef volatile union _acc_trig_intr_en3_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int trig_intr_en:16; /* [Position:0] */
        /* Default Value : , TYPE : RO */
        /* Description :
        trig_intr_en[18*3+:18].
trig_intr_en[54] = radm_inta_deasserted 
trig_intr_en[55] = radm_intb_deasserted 
trig_intr_en[56] = radm_intc_deasserted 
trig_intr_en[57] = radm_intd_deasserted 
trig_intr_en[58] = cfg_link_auto_bw_int 
trig_intr_en[59] = cfg_link_auto_bw_msi 
trig_intr_en[60] = cfg_bw_mgt_int 
trig_intr_en[61] = cfg_bw_mgt_msi 
trig_intr_en[62] = cfg_link_eq_req_int 
trig_intr_en[63] = usp_eq_redo_executed_int 
trig_intr_en[64] = assert_inta_grt 
trig_intr_en[65] = assert_intb_grt 
trig_intr_en[66] = assert_intc_grt 
trig_intr_en[67] = assert_intd_grt 
trig_intr_en[68] = deassert_inta_grt 
trig_intr_en[69] = deassert_intb_grt 
trig_intr_en[70] = deassert_intc_grt 
trig_intr_en[71] = deassert_intd_grt 
        */
        volatile unsigned int RESERVED_16:16; /* [Position:16] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_trig_intr_en3_0_t;
/* offset : 0x5e0 */
typedef volatile union _acc_levl_intr_en0_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int levl_intr_en:19; /* [Position:0] */
        /* Default Value : , TYPE : RO */
        /* Description :
        levl_intr_en[18*0+:18].
levl_intr_en[00] = cfg_aer_rc_err_int[0] 
levl_intr_en[01] = cfg_aer_rc_err_int[1] 
levl_intr_en[02] = cfg_aer_rc_err_int[2] 
levl_intr_en[03] = cfg_aer_rc_err_int[3] 
levl_intr_en[04] = cfg_aer_rc_err_msi[0] 
levl_intr_en[05] = cfg_aer_rc_err_msi[1] 
levl_intr_en[06] = cfg_aer_rc_err_msi[2] 
levl_intr_en[07] = cfg_aer_rc_err_msi[3] 
levl_intr_en[08] = cfg_pme_int[0] 
levl_intr_en[09] = cfg_pme_int[1] 
levl_intr_en[10] = cfg_pme_int[2] 
levl_intr_en[11] = cfg_pme_int[3] 
levl_intr_en[12] = cfg_pme_msi[0] 
levl_intr_en[13] = cfg_pme_msi[1] 
levl_intr_en[14] = cfg_pme_msi[2] 
levl_intr_en[15] = cfg_pme_msi[3] 
levl_intr_en[16] = hp_pme[0] 
levl_intr_en[17] = hp_pme[1] 
        */
        volatile unsigned int RESERVED_19:13; /* [Position:19] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_levl_intr_en0_0_t;
/* offset : 0x5e4 */
typedef volatile union _acc_levl_intr_en1_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int levl_intr_en:19; /* [Position:0] */
        /* Default Value : , TYPE : RO */
        /* Description :
        levl_intr_en[18*1+:18].
levl_intr_en[18] = hp_pme[2] 
levl_intr_en[19] = hp_pme[3] 
levl_intr_en[20] = hp_int[0] 
levl_intr_en[21] = hp_int[1] 
levl_intr_en[22] = hp_int[2] 
levl_intr_en[23] = hp_int[3] 
levl_intr_en[24] = hp_msi[0] 
levl_intr_en[25] = hp_msi[1] 
levl_intr_en[26] = hp_msi[2] 
levl_intr_en[27] = hp_msi[3] 
levl_intr_en[28] = edma_int[0] 
levl_intr_en[29] = edma_int[1] 
levl_intr_en[30] = edma_int[2] 
levl_intr_en[31] = edma_int[3] 
levl_intr_en[32] = edma_int[4] 
levl_intr_en[33] = edma_int[5] 
levl_intr_en[34] = edma_int[6] 
levl_intr_en[35] = edma_int[7] 
        */
        volatile unsigned int RESERVED_19:13; /* [Position:19] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_levl_intr_en1_0_t;
/* offset : 0x5e8 */
typedef volatile union _acc_levl_intr_en2_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int levl_intr_en:19; /* [Position:0] */
        /* Default Value : , TYPE : RO */
        /* Description :
        levl_intr_en[18*2+:18].
levl_intr_en[36] = dpc_int 
levl_intr_en[37] = dpc_msi 
levl_intr_en[38] = cfg_send_cor_err[0] 
levl_intr_en[39] = cfg_send_cor_err[1] 
levl_intr_en[40] = cfg_send_cor_err[2] 
levl_intr_en[41] = cfg_send_cor_err[3] 
levl_intr_en[42] = cfg_send_nf_err[0] 
levl_intr_en[43] = cfg_send_nf_err[1] 
levl_intr_en[44] = cfg_send_nf_err[2] 
levl_intr_en[45] = cfg_send_nf_err[3] 
levl_intr_en[46] = cfg_send_f_err[0] 
levl_intr_en[47] = cfg_send_f_err[1] 
levl_intr_en[48] = cfg_send_f_err[2] 
levl_intr_en[49] = cfg_send_f_err[3] 
levl_intr_en[50] = radm_inta_asserted 
levl_intr_en[51] = radm_intb_asserted 
levl_intr_en[52] = radm_intc_asserted 
levl_intr_en[53] = radm_intd_asserted 
        */
        volatile unsigned int RESERVED_19:13; /* [Position:19] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_levl_intr_en2_0_t;
/* offset : 0x5ec */
typedef volatile union _acc_levl_intr_en3_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int levl_intr_en:16; /* [Position:0] */
        /* Default Value : , TYPE : RO */
        /* Description :
        levl_intr_en[18*3+:18].
levl_intr_en[54] = radm_inta_deasserted 
levl_intr_en[55] = radm_intb_deasserted 
levl_intr_en[56] = radm_intc_deasserted 
levl_intr_en[57] = radm_intd_deasserted 
levl_intr_en[58] = cfg_link_auto_bw_int 
levl_intr_en[59] = cfg_link_auto_bw_msi 
levl_intr_en[60] = cfg_bw_mgt_int 
levl_intr_en[61] = cfg_bw_mgt_msi 
levl_intr_en[62] = cfg_link_eq_req_int 
levl_intr_en[63] = usp_eq_redo_executed_int 
levl_intr_en[64] = assert_inta_grt 
levl_intr_en[65] = assert_intb_grt 
levl_intr_en[66] = assert_intc_grt 
levl_intr_en[67] = assert_intd_grt 
levl_intr_en[68] = deassert_inta_grt 
levl_intr_en[69] = deassert_intb_grt 
levl_intr_en[70] = deassert_intc_grt 
levl_intr_en[71] = deassert_intd_grt 
        */
        volatile unsigned int RESERVED_16:16; /* [Position:16] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_levl_intr_en3_0_t;
/* offset : 0x600 */
typedef volatile union _acc_intr_st_part_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int intr_st_part:12; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        intr_st_part[00] = |r_edge_intr_st[0*18+:18] 
intr_st_part[01] = |r_edge_intr_st[1*18+:18] 
intr_st_part[02] = |r_edge_intr_st[2*18+:18] 
intr_st_part[03] = |r_edge_intr_st[3*18+:18] 
intr_st_part[04] = |r_trig_intr_st[0*18+:18] 
intr_st_part[05] = |r_trig_intr_st[1*18+:18] 
intr_st_part[06] = |r_trig_intr_st[2*18+:18] 
intr_st_part[07] = |r_trig_intr_st[3*18+:18] 
intr_st_part[08] = |r_levl_intr_st[0*18+:18] 
intr_st_part[09] = |r_levl_intr_st[1*18+:18] 
intr_st_part[10] = |r_levl_intr_st[2*18+:18] 
intr_st_part[11] = |r_levl_intr_st[3*18+:18] 
        */
        volatile unsigned int RESERVED_12:20; /* [Position:12] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_intr_st_part_0_t;
/* offset : 0x604 */
typedef volatile union _acc_edge_intr_st0_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int r_edge_intr_st:18; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        r_edge_intr_st[18*0+:18].
r_edge_intr_st[00] = cfg_aer_rc_err_int[0] 
r_edge_intr_st[01] = cfg_aer_rc_err_int[1] 
r_edge_intr_st[02] = cfg_aer_rc_err_int[2] 
r_edge_intr_st[03] = cfg_aer_rc_err_int[3] 
r_edge_intr_st[04] = cfg_aer_rc_err_msi[0] 
r_edge_intr_st[05] = cfg_aer_rc_err_msi[1] 
r_edge_intr_st[06] = cfg_aer_rc_err_msi[2] 
r_edge_intr_st[07] = cfg_aer_rc_err_msi[3] 
r_edge_intr_st[08] = cfg_pme_int[0] 
r_edge_intr_st[09] = cfg_pme_int[1] 
r_edge_intr_st[10] = cfg_pme_int[2] 
r_edge_intr_st[11] = cfg_pme_int[3] 
r_edge_intr_st[12] = cfg_pme_msi[0] 
r_edge_intr_st[13] = cfg_pme_msi[1] 
r_edge_intr_st[14] = cfg_pme_msi[2] 
r_edge_intr_st[15] = cfg_pme_msi[3] 
r_edge_intr_st[16] = hp_pme[0] 
r_edge_intr_st[17] = hp_pme[1] 
        */
        volatile unsigned int RESERVED_18:14; /* [Position:18] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_edge_intr_st0_0_t;
/* offset : 0x608 */
typedef volatile union _acc_edge_intr_st1_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int r_edge_intr_st:18; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        r_edge_intr_st[18*1+:18].
r_edge_intr_st[18] = hp_pme[2] 
r_edge_intr_st[19] = hp_pme[3] 
r_edge_intr_st[20] = hp_int[0] 
r_edge_intr_st[21] = hp_int[1] 
r_edge_intr_st[22] = hp_int[2] 
r_edge_intr_st[23] = hp_int[3] 
r_edge_intr_st[24] = hp_msi[0] 
r_edge_intr_st[25] = hp_msi[1] 
r_edge_intr_st[26] = hp_msi[2] 
r_edge_intr_st[27] = hp_msi[3] 
r_edge_intr_st[28] = edma_int[0] 
r_edge_intr_st[29] = edma_int[1] 
r_edge_intr_st[30] = edma_int[2] 
r_edge_intr_st[31] = edma_int[3] 
r_edge_intr_st[32] = edma_int[4] 
r_edge_intr_st[33] = edma_int[5] 
r_edge_intr_st[34] = edma_int[6] 
r_edge_intr_st[35] = edma_int[7] 
        */
        volatile unsigned int RESERVED_18:14; /* [Position:18] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_edge_intr_st1_0_t;
/* offset : 0x60c */
typedef volatile union _acc_edge_intr_st2_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int r_edge_intr_st:18; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        r_edge_intr_st[18*2+:18].
r_edge_intr_st[36] = dpc_int 
r_edge_intr_st[37] = dpc_msi 
r_edge_intr_st[38] = cfg_send_cor_err[0] 
r_edge_intr_st[39] = cfg_send_cor_err[1] 
r_edge_intr_st[40] = cfg_send_cor_err[2] 
r_edge_intr_st[41] = cfg_send_cor_err[3] 
r_edge_intr_st[42] = cfg_send_nf_err[0] 
r_edge_intr_st[43] = cfg_send_nf_err[1] 
r_edge_intr_st[44] = cfg_send_nf_err[2] 
r_edge_intr_st[45] = cfg_send_nf_err[3] 
r_edge_intr_st[46] = cfg_send_f_err[0] 
r_edge_intr_st[47] = cfg_send_f_err[1] 
r_edge_intr_st[48] = cfg_send_f_err[2] 
r_edge_intr_st[49] = cfg_send_f_err[3] 
r_edge_intr_st[50] = radm_inta_asserted 
r_edge_intr_st[51] = radm_intb_asserted 
r_edge_intr_st[52] = radm_intc_asserted 
r_edge_intr_st[53] = radm_intd_asserted 
        */
        volatile unsigned int RESERVED_18:14; /* [Position:18] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_edge_intr_st2_0_t;
/* offset : 0x610 */
typedef volatile union _acc_edge_intr_st3_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int r_edge_intr_st:18; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        r_edge_intr_st[18*3+:18].
r_edge_intr_st[54] = radm_inta_deasserted 
r_edge_intr_st[55] = radm_intb_deasserted 
r_edge_intr_st[56] = radm_intc_deasserted 
r_edge_intr_st[57] = radm_intd_deasserted 
r_edge_intr_st[58] = cfg_link_auto_bw_int 
r_edge_intr_st[59] = cfg_link_auto_bw_msi 
r_edge_intr_st[60] = cfg_bw_mgt_int 
r_edge_intr_st[61] = cfg_bw_mgt_msi 
r_edge_intr_st[62] = cfg_link_eq_req_int 
r_edge_intr_st[63] = usp_eq_redo_executed_int 
r_edge_intr_st[64] = assert_inta_grt 
r_edge_intr_st[65] = assert_intb_grt 
r_edge_intr_st[66] = assert_intc_grt 
r_edge_intr_st[67] = assert_intd_grt 
r_edge_intr_st[68] = deassert_inta_grt 
r_edge_intr_st[69] = deassert_intb_grt 
r_edge_intr_st[70] = deassert_intc_grt 
r_edge_intr_st[71] = deassert_intd_grt 
        */
        volatile unsigned int RESERVED_18:14; /* [Position:18] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_edge_intr_st3_0_t;
/* offset : 0x614 */
typedef volatile union _acc_trig_intr_st0_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int r_trig_intr_st:18; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        r_trig_intr_st[18*0+:18].
r_trig_intr_st[00] = cfg_aer_rc_err_int[0] 
r_trig_intr_st[01] = cfg_aer_rc_err_int[1] 
r_trig_intr_st[02] = cfg_aer_rc_err_int[2] 
r_trig_intr_st[03] = cfg_aer_rc_err_int[3] 
r_trig_intr_st[04] = cfg_aer_rc_err_msi[0] 
r_trig_intr_st[05] = cfg_aer_rc_err_msi[1] 
r_trig_intr_st[06] = cfg_aer_rc_err_msi[2] 
r_trig_intr_st[07] = cfg_aer_rc_err_msi[3] 
r_trig_intr_st[08] = cfg_pme_int[0] 
r_trig_intr_st[09] = cfg_pme_int[1] 
r_trig_intr_st[10] = cfg_pme_int[2] 
r_trig_intr_st[11] = cfg_pme_int[3] 
r_trig_intr_st[12] = cfg_pme_msi[0] 
r_trig_intr_st[13] = cfg_pme_msi[1] 
r_trig_intr_st[14] = cfg_pme_msi[2] 
r_trig_intr_st[15] = cfg_pme_msi[3] 
r_trig_intr_st[16] = hp_pme[0] 
r_trig_intr_st[17] = hp_pme[1] 
        */
        volatile unsigned int RESERVED_18:14; /* [Position:18] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_trig_intr_st0_0_t;
/* offset : 0x618 */
typedef volatile union _acc_trig_intr_st1_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int r_trig_intr_st:18; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        r_trig_intr_st[18*1+:18].
r_trig_intr_st[18] = hp_pme[2] 
r_trig_intr_st[19] = hp_pme[3] 
r_trig_intr_st[20] = hp_int[0] 
r_trig_intr_st[21] = hp_int[1] 
r_trig_intr_st[22] = hp_int[2] 
r_trig_intr_st[23] = hp_int[3] 
r_trig_intr_st[24] = hp_msi[0] 
r_trig_intr_st[25] = hp_msi[1] 
r_trig_intr_st[26] = hp_msi[2] 
r_trig_intr_st[27] = hp_msi[3] 
r_trig_intr_st[28] = edma_int[0] 
r_trig_intr_st[29] = edma_int[1] 
r_trig_intr_st[30] = edma_int[2] 
r_trig_intr_st[31] = edma_int[3] 
r_trig_intr_st[32] = edma_int[4] 
r_trig_intr_st[33] = edma_int[5] 
r_trig_intr_st[34] = edma_int[6] 
r_trig_intr_st[35] = edma_int[7] 
        */
        volatile unsigned int RESERVED_18:14; /* [Position:18] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_trig_intr_st1_0_t;
/* offset : 0x61c */
typedef volatile union _acc_trig_intr_st2_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int r_trig_intr_st:18; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        r_trig_intr_st[18*2+:18].
r_trig_intr_st[36] = dpc_int 
r_trig_intr_st[37] = dpc_msi 
r_trig_intr_st[38] = cfg_send_cor_err[0] 
r_trig_intr_st[39] = cfg_send_cor_err[1] 
r_trig_intr_st[40] = cfg_send_cor_err[2] 
r_trig_intr_st[41] = cfg_send_cor_err[3] 
r_trig_intr_st[42] = cfg_send_nf_err[0] 
r_trig_intr_st[43] = cfg_send_nf_err[1] 
r_trig_intr_st[44] = cfg_send_nf_err[2] 
r_trig_intr_st[45] = cfg_send_nf_err[3] 
r_trig_intr_st[46] = cfg_send_f_err[0] 
r_trig_intr_st[47] = cfg_send_f_err[1] 
r_trig_intr_st[48] = cfg_send_f_err[2] 
r_trig_intr_st[49] = cfg_send_f_err[3] 
r_trig_intr_st[50] = radm_inta_asserted 
r_trig_intr_st[51] = radm_intb_asserted 
r_trig_intr_st[52] = radm_intc_asserted 
r_trig_intr_st[53] = radm_intd_asserted 
        */
        volatile unsigned int RESERVED_18:14; /* [Position:18] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_trig_intr_st2_0_t;
/* offset : 0x620 */
typedef volatile union _acc_trig_intr_st3_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int r_trig_intr_st:18; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        r_trig_intr_st[18*3+:18].
r_trig_intr_st[54] = radm_inta_deasserted 
r_trig_intr_st[55] = radm_intb_deasserted 
r_trig_intr_st[56] = radm_intc_deasserted 
r_trig_intr_st[57] = radm_intd_deasserted 
r_trig_intr_st[58] = cfg_link_auto_bw_int 
r_trig_intr_st[59] = cfg_link_auto_bw_msi 
r_trig_intr_st[60] = cfg_bw_mgt_int 
r_trig_intr_st[61] = cfg_bw_mgt_msi 
r_trig_intr_st[62] = cfg_link_eq_req_int 
r_trig_intr_st[63] = usp_eq_redo_executed_int 
r_trig_intr_st[64] = assert_inta_grt 
r_trig_intr_st[65] = assert_intb_grt 
r_trig_intr_st[66] = assert_intc_grt 
r_trig_intr_st[67] = assert_intd_grt 
r_trig_intr_st[68] = deassert_inta_grt 
r_trig_intr_st[69] = deassert_intb_grt 
r_trig_intr_st[70] = deassert_intc_grt 
r_trig_intr_st[71] = deassert_intd_grt 
        */
        volatile unsigned int RESERVED_18:14; /* [Position:18] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_trig_intr_st3_0_t;
/* offset : 0x624 */
typedef volatile union _acc_levl_intr_st0_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int r_levl_intr_st:18; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        r_levl_intr_st[18*0+:18].
r_levl_intr_st[00] = cfg_aer_rc_err_int[0] 
r_levl_intr_st[01] = cfg_aer_rc_err_int[1] 
r_levl_intr_st[02] = cfg_aer_rc_err_int[2] 
r_levl_intr_st[03] = cfg_aer_rc_err_int[3] 
r_levl_intr_st[04] = cfg_aer_rc_err_msi[0] 
r_levl_intr_st[05] = cfg_aer_rc_err_msi[1] 
r_levl_intr_st[06] = cfg_aer_rc_err_msi[2] 
r_levl_intr_st[07] = cfg_aer_rc_err_msi[3] 
r_levl_intr_st[08] = cfg_pme_int[0] 
r_levl_intr_st[09] = cfg_pme_int[1] 
r_levl_intr_st[10] = cfg_pme_int[2] 
r_levl_intr_st[11] = cfg_pme_int[3] 
r_levl_intr_st[12] = cfg_pme_msi[0] 
r_levl_intr_st[13] = cfg_pme_msi[1] 
r_levl_intr_st[14] = cfg_pme_msi[2] 
r_levl_intr_st[15] = cfg_pme_msi[3] 
r_levl_intr_st[16] = hp_pme[0] 
r_levl_intr_st[17] = hp_pme[1] 
        */
        volatile unsigned int RESERVED_18:14; /* [Position:18] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_levl_intr_st0_0_t;
/* offset : 0x628 */
typedef volatile union _acc_levl_intr_st1_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int r_levl_intr_st:18; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        r_levl_intr_st[18*1+:18].
r_levl_intr_st[18] = hp_pme[2] 
r_levl_intr_st[19] = hp_pme[3] 
r_levl_intr_st[20] = hp_int[0] 
r_levl_intr_st[21] = hp_int[1] 
r_levl_intr_st[22] = hp_int[2] 
r_levl_intr_st[23] = hp_int[3] 
r_levl_intr_st[24] = hp_msi[0] 
r_levl_intr_st[25] = hp_msi[1] 
r_levl_intr_st[26] = hp_msi[2] 
r_levl_intr_st[27] = hp_msi[3] 
r_levl_intr_st[28] = edma_int[0] 
r_levl_intr_st[29] = edma_int[1] 
r_levl_intr_st[30] = edma_int[2] 
r_levl_intr_st[31] = edma_int[3] 
r_levl_intr_st[32] = edma_int[4] 
r_levl_intr_st[33] = edma_int[5] 
r_levl_intr_st[34] = edma_int[6] 
r_levl_intr_st[35] = edma_int[7] 
        */
        volatile unsigned int RESERVED_18:14; /* [Position:18] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_levl_intr_st1_0_t;
/* offset : 0x62c */
typedef volatile union _acc_levl_intr_st2_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int r_levl_intr_st:18; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        r_levl_intr_st[18*2+:18].
r_levl_intr_st[36] = dpc_int 
r_levl_intr_st[37] = dpc_msi 
r_levl_intr_st[38] = cfg_send_cor_err[0] 
r_levl_intr_st[39] = cfg_send_cor_err[1] 
r_levl_intr_st[40] = cfg_send_cor_err[2] 
r_levl_intr_st[41] = cfg_send_cor_err[3] 
r_levl_intr_st[42] = cfg_send_nf_err[0] 
r_levl_intr_st[43] = cfg_send_nf_err[1] 
r_levl_intr_st[44] = cfg_send_nf_err[2] 
r_levl_intr_st[45] = cfg_send_nf_err[3] 
r_levl_intr_st[46] = cfg_send_f_err[0] 
r_levl_intr_st[47] = cfg_send_f_err[1] 
r_levl_intr_st[48] = cfg_send_f_err[2] 
r_levl_intr_st[49] = cfg_send_f_err[3] 
r_levl_intr_st[50] = radm_inta_asserted 
r_levl_intr_st[51] = radm_intb_asserted 
r_levl_intr_st[52] = radm_intc_asserted 
r_levl_intr_st[53] = radm_intd_asserted 
        */
        volatile unsigned int RESERVED_18:14; /* [Position:18] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_levl_intr_st2_0_t;
/* offset : 0x630 */
typedef volatile union _acc_levl_intr_st3_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int r_levl_intr_st:18; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        r_levl_intr_st[18*3+:18].
r_levl_intr_st[54] = radm_inta_deasserted 
r_levl_intr_st[55] = radm_intb_deasserted 
r_levl_intr_st[56] = radm_intc_deasserted 
r_levl_intr_st[57] = radm_intd_deasserted 
r_levl_intr_st[58] = cfg_link_auto_bw_int 
r_levl_intr_st[59] = cfg_link_auto_bw_msi 
r_levl_intr_st[60] = cfg_bw_mgt_int 
r_levl_intr_st[61] = cfg_bw_mgt_msi 
r_levl_intr_st[62] = cfg_link_eq_req_int 
r_levl_intr_st[63] = usp_eq_redo_executed_int 
r_levl_intr_st[64] = assert_inta_grt 
r_levl_intr_st[65] = assert_intb_grt 
r_levl_intr_st[66] = assert_intc_grt 
r_levl_intr_st[67] = assert_intd_grt 
r_levl_intr_st[68] = deassert_inta_grt 
r_levl_intr_st[69] = deassert_intb_grt 
r_levl_intr_st[70] = deassert_intc_grt 
r_levl_intr_st[71] = deassert_intd_grt 
        */
        volatile unsigned int RESERVED_18:14; /* [Position:18] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_levl_intr_st3_0_t;
/* offset : 0x640 */
typedef volatile union _acc_edge_intr_cl0_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int edge_intr_cl0:18; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        edge_intr_cl[18*0+:18].
edge_intr_cl[00] = cfg_aer_rc_err_int[0] 
edge_intr_cl[01] = cfg_aer_rc_err_int[1] 
edge_intr_cl[02] = cfg_aer_rc_err_int[2] 
edge_intr_cl[03] = cfg_aer_rc_err_int[3] 
edge_intr_cl[04] = cfg_aer_rc_err_msi[0] 
edge_intr_cl[05] = cfg_aer_rc_err_msi[1] 
edge_intr_cl[06] = cfg_aer_rc_err_msi[2] 
edge_intr_cl[07] = cfg_aer_rc_err_msi[3] 
edge_intr_cl[08] = cfg_pme_int[0] 
edge_intr_cl[09] = cfg_pme_int[1] 
edge_intr_cl[10] = cfg_pme_int[2] 
edge_intr_cl[11] = cfg_pme_int[3] 
edge_intr_cl[12] = cfg_pme_msi[0] 
edge_intr_cl[13] = cfg_pme_msi[1] 
edge_intr_cl[14] = cfg_pme_msi[2] 
edge_intr_cl[15] = cfg_pme_msi[3] 
edge_intr_cl[16] = hp_pme[0] 
edge_intr_cl[17] = hp_pme[1] 
        */
        volatile unsigned int RESERVED_18:14; /* [Position:18] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_edge_intr_cl0_0_t;
/* offset : 0x644 */
typedef volatile union _acc_edge_intr_cl1_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int edge_intr_cl1:18; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        edge_intr_cl[18*1+:18].
edge_intr_cl[18] = hp_pme[2] 
edge_intr_cl[19] = hp_pme[3] 
edge_intr_cl[20] = hp_int[0] 
edge_intr_cl[21] = hp_int[1] 
edge_intr_cl[22] = hp_int[2] 
edge_intr_cl[23] = hp_int[3] 
edge_intr_cl[24] = hp_msi[0] 
edge_intr_cl[25] = hp_msi[1] 
edge_intr_cl[26] = hp_msi[2] 
edge_intr_cl[27] = hp_msi[3] 
edge_intr_cl[28] = edma_int[0] 
edge_intr_cl[29] = edma_int[1] 
edge_intr_cl[30] = edma_int[2] 
edge_intr_cl[31] = edma_int[3] 
edge_intr_cl[32] = edma_int[4] 
edge_intr_cl[33] = edma_int[5] 
edge_intr_cl[34] = edma_int[6] 
edge_intr_cl[35] = edma_int[7] 
        */
        volatile unsigned int RESERVED_18:14; /* [Position:18] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_edge_intr_cl1_0_t;
/* offset : 0x648 */
typedef volatile union _acc_edge_intr_cl2_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int edge_intr_cl2:18; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        edge_intr_cl[18*2+:18].
edge_intr_cl[36] = dpc_int 
edge_intr_cl[37] = dpc_msi 
edge_intr_cl[38] = cfg_send_cor_err[0] 
edge_intr_cl[39] = cfg_send_cor_err[1] 
edge_intr_cl[40] = cfg_send_cor_err[2] 
edge_intr_cl[41] = cfg_send_cor_err[3] 
edge_intr_cl[42] = cfg_send_nf_err[0] 
edge_intr_cl[43] = cfg_send_nf_err[1] 
edge_intr_cl[44] = cfg_send_nf_err[2] 
edge_intr_cl[45] = cfg_send_nf_err[3] 
edge_intr_cl[46] = cfg_send_f_err[0] 
edge_intr_cl[47] = cfg_send_f_err[1] 
edge_intr_cl[48] = cfg_send_f_err[2] 
edge_intr_cl[49] = cfg_send_f_err[3] 
edge_intr_cl[50] = radm_inta_asserted 
edge_intr_cl[51] = radm_intb_asserted 
edge_intr_cl[52] = radm_intc_asserted 
edge_intr_cl[53] = radm_intd_asserted 
        */
        volatile unsigned int RESERVED_18:14; /* [Position:18] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_edge_intr_cl2_0_t;
/* offset : 0x64c */
typedef volatile union _acc_edge_intr_cl3_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int edge_intr_cl3:18; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        edge_intr_cl[18*3+:18].
edge_intr_cl[54] = radm_inta_deasserted 
edge_intr_cl[55] = radm_intb_deasserted 
edge_intr_cl[56] = radm_intc_deasserted 
edge_intr_cl[57] = radm_intd_deasserted 
edge_intr_cl[58] = cfg_link_auto_bw_int 
edge_intr_cl[59] = cfg_link_auto_bw_msi 
edge_intr_cl[60] = cfg_bw_mgt_int 
edge_intr_cl[61] = cfg_bw_mgt_msi 
edge_intr_cl[62] = cfg_link_eq_req_int 
edge_intr_cl[63] = usp_eq_redo_executed_int 
edge_intr_cl[64] = assert_inta_grt 
edge_intr_cl[65] = assert_intb_grt 
edge_intr_cl[66] = assert_intc_grt 
edge_intr_cl[67] = assert_intd_grt 
edge_intr_cl[68] = deassert_inta_grt 
edge_intr_cl[69] = deassert_intb_grt 
edge_intr_cl[70] = deassert_intc_grt 
edge_intr_cl[71] = deassert_intd_grt 
        */
        volatile unsigned int RESERVED_18:14; /* [Position:18] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_edge_intr_cl3_0_t;
/* offset : 0x650 */
typedef volatile union _acc_trig_intr_cl0_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int trig_intr_cl0:18; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        trig_intr_cl[18*0+:18].
trig_intr_cl[00] = cfg_aer_rc_err_int[0] 
trig_intr_cl[01] = cfg_aer_rc_err_int[1] 
trig_intr_cl[02] = cfg_aer_rc_err_int[2] 
trig_intr_cl[03] = cfg_aer_rc_err_int[3] 
trig_intr_cl[04] = cfg_aer_rc_err_msi[0] 
trig_intr_cl[05] = cfg_aer_rc_err_msi[1] 
trig_intr_cl[06] = cfg_aer_rc_err_msi[2] 
trig_intr_cl[07] = cfg_aer_rc_err_msi[3] 
trig_intr_cl[08] = cfg_pme_int[0] 
trig_intr_cl[09] = cfg_pme_int[1] 
trig_intr_cl[10] = cfg_pme_int[2] 
trig_intr_cl[11] = cfg_pme_int[3] 
trig_intr_cl[12] = cfg_pme_msi[0] 
trig_intr_cl[13] = cfg_pme_msi[1] 
trig_intr_cl[14] = cfg_pme_msi[2] 
trig_intr_cl[15] = cfg_pme_msi[3] 
trig_intr_cl[16] = hp_pme[0] 
trig_intr_cl[17] = hp_pme[1] 
        */
        volatile unsigned int RESERVED_18:14; /* [Position:18] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_trig_intr_cl0_0_t;
/* offset : 0x654 */
typedef volatile union _acc_trig_intr_cl1_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int trig_intr_cl1:18; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        trig_intr_cl[18*1+:18].
trig_intr_cl[18] = hp_pme[2] 
trig_intr_cl[19] = hp_pme[3] 
trig_intr_cl[20] = hp_int[0] 
trig_intr_cl[21] = hp_int[1] 
trig_intr_cl[22] = hp_int[2] 
trig_intr_cl[23] = hp_int[3] 
trig_intr_cl[24] = hp_msi[0] 
trig_intr_cl[25] = hp_msi[1] 
trig_intr_cl[26] = hp_msi[2] 
trig_intr_cl[27] = hp_msi[3] 
trig_intr_cl[28] = edma_int[0] 
trig_intr_cl[29] = edma_int[1] 
trig_intr_cl[30] = edma_int[2] 
trig_intr_cl[31] = edma_int[3] 
trig_intr_cl[32] = edma_int[4] 
trig_intr_cl[33] = edma_int[5] 
trig_intr_cl[34] = edma_int[6] 
trig_intr_cl[35] = edma_int[7] 
        */
        volatile unsigned int RESERVED_18:14; /* [Position:18] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_trig_intr_cl1_0_t;
/* offset : 0x658 */
typedef volatile union _acc_trig_intr_cl2_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int trig_intr_cl2:18; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        trig_intr_cl[18*2+:18].
trig_intr_cl[36] = dpc_int 
trig_intr_cl[37] = dpc_msi 
trig_intr_cl[38] = cfg_send_cor_err[0] 
trig_intr_cl[39] = cfg_send_cor_err[1] 
trig_intr_cl[40] = cfg_send_cor_err[2] 
trig_intr_cl[41] = cfg_send_cor_err[3] 
trig_intr_cl[42] = cfg_send_nf_err[0] 
trig_intr_cl[43] = cfg_send_nf_err[1] 
trig_intr_cl[44] = cfg_send_nf_err[2] 
trig_intr_cl[45] = cfg_send_nf_err[3] 
trig_intr_cl[46] = cfg_send_f_err[0] 
trig_intr_cl[47] = cfg_send_f_err[1] 
trig_intr_cl[48] = cfg_send_f_err[2] 
trig_intr_cl[49] = cfg_send_f_err[3] 
trig_intr_cl[50] = radm_inta_asserted 
trig_intr_cl[51] = radm_intb_asserted 
trig_intr_cl[52] = radm_intc_asserted 
trig_intr_cl[53] = radm_intd_asserted 
        */
        volatile unsigned int RESERVED_18:14; /* [Position:18] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_trig_intr_cl2_0_t;
/* offset : 0x65c */
typedef volatile union _acc_trig_intr_cl3_0_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int trig_intr_cl3:18; /* [Position:0] */
        /* Default Value : 0, TYPE : RO */
        /* Description :
        trig_intr_cl[18*3+:18].
trig_intr_cl[54] = radm_inta_deasserted 
trig_intr_cl[55] = radm_intb_deasserted 
trig_intr_cl[56] = radm_intc_deasserted 
trig_intr_cl[57] = radm_intd_deasserted 
trig_intr_cl[58] = cfg_link_auto_bw_int 
trig_intr_cl[59] = cfg_link_auto_bw_msi 
trig_intr_cl[60] = cfg_bw_mgt_int 
trig_intr_cl[61] = cfg_bw_mgt_msi 
trig_intr_cl[62] = cfg_link_eq_req_int 
trig_intr_cl[63] = usp_eq_redo_executed_int 
trig_intr_cl[64] = assert_inta_grt 
trig_intr_cl[65] = assert_intb_grt 
trig_intr_cl[66] = assert_intc_grt 
trig_intr_cl[67] = assert_intd_grt 
trig_intr_cl[68] = deassert_inta_grt 
trig_intr_cl[69] = deassert_intb_grt 
trig_intr_cl[70] = deassert_intc_grt 
trig_intr_cl[71] = deassert_intd_grt 
        */
        volatile unsigned int RESERVED_18:14; /* [Position:18] */
        /* Default Value : x, TYPE : U */
        /* Description :
        RESERVED
        */
    };
} acc_trig_intr_cl3_0_t;

typedef volatile struct _pcie_hif_reg_t
{
    acc_ctrl00_0_t acc_ctrl00_0; /* 0x0 */
    acc_ctrl01_0_t acc_ctrl01_0; /* 0x4 */
    volatile unsigned int rsvd_0x8[16];
    acc_pipe02_0_t acc_pipe02_0; /* 0x48 */
    volatile unsigned int rsvd_0x4c[1];
    acc_pipe04_0_t acc_pipe04_0; /* 0x50 */
    acc_pipe05_0_t acc_pipe05_0; /* 0x54 */
    acc_pipe06_0_t acc_pipe06_0; /* 0x58 */
    volatile unsigned int rsvd_0x5c[9];
    acc_etc_rw00_0_t acc_etc_rw00_0; /* 0x80 */
    acc_etc_rw01_0_t acc_etc_rw01_0; /* 0x84 */
    acc_etc_rw02_0_t acc_etc_rw02_0; /* 0x88 */
    acc_etc_rw03_0_t acc_etc_rw03_0; /* 0x8c */
    acc_etc_rw04_0_t acc_etc_rw04_0; /* 0x90 */
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
    acc_etc_rw27_0_t acc_etc_rw27_0; /* 0xec */
    volatile unsigned int rsvd_0xf0[2];
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
    acc_etc_ro001_0_t acc_etc_ro001_0; /* 0x158 */
    acc_etc_ro002_0_t acc_etc_ro002_0; /* 0x15c */
    acc_etc_ro003_0_t acc_etc_ro003_0; /* 0x160 */
    acc_etc_ro004_0_t acc_etc_ro004_0; /* 0x164 */
    acc_etc_ro005_0_t acc_etc_ro005_0; /* 0x168 */
    acc_etc_ro006_0_t acc_etc_ro006_0; /* 0x16c */
    acc_etc_ro007_0_t acc_etc_ro007_0; /* 0x170 */
    acc_etc_ro008_0_t acc_etc_ro008_0; /* 0x174 */
    acc_etc_ro009_0_t acc_etc_ro009_0; /* 0x178 */
    acc_etc_ro010_0_t acc_etc_ro010_0; /* 0x17c */
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
    acc_etc_ro037_0_t acc_etc_ro037_0; /* 0x1e8 */
    acc_etc_ro038_0_t acc_etc_ro038_0; /* 0x1ec */
    acc_etc_ro039_0_t acc_etc_ro039_0; /* 0x1f0 */
    acc_etc_ro040_0_t acc_etc_ro040_0; /* 0x1f4 */
    acc_etc_ro041_0_t acc_etc_ro041_0; /* 0x1f8 */
    acc_etc_ro042_0_t acc_etc_ro042_0; /* 0x1fc */
    acc_etc_ro043_0_t acc_etc_ro043_0; /* 0x200 */
    acc_etc_ro044_0_t acc_etc_ro044_0; /* 0x204 */
    acc_etc_ro045_0_t acc_etc_ro045_0; /* 0x208 */
    acc_etc_ro046_0_t acc_etc_ro046_0; /* 0x20c */
    acc_etc_ro047_0_t acc_etc_ro047_0; /* 0x210 */
    acc_etc_ro048_0_t acc_etc_ro048_0; /* 0x214 */
    acc_etc_ro049_0_t acc_etc_ro049_0; /* 0x218 */
    acc_etc_ro050_0_t acc_etc_ro050_0; /* 0x21c */
    acc_etc_ro051_0_t acc_etc_ro051_0; /* 0x220 */
    acc_etc_ro052_0_t acc_etc_ro052_0; /* 0x224 */
    acc_etc_ro053_0_t acc_etc_ro053_0; /* 0x228 */
    acc_etc_ro054_0_t acc_etc_ro054_0; /* 0x22c */
    acc_etc_ro055_0_t acc_etc_ro055_0; /* 0x230 */
    acc_etc_ro056_0_t acc_etc_ro056_0; /* 0x234 */
    acc_etc_ro057_0_t acc_etc_ro057_0; /* 0x238 */
    acc_etc_ro058_0_t acc_etc_ro058_0; /* 0x23c */
    acc_etc_ro059_0_t acc_etc_ro059_0; /* 0x240 */
    acc_etc_ro060_0_t acc_etc_ro060_0; /* 0x244 */
    acc_etc_ro061_0_t acc_etc_ro061_0; /* 0x248 */
    acc_etc_ro062_0_t acc_etc_ro062_0; /* 0x24c */
    acc_etc_ro063_0_t acc_etc_ro063_0; /* 0x250 */
    acc_etc_ro064_0_t acc_etc_ro064_0; /* 0x254 */
    acc_etc_ro065_0_t acc_etc_ro065_0; /* 0x258 */
    acc_etc_ro066_0_t acc_etc_ro066_0; /* 0x25c */
    acc_etc_ro067_0_t acc_etc_ro067_0; /* 0x260 */
    acc_etc_ro068_0_t acc_etc_ro068_0; /* 0x264 */
    acc_etc_ro069_0_t acc_etc_ro069_0; /* 0x268 */
    acc_etc_ro070_0_t acc_etc_ro070_0; /* 0x26c */
    acc_etc_ro071_0_t acc_etc_ro071_0; /* 0x270 */
    acc_etc_ro072_0_t acc_etc_ro072_0; /* 0x274 */
    acc_etc_ro073_0_t acc_etc_ro073_0; /* 0x278 */
    acc_etc_ro074_0_t acc_etc_ro074_0; /* 0x27c */
    acc_etc_ro075_0_t acc_etc_ro075_0; /* 0x280 */
    acc_etc_ro076_0_t acc_etc_ro076_0; /* 0x284 */
    acc_etc_ro077_0_t acc_etc_ro077_0; /* 0x288 */
    acc_etc_ro078_0_t acc_etc_ro078_0; /* 0x28c */
    acc_etc_ro079_0_t acc_etc_ro079_0; /* 0x290 */
    acc_etc_ro080_0_t acc_etc_ro080_0; /* 0x294 */
    acc_etc_ro081_0_t acc_etc_ro081_0; /* 0x298 */
    acc_etc_ro082_0_t acc_etc_ro082_0; /* 0x29c */
    acc_etc_ro083_0_t acc_etc_ro083_0; /* 0x2a0 */
    acc_etc_ro084_0_t acc_etc_ro084_0; /* 0x2a4 */
    acc_etc_ro085_0_t acc_etc_ro085_0; /* 0x2a8 */
    acc_etc_ro086_0_t acc_etc_ro086_0; /* 0x2ac */
    acc_etc_ro087_0_t acc_etc_ro087_0; /* 0x2b0 */
    acc_etc_ro088_0_t acc_etc_ro088_0; /* 0x2b4 */
    acc_etc_ro089_0_t acc_etc_ro089_0; /* 0x2b8 */
    acc_etc_ro090_0_t acc_etc_ro090_0; /* 0x2bc */
    acc_etc_ro091_0_t acc_etc_ro091_0; /* 0x2c0 */
    acc_etc_ro092_0_t acc_etc_ro092_0; /* 0x2c4 */
    acc_etc_ro093_0_t acc_etc_ro093_0; /* 0x2c8 */
    acc_etc_ro094_0_t acc_etc_ro094_0; /* 0x2cc */
    acc_etc_ro095_0_t acc_etc_ro095_0; /* 0x2d0 */
    acc_etc_ro096_0_t acc_etc_ro096_0; /* 0x2d4 */
    acc_etc_ro097_0_t acc_etc_ro097_0; /* 0x2d8 */
    acc_etc_ro098_0_t acc_etc_ro098_0; /* 0x2dc */
    acc_etc_ro099_0_t acc_etc_ro099_0; /* 0x2e0 */
    acc_etc_ro100_0_t acc_etc_ro100_0; /* 0x2e4 */
    acc_etc_ro101_0_t acc_etc_ro101_0; /* 0x2e8 */
    acc_etc_ro102_0_t acc_etc_ro102_0; /* 0x2ec */
    acc_etc_ro103_0_t acc_etc_ro103_0; /* 0x2f0 */
    acc_etc_ro104_0_t acc_etc_ro104_0; /* 0x2f4 */
    acc_etc_ro105_0_t acc_etc_ro105_0; /* 0x2f8 */
    acc_etc_ro106_0_t acc_etc_ro106_0; /* 0x2fc */
    acc_etc_ro107_0_t acc_etc_ro107_0; /* 0x300 */
    acc_etc_ro108_0_t acc_etc_ro108_0; /* 0x304 */
    acc_etc_ro109_0_t acc_etc_ro109_0; /* 0x308 */
    acc_etc_ro110_0_t acc_etc_ro110_0; /* 0x30c */
    acc_etc_ro111_0_t acc_etc_ro111_0; /* 0x310 */
    acc_etc_ro112_0_t acc_etc_ro112_0; /* 0x314 */
    acc_etc_ro113_0_t acc_etc_ro113_0; /* 0x318 */
    acc_etc_ro114_0_t acc_etc_ro114_0; /* 0x31c */
    acc_etc_ro115_0_t acc_etc_ro115_0; /* 0x320 */
    acc_etc_ro116_0_t acc_etc_ro116_0; /* 0x324 */
    acc_etc_ro117_0_t acc_etc_ro117_0; /* 0x328 */
    acc_etc_ro118_0_t acc_etc_ro118_0; /* 0x32c */
    acc_etc_ro119_0_t acc_etc_ro119_0; /* 0x330 */
    acc_etc_ro120_0_t acc_etc_ro120_0; /* 0x334 */
    acc_etc_ro121_0_t acc_etc_ro121_0; /* 0x338 */
    acc_etc_ro122_0_t acc_etc_ro122_0; /* 0x33c */
    acc_etc_ro123_0_t acc_etc_ro123_0; /* 0x340 */
    acc_etc_ro124_0_t acc_etc_ro124_0; /* 0x344 */
    acc_etc_ro125_0_t acc_etc_ro125_0; /* 0x348 */
    acc_etc_ro126_0_t acc_etc_ro126_0; /* 0x34c */
    acc_etc_ro127_0_t acc_etc_ro127_0; /* 0x350 */
    acc_etc_ro128_0_t acc_etc_ro128_0; /* 0x354 */
    acc_etc_ro129_0_t acc_etc_ro129_0; /* 0x358 */
    acc_etc_ro130_0_t acc_etc_ro130_0; /* 0x35c */
    acc_etc_ro131_0_t acc_etc_ro131_0; /* 0x360 */
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
    acc_etc_ro152_0_t acc_etc_ro152_0; /* 0x3b4 */
    acc_etc_ro153_0_t acc_etc_ro153_0; /* 0x3b8 */
    acc_etc_ro154_0_t acc_etc_ro154_0; /* 0x3bc */
    acc_etc_ro155_0_t acc_etc_ro155_0; /* 0x3c0 */
    acc_etc_ro156_0_t acc_etc_ro156_0; /* 0x3c4 */
    volatile unsigned int rsvd_0x3c8[14];
    acc_ras_rw00_0_t acc_ras_rw00_0; /* 0x400 */
    acc_ras_st_bus_ro00_0_t acc_ras_st_bus_ro00_0; /* 0x404 */
    acc_ras_st_bus_ro00_1_t acc_ras_st_bus_ro00_1; /* 0x408 */
    acc_ras_st_bus_ro00_2_t acc_ras_st_bus_ro00_2; /* 0x40c */
    acc_ras_st_bus_ro00_3_t acc_ras_st_bus_ro00_3; /* 0x410 */
    acc_ras_st_bus_ro00_4_t acc_ras_st_bus_ro00_4; /* 0x414 */
    acc_ras_st_bus_ro00_5_t acc_ras_st_bus_ro00_5; /* 0x418 */
    acc_ras_st_bus_ro00_6_t acc_ras_st_bus_ro00_6; /* 0x41c */
    acc_ras_st_bus_ro00_7_t acc_ras_st_bus_ro00_7; /* 0x420 */
    acc_ras_st_bus_ro00_8_t acc_ras_st_bus_ro00_8; /* 0x424 */
    acc_ras_st_bus_ro00_9_t acc_ras_st_bus_ro00_9; /* 0x428 */
    acc_ras_st_bus_ro00_10_t acc_ras_st_bus_ro00_10; /* 0x42c */
    acc_ras_st_bus_ro00_11_t acc_ras_st_bus_ro00_11; /* 0x430 */
    acc_ras_st_bus_ro00_12_t acc_ras_st_bus_ro00_12; /* 0x434 */
    acc_ras_st_bus_ro00_13_t acc_ras_st_bus_ro00_13; /* 0x438 */
    acc_ras_st_bus_ro00_14_t acc_ras_st_bus_ro00_14; /* 0x43c */
    acc_ras_st_bus_ro00_15_t acc_ras_st_bus_ro00_15; /* 0x440 */
    acc_ras_st_bus_ro00_16_t acc_ras_st_bus_ro00_16; /* 0x444 */
    acc_ras_st_bus_ro00_17_t acc_ras_st_bus_ro00_17; /* 0x448 */
    acc_ras_st_bus_ro00_18_t acc_ras_st_bus_ro00_18; /* 0x44c */
    acc_ras_st_bus_ro00_19_t acc_ras_st_bus_ro00_19; /* 0x450 */
    acc_ras_st_bus_ro00_20_t acc_ras_st_bus_ro00_20; /* 0x454 */
    acc_ras_st_bus_ro00_21_t acc_ras_st_bus_ro00_21; /* 0x458 */
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
    acc_ras_sd_lxx_ro07_0_t acc_ras_sd_lxx_ro07_0; /* 0x4a4 */
    acc_ras_sd_lxx_ro07_1_t acc_ras_sd_lxx_ro07_1; /* 0x4a8 */
    acc_ras_sd_lxx_ro07_2_t acc_ras_sd_lxx_ro07_2; /* 0x4ac */
    acc_ras_sd_lxx_ro07_3_t acc_ras_sd_lxx_ro07_3; /* 0x4b0 */
    acc_ras_sd_lxx_ro07_4_t acc_ras_sd_lxx_ro07_4; /* 0x4b4 */
    acc_ras_sd_lxx_ro07_5_t acc_ras_sd_lxx_ro07_5; /* 0x4b8 */
    acc_ras_sd_lxx_ro07_6_t acc_ras_sd_lxx_ro07_6; /* 0x4bc */
    acc_ras_sd_lxx_ro14_0_t acc_ras_sd_lxx_ro14_0; /* 0x4c0 */
    acc_ras_sd_lxx_ro14_1_t acc_ras_sd_lxx_ro14_1; /* 0x4c4 */
    acc_ras_sd_lxx_ro14_2_t acc_ras_sd_lxx_ro14_2; /* 0x4c8 */
    acc_ras_sd_lxx_ro14_3_t acc_ras_sd_lxx_ro14_3; /* 0x4cc */
    acc_ras_sd_lxx_ro14_4_t acc_ras_sd_lxx_ro14_4; /* 0x4d0 */
    acc_ras_sd_lxx_ro14_5_t acc_ras_sd_lxx_ro14_5; /* 0x4d4 */
    acc_ras_sd_lxx_ro14_6_t acc_ras_sd_lxx_ro14_6; /* 0x4d8 */
    acc_ras_sd_lxx_ro21_0_t acc_ras_sd_lxx_ro21_0; /* 0x4dc */
    acc_ras_sd_lxx_ro21_1_t acc_ras_sd_lxx_ro21_1; /* 0x4e0 */
    acc_ras_sd_lxx_ro21_2_t acc_ras_sd_lxx_ro21_2; /* 0x4e4 */
    acc_ras_sd_lxx_ro21_3_t acc_ras_sd_lxx_ro21_3; /* 0x4e8 */
    acc_ras_sd_lxx_ro21_4_t acc_ras_sd_lxx_ro21_4; /* 0x4ec */
    acc_ras_sd_lxx_ro21_5_t acc_ras_sd_lxx_ro21_5; /* 0x4f0 */
    acc_ras_sd_lxx_ro21_6_t acc_ras_sd_lxx_ro21_6; /* 0x4f4 */
    acc_ras_sd_vxx_ro00_0_t acc_ras_sd_vxx_ro00_0; /* 0x4f8 */
    acc_ras_sd_vxx_ro00_1_t acc_ras_sd_vxx_ro00_1; /* 0x4fc */
    acc_ras_sd_vxx_ro00_2_t acc_ras_sd_vxx_ro00_2; /* 0x500 */
    acc_ras_sd_vxx_ro00_3_t acc_ras_sd_vxx_ro00_3; /* 0x504 */
    acc_ras_sd_vxx_ro00_4_t acc_ras_sd_vxx_ro00_4; /* 0x508 */
    acc_ras_sd_vxx_ro00_5_t acc_ras_sd_vxx_ro00_5; /* 0x50c */
    acc_ras_sd_vxx_ro00_6_t acc_ras_sd_vxx_ro00_6; /* 0x510 */
    acc_ras_sd_vxx_ro00_7_t acc_ras_sd_vxx_ro00_7; /* 0x514 */
    acc_ras_sd_vxx_ro08_0_t acc_ras_sd_vxx_ro08_0; /* 0x518 */
    acc_ras_sd_vxx_ro08_1_t acc_ras_sd_vxx_ro08_1; /* 0x51c */
    acc_ras_sd_vxx_ro08_2_t acc_ras_sd_vxx_ro08_2; /* 0x520 */
    acc_ras_sd_vxx_ro08_3_t acc_ras_sd_vxx_ro08_3; /* 0x524 */
    acc_ras_sd_vxx_ro08_4_t acc_ras_sd_vxx_ro08_4; /* 0x528 */
    acc_ras_sd_vxx_ro08_5_t acc_ras_sd_vxx_ro08_5; /* 0x52c */
    acc_ras_sd_vxx_ro08_6_t acc_ras_sd_vxx_ro08_6; /* 0x530 */
    acc_ras_sd_vxx_ro08_7_t acc_ras_sd_vxx_ro08_7; /* 0x534 */
    acc_ras_sd_vxx_ro16_0_t acc_ras_sd_vxx_ro16_0; /* 0x538 */
    acc_ras_sd_vxx_ro16_1_t acc_ras_sd_vxx_ro16_1; /* 0x53c */
    acc_ras_sd_vxx_ro16_2_t acc_ras_sd_vxx_ro16_2; /* 0x540 */
    acc_ras_sd_vxx_ro16_3_t acc_ras_sd_vxx_ro16_3; /* 0x544 */
    acc_ras_sd_vxx_ro16_4_t acc_ras_sd_vxx_ro16_4; /* 0x548 */
    acc_ras_sd_vxx_ro16_5_t acc_ras_sd_vxx_ro16_5; /* 0x54c */
    acc_ras_sd_vxx_ro16_6_t acc_ras_sd_vxx_ro16_6; /* 0x550 */
    acc_ras_sd_vxx_ro16_7_t acc_ras_sd_vxx_ro16_7; /* 0x554 */
    acc_ras_sd_vxx_ro24_0_t acc_ras_sd_vxx_ro24_0; /* 0x558 */
    acc_ras_sd_vxx_ro24_1_t acc_ras_sd_vxx_ro24_1; /* 0x55c */
    acc_ras_sd_vxx_ro24_2_t acc_ras_sd_vxx_ro24_2; /* 0x560 */
    acc_ras_sd_vxx_ro24_3_t acc_ras_sd_vxx_ro24_3; /* 0x564 */
    acc_ras_sd_vxx_ro24_4_t acc_ras_sd_vxx_ro24_4; /* 0x568 */
    acc_ras_sd_vxx_ro24_5_t acc_ras_sd_vxx_ro24_5; /* 0x56c */
    acc_ras_sd_vxx_ro24_6_t acc_ras_sd_vxx_ro24_6; /* 0x570 */
    acc_ras_sd_vxx_ro24_7_t acc_ras_sd_vxx_ro24_7; /* 0x574 */
    acc_ras_tba_com_ro00_0_t acc_ras_tba_com_ro00_0; /* 0x578 */
    acc_rbar_ctl_ro00_0_t acc_rbar_ctl_ro00_0; /* 0x57c */
    acc_rbar_ro00_0_t acc_rbar_ro00_0; /* 0x580 */
    acc_rbar_ro01_0_t acc_rbar_ro01_0; /* 0x584 */
    acc_rbar_ro02_0_t acc_rbar_ro02_0; /* 0x588 */
    acc_rbar_ro04_0_t acc_rbar_ro04_0; /* 0x58c */
    acc_rbar_vf_ctl_ro00_0_t acc_rbar_vf_ctl_ro00_0; /* 0x590 */
    acc_rbar_vf_ro00_0_t acc_rbar_vf_ro00_0; /* 0x594 */
    acc_rbar_vf_ro01_0_t acc_rbar_vf_ro01_0; /* 0x598 */
    acc_rbar_vf_ro02_0_t acc_rbar_vf_ro02_0; /* 0x59c */
    acc_rbar_vf_ro03_0_t acc_rbar_vf_ro03_0; /* 0x5a0 */
    acc_rbar_vf_ro04_0_t acc_rbar_vf_ro04_0; /* 0x5a4 */
    acc_rbar_ro03_0_t acc_rbar_ro03_0; /* 0x5a8 */
    volatile unsigned int rsvd_0x5ac[5];
    acc_edge_intr_en0_0_t acc_edge_intr_en0_0; /* 0x5c0 */
    acc_edge_intr_en1_0_t acc_edge_intr_en1_0; /* 0x5c4 */
    acc_edge_intr_en2_0_t acc_edge_intr_en2_0; /* 0x5c8 */
    acc_edge_intr_en3_0_t acc_edge_intr_en3_0; /* 0x5cc */
    acc_trig_intr_en0_0_t acc_trig_intr_en0_0; /* 0x5d0 */
    acc_trig_intr_en1_0_t acc_trig_intr_en1_0; /* 0x5d4 */
    acc_trig_intr_en2_0_t acc_trig_intr_en2_0; /* 0x5d8 */
    acc_trig_intr_en3_0_t acc_trig_intr_en3_0; /* 0x5dc */
    acc_levl_intr_en0_0_t acc_levl_intr_en0_0; /* 0x5e0 */
    acc_levl_intr_en1_0_t acc_levl_intr_en1_0; /* 0x5e4 */
    acc_levl_intr_en2_0_t acc_levl_intr_en2_0; /* 0x5e8 */
    acc_levl_intr_en3_0_t acc_levl_intr_en3_0; /* 0x5ec */
    volatile unsigned int rsvd_0x5f0[4];
    acc_intr_st_part_0_t acc_intr_st_part_0; /* 0x600 */
    acc_edge_intr_st0_0_t acc_edge_intr_st0_0; /* 0x604 */
    acc_edge_intr_st1_0_t acc_edge_intr_st1_0; /* 0x608 */
    acc_edge_intr_st2_0_t acc_edge_intr_st2_0; /* 0x60c */
    acc_edge_intr_st3_0_t acc_edge_intr_st3_0; /* 0x610 */
    acc_trig_intr_st0_0_t acc_trig_intr_st0_0; /* 0x614 */
    acc_trig_intr_st1_0_t acc_trig_intr_st1_0; /* 0x618 */
    acc_trig_intr_st2_0_t acc_trig_intr_st2_0; /* 0x61c */
    acc_trig_intr_st3_0_t acc_trig_intr_st3_0; /* 0x620 */
    acc_levl_intr_st0_0_t acc_levl_intr_st0_0; /* 0x624 */
    acc_levl_intr_st1_0_t acc_levl_intr_st1_0; /* 0x628 */
    acc_levl_intr_st2_0_t acc_levl_intr_st2_0; /* 0x62c */
    acc_levl_intr_st3_0_t acc_levl_intr_st3_0; /* 0x630 */
    volatile unsigned int rsvd_0x634[3];
    acc_edge_intr_cl0_0_t acc_edge_intr_cl0_0; /* 0x640 */
    acc_edge_intr_cl1_0_t acc_edge_intr_cl1_0; /* 0x644 */
    acc_edge_intr_cl2_0_t acc_edge_intr_cl2_0; /* 0x648 */
    acc_edge_intr_cl3_0_t acc_edge_intr_cl3_0; /* 0x64c */
    acc_trig_intr_cl0_0_t acc_trig_intr_cl0_0; /* 0x650 */
    acc_trig_intr_cl1_0_t acc_trig_intr_cl1_0; /* 0x654 */
    acc_trig_intr_cl2_0_t acc_trig_intr_cl2_0; /* 0x658 */
    acc_trig_intr_cl3_0_t acc_trig_intr_cl3_0; /* 0x65c */
} pcie_hif_reg_t;

#endif /* __DX_PCIE_HIF_H */