## Common Python module ##
import numpy as np

### Device Type ###
DX_M1   = 0
DX_M1A  = 1
DEV_T = {
    DX_M1   : "DX_M1",
    DX_M1A  : "DX_M1A",
}
########################    PCIe Configuration for diag_status_bus    ########################
DIAG_STS_BUS_OFFSET = {
    DEV_T[DX_M1] : "0x800",
    DEV_T[DX_M1A] : "0xC00",
}
DIAG_STS_BUS = "acc_ras_st_bus_ro00"
PCIE_CONFIG_V_FPGA = { ## FPGA
    DEV_T[DX_M1] : {
        "RX_TLP"            : "2",
        "TRGT_DATA_PROT_WD" : "0",
        "NW"                : "8",
        "XTLH_XDLH_CTL_WD"  : "8", 
        "NVC"               : "4",
        "NCL"               : "3",
        "BUSNUM_WD"         : "8",
        "DEVNUM_ID"         : "0",
        "DEVNUM_WD"         : "5",
        "NF"                : "4",
        "NL"                : "4",
    },
    DEV_T[DX_M1A] : {
        "RX_TLP"            : "2",
        "TRGT_DATA_PROT_WD" : "32",
        "NW"                : "8",
        "XTLH_XDLH_CTL_WD"  : "8", 
        "NVC"               : "1",
        "NCL"               : "3",
        "BUSNUM_WD"         : "8",
        "DEVNUM_ID"         : "0",
        "DEVNUM_WD"         : "5",
        "NF"                : "1",
        "NL"                : "4",
    },
}
PCIE_CONFIG_V = { ## ASIC
    DEV_T[DX_M1] : {
        "RX_TLP"            : "1",
        "TRGT_DATA_PROT_WD" : "0",
        "NW"                : "2",
        "XTLH_XDLH_CTL_WD"  : "1", 
        "NVC"               : "4",
        "NCL"               : "3",
        "BUSNUM_WD"         : "8",
        "DEVNUM_ID"         : "0",
        "DEVNUM_WD"         : "5",
        "NF"                : "4",
        "NL"                : "4",
    },
    DEV_T[DX_M1A] : {
        "RX_TLP"            : "2",
        "TRGT_DATA_PROT_WD" : "32",
        "NW"                : "8",
        "XTLH_XDLH_CTL_WD"  : "8", 
        "NVC"               : "1",
        "NCL"               : "3",
        "BUSNUM_WD"         : "8",
        "DEVNUM_ID"         : "0",
        "DEVNUM_WD"         : "5",
        "NF"                : "1",
        "NL"                : "4",
    },
}

## [Module Name, enable for including header file]
MODULE_LIST_HIF = {
    DEV_T[DX_M1] :
        [ "acc_etc_rw01",
          "acc_etc_rw39",
          "acc_etc_ro133",
          "acc_etc_ro134",
          "acc_etc_ro135",
          "acc_etc_ro136",
          "acc_etc_ro154",
          DIAG_STS_BUS,
          "acc_ras_ec_com_ro00",
          "acc_ras_ec_lxx_ro00",
          "acc_ras_ec_lxx_ro01",
          "acc_ras_sd_com_ro00",
          "acc_ras_sd_lxx_ro00",
          "acc_ras_sd_lxx_ro07",
          "acc_ras_sd_lxx_ro14",
          "acc_ras_sd_lxx_ro21",
          "acc_ras_sd_vxx_ro00",
          "acc_ras_sd_vxx_ro08",
          "acc_ras_sd_vxx_ro16",
          "acc_ras_sd_vxx_ro24",
          "acc_ras_tba_com_ro00"],
    DEV_T[DX_M1A] :
        [ "acc_pipe02",
          "acc_etc_rw01",
          "acc_etc_rw06",
          "acc_etc_rw39",
          "acc_etc_ro044",
          "acc_etc_ro045",
          "acc_etc_ro132",
          "acc_etc_ro133",
          "acc_etc_ro134",
          "acc_etc_ro135",
          "acc_etc_ro136",
          "acc_etc_ro137",
          "acc_etc_ro145",
          "acc_etc_ro148",
          "acc_etc_ro154",
          "acc_etc_ro158",
          DIAG_STS_BUS,
          "acc_ras_ec_com_ro00",
          "acc_ras_ec_lxx_ro00",
          "acc_ras_ec_lxx_ro01",
          "acc_ras_sd_com_ro00",
          "acc_ras_sd_lxx_ro00",
          "acc_ras_sd_lxx_ro08",
          "acc_ras_sd_lxx_ro16",
          "acc_ras_sd_lxx_ro24",
          "acc_ras_sd_vxx_ro00",
          "acc_ras_tba_com_ro00"],
}

