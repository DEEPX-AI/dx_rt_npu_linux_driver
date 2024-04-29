/* Automatically generated files */
/* registers are not duplicated and register unit : 32bits */

#ifndef __PCIE_AXI_RAS_H
#define __PCIE_AXI_RAS_H
/* Address */
#define PCIE0_AXI_RAS_BASE     0xc900030c
#define PCIE1_AXI_RAS_BASE     0xcb00030c

/* Register Definition */
typedef volatile union _RAS_DES_CAP_HEADER_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EXTENDED_CAP_ID:16;
        /* Default Value : 0xb, TYPE : read-write, Position : 0 */
        /* Description :
        PCI Express Extended Capability ID.
This field is a PCI-SIG defined ID number that indicates the nature and format of the Extended Capability.
Extended Capability ID for the Vendor-Specific Extended Capability is 000Bh.


Note: The access attributes of this field are as follows:
 - Wire: R (sticky) 
 - Dbi: if (DBI_RO_WR_EN == 1) then R/W(sticky) else R(sticky) 
Note: This register field is sticky.
        */
        volatile unsigned int CAP_VERSION:4;
        /* Default Value : 0x1, TYPE : read-write, Position : 16 */
        /* Description :
        Capability Version.
This field is a PCI-SIG defined version number that indicates the version of the Capability structure present.
Value of this field is depends on the version of the specification.


Note: The access attributes of this field are as follows:
 - Wire: R (sticky) 
 - Dbi: if (DBI_RO_WR_EN == 1) then R/W(sticky) else R(sticky) 
Note: This register field is sticky.
        */
        volatile unsigned int NEXT_OFFSET:12;
        /* Default Value : 0x40c, TYPE : read-write, Position : 20 */
        /* Description :
        Next Capability Offset.
This field contains the offset to the next PCI Express Capability structure or 000h if no other items exist in the linked list of Capabilities.
For Extended Capabilities implemented in Configuration Space, this offset is relative to the beginning of PCI-compatible Configuration Space and thus must always be either 000h (for terminating list of Capabilities) or greater than 0FFh.


Note: The access attributes of this field are as follows:
 - Wire: R (sticky) 
 - Dbi: if (DBI_RO_WR_EN == 1) then R/W(sticky) else R(sticky) 
Note: This register field is sticky.
        */
    };
} RAS_DES_CAP_HEADER_REG_t;

typedef volatile union _VENDOR_SPECIFIC_HEADER_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int VSEC_ID:16;
        /* Default Value : 0x2, TYPE : read-only, Position : 0 */
        /* Description :
        VSEC ID.
This field is a vendor-defined ID number that indicates the nature and format of the VSEC structure.
Software must qualify the Vendor ID before interpreting this field.
        */
        volatile unsigned int VSEC_REV:4;
        /* Default Value : 0x4, TYPE : read-only, Position : 16 */
        /* Description :
        VSEC Rev.
This field is a vendor-defined version number that indicates the version of the VSEC structure.
Software must qualify the Vendor ID and VSEC ID before interpreting this field.
        */
        volatile unsigned int VSEC_LENGTH:12;
        /* Default Value : 0x100, TYPE : read-only, Position : 20 */
        /* Description :
        VSEC Length.
This field indicates the number of bytes in the entire VSEC structure, including the Vendor-Specific Extended Capability Header, the Vendor-Specific Header, and the vendor-specific registers.
        */
    };
} VENDOR_SPECIFIC_HEADER_REG_t;

typedef volatile union _EVENT_COUNTER_CONTROL_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EVENT_COUNTER_CLEAR:2;
        /* Default Value : 0x0, TYPE : write-only, Position : 0 */
        /* Description :
        Event Counter Clear.
Clears the Event Counter selected by the EVENT_COUNTER_EVENT_SELECT and EVENT_COUNTER_LANE_SELECT fields in this register.
You can clear the value of a specific Event Counter by writing the 'per clear' code and you can clear all event counters at once by writing the 'all clear' code.
The read value is always '0'.
Other values are reserved.
        */
        volatile unsigned int EVENT_COUNTER_ENABLE:3;
        /* Default Value : 0x0, TYPE : write-only, Position : 2 */
        /* Description :
        Event Counter Enable.
Enables/disables the Event Counter selected by the EVENT_COUNTER_EVENT_SELECT and EVENT_COUNTER_LANE_SELECT fields in this register.
By default, all event counters are disabled.
You can enable/disable a specific Event Counter by writing the 'per event off' or 'per event on' codes.
You can enable/disable all event counters by writing the 'all on' or 'all off' codes.
The read value is always '0'.
For other values no change.
        */
        volatile unsigned int RSVDP_5:2;
        /* Default Value : x, TYPE : U, Position : 5 */
        /* Description :
        Reserved for future use.
        */
        volatile unsigned int EVENT_COUNTER_STATUS:1;
        /* Default Value : 0x0, TYPE : read-only, Position : 7 */
        /* Description :
        Event Counter Status.
This register returns the current value of the Event Counter selected by the following fields:
 - EVENT_COUNTER_EVENT_SELECT
 - EVENT_COUNTER_LANE_SELECT


Note: This register field is sticky.
        */
        volatile unsigned int EVENT_COUNTER_LANE_SELECT:4;
        /* Default Value : 0x0, TYPE : read-write, Position : 8 */
        /* Description :
        Event Counter Lane Select.
This field in conjunction with EVENT_COUNTER_EVENT_SELECT indexes the Event Counter data returned by the EVENT_COUNTER_DATA_REG register.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_12:4;
        /* Default Value : x, TYPE : U, Position : 12 */
        /* Description :
        Reserved for future use.
        */
        volatile unsigned int EVENT_COUNTER_EVENT_SELECT:12;
        /* Default Value : 0x0, TYPE : read-write, Position : 16 */
        /* Description :
        Event Counter Data Select.
This field in conjunction with the EVENT_COUNTER_LANE_SELECT field indexes the Event Counter data returned by the EVENT_COUNTER_DATA_REG register.
 - 27-24: Group number(4-bit: 0..0x7)
 - 23-16: Event number(8-bit: 0..0x13) within the Group
 - ..
For detailed definitions of Group number and Event number, see the RAS DES chapter in the Databook.

Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_28:4;
        /* Default Value : x, TYPE : U, Position : 28 */
        /* Description :
        Reserved for future use.
        */
    };
} EVENT_COUNTER_CONTROL_REG_t;

typedef volatile union _EVENT_COUNTER_DATA_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EVENT_COUNTER_DATA:32;
        /* Default Value : 0x0, TYPE : read-only, Position : 0 */
        /* Description :
        Event Counter Data.
This register returns the data selected by the following fields:
 - EVENT_COUNTER_EVENT_SELECT in EVENT_COUNTER_CONTROL_REG


Note: This register field is sticky.
        */
    };
} EVENT_COUNTER_DATA_REG_t;