RAS_DES_MODULE_LIST_AXI = [
    "RAS_DES_CAP_HEADER_REG",
    "VENDOR_SPECIFIC_HEADER_REG",
    "EVENT_COUNTER_CONTROL_REG",
    "EVENT_COUNTER_DATA_REG",
    "TIME_BASED_ANALYSIS_CONTROL_REG",
    "TIME_BASED_ANALYSIS_DATA_REG",
    "TIME_BASED_ANALYSIS_DATA_63_32_REG",
    "EINJ_ENABLE_REG",
    "EINJ0_CRC_REG",
    "EINJ1_SEQNUM_REG",
    "EINJ2_DLLP_REG",
    "EINJ3_SYMBOL_REG",
    "EINJ4_FC_REG",
    "EINJ5_SP_TLP_REG",
    "EINJ6_COMPARE_POINT_H0_REG",
    "EINJ6_COMPARE_POINT_H1_REG",
    "EINJ6_COMPARE_POINT_H2_REG",
    "EINJ6_COMPARE_POINT_H3_REG",
    "EINJ6_COMPARE_VALUE_H0_REG",
    "EINJ6_COMPARE_VALUE_H1_REG",
    "EINJ6_COMPARE_VALUE_H2_REG",
    "EINJ6_COMPARE_VALUE_H3_REG",
    "EINJ6_CHANGE_POINT_H0_REG",
    "EINJ6_CHANGE_POINT_H1_REG",
    "EINJ6_CHANGE_POINT_H2_REG",
    "EINJ6_CHANGE_POINT_H3_REG",
    "EINJ6_CHANGE_VALUE_H0_REG",
    "EINJ6_CHANGE_VALUE_H1_REG",
    "EINJ6_CHANGE_VALUE_H2_REG",
    "EINJ6_CHANGE_VALUE_H3_REG",
    "EINJ6_TLP_REG",
    "SD_CONTROL1_REG",
    "SD_CONTROL2_REG",
    "SD_STATUS_L1LANE_REG",
    "SD_STATUS_L1LTSSM_REG",
    "SD_STATUS_PM_REG",
    "SD_STATUS_L2_REG",
    "SD_STATUS_L3FC_REG",
    "SD_STATUS_L3_REG",
    "SD_EQ_CONTROL1_REG",
    "SD_EQ_CONTROL2_REG",
    "SD_EQ_CONTROL3_REG",
    "SD_EQ_STATUS1_REG",
    "SD_EQ_STATUS2_REG",
    "SD_EQ_STATUS3_REG",
]
####################################################################################
GEN_ALL_REGSTER = "ALL_GEN"
PARTIAL_REGISTER = "PARTIAL_GEN"
# value : [name, base_address, axi, gen_part, gen_lists, gen_sheet_n, gen_file_n, mako_file_n]
CONFIG_LIST = {
    DEV_T[DX_M1] : {
        "CTRL_HIF_CMM"      : ["HIF"    , 0xC1000000, 0, PARTIAL_REGISTER, MODULE_LIST_HIF[DEV_T[DX_M1]], "pcie_ctrl_4lane_hif_register", "pcie_fail_reg.cmm",     ""],
        "CTRL_HIF_CODE"     : ["HIF"    , 0xC1000000, 0, GEN_ALL_REGSTER , "",                            "pcie_ctrl_4lane_hif_register", "dx_pcie_hif.h",         "dx_pcie_hif.h.mako"],
        "CTRL_AXI_RAS_CODE" : ["AXI_RAS", 0xC900030C, 1, PARTIAL_REGISTER, RAS_DES_MODULE_LIST_AXI,       "4lane_dbi_cpcie_usp",          "dx_pcie_ras_des.h",     "dx_pcie_single_reg.h.mako"],
        "CTRL_AXI_RAS_CMM"  : ["AXI_RAS", 0xC900030C, 1, PARTIAL_REGISTER, RAS_DES_MODULE_LIST_AXI,       "4lane_dbi_cpcie_usp",          "pcie_fail_reg_axi.cmm", ""],
        "PHY_HIF_CODE"      : ["PHY_HIF", 0xC1040000, 0, GEN_ALL_REGSTER , "",                            "pcie_phy_x4_hif_register",     "dx_pcie_phy_hif.h",     "dx_pcie_single_reg.h.mako"],
        "PHY_REG_CODE"      : ["PHY_REG", 0xC1080000, 0, GEN_ALL_REGSTER , "",                            "pcie_phy_register",            "dx_pcie_phy_reg.h",     "dx_pcie_single_reg.h.mako"],
    },
    DEV_T[DX_M1A] : {
        "CTRL_HIF_CMM"      : ["HIF"    , 0xC1000000, 0, PARTIAL_REGISTER, MODULE_LIST_HIF[DEV_T[DX_M1A]],"4lane_ctrl_hif",               "pcie_fail_reg.cmm",     ""],
        "CTRL_HIF_CODE"     : ["HIF"    , 0xC1000000, 0, GEN_ALL_REGSTER , "",                            "4lane_ctrl_hif",               "dx_pcie_hif.h",         "dx_pcie_hif.h.mako"],
        "CTRL_AXI_RAS_CODE" : ["AXI_RAS", 0xD0000248, 1, PARTIAL_REGISTER, RAS_DES_MODULE_LIST_AXI,       "DWC_pcie_dbi_cpcie_usp",       "dx_pcie_ras_des.h",     "dx_pcie_single_reg.h.mako"],
        "CTRL_AXI_RAS_CMM"  : ["AXI_RAS", 0xD0000248, 1, PARTIAL_REGISTER, RAS_DES_MODULE_LIST_AXI,       "DWC_pcie_dbi_cpcie_usp",       "pcie_fail_reg_axi.cmm", ""],
        "PHY_HIF_CODE"      : ["PHY_HIF", 0xC1040000, 0, GEN_ALL_REGSTER , "",                            "phy_hif",                      "dx_pcie_phy_hif.h",     "dx_pcie_single_reg.h.mako"],
        "PHY_REG_CODE"      : ["PHY_REG", 0xC10C0000, 0, GEN_ALL_REGSTER , "",                            "pcie_phy_register",            "dx_pcie_phy_reg.h",     "dx_pcie_single_reg.h.mako"],
        "PCIE_TOP_CRG"      : ["PCIE_TOP", 0xC1080000, 0, GEN_ALL_REGSTER , "",                           "pcie_syn_top_crg",             "pcie_syn_top_crg.h",    "dx_pcie_single_reg.h.mako"],
    },
}
####################################################################################
def get_reg_map_f(dev):
    dev_dict = {
        DX_M1: lambda: "DX_M1_PCIe_Reg.xlsx",
        DX_M1A: lambda: "DX_M1A_PCIe_Reg.xlsx",
    }
    return dev_dict.get(dev, lambda: print("Not defined device~"))()