typedef volatile union _TIME_BASED_ANALYSIS_CONTROL_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int TIMER_START:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Timer Start.
This bit will be cleared automatically when the measurement is finished.
Note: The app_ras_des_tba_ctrl input also sets the contents of this field and controls the measurement start/stop.

Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_1:7;
        /* Default Value : x, TYPE : U, Position : 1 */
        /* Description :
        Reserved for future use.
        */
        volatile unsigned int TIME_BASED_DURATION_SELECT:8;
        /* Default Value : 0x1, TYPE : read-write, Position : 8 */
        /* Description :
        Time-based Duration Select.
Selects the duration of time-based analysis.
When "manual control" is selected and TIMER_START is set to '1', this analysis never stops until TIMER_STOP is set to '0'. All encodings other than the defined encodings are reserved.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_16:8;
        /* Default Value : x, TYPE : U, Position : 16 */
        /* Description :
        Reserved for future use.
        */
        volatile unsigned int TIME_BASED_REPORT_SELECT:8;
        /* Default Value : 0x0, TYPE : read-write, Position : 24 */
        /* Description :
        Time-based Report Select.
Selects what type of data is measured for the selected duration (TIME_BASED_DURATION_SELECT), and returned in TIME_BASED_ANALYSIS_DATA.
Each type of data is measured using one of three types of units:
 - Core_clk Cycles for 2.5GT/s, 5.0GT/s, 8.0GT/s, 16.0GT/s, 32GT/s. Total time in ps is [Value of TIME_BASED_ANALYSIS_DATA returned when TIME_BASED_REPORT_SELECT=0x00] * TIME_BASED_ANALYSIS_DATA. Values 0-4 and 7-8 correspond to Core_clk Cycles for 2.5GT/s, 5.0GT/s, 8.0GT/s, 16.0GT/s, 32GT/s.
 - Aux_clk Cycles. Total time in ps is [Period of platform specific clock] * TIME_BASED_ANALYSIS_DATA. Values 5, 6, and 9 correspond to aux_clk Cycles.
 - Core_clk Cycles for 20GT/s, 25GT/s (CCIX ESM data rate). Total time in ps is [Value of TIME_BASED_ANALYSIS_DATA returned when TIME_BASED_REPORT_SELECT=0x10] * TIME_BASED_ANALYSIS_DATA. Values 10-14 and 17-18 correspond to Core_clk Cycles for 2.5GT/s, 5.0GT/s, 8.0GT/s, 16.0GT/s, 32GT/s.
 - Data Bytes. Actual amount of bytes is 16 * TIME_BASED_ANALYSIS_DATA. Values 20-23 correspond to data bytes.
All encodings other than the defined encodings are reserved.


Note: This register field is sticky.
        */
    };
} TIME_BASED_ANALYSIS_CONTROL_REG_t;

typedef volatile union _TIME_BASED_ANALYSIS_DATA_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int TIME_BASED_ANALYSIS_DATA:32;
        /* Default Value : 0x0, TYPE : read-only, Position : 0 */
        /* Description :
        Time Based Analysis Data.
This register returns the data selected by the TIME_BASED_REPORT_SELECT field in TIME_BASED_ANALYSIS_CONTROL_REG.
The results are cleared when next measurement starts.


Note: This register field is sticky.
        */
    };
} TIME_BASED_ANALYSIS_DATA_REG_t;

typedef volatile union _TIME_BASED_ANALYSIS_DATA_63_32_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int TIME_BASED_ANALYSIS_DATA_63_32:32;
        /* Default Value : 0x0, TYPE : read-only, Position : 0 */
        /* Description :
        Upper 32 bits of Time Based Analysis Data.


Note: This register field is sticky.
        */
    };
} TIME_BASED_ANALYSIS_DATA_63_32_REG_t;

typedef volatile union _EINJ_ENABLE_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int ERROR_INJECTION0_ENABLE:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Error Injection0 Enable (CRC Error). Enables insertion of errors into various CRC. For more information, see the EINJ0_CRC_REG register.


Note: This register field is sticky.
        */
        volatile unsigned int ERROR_INJECTION1_ENABLE:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 1 */
        /* Description :
        Error Injection1 Enable (Sequence Number Error).
Enables insertion of errors into sequence numbers.
For more information, see the EINJ1_SEQNUM_REG register.


Note: This register field is sticky.
        */
        volatile unsigned int ERROR_INJECTION2_ENABLE:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 2 */
        /* Description :
        Error Injection2 Enable (DLLP Error).
Enables insertion of DLLP errors.
For more information, see the EINJ2_DLLP_REG register.


Note: This register field is sticky.
        */
        volatile unsigned int ERROR_INJECTION3_ENABLE:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 3 */
        /* Description :
        Error Injection3 Enable (Symbol DataK Mask Error or Sync Header Error).
Enables DataK masking of special symbols or the breaking of the sync header.
For more information, see the EINJ3_SYMBOL_REG register.


Note: This register field is sticky.
        */
        volatile unsigned int ERROR_INJECTION4_ENABLE:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 4 */
        /* Description :
        Error Injection4 Enable (FC Credit Update Error).
Enables insertion of errors into UpdateFCs.
For more information, see the EINJ4_FC_REG register.


Note: This register field is sticky.
        */
        volatile unsigned int ERROR_INJECTION5_ENABLE:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 5 */
        /* Description :
        Error Injection5 Enable (TLP Duplicate/Nullify Error).
Enables insertion of duplicate/nullified TLPs.
For more information, see the EINJ5_SP_TLP_REG register.


Note: This register field is sticky.
        */
        volatile unsigned int ERROR_INJECTION6_ENABLE:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 6 */
        /* Description :
        Error Injection6 Enable (Specific TLP Error).
Enables insertion of errors into the packets that you select.

You can set this bit to '1' when you have disabled RAS datapath protection (DP) by setting CX_RASDP = CX_RASDP_RAM_PROT =0.

You can set this bit to '1' when you have disabled the address translation by setting ADDR_TRANSLATION_SUPPORT_EN=0.
For more information, see the EINJ6_COMPARE_*_REG/EINJ6_CHANGE_*_REG/EINJ6_TLP_REG registers.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_7:25;
        /* Default Value : x, TYPE : U, Position : 7 */
        /* Description :
        Reserved for future use.
        */
    };
} EINJ_ENABLE_REG_t;

typedef volatile union _EINJ0_CRC_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EINJ0_COUNT:8;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Error injection count.
Indicates the number of errors.
This register is decremented when the errors have been inserted.

 If the counter value is 0x01 and error is inserted, ERROR_INJECTION0_ENABLE in EINJ_ENABLE_REG returns 0b.

 If the counter value is 0x00 and ERROR_INJECTION0_ENABLE=1, the errors are inserted until ERROR_INJECTION0_ENABLE is set to '0'.


Note: This register field is sticky.
        */
        volatile unsigned int EINJ0_CRC_TYPE:4;
        /* Default Value : 0x0, TYPE : read-write, Position : 8 */
        /* Description :
        Error injection type.
Selects the type of CRC error to be inserted. All encodings other than the defined encodings are reserved.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_12:20;
        /* Default Value : x, TYPE : U, Position : 12 */
        /* Description :
        Reserved for future use.
        */
    };
} EINJ0_CRC_REG_t;

typedef volatile union _EINJ1_SEQNUM_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EINJ1_COUNT:8;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Error injection count.
Indicates the number of errors.
This register is decremented as the errors are being inserted.

 If the counter value is 0x01 and error is inserted, ERROR_INJECTION1_ENABLE in EINJ_ENABLE_REG returns '0'.

 If the counter value is 0x00 and ERROR_INJECTION1_ENABLE=1, the errors are inserted until ERROR_INJECTION1_ENABLE is set to '0'.


Note: This register field is sticky.
        */
        volatile unsigned int EINJ1_SEQNUM_TYPE:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 8 */
        /* Description :
        Sequence number type.
Selects the type of sequence number.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_9:7;
        /* Default Value : x, TYPE : U, Position : 9 */
        /* Description :
        Reserved for future use.
        */
        volatile unsigned int EINJ1_BAD_SEQNUM:13;
        /* Default Value : 0x0, TYPE : read-write, Position : 16 */
        /* Description :
        Bad sequence number.
Indicates the value to add/subtract from the naturally-assigned sequence numbers. This value is represented by two's complement.
For example:
 - Set Type, SEQ# and Count
 -- EINJ1_SEQNUM_TYPE =0 (Insert errors into new TLPs)
 -- EINJ1_BAD_SEQNUM =0x1FFD (represents -3)
 -- EINJ1_COUNT =1
 - Enable Error Injection
 -- ERROR_INJECTION1_ENABLE =1
 - Send a TLP from the controller's Application Interface
 -- Assume SEQ#5 is given to the TLP.
 - The SEQ# is Changed to #2 by the Error Injection Function in Layer2.
 -- 5 + (-3) = 2
 - The TLP with SEQ#2 is Transmitted to PCIe Link.
Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_29:3;
        /* Default Value : x, TYPE : U, Position : 29 */
        /* Description :
        Reserved for future use.
        */
    };
} EINJ1_SEQNUM_REG_t;