class Config:
    def __init__(self, n, b, a, g_p, g_l, g_s_n, g_f_n, m_f_n):
        self.name           = n
        self.base_address   = b
        self.axi            = a
        self.gen_part       = g_p
        self.gen_lists      = g_l
        self.gen_sheet_n    = g_s_n
        self.gen_file_n     = g_f_n
        self.mako_file_n    = m_f_n
    def show(self):
        print(f"name         : {self.name}")
        print(f"base_address : {hex(self.base_address).upper()}")
        print(f"axi          : {self.axi}")
        print(f"gen_part     : {self.gen_part}")
        print(f"gen_lists    : {self.gen_lists}")
        print(f"gen_sheet_n  : {self.gen_sheet_n}")
        print(f"gen_file_n   : {self.gen_file_n}")
        print(f"mako_file_n  : {self.mako_file_n}")

class ConfigList:
    def __init__(self, dev):
        self.reg_map_f      = get_reg_map_f(dev)
        self.lists          = dict()
    def append_list(self, list, datas):
        assert len(datas) == 8, f"The size of datas must be 7 , {len(datas)}"
        self.lists[list] = Config(*datas)
    def show(self):
        print(f"Register Map File : {self.reg_map_f}")
        for key, config in self.lists.items():
            print(f"************ [{key}] ************")
            config.show()

## Test Code ##
# dev = DX_M1
# cl = ConfigList(dev)
# for key, value in CONFIG_LIST[DEV_T[dev]].items():
#     cl.append_list(key, value)
# cl.show()
####################################################################################