typedef volatile union _EINJ2_DLLP_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EINJ2_COUNT:8;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Error injection count.
Indicates the number of errors.
This register is decremented as the errors are being inserted.

 If the counter value is 0x01 and the error is inserted, ERROR_INJECTION2_ENABLE in EINJ_ENABLE_REG returns '0'.

 If the counter value is 0x00 and ERROR_INJECTION2_ENABLE =1, the errors are inserted until ERROR_INJECTION2_ENABLE is set to '0'.
This register is affected only when EINJ2_DLLP_TYPE =2'10b.


Note: This register field is sticky.
        */
        volatile unsigned int EINJ2_DLLP_TYPE:2;
        /* Default Value : 0x0, TYPE : read-write, Position : 8 */
        /* Description :
        DLLP Type.
Selects the type of DLLP errors to be inserted.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_10:22;
        /* Default Value : x, TYPE : U, Position : 10 */
        /* Description :
        Reserved for future use.
        */
    };
} EINJ2_DLLP_REG_t;

typedef volatile union _EINJ3_SYMBOL_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EINJ3_COUNT:8;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Error injection count.
Indicates the number of errors.
This register is decremented as the errors are being inserted.

 If the counter value is 0x01 and error is inserted, ERROR_INJECTION3_ENABLE in EINJ_ENABLE_REG returns '0'.

 If the counter value is 0x00 and ERROR_INJECTION3_ENABLE =1, the errors are inserted until ERROR_INJECTION3_ENABLE is set to '0'.


Note: This register field is sticky.
        */
        volatile unsigned int EINJ3_SYMBOL_TYPE:3;
        /* Default Value : 0x0, TYPE : read-write, Position : 8 */
        /* Description :
        Error Type.
8b/10b encoding - Mask K symbol. It is not supported to insert errors into the first ordered-set after exiting from TxElecIdle when CX_FREQ_STEP_EN has been enabled. All encodings other than the defined encodings are reserved.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_11:21;
        /* Default Value : x, TYPE : U, Position : 11 */
        /* Description :
        Reserved for future use.
        */
    };
} EINJ3_SYMBOL_REG_t;

typedef volatile union _EINJ4_FC_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EINJ4_COUNT:8;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Error injection count.
Indicates the number of errors.
This register is decremented as the errors are being inserted.

 If the counter value is 0x01 and error is inserted, ERROR_INJECTION4_ENABLE in EINJ_ENABLE_REG returns '0'.

 If the counter value is 0x00 and ERROR_INJECTION4_ENABLE =1, the errors are inserted until ERROR_INJECTION4_ENABLE is set to '0'.


Note: This register field is sticky.
        */
        volatile unsigned int EINJ4_UPDFC_TYPE:3;
        /* Default Value : 0x0, TYPE : read-write, Position : 8 */
        /* Description :
        Update-FC type.
Selects the credit type.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_11:1;
        /* Default Value : x, TYPE : U, Position : 11 */
        /* Description :
        Reserved for future use.
        */
        volatile unsigned int EINJ4_VC_NUMBER:3;
        /* Default Value : 0x0, TYPE : read-write, Position : 12 */
        /* Description :
        VC Number.
Indicates target VC Number.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_15:1;
        /* Default Value : x, TYPE : U, Position : 15 */
        /* Description :
        Reserved for future use.
        */
        volatile unsigned int EINJ4_BAD_UPDFC_VALUE:13;
        /* Default Value : 0x0, TYPE : read-write, Position : 16 */
        /* Description :
        Bad update-FC credit value.
Indicates the value to add/subtract from the UpdateFC credit. This value is represented by two's complement.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_29:3;
        /* Default Value : x, TYPE : U, Position : 29 */
        /* Description :
        Reserved for future use.
        */
    };
} EINJ4_FC_REG_t;

typedef volatile union _EINJ5_SP_TLP_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EINJ5_COUNT:8;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Error injection count.
Indicates the number of errors.
This register is decremented as the errors are being inserted.

 If the counter value is 0x01 and error is inserted, ERROR_INJECTION5_ENABLE in EINJ_ENABLE_REG returns '0'.

 If the counter value is 0x00 and ERROR_INJECTION5_ENABLE =1, the errors are inserted until ERROR_INJECTION5_ENABLE is set to '0'.


Note: This register field is sticky.
        */
        volatile unsigned int EINJ5_SPECIFIED_TLP:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 8 */
        /* Description :
        Specified TLP.
Selects the specified TLP to be inserted.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_9:23;
        /* Default Value : x, TYPE : U, Position : 9 */
        /* Description :
        Reserved for future use.
        */
    };
} EINJ5_SP_TLP_REG_t;

typedef volatile union _EINJ6_COMPARE_POINT_H0_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EINJ6_COMPARE_POINT_H0:32;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Packet Compare Point: 1st DWORD.

Specifies which Tx TLP header DWORD#0 bits to compare with the corresponding bits in the Packet Compare Value registers (EINJ6_COMPARE_VALUE*).

When all specified bits (in the Tx TLP header and EINJ6_COMPARE_VALUE*) match, the controller inserts errors into the TLP.


Note: This register field is sticky.
        */
    };
} EINJ6_COMPARE_POINT_H0_REG_t;

typedef volatile union _EINJ6_COMPARE_POINT_H1_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EINJ6_COMPARE_POINT_H1:32;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Packet Compare Point: 2nd DWORD.

Specifies which Tx TLP header DWORD#1 bits to compare with the corresponding bits in the Packet Compare Value registers (EINJ6_COMPARE_VALUE*).

When all specified bits (in the Tx TLP header and EINJ6_COMPARE_VALUE*) match, the controller inserts errors into the TLP.


Note: This register field is sticky.
        */
    };
} EINJ6_COMPARE_POINT_H1_REG_t;

typedef volatile union _EINJ6_COMPARE_POINT_H2_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EINJ6_COMPARE_POINT_H2:32;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Packet Compare Point: 3rd DWORD.
Specifies which Tx TLP header DWORD#2 bits to compare with the corresponding bits in the Packet Compare Value registers (EINJ6_COMPARE_VALUE*).
When all specified bits (in the Tx TLP header and EINJ6_COMPARE_VALUE*) match, the controller inserts errors into the TLP.


Note: This register field is sticky.
        */
    };
} EINJ6_COMPARE_POINT_H2_REG_t;

typedef volatile union _EINJ6_COMPARE_POINT_H3_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EINJ6_COMPARE_POINT_H3:32;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Packet Compare Point: 4th DWORD.

Specifies which Tx TLP header DWORD#3 bits to compare with the corresponding bits in the Packet Compare Value registers (EINJ6_COMPARE_VALUE*).

When all specified bits (in the Tx TLP header and EINJ6_COMPARE_VALUE*) match, the controller inserts errors into the TLP.


Note: This register field is sticky.
        */
    };
} EINJ6_COMPARE_POINT_H3_REG_t;

typedef volatile union _EINJ6_COMPARE_VALUE_H0_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EINJ6_COMPARE_VALUE_H0:32;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Packet Compare Value: 1st DWORD.

Specifies the value to compare against Tx the TLP header DWORD#0 bits specified in the Packet Compare Point registers (EINJ6_COMPARE_POINT*).


Note: This register field is sticky.
        */
    };
} EINJ6_COMPARE_VALUE_H0_REG_t;

typedef volatile union _EINJ6_COMPARE_VALUE_H1_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EINJ6_COMPARE_VALUE_H1:32;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Packet Compare Value: 2nd DWORD.

Specifies the value to compare against Tx the TLP header DWORD#1 bits specified in the Packet Compare Point registers (EINJ6_COMPARE_POINT*).


Note: This register field is sticky.
        */
    };
} EINJ6_COMPARE_VALUE_H1_REG_t;

typedef volatile union _EINJ6_COMPARE_VALUE_H2_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EINJ6_COMPARE_VALUE_H2:32;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Packet Compare Value: 3rd DWORD.

Specifies the value to compare against Tx the TLP header DWORD#2 bits specified in the Packet Compare Point registers (EINJ6_COMPARE_POINT*).


Note: This register field is sticky.
        */
    };
} EINJ6_COMPARE_VALUE_H2_REG_t;

typedef volatile union _EINJ6_COMPARE_VALUE_H3_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EINJ6_COMPARE_VALUE_H3:32;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Packet Compare Value: 4th DWORD.

Specifies the value to compare against Tx the TLP header DWORD#3 bits specified in the Packet Compare Point registers (EINJ6_COMPARE_POINT*).


Note: This register field is sticky.
        */
    };
} EINJ6_COMPARE_VALUE_H3_REG_t;

typedef volatile union _EINJ6_CHANGE_POINT_H0_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EINJ6_CHANGE_POINT_H0:32;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Packet Change Point: 1st DWORD.

Specifies which Tx TLP header DWORD#0 bits to replace with the corresponding bits in the Packet Change Value registers (EINJ6_CHANGE_VALUE*).


Note: This register field is sticky.
        */
    };
} EINJ6_CHANGE_POINT_H0_REG_t;

typedef volatile union _EINJ6_CHANGE_POINT_H1_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EINJ6_CHANGE_POINT_H1:32;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Packet Change Point: 2nd DWORD.

Specifies which Tx TLP header DWORD#1 bits to replace with the corresponding bits in the Packet Change Value registers (EINJ6_CHANGE_VALUE*).


Note: This register field is sticky.
        */
    };
} EINJ6_CHANGE_POINT_H1_REG_t;

typedef volatile union _EINJ6_CHANGE_POINT_H2_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EINJ6_CHANGE_POINT_H2:32;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Packet Change Point: 3rd DWORD.

Specifies which Tx TLP header DWORD#2 bits to replace with the corresponding bits in the Packet Change Value registers (EINJ6_CHANGE_VALUE*).


Note: This register field is sticky.
        */
    };
} EINJ6_CHANGE_POINT_H2_REG_t;

typedef volatile union _EINJ6_CHANGE_POINT_H3_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EINJ6_CHANGE_POINT_H3:32;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Packet Change Point: 4th DWORD.

Specifies which Tx TLP header DWORD#3 bits to replace with the corresponding bits in the Packet Change Value registers (EINJ6_CHANGE_VALUE*).


Note: This register field is sticky.
        */
    };
} EINJ6_CHANGE_POINT_H3_REG_t;

typedef volatile union _EINJ6_CHANGE_VALUE_H0_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EINJ6_CHANGE_VALUE_H0:32;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Packet Change Value: 1st DWORD.

Specifies replacement values for the Tx TLP header DWORD#0 bits defined in the Packet Change Point registers (EINJ6_CHANGE_POINT*).

Only applies when the EINJ6_INVERTED_CONTROL field in EINJ6_TLP_REG is '0'.


Note: This register field is sticky.
        */
    };
} EINJ6_CHANGE_VALUE_H0_REG_t;

typedef volatile union _EINJ6_CHANGE_VALUE_H1_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EINJ6_CHANGE_VALUE_H1:32;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Packet Change Value: 2nd DWORD.

Specifies replacement values for the Tx TLP header DWORD#1 bits defined in the Packet Change Point registers (EINJ6_CHANGE_POINT*).

Only applies when the EINJ6_INVERTED_CONTROL field in EINJ6_TLP_REG is '0'.


Note: This register field is sticky.
        */
    };
} EINJ6_CHANGE_VALUE_H1_REG_t;

typedef volatile union _EINJ6_CHANGE_VALUE_H2_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EINJ6_CHANGE_VALUE_H2:32;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Packet Change Value: 3rd DWORD.

Specifies replacement values for the Tx TLP header DWORD#2 bits defined in the Packet Change Point registers (EINJ6_CHANGE_POINT*).

Only applies when the EINJ6_INVERTED_CONTROL field in EINJ6_TLP_REG is '0'.


Note: This register field is sticky.
        */
    };
} EINJ6_CHANGE_VALUE_H2_REG_t;

typedef volatile union _EINJ6_CHANGE_VALUE_H3_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EINJ6_CHANGE_VALUE_H3:32;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Packet Change Value: 4th DWORD.

Specifies replacement values for the Tx TLP header DWORD#3 bits defined in the Packet Change Point registers (EINJ6_CHANGE_POINT*).

Only applies when the EINJ6_INVERTED_CONTROL field in EINJ6_TLP_REG is '0'.


Note: This register field is sticky.
        */
    };
} EINJ6_CHANGE_VALUE_H3_REG_t;

typedef volatile union _EINJ6_TLP_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EINJ6_COUNT:8;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Error Injection Count.
Indicates the number of errors to insert.
This counter is decremented while errors are been inserted.

 If the counter value is 0x01 and error is inserted, ERROR_INJECTION6_ENABLE in EINJ_ENABLE_REG returns '0'.

 If the counter value is 0x00 and ERROR_INJECTION6_ENABLE=1, errors are inserted until ERROR_INJECTION6_ENABLE is set to '0'.


Note: This register field is sticky.
        */
        volatile unsigned int EINJ6_INVERTED_CONTROL:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 8 */
        /* Description :
        Inverted Error Injection Control.
Encodded values given as above.

Note: This register field is sticky.
        */
        volatile unsigned int EINJ6_PACKET_TYPE:3;
        /* Default Value : 0x0, TYPE : read-write, Position : 9 */
        /* Description :
        Packet type.
Selects the TLP packets to inject errors into. All encodings other than the specified encodings are reserved.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_12:20;
        /* Default Value : x, TYPE : U, Position : 12 */
        /* Description :
        Reserved for future use.
        */
    };
} EINJ6_TLP_REG_t;

typedef volatile union _SD_CONTROL1_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int FORCE_DETECT_LANE:16;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Force Detect Lane. This field is a bit vector of lanes to force receiver detection on.
When the FORCE_DETECT_LANE_EN field is set, the controller ignores receiver detection from PHY during LTSSM Detect state and uses this value instead.


Note: This register field is sticky.
        */
        volatile unsigned int FORCE_DETECT_LANE_EN:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 16 */
        /* Description :
        Force Detect Lane Enable.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_17:3;
        /* Default Value : x, TYPE : U, Position : 17 */
        /* Description :
        Reserved for future use.
        */
        volatile unsigned int TX_EIOS_NUM:2;
        /* Default Value : 0x0, TYPE : read-write, Position : 20 */
        /* Description :
        Number of Tx EIOS.
This register sets the number of transmit EIOS for L0s/L1 entry and Disable/Loopback/Hot-reset exit. The controller selects the greater value between this register and the value defined by the PCI-SIG specification.


Note: This register field is sticky.
        */
        volatile unsigned int LOW_POWER_INTERVAL:2;
        /* Default Value : 0x0, TYPE : read-write, Position : 22 */
        /* Description :
        Low Power Entry Interval Time.

Interval Time that the controller starts monitoring RXELECIDLE signal after L0s/L1/L2 entry. You should set the value according to the latency from receiving EIOS to, RXELECIDLE assertion at the PHY.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_24:8;
        /* Default Value : x, TYPE : U, Position : 24 */
        /* Description :
        Reserved for future use.
        */
    };
} SD_CONTROL1_REG_t;

typedef volatile union _SD_CONTROL2_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int HOLD_LTSSM:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Hold and Release LTSSM.


Note: This register field is sticky.
        */
        volatile unsigned int RECOVERY_REQUEST:1;
        /* Default Value : 0x0, TYPE : write-only, Position : 1 */
        /* Description :
        Recovery Request.
        */
        volatile unsigned int NOACK_FORCE_LINKDOWN:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 2 */
        /* Description :
        Force LinkDown.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_3:5;
        /* Default Value : x, TYPE : U, Position : 3 */
        /* Description :
        Reserved for future use.
        */
        volatile unsigned int DIRECT_RECIDLE_TO_CONFIG:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 8 */
        /* Description :
        Direct Recovery.Idle to Configuration.


Note: This register field is sticky.
        */
        volatile unsigned int DIRECT_POLCOMP_TO_DETECT:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 9 */
        /* Description :
        Direct Polling.Compliance to Detect.


Note: This register field is sticky.
        */
        volatile unsigned int DIRECT_LPBKSLV_TO_EXIT:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 10 */
        /* Description :
        Direct Loopback Slave To Exit.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_11:5;
        /* Default Value : x, TYPE : U, Position : 11 */
        /* Description :
        Reserved for future use.
        */
        volatile unsigned int FRAMING_ERR_RECOVERY_DISABLE:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 16 */
        /* Description :
        Framing Error Recovery Disable.

This bit disables a transition to Recovery state when a Framing Error is occurred.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_17:15;
        /* Default Value : x, TYPE : U, Position : 17 */
        /* Description :
        Reserved for future use.
        */
    };
} SD_CONTROL2_REG_t;

typedef volatile union _SD_STATUS_L1LANE_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int LANE_SELECT:4;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Lane Select.

Lane Select register for Silicon Debug Status Register of Layer1-PerLane.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_4:12;
        /* Default Value : x, TYPE : U, Position : 4 */
        /* Description :
        Reserved for future use.
        */
        volatile unsigned int PIPE_RXPOLARITY:1;
        /* Default Value : 0x0, TYPE : read-only, Position : 16 */
        /* Description :
        PIPE:RxPolarity.

Indicates PIPE RXPOLARITY signal of selected lane number(LANE_SELECT).


Note: This register field is sticky.
        */
        volatile unsigned int PIPE_DETECT_LANE:1;
        /* Default Value : 0x0, TYPE : read-only, Position : 17 */
        /* Description :
        PIPE:Detect Lane.

Indicates whether PHY indicates receiver detection or not on selected lane number(LANE_SELECT).


Note: This register field is sticky.
        */
        volatile unsigned int PIPE_RXVALID:1;
        /* Default Value : 0x0, TYPE : read-only, Position : 18 */
        /* Description :
        PIPE:RxValid.

Indicates PIPE RXVALID signal of selected lane number(LANE_SELECT).


Note: This register field is sticky.
        */
        volatile unsigned int PIPE_RXELECIDLE:1;
        /* Default Value : 0x1, TYPE : read-only, Position : 19 */
        /* Description :
        PIPE:RxElecIdle.

Indicates PIPE RXELECIDLE signal of selected lane number(LANE_SELECT).


Note: This register field is sticky.
        */
        volatile unsigned int PIPE_TXELECIDLE:1;
        /* Default Value : 0x1, TYPE : read-only, Position : 20 */
        /* Description :
        PIPE:TxElecIdle.

Indicates PIPE TXELECIDLE signal of selected lane number(LANE_SELECT).


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_21:3;
        /* Default Value : x, TYPE : U, Position : 21 */
        /* Description :
        Reserved for future use.
        */
        volatile unsigned int DESKEW_POINTER:8;
        /* Default Value : 0x0, TYPE : read-only, Position : 24 */
        /* Description :
        Deskew Pointer.

Indicates Deskew pointer of internal Deskew buffer of selected lane number(LANE_SELECT).


Note: This register field is sticky.
        */
    };
} SD_STATUS_L1LANE_REG_t;

typedef volatile union _SD_STATUS_L1LTSSM_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int FRAMING_ERR_PTR:7;
        /* Default Value : 0x0, TYPE : read-only, Position : 0 */
        /* Description :
        First Framing Error Pointer.

Identifies the first Framing Error using the following encoding. The field contents are only valid value when FRAMING_ERR =1.
 - Received Unexpected Framing Token (Values 01h to 06h)
 - Received Unexpected STP Token (Values 11h to 13h)
 - Received Unexpected Block (Values 21h to 2Eh)
All encodings other than the defined encodings are reserved.


Note: This register field is sticky.
        */
        volatile unsigned int FRAMING_ERR:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 7 */
        /* Description :
        Framing Error.

Indicates Framing Error detection status.


Note: This register field is sticky.
        */
        volatile unsigned int PIPE_POWER_DOWN:3;
        /* Default Value : 0x2, TYPE : read-only, Position : 8 */
        /* Description :
        PIPE:PowerDown.

Indicates PIPE PowerDown signal.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_11:4;
        /* Default Value : x, TYPE : U, Position : 11 */
        /* Description :
        Reserved for future use.
        */
        volatile unsigned int LANE_REVERSAL:1;
        /* Default Value : 0x0, TYPE : read-only, Position : 15 */
        /* Description :
        Lane Reversal Operation.

Receiver detected lane reversal.

This field is only valid in the L0 LTSSM state.


Note: This register field is sticky.
        */
        volatile unsigned int LTSSM_VARIABLE:16;
        /* Default Value : 0x0, TYPE : read-only, Position : 16 */
        /* Description :
        LTSSM Variable.

Indicates internal LTSSM variables defined in the PCI Express Base Specification.
For other value idle_to_rlock_transitioned.


Note: This register field is sticky.
        */
    };
} SD_STATUS_L1LTSSM_REG_t;

typedef volatile union _SD_STATUS_PM_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int INTERNAL_PM_MSTATE:5;
        /* Default Value : 0x0, TYPE : read-only, Position : 0 */
        /* Description :
        Internal PM State(Master).

Indicates internal state machine of Power Management Master controller.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_5:3;
        /* Default Value : x, TYPE : U, Position : 5 */
        /* Description :
        Reserved for future use.
        */
        volatile unsigned int INTERNAL_PM_SSTATE:4;
        /* Default Value : 0x0, TYPE : read-only, Position : 8 */
        /* Description :
        Internal PM State(Slave).

Indicates internal state machine of Power Management Slave controller.


Note: This register field is sticky.
        */
        volatile unsigned int PME_RESEND_FLAG:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 12 */
        /* Description :
        PME Re-send flag.

When the DUT sends a PM_PME message TLP, the DUT sets PME_Status bit. If host software does not clear PME_Status bit for 100ms(+50%/-5%), the DUT resends the PM_PME Message. This bit indicates that a PM_PME was resent.


Note: This register field is sticky.
        */
        volatile unsigned int L1SUB_STATE:3;
        /* Default Value : 0x0, TYPE : read-only, Position : 13 */
        /* Description :
        L1Sub State.
Indicates internal state machine of L1Sub state.


Note: This register field is sticky.
        */
        volatile unsigned int LATCHED_NFTS:8;
        /* Default Value : 0x0, TYPE : read-only, Position : 16 */
        /* Description :
        Latched N_FTS.
Indicates the value of N_FTS in the received TS Ordered Sets from the Link Partner.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_24:8;
        /* Default Value : x, TYPE : U, Position : 24 */
        /* Description :
        Reserved for future use.
        */
    };
} SD_STATUS_PM_REG_t;

typedef volatile union _SD_STATUS_L2_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int TX_TLP_SEQ_NO:12;
        /* Default Value : 0x0, TYPE : read-only, Position : 0 */
        /* Description :
        Tx Tlp Sequence Number.
Indicates next transmit sequence number for transmit TLP.


Note: This register field is sticky.
        */
        volatile unsigned int RX_ACK_SEQ_NO:12;
        /* Default Value : 0xfff, TYPE : read-only, Position : 12 */
        /* Description :
        Tx Ack Sequence Number.
Indicates ACKD_SEQ which is updated by receiving ACK/NAK DLLP.


Note: This register field is sticky.
        */
        volatile unsigned int DLCMSM:2;
        /* Default Value : 0x0, TYPE : read-only, Position : 24 */
        /* Description :
        DLCMSM.
Indicates the current DLCMSM.


Note: This register field is sticky.
        */
        volatile unsigned int FC_INIT1:1;
        /* Default Value : 0x0, TYPE : read-only, Position : 26 */
        /* Description :
        FC_INIT1.
Indicates the controller is in FC_INIT1(VC0) state.


Note: This register field is sticky.
        */
        volatile unsigned int FC_INIT2:1;
        /* Default Value : 0x0, TYPE : read-only, Position : 27 */
        /* Description :
        FC_INIT2.
Indicates the controller is in FC_INIT2(VC0) state.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_28:4;
        /* Default Value : x, TYPE : U, Position : 28 */
        /* Description :
        Reserved for future use.
        */
    };
} SD_STATUS_L2_REG_t;

typedef volatile union _SD_STATUS_L3FC_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int CREDIT_SEL_VC:3;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Credit Select(VC).
This field in conjunction with the CREDIT_SEL_CREDIT_TYPE, CREDIT_SEL_TLP_TYPE, and CREDIT_SEL_HD viewport-select fields determines that data that is returned by the CREDIT_DATA0 and CREDIT_DATA1 data fields.


Note: This register field is sticky.
        */
        volatile unsigned int CREDIT_SEL_CREDIT_TYPE:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 3 */
        /* Description :
        Credit Select(Credit Type).
This field in conjunction with the CREDIT_SEL_VC, CREDIT_SEL_TLP_TYPE, and CREDIT_SEL_HD viewport-select fields determines that data that is returned by the CREDIT_DATA0 and CREDIT_DATA1 data fields.


Note: This register field is sticky.
        */
        volatile unsigned int CREDIT_SEL_TLP_TYPE:2;
        /* Default Value : 0x0, TYPE : read-write, Position : 4 */
        /* Description :
        Credit Select(TLP Type).
This field in conjunction with the CREDIT_SEL_VC, CREDIT_SEL_CREDIT_TYPE, and CREDIT_SEL_HD viewport-select fields determines that data that is returned by the CREDIT_DATA0 and CREDIT_DATA1 data fields.


Note: This register field is sticky.
        */
        volatile unsigned int CREDIT_SEL_HD:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 6 */
        /* Description :
        Credit Select(HeaderData).
This field in conjunction with the CREDIT_SEL_VC, CREDIT_SEL_CREDIT_TYPE, and CREDIT_SEL_TLP_TYPE viewport-select fields determines that data that is returned by the CREDIT_DATA0 and CREDIT_DATA1 data fields.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_7:1;
        /* Default Value : x, TYPE : U, Position : 7 */
        /* Description :
        Reserved for future use.
        */
        volatile unsigned int CREDIT_DATA0:12;
        /* Default Value : 0x0, TYPE : read-only, Position : 8 */
        /* Description :
        Credit Data0.
Current FC credit data selected by the CREDIT_SEL_VC, CREDIT_SEL_CREDIT_TYPE, CREDIT_SEL_TLP_TYPE, and CREDIT_SEL_HD viewport-select fields.
 - Rx: Credit Received Value
 - Tx: Credit Consumed Value


Note: This register field is sticky.
        */
        volatile unsigned int CREDIT_DATA1:12;
        /* Default Value : 0x0, TYPE : read-only, Position : 20 */
        /* Description :
        Credit Data1.
Current FC credit data selected by the CREDIT_SEL_VC, CREDIT_SEL_CREDIT_TYPE, CREDIT_SEL_TLP_TYPE, and CREDIT_SEL_HD viewport-select fields.
 - Rx: Credit Allocated Value
 - Tx: Credit Limit Value. This value is valid when DLCMSM=0x3(DL_ACTIVE).


Note: This register field is sticky.
        */
    };
} SD_STATUS_L3FC_REG_t;

typedef volatile union _SD_STATUS_L3_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int MFTLP_POINTER:7;
        /* Default Value : 0x0, TYPE : read-only, Position : 0 */
        /* Description :
        First Malformed TLP Error Pointer.
Indicates the element of the received first malformed TLP. This pointer is validated by MFTLP_STATUS.
All encodings other than the defined encodings are reserved.


Note: This register field is sticky.
        */
        volatile unsigned int MFTLP_STATUS:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 7 */
        /* Description :
        Malformed TLP Status.
Indicates malformed TLP has occurred.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_8:24;
        /* Default Value : x, TYPE : U, Position : 8 */
        /* Description :
        Reserved for future use.
        */
    };
} SD_STATUS_L3_REG_t;

typedef volatile union _SD_EQ_CONTROL1_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EQ_LANE_SEL:4;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        EQ Status Lane Select.
Setting this field in conjunction with the EQ_RATE_SEL field determines the per-lane Silicon Debug EQ Status data returned by the SD_EQ_CONTROL[2/3] and SD_EQ_STATUS[1/2/3] viewport registers.


Note: This register field is sticky.
        */
        volatile unsigned int EQ_RATE_SEL:2;
        /* Default Value : 0x0, TYPE : read-write, Position : 4 */
        /* Description :
        EQ Status Rate Select.
Setting this field in conjunction with the EQ_LANE_SEL field determines the per-lane Silicon Debug EQ Status data returned by the SD_EQ_CONTROL[2/3] and SD_EQ_STATUS[1/2/3] viewport registers.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_6:2;
        /* Default Value : x, TYPE : U, Position : 6 */
        /* Description :
        Reserved for future use.
        */
        volatile unsigned int EXT_EQ_TIMEOUT:2;
        /* Default Value : 0x0, TYPE : read-write, Position : 8 */
        /* Description :
        Extends EQ Phase2/3 Timeout.
This field is used when the LTSSM is in Recovery.EQ2/3. When this field is set, the value of EQ2/3 timeout is extended.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_10:6;
        /* Default Value : x, TYPE : U, Position : 10 */
        /* Description :
        Reserved for future use.
        */
        volatile unsigned int EVAL_INTERVAL_TIME:2;
        /* Default Value : 0x0, TYPE : read-write, Position : 16 */
        /* Description :
        Eval Interval Time.
Indicates interval time of RxEqEval assertion.
This field is used for EQ Master(DSP in EQ Phase3/USP in EQ Phase2).

Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_18:5;
        /* Default Value : x, TYPE : U, Position : 18 */
        /* Description :
        Reserved for future use.
        */
        volatile unsigned int FOM_TARGET_ENABLE:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 23 */
        /* Description :
        FOM Target Enable.
Enables the FOM_TARGET fields.


Note: This register field is sticky.
        */
        volatile unsigned int FOM_TARGET:8;
        /* Default Value : 0x0, TYPE : read-write, Position : 24 */
        /* Description :
        FOM Target.
Indicates figure of merit target criteria value of EQ Master(DSP in EQ Phase3/USP in EQ Phase2).

This field is only valid when GEN3_EQ_FB_MODE is 0001b(Figure Of Merit).


Note: This register field is sticky.
        */
    };
} SD_EQ_CONTROL1_REG_t;

typedef volatile union _SD_EQ_CONTROL2_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int FORCE_LOCAL_TX_PRE_CURSOR:6;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Force Local Transmitter Pre-cursor.
Indicates the coefficient value of EQ Slave(DSP in EQ Phase2/USP in EQ Phase3), instead of the value instructed from link partner.


Note: This register field is sticky.
        */
        volatile unsigned int FORCE_LOCAL_TX_CURSOR:6;
        /* Default Value : 0x0, TYPE : read-write, Position : 6 */
        /* Description :
        Force Local Transmitter Cursor.
Indicates the coefficient value of EQ Slave(DSP in EQ Phase2/USP in EQ Phase3), instead of the value instructed from link partner.


Note: This register field is sticky.
        */
        volatile unsigned int FORCE_LOCAL_TX_POST_CURSOR:6;
        /* Default Value : 0x0, TYPE : read-write, Position : 12 */
        /* Description :
        Force Local Transmitter Post-Cursor.
Indicates the coefficient value of EQ Slave(DSP in EQ Phase2/USP in EQ Phase3), instead of the value instructed from link partner.


Note: This register field is sticky.
        */
        volatile unsigned int FORCE_LOCAL_RX_HINT:3;
        /* Default Value : 0x0, TYPE : read-write, Position : 18 */
        /* Description :
        Force Local Receiver Preset Hint.
Indicates the RxPresetHint value of EQ Slave(DSP in EQ Phase2/USP in EQ Phase3), instead of received or set value.
If select rate in the EQ_RATE_SEL field is other than 8.0GT/s Speed, this feature is not available.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_21:3;
        /* Default Value : x, TYPE : U, Position : 21 */
        /* Description :
        Reserved for future use.
        */
        volatile unsigned int FORCE_LOCAL_TX_PRESET:4;
        /* Default Value : 0x0, TYPE : read-write, Position : 24 */
        /* Description :
        Force Local Transmitter Preset.
Indicates initial preset value of USP in EQ Slave(EQ Phase2) instead of receiving EQ TS2.
If select rate in the EQ_RATE_SEL field is 32.0GT/s Speed, this feature is not available.


Note: This register field is sticky.
        */
        volatile unsigned int FORCE_LOCAL_TX_COEF_ENABLE:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 28 */
        /* Description :
        Force Local Transmitter Coefficient Enable.
Enables the following fields:
 - FORCE_LOCAL_TX_PRE_CURSOR
 - FORCE_LOCAL_TX_CURSOR
 - FORCE_LOCAL_TX_POST_CURSOR


Note: This register field is sticky.
        */
        volatile unsigned int FORCE_LOCAL_RX_HINT_ENABLE:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 29 */
        /* Description :
        Force Local Receiver Preset Hint Enable.
Enables the FORCE_LOCAL_RX_HINT field.
If select rate in the EQ_RATE_SEL field is other than 8.0GT/s Speed, this feature is not available.


Note: This register field is sticky.
        */
        volatile unsigned int FORCE_LOCAL_TX_PRESET_ENABLE:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 30 */
        /* Description :
        Force Local Transmitter Preset Enable.
Enables the FORCE_LOCAL_TX_PRESET field.
If select rate in the EQ_RATE_SEL field is 32.0GT/s Speed, this feature is not available.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_31:1;
        /* Default Value : x, TYPE : U, Position : 31 */
        /* Description :
        Reserved for future use.
        */
    };
} SD_EQ_CONTROL2_REG_t;

typedef volatile union _SD_EQ_CONTROL3_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int FORCE_REMOTE_TX_PRE_CURSOR:6;
        /* Default Value : 0x0, TYPE : read-write, Position : 0 */
        /* Description :
        Force Remote Transmitter Pre-Cursor.
Indicates the coefficient value of EQ Master(DSP in EQ Phase3/USP in EQ Phase2), instead of the value instructed from local PHY in dirchange mode.


Note: This register field is sticky.
        */
        volatile unsigned int FORCE_REMOTE_TX_CURSOR:6;
        /* Default Value : 0x0, TYPE : read-write, Position : 6 */
        /* Description :
        Force Remote Transmitter Cursor.
Indicates the coefficient value of EQ Master(DSP in EQ Phase3/USP in EQ Phase2), instead of the value instructed from local PHY in dirchange mode.


Note: This register field is sticky.
        */
        volatile unsigned int FORCE_REMOTE_TX_POST_CURSOR:6;
        /* Default Value : 0x0, TYPE : read-write, Position : 12 */
        /* Description :
        Force Remote Transmitter Post-Cursor.
Indicates the coefficient value of EQ Master(DSP in EQ Phase3/USP in EQ Phase2), instead of the value instructed from local PHY in dirchange mode.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_18:10;
        /* Default Value : x, TYPE : U, Position : 18 */
        /* Description :
        Reserved for future use.
        */
        volatile unsigned int FORCE_REMOTE_TX_COEF_ENABLE:1;
        /* Default Value : 0x0, TYPE : read-write, Position : 28 */
        /* Description :
        Force Remote Transmitter Coefficient Enable.
Enables the following fields:
 - FORCE_REMOTE_TX_PRE_CURSOR
 - FORCE_REMOTE_TX_CURSOR
 - FORCE_REMOTE_TX_POST_CURSOR
This function can only be used when GEN3_EQ_FB_MODE = 0000b(Direction Change)


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_29:3;
        /* Default Value : x, TYPE : U, Position : 29 */
        /* Description :
        Reserved for future use.
        */
    };
} SD_EQ_CONTROL3_REG_t;

typedef volatile union _SD_EQ_STATUS1_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EQ_SEQUENCE:1;
        /* Default Value : 0x0, TYPE : read-only, Position : 0 */
        /* Description :
        EQ Sequence.
Indicates that the controller is starting the equalization sequence.


Note: This register field is sticky.
        */
        volatile unsigned int EQ_CONVERGENCE_INFO:2;
        /* Default Value : 0x0, TYPE : read-only, Position : 1 */
        /* Description :
        EQ Convergence Info.
Indicates equalization convergence information.
This bit is automatically cleared when the controller starts EQ Master phase again.

Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_3:1;
        /* Default Value : x, TYPE : U, Position : 3 */
        /* Description :
        Reserved for future use.
        */
        volatile unsigned int EQ_RULEA_VIOLATION:1;
        /* Default Value : 0x0, TYPE : read-only, Position : 4 */
        /* Description :
        EQ Rule A Violation.
Indicates that coefficients rule A violation is detected in the values provided by PHY using direction change method during EQ Master phase(DSP in EQ Phase3/USP in EQ Phase2). The coefficients rules A correspond to the rules a) from section "Rules for Transmitter Coefficents" in the PCI Express Base Specification.

This bit is automatically cleared when the controller starts EQ Master phase again.


Note: This register field is sticky.
        */
        volatile unsigned int EQ_RULEB_VIOLATION:1;
        /* Default Value : 0x0, TYPE : read-only, Position : 5 */
        /* Description :
        EQ Rule B Violation.
Indicates that coefficients rule B violation is detected in the values provided by PHY using direction change method during EQ Master phase(DSP in EQ Phase3/USP in EQ Phase2). The coefficients rules B correspond to the rules b) from section "Rules for Transmitter Coefficents" in the PCI Express Base Specification.

This bit is automatically cleared when the controller starts EQ Master phase again.


Note: This register field is sticky.
        */
        volatile unsigned int EQ_RULEC_VIOLATION:1;
        /* Default Value : 0x0, TYPE : read-only, Position : 6 */
        /* Description :
        EQ Rule C Violation.
Indicates that coefficients rule C violation is detected in the values provided by PHY using direction change method during EQ Master phase(DSP in EQ Phase3/USP in EQ Phase2). The coefficients rule C  correspond to the rules c) from section "Rules for Transmitter Coefficents" in the PCI Express Base Specification.

This bit is automatically cleared when the controller starts EQ Master phase again.


Note: This register field is sticky.
        */
        volatile unsigned int EQ_REJECT_EVENT:1;
        /* Default Value : 0x0, TYPE : read-only, Position : 7 */
        /* Description :
        EQ Reject Event.
Indicates that the controller receives two consecutive TS1 OS w/Reject=1b during EQ Master phase(DSP in EQ Phase3/USP in EQ Phase2).
This bit is automatically cleared when the controller starts EQ Master phase again.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_8:24;
        /* Default Value : x, TYPE : U, Position : 8 */
        /* Description :
        Reserved for future use.
        */
    };
} SD_EQ_STATUS1_REG_t;

typedef volatile union _SD_EQ_STATUS2_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EQ_LOCAL_PRE_CURSOR:6;
        /* Default Value : 0x0, TYPE : read-only, Position : 0 */
        /* Description :
        EQ Local Pre-Cursor.
Indicates Local pre cursor coefficient value.


Note: This register field is sticky.
        */
        volatile unsigned int EQ_LOCAL_CURSOR:6;
        /* Default Value : 0x0, TYPE : read-only, Position : 6 */
        /* Description :
        EQ Local Cursor.
Indicates Local cursor coefficient value.


Note: This register field is sticky.
        */
        volatile unsigned int EQ_LOCAL_POST_CURSOR:6;
        /* Default Value : 0x0, TYPE : read-only, Position : 12 */
        /* Description :
        EQ Local Post-Cursor.
Indicates Local post cursor coefficient value.


Note: This register field is sticky.
        */
        volatile unsigned int EQ_LOCAL_RX_HINT:3;
        /* Default Value : 0x0, TYPE : read-only, Position : 18 */
        /* Description :
        EQ Local Receiver Preset Hint.
Indicates Local Receiver Preset Hint value.
If select rate in the EQ_RATE_SEL field is other than 8.0GT/s Speed, this feature is not available.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_21:3;
        /* Default Value : x, TYPE : U, Position : 21 */
        /* Description :
        Reserved for future use.
        */
        volatile unsigned int EQ_LOCAL_FOM_VALUE:8;
        /* Default Value : 0x0, TYPE : read-only, Position : 24 */
        /* Description :
        EQ Local Figure of Merit.
Indicates Local maximum Figure of Merit value.


Note: This register field is sticky.
        */
    };
} SD_EQ_STATUS2_REG_t;

typedef volatile union _SD_EQ_STATUS3_REG_t
{
    volatile unsigned int U;  /* Register-Access */
    struct {                  /* Register-Bitfields-Access */
        volatile unsigned int EQ_REMOTE_PRE_CURSOR:6;
        /* Default Value : 0x0, TYPE : read-only, Position : 0 */
        /* Description :
        EQ Remote Pre-Cursor.
Indicates Remote pre cursor coefficient value.


Note: This register field is sticky.
        */
        volatile unsigned int EQ_REMOTE_CURSOR:6;
        /* Default Value : 0x0, TYPE : read-only, Position : 6 */
        /* Description :
        EQ Remote Cursor.
Indicates Remote cursor coefficient value.


Note: This register field is sticky.
        */
        volatile unsigned int EQ_REMOTE_POST_CURSOR:6;
        /* Default Value : 0x0, TYPE : read-only, Position : 12 */
        /* Description :
        EQ Remote Post-Cursor.
Indicates Remote post cursor coefficient value.


Note: This register field is sticky.
        */
        volatile unsigned int EQ_REMOTE_LF:6;
        /* Default Value : 0x0, TYPE : read-only, Position : 18 */
        /* Description :
        EQ Remote LF.
Indicates Remote LF value.


Note: This register field is sticky.
        */
        volatile unsigned int EQ_REMOTE_FS:6;
        /* Default Value : 0x0, TYPE : read-only, Position : 24 */
        /* Description :
        EQ Remote FS.
Indicates Remote FS value.


Note: This register field is sticky.
        */
        volatile unsigned int RSVDP_30:2;
        /* Default Value : x, TYPE : U, Position : 30 */
        /* Description :
        Reserved for future use.
        */
    };
} SD_EQ_STATUS3_REG_t;


/* PCIe AXI_RAS Registers */
typedef volatile struct _pcie_axi_ras_reg_t
{
    RAS_DES_CAP_HEADER_REG_t RAS_DES_CAP_HEADER_REG; /* 0x0 */
    VENDOR_SPECIFIC_HEADER_REG_t VENDOR_SPECIFIC_HEADER_REG; /* 0x4 */
    EVENT_COUNTER_CONTROL_REG_t EVENT_COUNTER_CONTROL_REG; /* 0x8 */
    EVENT_COUNTER_DATA_REG_t EVENT_COUNTER_DATA_REG; /* 0xc */
    TIME_BASED_ANALYSIS_CONTROL_REG_t TIME_BASED_ANALYSIS_CONTROL_REG; /* 0x10 */
    TIME_BASED_ANALYSIS_DATA_REG_t TIME_BASED_ANALYSIS_DATA_REG; /* 0x14 */
    TIME_BASED_ANALYSIS_DATA_63_32_REG_t TIME_BASED_ANALYSIS_DATA_63_32_REG; /* 0x18 */
    volatile int reserved_0x1c[5];
    EINJ_ENABLE_REG_t EINJ_ENABLE_REG; /* 0x30 */
    EINJ0_CRC_REG_t EINJ0_CRC_REG; /* 0x34 */
    EINJ1_SEQNUM_REG_t EINJ1_SEQNUM_REG; /* 0x38 */
    EINJ2_DLLP_REG_t EINJ2_DLLP_REG; /* 0x3c */
    EINJ3_SYMBOL_REG_t EINJ3_SYMBOL_REG; /* 0x40 */
    EINJ4_FC_REG_t EINJ4_FC_REG; /* 0x44 */
    EINJ5_SP_TLP_REG_t EINJ5_SP_TLP_REG; /* 0x48 */
    EINJ6_COMPARE_POINT_H0_REG_t EINJ6_COMPARE_POINT_H0_REG; /* 0x4c */
    EINJ6_COMPARE_POINT_H1_REG_t EINJ6_COMPARE_POINT_H1_REG; /* 0x50 */
    EINJ6_COMPARE_POINT_H2_REG_t EINJ6_COMPARE_POINT_H2_REG; /* 0x54 */
    EINJ6_COMPARE_POINT_H3_REG_t EINJ6_COMPARE_POINT_H3_REG; /* 0x58 */
    EINJ6_COMPARE_VALUE_H0_REG_t EINJ6_COMPARE_VALUE_H0_REG; /* 0x5c */
    EINJ6_COMPARE_VALUE_H1_REG_t EINJ6_COMPARE_VALUE_H1_REG; /* 0x60 */
    EINJ6_COMPARE_VALUE_H2_REG_t EINJ6_COMPARE_VALUE_H2_REG; /* 0x64 */
    EINJ6_COMPARE_VALUE_H3_REG_t EINJ6_COMPARE_VALUE_H3_REG; /* 0x68 */
    EINJ6_CHANGE_POINT_H0_REG_t EINJ6_CHANGE_POINT_H0_REG; /* 0x6c */
    EINJ6_CHANGE_POINT_H1_REG_t EINJ6_CHANGE_POINT_H1_REG; /* 0x70 */
    EINJ6_CHANGE_POINT_H2_REG_t EINJ6_CHANGE_POINT_H2_REG; /* 0x74 */
    EINJ6_CHANGE_POINT_H3_REG_t EINJ6_CHANGE_POINT_H3_REG; /* 0x78 */
    EINJ6_CHANGE_VALUE_H0_REG_t EINJ6_CHANGE_VALUE_H0_REG; /* 0x7c */
    EINJ6_CHANGE_VALUE_H1_REG_t EINJ6_CHANGE_VALUE_H1_REG; /* 0x80 */
    EINJ6_CHANGE_VALUE_H2_REG_t EINJ6_CHANGE_VALUE_H2_REG; /* 0x84 */
    EINJ6_CHANGE_VALUE_H3_REG_t EINJ6_CHANGE_VALUE_H3_REG; /* 0x88 */
    EINJ6_TLP_REG_t EINJ6_TLP_REG; /* 0x8c */
    volatile int reserved_0x90[4];
    SD_CONTROL1_REG_t SD_CONTROL1_REG; /* 0xa0 */
    SD_CONTROL2_REG_t SD_CONTROL2_REG; /* 0xa4 */
    volatile int reserved_0xa8[2];
    SD_STATUS_L1LANE_REG_t SD_STATUS_L1LANE_REG; /* 0xb0 */
    SD_STATUS_L1LTSSM_REG_t SD_STATUS_L1LTSSM_REG; /* 0xb4 */
    SD_STATUS_PM_REG_t SD_STATUS_PM_REG; /* 0xb8 */
    SD_STATUS_L2_REG_t SD_STATUS_L2_REG; /* 0xbc */
    SD_STATUS_L3FC_REG_t SD_STATUS_L3FC_REG; /* 0xc0 */
    SD_STATUS_L3_REG_t SD_STATUS_L3_REG; /* 0xc4 */
    volatile int reserved_0xc8[2];
    SD_EQ_CONTROL1_REG_t SD_EQ_CONTROL1_REG; /* 0xd0 */
    SD_EQ_CONTROL2_REG_t SD_EQ_CONTROL2_REG; /* 0xd4 */
    SD_EQ_CONTROL3_REG_t SD_EQ_CONTROL3_REG; /* 0xd8 */
    volatile int reserved_0xdc[1];
    SD_EQ_STATUS1_REG_t SD_EQ_STATUS1_REG; /* 0xe0 */
    SD_EQ_STATUS2_REG_t SD_EQ_STATUS2_REG; /* 0xe4 */
    SD_EQ_STATUS3_REG_t SD_EQ_STATUS3_REG; /* 0xe8 */
} pcie_axi_ras_reg_t;

#endif /* __PCIE_AXI_RAS_H */