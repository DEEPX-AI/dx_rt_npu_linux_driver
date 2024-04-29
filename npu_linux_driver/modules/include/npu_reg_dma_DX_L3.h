// SPDX-License-Identifier: GPL-2.0
/*
 * Deepx Runtime Driver
 *
 * Copyright (C) 2023 Deepx, Inc.
 *
 */
#ifndef __NPU_REG_DMA_DX_L3_H
#define __NPU_REG_DMA_DX_L3_H

#include <linux/types.h>

/* Address */
#define REG_DMA_OFFSET 0x10000
#define REG_DMA_AXI_BASE_ADDR_LOW    (REG_DMA_OFFSET + 0x00000000) /* 0 */
#define REG_DMA_AXI_BASE_ADDR_HIGH    (REG_DMA_OFFSET + 0x00000004) /* 4 */
#define REG_DMA_AXI4_RADDR    (REG_DMA_OFFSET + 0x00000008) /* 8 */
#define REG_DMA_AXI4_WADDR    (REG_DMA_OFFSET + 0x0000000c) /* 12 */
#define REG_DMA_RSVD0    (REG_DMA_OFFSET + 0x00000010) /* 16 */
#define REG_DMA_RSVD1    (REG_DMA_OFFSET + 0x00000014) /* 20 */
#define REG_DMA_SRAM_DST_ADDR0    (REG_DMA_OFFSET + 0x00000018) /* 24 */
#define REG_DMA_SRAM_DST_ADDR1    (REG_DMA_OFFSET + 0x0000001c) /* 28 */
#define REG_DMA_SRAM_SRC_ADDR    (REG_DMA_OFFSET + 0x00000020) /* 32 */
#define REG_DMA_READ_SIZE    (REG_DMA_OFFSET + 0x00000024) /* 36 */
#define REG_DMA_WRITE_SIZE    (REG_DMA_OFFSET + 0x00000028) /* 40 */
#define REG_DMA_RSVD2    (REG_DMA_OFFSET + 0x0000002c) /* 44 */
#define REG_DMA_RSVD3    (REG_DMA_OFFSET + 0x00000030) /* 48 */
#define REG_DMA_RSVD4    (REG_DMA_OFFSET + 0x00000034) /* 52 */
#define REG_DMA_RSVD5    (REG_DMA_OFFSET + 0x00000038) /* 56 */
#define REG_DMA_IRQ    (REG_DMA_OFFSET + 0x0000003c) /* 60 */
#define REG_DMA_CMD    (REG_DMA_OFFSET + 0x00000040) /* 64 */
#define REG_DMA_STATUS    (REG_DMA_OFFSET + 0x00000044) /* 68 */
#define REG_DMA_SFR_BASE_ADDR_LOW    (REG_DMA_OFFSET + 0x00000048) /* 72 */
#define REG_DMA_SFR_BASE_ADDR_HIGH    (REG_DMA_OFFSET + 0x0000004c) /* 76 */
#define REG_DMA_SFR_ADDR_OFFS    (REG_DMA_OFFSET + 0x00000050) /* 80 */
#define REG_DMA_SFR_START_ADDR    (REG_DMA_OFFSET + 0x00000054) /* 84 */
#define REG_DMA_SFR_MOVE_CNT    (REG_DMA_OFFSET + 0x00000058) /* 88 */
#define REG_DMA_SFR_ID    (REG_DMA_OFFSET + 0x0000005c) /* 92 */
#define REG_DMA_SFR_DMA_STATUS    (REG_DMA_OFFSET + 0x00000060) /* 96 */
#define REG_DMA_DMA_CTRL    (REG_DMA_OFFSET + 0x00000064) /* 100 */
#define REG_DMA_FLUSH_STATUS    (REG_DMA_OFFSET + 0x00000068) /* 104 */
#define REG_DMA_AXI_CFG0    (REG_DMA_OFFSET + 0x0000006c) /* 108 */
#define REG_DMA_AXI_CFG1    (REG_DMA_OFFSET + 0x00000070) /* 112 */
#define REG_DMA_AXI_CFG2    (REG_DMA_OFFSET + 0x00000074) /* 116 */
#define REG_DMA_AXI_CFG3    (REG_DMA_OFFSET + 0x00000078) /* 120 */
#define REG_DMA_AXI_CFG4    (REG_DMA_OFFSET + 0x0000007c) /* 124 */
#define REG_DMA_AXI_CFG5    (REG_DMA_OFFSET + 0x00000080) /* 128 */
#define REG_DMA_AXI_CFG6    (REG_DMA_OFFSET + 0x00000084) /* 132 */
#define REG_DMA_VIRT_RD_EN    (REG_DMA_OFFSET + 0x00000088) /* 136 */
#define REG_DMA_VIRT_RD_PTRN    (REG_DMA_OFFSET + 0x0000008c) /* 140 */

/* Register Definition */
typedef volatile union _DMA_AXI_BASE_ADDR_LOW_t
{
    volatile uint32_t U;  /* Register-Access : DMA.AXI_BASE_ADDR_LOW*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t AXI4_BASE_ADDR_LOW:32;  /* DMA.AXI_BASE_ADDR_LOW.AXI4_BASE_ADDR_LOW [31:0] (RW, 0x0) AXI4 BASE ADDRESS LOW 32BIT */
    };
} DMA_AXI_BASE_ADDR_LOW_t;
typedef volatile union _DMA_AXI_BASE_ADDR_HIGH_t
{
    volatile uint32_t U;  /* Register-Access : DMA.AXI_BASE_ADDR_HIGH*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t AXI4_BASE_ADDR_HIGH:32;  /* DMA.AXI_BASE_ADDR_HIGH.AXI4_BASE_ADDR_HIGH [31:0] (RW, 0x0) AXI4 BASE ADDRESS HIGH 32BIT */
    };
} DMA_AXI_BASE_ADDR_HIGH_t;
typedef volatile union _DMA_AXI4_RADDR_t
{
    volatile uint32_t U;  /* Register-Access : DMA.AXI4_RADDR*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t DMA0_AXI4_RADDR:32;  /* DMA.AXI4_RADDR.DMA0_AXI4_RADDR [31:0] (RW, 0x0) DMA0 AXI4 READ ADDRESS */
    };
} DMA_AXI4_RADDR_t;
typedef volatile union _DMA_AXI4_WADDR_t
{
    volatile uint32_t U;  /* Register-Access : DMA.AXI4_WADDR*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t DMA0_AXI4_WADDR:32;  /* DMA.AXI4_WADDR.DMA0_AXI4_WADDR [31:0] (RW, 0x0) DMA0 AXI4 WRITE ADDRESS */
    };
} DMA_AXI4_WADDR_t;
typedef volatile union _DMA_RSVD0_t
{
    volatile uint32_t U;  /* Register-Access : DMA.RSVD0*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t RESERVED0:32;  /* DMA.RSVD0.RESERVED0 [31:0] (R, 0x0)  */
    };
} DMA_RSVD0_t;
typedef volatile union _DMA_RSVD1_t
{
    volatile uint32_t U;  /* Register-Access : DMA.RSVD1*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t RESERVED0:32;  /* DMA.RSVD1.RESERVED0 [31:0] (R, 0x0)  */
    };
} DMA_RSVD1_t;
typedef volatile union _DMA_SRAM_DST_ADDR0_t
{
    volatile uint32_t U;  /* Register-Access : DMA.SRAM_DST_ADDR0*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t DMA0_SRAM_DST_ADDR0:32;  /* DMA.SRAM_DST_ADDR0.DMA0_SRAM_DST_ADDR0 [31:0] (RW, 0x0) DMA0 SRAM DESTINATION ADDRESS 0 */
    };
} DMA_SRAM_DST_ADDR0_t;
typedef volatile union _DMA_SRAM_DST_ADDR1_t
{
    volatile uint32_t U;  /* Register-Access : DMA.SRAM_DST_ADDR1*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t DMA0_SRAM_DST_ADDR1:32;  /* DMA.SRAM_DST_ADDR1.DMA0_SRAM_DST_ADDR1 [31:0] (RW, 0x0) DMA0 SRAM DESTINATION ADDRESS 1 WHEN 2 SRAM WRITE COMMAND */
    };
} DMA_SRAM_DST_ADDR1_t;
typedef volatile union _DMA_SRAM_SRC_ADDR_t
{
    volatile uint32_t U;  /* Register-Access : DMA.SRAM_SRC_ADDR*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t DMA0_SRAM_SRC_ADDR:32;  /* DMA.SRAM_SRC_ADDR.DMA0_SRAM_SRC_ADDR [31:0] (RW, 0x0) DMA0 SRAM SOURCE ADDRESS */
    };
} DMA_SRAM_SRC_ADDR_t;
typedef volatile union _DMA_READ_SIZE_t
{
    volatile uint32_t U;  /* Register-Access : DMA.READ_SIZE*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t RESERVED1:6;  /* DMA.READ_SIZE.RESERVED1 [5:0] (R, 0x0)  */
        volatile uint32_t DMA0_READ_SIZE:16;  /* DMA.READ_SIZE.DMA0_READ_SIZE [21:6] (RW, 0x0) DATA MOVE BYTE SIZE. MULTIPLE OF 16BYTE(128BIT) */
        volatile uint32_t RESERVED0:10;  /* DMA.READ_SIZE.RESERVED0 [31:22] (R, 0x0)  */
    };
} DMA_READ_SIZE_t;
typedef volatile union _DMA_WRITE_SIZE_t
{
    volatile uint32_t U;  /* Register-Access : DMA.WRITE_SIZE*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t RESERVED1:6;  /* DMA.WRITE_SIZE.RESERVED1 [5:0] (R, 0x0)  */
        volatile uint32_t DMA0_WRITE_SIZE:16;  /* DMA.WRITE_SIZE.DMA0_WRITE_SIZE [21:6] (RW, 0x0) DATA MOVE BYTE SIZE. MULTIPLE OF 16BYTE(128BIT) */
        volatile uint32_t RESERVED0:10;  /* DMA.WRITE_SIZE.RESERVED0 [31:22] (R, 0x0)  */
    };
} DMA_WRITE_SIZE_t;
typedef volatile union _DMA_RSVD2_t
{
    volatile uint32_t U;  /* Register-Access : DMA.RSVD2*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t RESERVED0:32;  /* DMA.RSVD2.RESERVED0 [31:0] (R, 0x0)  */
    };
} DMA_RSVD2_t;
typedef volatile union _DMA_RSVD3_t
{
    volatile uint32_t U;  /* Register-Access : DMA.RSVD3*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t RESERVED0:32;  /* DMA.RSVD3.RESERVED0 [31:0] (R, 0x0)  */
    };
} DMA_RSVD3_t;
typedef volatile union _DMA_RSVD4_t
{
    volatile uint32_t U;  /* Register-Access : DMA.RSVD4*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t RESERVED0:32;  /* DMA.RSVD4.RESERVED0 [31:0] (R, 0x0)  */
    };
} DMA_RSVD4_t;
typedef volatile union _DMA_RSVD5_t
{
    volatile uint32_t U;  /* Register-Access : DMA.RSVD5*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t RESERVED0:32;  /* DMA.RSVD5.RESERVED0 [31:0] (R, 0x0)  */
    };
} DMA_RSVD5_t;
typedef volatile union _DMA_IRQ_t
{
    volatile uint32_t U;  /* Register-Access : DMA.IRQ*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t DMA_IRQ_EN:5;  /* DMA.IRQ.DMA_IRQ_EN [4:0] (RW, 0x18) DMA IRQ EN [0] : READ DMA IRQ EN [1] : WRITE DMA IRQ EN [2] : MEM COPY IRQ EN [3] : RDMA ERR EN [4] : WDMA ERR EN */
        volatile uint32_t RESERVED0:27;  /* DMA.IRQ.RESERVED0 [31:5] (R, 0x0)  */
    };
} DMA_IRQ_t;
typedef volatile union _DMA_CMD_t
{
    volatile uint32_t U;  /* Register-Access : DMA.CMD*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t DMA_START_CMD:5;  /* DMA.CMD.DMA_START_CMD [4:0] (W, 0x0) DMA START COMMAND [0] : READ DMA START [1] : WRITE DMA START [2] : MEM COPY START [3] : READ DMA WITH 2-SRAM WRITE [4] : CPU DMA MODE */
        volatile uint32_t RESERVED0:27;  /* DMA.CMD.RESERVED0 [31:5] (R, 0x0)  */
    };
} DMA_CMD_t;
typedef volatile union _DMA_STATUS_t
{
    volatile uint32_t U;  /* Register-Access : DMA.STATUS*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t DMA_DONE_STATUS:9;  /* DMA.STATUS.DMA_DONE_STATUS [8:0] (R, 0x0) DMA DONE STATUS [0] : READ DMA DONE [1] : WRITE DMA DONE [2] : MEM COPY DONE [3] : READ DMA ERROR [4] : WRITE DMA ERROR [6:5] : READ DMA ERROR CODE [8:7] : WRITE DMA ERROR CODE */
        volatile uint32_t RESERVED0:23;  /* DMA.STATUS.RESERVED0 [31:9] (R, 0x0)  */
    };
} DMA_STATUS_t;
typedef volatile union _DMA_SFR_BASE_ADDR_LOW_t
{
    volatile uint32_t U;  /* Register-Access : DMA.SFR_BASE_ADDR_LOW*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t SFR_BASE_ADDR_LOW:32;  /* DMA.SFR_BASE_ADDR_LOW.SFR_BASE_ADDR_LOW [31:0] (RW, 0x0) SFR BASE AXI4 ADDRESS LOW 32BIT */
    };
} DMA_SFR_BASE_ADDR_LOW_t;
typedef volatile union _DMA_SFR_BASE_ADDR_HIGH_t
{
    volatile uint32_t U;  /* Register-Access : DMA.SFR_BASE_ADDR_HIGH*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t SFR_BASE_ADDR_HIGH:32;  /* DMA.SFR_BASE_ADDR_HIGH.SFR_BASE_ADDR_HIGH [31:0] (RW, 0x0) SFR BASE AXI4 ADDRESS HIGH 32BIT */
    };
} DMA_SFR_BASE_ADDR_HIGH_t;
typedef volatile union _DMA_SFR_ADDR_OFFS_t
{
    volatile uint32_t U;  /* Register-Access : DMA.SFR_ADDR_OFFS*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t RESERVED0:4;  /* DMA.SFR_ADDR_OFFS.RESERVED0 [3:0] (R, 0x0) SFR_ADDR_OFFS[3:0] */
        volatile uint32_t SFR_ADDR_OFFS:28;  /* DMA.SFR_ADDR_OFFS.SFR_ADDR_OFFS [31:4] (RW, 0x0) SFR ADDRESS OFFSET. AXI ADDRESS = SFR BASE ADDRESS + (LAYER ID X SFR ADDRESS OFFSET) */
    };
} DMA_SFR_ADDR_OFFS_t;
typedef volatile union _DMA_SFR_START_ADDR_t
{
    volatile uint32_t U;  /* Register-Access : DMA.SFR_START_ADDR*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t SFR_START_ADDR:16;  /* DMA.SFR_START_ADDR.SFR_START_ADDR [15:0] (RW, 0x0) SFR WRITE START ADDRESS */
        volatile uint32_t RESERVED0:16;  /* DMA.SFR_START_ADDR.RESERVED0 [31:16] (R, 0x0)  */
    };
} DMA_SFR_START_ADDR_t;
typedef volatile union _DMA_SFR_MOVE_CNT_t
{
    volatile uint32_t U;  /* Register-Access : DMA.SFR_MOVE_CNT*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t RESERVED1:2;  /* DMA.SFR_MOVE_CNT.RESERVED1 [1:0] (R, 0x0)  */
        volatile uint32_t SFR_MOVE_CNT:12;  /* DMA.SFR_MOVE_CNT.SFR_MOVE_CNT [13:2] (RW, 0x0) DATA MOVE BYTE COUNT */
        volatile uint32_t RESERVED0:18;  /* DMA.SFR_MOVE_CNT.RESERVED0 [31:14] (R, 0x0)  */
    };
} DMA_SFR_MOVE_CNT_t;
typedef volatile union _DMA_SFR_ID_t
{
    volatile uint32_t U;  /* Register-Access : DMA.SFR_ID*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t SFR_ID:16;  /* DMA.SFR_ID.SFR_ID [15:0] (W, 0x0) LAYER ID REGISTER. SFR DMA STARTS AFTER SETTING THIS REGISTER. */
        volatile uint32_t RESERVED0:16;  /* DMA.SFR_ID.RESERVED0 [31:16] (R, 0x0)  */
    };
} DMA_SFR_ID_t;
typedef volatile union _DMA_SFR_DMA_STATUS_t
{
    volatile uint32_t U;  /* Register-Access : DMA.SFR_DMA_STATUS*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t SFR_DMA_DONE_STATUS:1;  /* DMA.SFR_DMA_STATUS.SFR_DMA_DONE_STATUS [0:0] (R, 0x0) SFR DMA DONE IQR REGISTER. CLEAR THIS BIT AFTER CHECK IT DONE. */
        volatile uint32_t RESERVED0:31;  /* DMA.SFR_DMA_STATUS.RESERVED0 [31:1] (R, 0x0)  */
    };
} DMA_SFR_DMA_STATUS_t;
typedef volatile union _DMA_DMA_CTRL_t
{
    volatile uint32_t U;  /* Register-Access : DMA.DMA_CTRL*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t DMA_CTRL_ENABLE:3;  /* DMA.DMA_CTRL.DMA_CTRL_ENABLE [2:0] (RW, 0x4) [0] : DMA FLUSH    0: NORMAL OPERATION,  1: FLUSH CURRENT DMA & DON'T ACCEPT DMA REQUESTS. [1] : USER DMA EN    0: DISABLE,   1: ENABLE [2] : NPU DMA EN    0: DISABLE,   1: ENABLE */
        volatile uint32_t RESERVED0:29;  /* DMA.DMA_CTRL.RESERVED0 [31:3] (R, 0x0)  */
    };
} DMA_DMA_CTRL_t;
typedef volatile union _DMA_FLUSH_STATUS_t
{
    volatile uint32_t U;  /* Register-Access : DMA.FLUSH_STATUS*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t FLUSH_STATUS:1;  /* DMA.FLUSH_STATUS.FLUSH_STATUS [0:0] (R, 0x0) NPU DMA FLUSH DONE.   0: FLUSH IS NOT DONE,  1: FLUSH IS DONE. */
        volatile uint32_t RESERVED0:31;  /* DMA.FLUSH_STATUS.RESERVED0 [31:1] (R, 0x0)  */
    };
} DMA_FLUSH_STATUS_t;
typedef volatile union _DMA_AXI_CFG0_t
{
    volatile uint32_t U;  /* Register-Access : DMA.AXI_CFG0*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t AXI_RDMA_BURST_LENGTH:9;  /* DMA.AXI_CFG0.AXI_RDMA_BURST_LENGTH [8:0] (RW, 0x100) AXI READ BURST LENGTH */
        volatile uint32_t RESERVED0:23;  /* DMA.AXI_CFG0.RESERVED0 [31:9] (R, 0x0)  */
    };
} DMA_AXI_CFG0_t;
typedef volatile union _DMA_AXI_CFG1_t
{
    volatile uint32_t U;  /* Register-Access : DMA.AXI_CFG1*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t AXI_WDMA_BURST_LENGTH:9;  /* DMA.AXI_CFG1.AXI_WDMA_BURST_LENGTH [8:0] (RW, 0x100) AXI WRITE BURST LENGTH */
        volatile uint32_t RESERVED0:23;  /* DMA.AXI_CFG1.RESERVED0 [31:9] (R, 0x0)  */
    };
} DMA_AXI_CFG1_t;
typedef volatile union _DMA_AXI_CFG2_t
{
    volatile uint32_t U;  /* Register-Access : DMA.AXI_CFG2*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t AXI_RDMA_MO:4;  /* DMA.AXI_CFG2.AXI_RDMA_MO [3:0] (RW, 0xf) AXI READ MUTI-OUTSTANDING NUMBER */
        volatile uint32_t RESERVED0:28;  /* DMA.AXI_CFG2.RESERVED0 [31:4] (R, 0x0)  */
    };
} DMA_AXI_CFG2_t;
typedef volatile union _DMA_AXI_CFG3_t
{
    volatile uint32_t U;  /* Register-Access : DMA.AXI_CFG3*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t AXI_WDMA_MO:4;  /* DMA.AXI_CFG3.AXI_WDMA_MO [3:0] (RW, 0xf) AXI WRITE MULTI-OUTSTANDING NUMBER */
        volatile uint32_t RESERVED0:28;  /* DMA.AXI_CFG3.RESERVED0 [31:4] (R, 0x0)  */
    };
} DMA_AXI_CFG3_t;
typedef volatile union _DMA_AXI_CFG4_t
{
    volatile uint32_t U;  /* Register-Access : DMA.AXI_CFG4*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t AXI_4KB_BOUNDARY:1;  /* DMA.AXI_CFG4.AXI_4KB_BOUNDARY [0:0] (RW, 0x1) 4KB BOUNDARY PROCESS. ‘0’ : OFF , ‘1’ : ON */
        volatile uint32_t RESERVED0:31;  /* DMA.AXI_CFG4.RESERVED0 [31:1] (R, 0x0)  */
    };
} DMA_AXI_CFG4_t;
typedef volatile union _DMA_AXI_CFG5_t
{
    volatile uint32_t U;  /* Register-Access : DMA.AXI_CFG5*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t AXI_RDMA_QOS:4;  /* DMA.AXI_CFG5.AXI_RDMA_QOS [3:0] (RW, 0x0) AXI READ QUALITY OF SERVICE */
        volatile uint32_t RESERVED0:28;  /* DMA.AXI_CFG5.RESERVED0 [31:4] (R, 0x0)  */
    };
} DMA_AXI_CFG5_t;
typedef volatile union _DMA_AXI_CFG6_t
{
    volatile uint32_t U;  /* Register-Access : DMA.AXI_CFG6*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t AXI_WDMA_QOS:4;  /* DMA.AXI_CFG6.AXI_WDMA_QOS [3:0] (RW, 0x0) AXI WRITE QUALITY OF SERVICE */
        volatile uint32_t RESERVED0:28;  /* DMA.AXI_CFG6.RESERVED0 [31:4] (R, 0x0)  */
    };
} DMA_AXI_CFG6_t;
typedef volatile union _DMA_VIRT_RD_EN_t
{
    volatile uint32_t U;  /* Register-Access : DMA.VIRT_RD_EN*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t VIRTUAL_READ_EN:1;  /* DMA.VIRT_RD_EN.VIRTUAL_READ_EN [0:0] (RW, 0x0) READ AXI DATA FROM INTERNAL REGISTER FOR BIST */
        volatile uint32_t RESERVED0:31;  /* DMA.VIRT_RD_EN.RESERVED0 [31:1] (R, 0x0)  */
    };
} DMA_VIRT_RD_EN_t;
typedef volatile union _DMA_VIRT_RD_PTRN_t
{
    volatile uint32_t U;  /* Register-Access : DMA.VIRT_RD_PTRN*/
    struct {     /* Register-Bitfields-Access */
        volatile uint32_t VIRTUAL_READ_PATTERN:32;  /* DMA.VIRT_RD_PTRN.VIRTUAL_READ_PATTERN [31:0] (RW, 0x0) DUPLICATE REG_VIRTUAL_READ_PATTERN 16 TIMES FOR VIRTUAL AXI DATA */
    };
} DMA_VIRT_RD_PTRN_t;

/* DMA Module Definition */
typedef volatile struct _dxDMA_t
{
    DMA_AXI_BASE_ADDR_LOW_t AXI_BASE_ADDR_LOW;  /* DMA 0x0 */
    DMA_AXI_BASE_ADDR_HIGH_t AXI_BASE_ADDR_HIGH;  /* DMA 0x4 */
    DMA_AXI4_RADDR_t AXI4_RADDR;  /* DMA 0x8 */
    DMA_AXI4_WADDR_t AXI4_WADDR;  /* DMA 0xc */
    DMA_RSVD0_t RSVD0;  /* DMA 0x10 */
    DMA_RSVD1_t RSVD1;  /* DMA 0x14 */
    DMA_SRAM_DST_ADDR0_t SRAM_DST_ADDR0;  /* DMA 0x18 */
    DMA_SRAM_DST_ADDR1_t SRAM_DST_ADDR1;  /* DMA 0x1c */
    DMA_SRAM_SRC_ADDR_t SRAM_SRC_ADDR;  /* DMA 0x20 */
    DMA_READ_SIZE_t READ_SIZE;  /* DMA 0x24 */
    DMA_WRITE_SIZE_t WRITE_SIZE;  /* DMA 0x28 */
    DMA_RSVD2_t RSVD2;  /* DMA 0x2c */
    DMA_RSVD3_t RSVD3;  /* DMA 0x30 */
    DMA_RSVD4_t RSVD4;  /* DMA 0x34 */
    DMA_RSVD5_t RSVD5;  /* DMA 0x38 */
    DMA_IRQ_t IRQ;  /* DMA 0x3c */
    DMA_CMD_t CMD;  /* DMA 0x40 */
    DMA_STATUS_t STATUS;  /* DMA 0x44 */
    DMA_SFR_BASE_ADDR_LOW_t SFR_BASE_ADDR_LOW;  /* DMA 0x48 */
    DMA_SFR_BASE_ADDR_HIGH_t SFR_BASE_ADDR_HIGH;  /* DMA 0x4c */
    DMA_SFR_ADDR_OFFS_t SFR_ADDR_OFFS;  /* DMA 0x50 */
    DMA_SFR_START_ADDR_t SFR_START_ADDR;  /* DMA 0x54 */
    DMA_SFR_MOVE_CNT_t SFR_MOVE_CNT;  /* DMA 0x58 */
    DMA_SFR_ID_t SFR_ID;  /* DMA 0x5c */
    DMA_SFR_DMA_STATUS_t SFR_DMA_STATUS;  /* DMA 0x60 */
    DMA_DMA_CTRL_t DMA_CTRL;  /* DMA 0x64 */
    DMA_FLUSH_STATUS_t FLUSH_STATUS;  /* DMA 0x68 */
    DMA_AXI_CFG0_t AXI_CFG0;  /* DMA 0x6c */
    DMA_AXI_CFG1_t AXI_CFG1;  /* DMA 0x70 */
    DMA_AXI_CFG2_t AXI_CFG2;  /* DMA 0x74 */
    DMA_AXI_CFG3_t AXI_CFG3;  /* DMA 0x78 */
    DMA_AXI_CFG4_t AXI_CFG4;  /* DMA 0x7c */
    DMA_AXI_CFG5_t AXI_CFG5;  /* DMA 0x80 */
    DMA_AXI_CFG6_t AXI_CFG6;  /* DMA 0x84 */
    DMA_VIRT_RD_EN_t VIRT_RD_EN;  /* DMA 0x88 */
    DMA_VIRT_RD_PTRN_t VIRT_RD_PTRN;  /* DMA 0x8c */
} dxDMA_t;

/* Register access API */
/* DMA.AXI_BASE_ADDR_LOW (Category: DMA) */
#define READ_DMA_AXI_BASE_ADDR_LOW(base) npu_reg_read(base, REG_DMA_AXI_BASE_ADDR_LOW)
#define WRITE_DMA_AXI_BASE_ADDR_LOW(base, val) npu_reg_write(base, REG_DMA_AXI_BASE_ADDR_LOW, val)
/* DMA.AXI_BASE_ADDR_LOW.AXI4_BASE_ADDR_LOW (Category: DMA) */
#define DMA_AXI_BASE_ADDR_LOW_AXI4_BASE_ADDR_LOW_LEN        32
#define DMA_AXI_BASE_ADDR_LOW_AXI4_BASE_ADDR_LOW_OFFSET     0
#define DMA_AXI_BASE_ADDR_LOW_AXI4_BASE_ADDR_LOW_MASK_BITS  0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define DMA_AXI_BASE_ADDR_LOW_AXI4_BASE_ADDR_LOW_MASK       0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define READ_DMA_AXI_BASE_ADDR_LOW_AXI4_BASE_ADDR_LOW(base) \
    npu_reg_read_mask(base, REG_DMA_AXI_BASE_ADDR_LOW, DMA_AXI_BASE_ADDR_LOW_AXI4_BASE_ADDR_LOW_MASK, DMA_AXI_BASE_ADDR_LOW_AXI4_BASE_ADDR_LOW_OFFSET)
#define WRITE_DMA_AXI_BASE_ADDR_LOW_AXI4_BASE_ADDR_LOW(base, val) \
    npu_reg_write_mask(base, REG_DMA_AXI_BASE_ADDR_LOW, val, DMA_AXI_BASE_ADDR_LOW_AXI4_BASE_ADDR_LOW_MASK, DMA_AXI_BASE_ADDR_LOW_AXI4_BASE_ADDR_LOW_OFFSET)

/* DMA.AXI_BASE_ADDR_HIGH (Category: DMA) */
#define READ_DMA_AXI_BASE_ADDR_HIGH(base) npu_reg_read(base, REG_DMA_AXI_BASE_ADDR_HIGH)
#define WRITE_DMA_AXI_BASE_ADDR_HIGH(base, val) npu_reg_write(base, REG_DMA_AXI_BASE_ADDR_HIGH, val)
/* DMA.AXI_BASE_ADDR_HIGH.AXI4_BASE_ADDR_HIGH (Category: DMA) */
#define DMA_AXI_BASE_ADDR_HIGH_AXI4_BASE_ADDR_HIGH_LEN        32
#define DMA_AXI_BASE_ADDR_HIGH_AXI4_BASE_ADDR_HIGH_OFFSET     0
#define DMA_AXI_BASE_ADDR_HIGH_AXI4_BASE_ADDR_HIGH_MASK_BITS  0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define DMA_AXI_BASE_ADDR_HIGH_AXI4_BASE_ADDR_HIGH_MASK       0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define READ_DMA_AXI_BASE_ADDR_HIGH_AXI4_BASE_ADDR_HIGH(base) \
    npu_reg_read_mask(base, REG_DMA_AXI_BASE_ADDR_HIGH, DMA_AXI_BASE_ADDR_HIGH_AXI4_BASE_ADDR_HIGH_MASK, DMA_AXI_BASE_ADDR_HIGH_AXI4_BASE_ADDR_HIGH_OFFSET)
#define WRITE_DMA_AXI_BASE_ADDR_HIGH_AXI4_BASE_ADDR_HIGH(base, val) \
    npu_reg_write_mask(base, REG_DMA_AXI_BASE_ADDR_HIGH, val, DMA_AXI_BASE_ADDR_HIGH_AXI4_BASE_ADDR_HIGH_MASK, DMA_AXI_BASE_ADDR_HIGH_AXI4_BASE_ADDR_HIGH_OFFSET)

/* DMA.AXI4_RADDR (Category: DMA) */
#define READ_DMA_AXI4_RADDR(base) npu_reg_read(base, REG_DMA_AXI4_RADDR)
#define WRITE_DMA_AXI4_RADDR(base, val) npu_reg_write(base, REG_DMA_AXI4_RADDR, val)
/* DMA.AXI4_RADDR.DMA0_AXI4_RADDR (Category: DMA) */
#define DMA_AXI4_RADDR_DMA0_AXI4_RADDR_LEN        32
#define DMA_AXI4_RADDR_DMA0_AXI4_RADDR_OFFSET     0
#define DMA_AXI4_RADDR_DMA0_AXI4_RADDR_MASK_BITS  0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define DMA_AXI4_RADDR_DMA0_AXI4_RADDR_MASK       0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define READ_DMA_AXI4_RADDR_DMA0_AXI4_RADDR(base) \
    npu_reg_read_mask(base, REG_DMA_AXI4_RADDR, DMA_AXI4_RADDR_DMA0_AXI4_RADDR_MASK, DMA_AXI4_RADDR_DMA0_AXI4_RADDR_OFFSET)
#define WRITE_DMA_AXI4_RADDR_DMA0_AXI4_RADDR(base, val) \
    npu_reg_write_mask(base, REG_DMA_AXI4_RADDR, val, DMA_AXI4_RADDR_DMA0_AXI4_RADDR_MASK, DMA_AXI4_RADDR_DMA0_AXI4_RADDR_OFFSET)

/* DMA.AXI4_WADDR (Category: DMA) */
#define READ_DMA_AXI4_WADDR(base) npu_reg_read(base, REG_DMA_AXI4_WADDR)
#define WRITE_DMA_AXI4_WADDR(base, val) npu_reg_write(base, REG_DMA_AXI4_WADDR, val)
/* DMA.AXI4_WADDR.DMA0_AXI4_WADDR (Category: DMA) */
#define DMA_AXI4_WADDR_DMA0_AXI4_WADDR_LEN        32
#define DMA_AXI4_WADDR_DMA0_AXI4_WADDR_OFFSET     0
#define DMA_AXI4_WADDR_DMA0_AXI4_WADDR_MASK_BITS  0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define DMA_AXI4_WADDR_DMA0_AXI4_WADDR_MASK       0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define READ_DMA_AXI4_WADDR_DMA0_AXI4_WADDR(base) \
    npu_reg_read_mask(base, REG_DMA_AXI4_WADDR, DMA_AXI4_WADDR_DMA0_AXI4_WADDR_MASK, DMA_AXI4_WADDR_DMA0_AXI4_WADDR_OFFSET)
#define WRITE_DMA_AXI4_WADDR_DMA0_AXI4_WADDR(base, val) \
    npu_reg_write_mask(base, REG_DMA_AXI4_WADDR, val, DMA_AXI4_WADDR_DMA0_AXI4_WADDR_MASK, DMA_AXI4_WADDR_DMA0_AXI4_WADDR_OFFSET)

/* DMA.RSVD0 (Category: DMA) */
#define READ_DMA_RSVD0(base) npu_reg_read(base, REG_DMA_RSVD0)
#define WRITE_DMA_RSVD0(base, val) npu_reg_write(base, REG_DMA_RSVD0, val)
/* DMA.RSVD0.RESERVED0 (Category: DMA) */
#define DMA_RSVD0_RESERVED0_LEN        32
#define DMA_RSVD0_RESERVED0_OFFSET     0
#define DMA_RSVD0_RESERVED0_MASK_BITS  0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define DMA_RSVD0_RESERVED0_MASK       0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define READ_DMA_RSVD0_RESERVED0(base) \
    npu_reg_read_mask(base, REG_DMA_RSVD0, DMA_RSVD0_RESERVED0_MASK, DMA_RSVD0_RESERVED0_OFFSET)
#define WRITE_DMA_RSVD0_RESERVED0(base, val) \
    npu_reg_write_mask(base, REG_DMA_RSVD0, val, DMA_RSVD0_RESERVED0_MASK, DMA_RSVD0_RESERVED0_OFFSET)

/* DMA.RSVD1 (Category: DMA) */
#define READ_DMA_RSVD1(base) npu_reg_read(base, REG_DMA_RSVD1)
#define WRITE_DMA_RSVD1(base, val) npu_reg_write(base, REG_DMA_RSVD1, val)
/* DMA.RSVD1.RESERVED0 (Category: DMA) */
#define DMA_RSVD1_RESERVED0_LEN        32
#define DMA_RSVD1_RESERVED0_OFFSET     0
#define DMA_RSVD1_RESERVED0_MASK_BITS  0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define DMA_RSVD1_RESERVED0_MASK       0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define READ_DMA_RSVD1_RESERVED0(base) \
    npu_reg_read_mask(base, REG_DMA_RSVD1, DMA_RSVD1_RESERVED0_MASK, DMA_RSVD1_RESERVED0_OFFSET)
#define WRITE_DMA_RSVD1_RESERVED0(base, val) \
    npu_reg_write_mask(base, REG_DMA_RSVD1, val, DMA_RSVD1_RESERVED0_MASK, DMA_RSVD1_RESERVED0_OFFSET)

/* DMA.SRAM_DST_ADDR0 (Category: DMA) */
#define READ_DMA_SRAM_DST_ADDR0(base) npu_reg_read(base, REG_DMA_SRAM_DST_ADDR0)
#define WRITE_DMA_SRAM_DST_ADDR0(base, val) npu_reg_write(base, REG_DMA_SRAM_DST_ADDR0, val)
/* DMA.SRAM_DST_ADDR0.DMA0_SRAM_DST_ADDR0 (Category: DMA) */
#define DMA_SRAM_DST_ADDR0_DMA0_SRAM_DST_ADDR0_LEN        32
#define DMA_SRAM_DST_ADDR0_DMA0_SRAM_DST_ADDR0_OFFSET     0
#define DMA_SRAM_DST_ADDR0_DMA0_SRAM_DST_ADDR0_MASK_BITS  0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define DMA_SRAM_DST_ADDR0_DMA0_SRAM_DST_ADDR0_MASK       0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define READ_DMA_SRAM_DST_ADDR0_DMA0_SRAM_DST_ADDR0(base) \
    npu_reg_read_mask(base, REG_DMA_SRAM_DST_ADDR0, DMA_SRAM_DST_ADDR0_DMA0_SRAM_DST_ADDR0_MASK, DMA_SRAM_DST_ADDR0_DMA0_SRAM_DST_ADDR0_OFFSET)
#define WRITE_DMA_SRAM_DST_ADDR0_DMA0_SRAM_DST_ADDR0(base, val) \
    npu_reg_write_mask(base, REG_DMA_SRAM_DST_ADDR0, val, DMA_SRAM_DST_ADDR0_DMA0_SRAM_DST_ADDR0_MASK, DMA_SRAM_DST_ADDR0_DMA0_SRAM_DST_ADDR0_OFFSET)

/* DMA.SRAM_DST_ADDR1 (Category: DMA) */
#define READ_DMA_SRAM_DST_ADDR1(base) npu_reg_read(base, REG_DMA_SRAM_DST_ADDR1)
#define WRITE_DMA_SRAM_DST_ADDR1(base, val) npu_reg_write(base, REG_DMA_SRAM_DST_ADDR1, val)
/* DMA.SRAM_DST_ADDR1.DMA0_SRAM_DST_ADDR1 (Category: DMA) */
#define DMA_SRAM_DST_ADDR1_DMA0_SRAM_DST_ADDR1_LEN        32
#define DMA_SRAM_DST_ADDR1_DMA0_SRAM_DST_ADDR1_OFFSET     0
#define DMA_SRAM_DST_ADDR1_DMA0_SRAM_DST_ADDR1_MASK_BITS  0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define DMA_SRAM_DST_ADDR1_DMA0_SRAM_DST_ADDR1_MASK       0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define READ_DMA_SRAM_DST_ADDR1_DMA0_SRAM_DST_ADDR1(base) \
    npu_reg_read_mask(base, REG_DMA_SRAM_DST_ADDR1, DMA_SRAM_DST_ADDR1_DMA0_SRAM_DST_ADDR1_MASK, DMA_SRAM_DST_ADDR1_DMA0_SRAM_DST_ADDR1_OFFSET)
#define WRITE_DMA_SRAM_DST_ADDR1_DMA0_SRAM_DST_ADDR1(base, val) \
    npu_reg_write_mask(base, REG_DMA_SRAM_DST_ADDR1, val, DMA_SRAM_DST_ADDR1_DMA0_SRAM_DST_ADDR1_MASK, DMA_SRAM_DST_ADDR1_DMA0_SRAM_DST_ADDR1_OFFSET)

/* DMA.SRAM_SRC_ADDR (Category: DMA) */
#define READ_DMA_SRAM_SRC_ADDR(base) npu_reg_read(base, REG_DMA_SRAM_SRC_ADDR)
#define WRITE_DMA_SRAM_SRC_ADDR(base, val) npu_reg_write(base, REG_DMA_SRAM_SRC_ADDR, val)
/* DMA.SRAM_SRC_ADDR.DMA0_SRAM_SRC_ADDR (Category: DMA) */
#define DMA_SRAM_SRC_ADDR_DMA0_SRAM_SRC_ADDR_LEN        32
#define DMA_SRAM_SRC_ADDR_DMA0_SRAM_SRC_ADDR_OFFSET     0
#define DMA_SRAM_SRC_ADDR_DMA0_SRAM_SRC_ADDR_MASK_BITS  0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define DMA_SRAM_SRC_ADDR_DMA0_SRAM_SRC_ADDR_MASK       0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define READ_DMA_SRAM_SRC_ADDR_DMA0_SRAM_SRC_ADDR(base) \
    npu_reg_read_mask(base, REG_DMA_SRAM_SRC_ADDR, DMA_SRAM_SRC_ADDR_DMA0_SRAM_SRC_ADDR_MASK, DMA_SRAM_SRC_ADDR_DMA0_SRAM_SRC_ADDR_OFFSET)
#define WRITE_DMA_SRAM_SRC_ADDR_DMA0_SRAM_SRC_ADDR(base, val) \
    npu_reg_write_mask(base, REG_DMA_SRAM_SRC_ADDR, val, DMA_SRAM_SRC_ADDR_DMA0_SRAM_SRC_ADDR_MASK, DMA_SRAM_SRC_ADDR_DMA0_SRAM_SRC_ADDR_OFFSET)

/* DMA.READ_SIZE (Category: DMA) */
#define READ_DMA_READ_SIZE(base) npu_reg_read(base, REG_DMA_READ_SIZE)
#define WRITE_DMA_READ_SIZE(base, val) npu_reg_write(base, REG_DMA_READ_SIZE, val)
/* DMA.READ_SIZE.RESERVED1 (Category: DMA) */
#define DMA_READ_SIZE_RESERVED1_LEN        6
#define DMA_READ_SIZE_RESERVED1_OFFSET     0
#define DMA_READ_SIZE_RESERVED1_MASK_BITS  0x0000003f /* [5:0] 0b00000000000000000000000000111111 */
#define DMA_READ_SIZE_RESERVED1_MASK       0x0000003f /* [5:0] 0b00000000000000000000000000111111 */
#define READ_DMA_READ_SIZE_RESERVED1(base) \
    npu_reg_read_mask(base, REG_DMA_READ_SIZE, DMA_READ_SIZE_RESERVED1_MASK, DMA_READ_SIZE_RESERVED1_OFFSET)
#define WRITE_DMA_READ_SIZE_RESERVED1(base, val) \
    npu_reg_write_mask(base, REG_DMA_READ_SIZE, val, DMA_READ_SIZE_RESERVED1_MASK, DMA_READ_SIZE_RESERVED1_OFFSET)

/* DMA.READ_SIZE.DMA0_READ_SIZE (Category: DMA) */
#define DMA_READ_SIZE_DMA0_READ_SIZE_LEN        16
#define DMA_READ_SIZE_DMA0_READ_SIZE_OFFSET     6
#define DMA_READ_SIZE_DMA0_READ_SIZE_MASK_BITS  0x0000ffff /* [15:0] 0b00000000000000001111111111111111 */
#define DMA_READ_SIZE_DMA0_READ_SIZE_MASK       0x003fffc0 /* [21:6] 0b00000000001111111111111111000000 */
#define READ_DMA_READ_SIZE_DMA0_READ_SIZE(base) \
    npu_reg_read_mask(base, REG_DMA_READ_SIZE, DMA_READ_SIZE_DMA0_READ_SIZE_MASK, DMA_READ_SIZE_DMA0_READ_SIZE_OFFSET)
#define WRITE_DMA_READ_SIZE_DMA0_READ_SIZE(base, val) \
    npu_reg_write_mask(base, REG_DMA_READ_SIZE, val, DMA_READ_SIZE_DMA0_READ_SIZE_MASK, DMA_READ_SIZE_DMA0_READ_SIZE_OFFSET)

/* DMA.READ_SIZE.RESERVED0 (Category: DMA) */
#define DMA_READ_SIZE_RESERVED0_LEN        10
#define DMA_READ_SIZE_RESERVED0_OFFSET     22
#define DMA_READ_SIZE_RESERVED0_MASK_BITS  0x000003ff /* [9:0] 0b00000000000000000000001111111111 */
#define DMA_READ_SIZE_RESERVED0_MASK       0xffc00000 /* [31:22] 0b11111111110000000000000000000000 */
#define READ_DMA_READ_SIZE_RESERVED0(base) \
    npu_reg_read_mask(base, REG_DMA_READ_SIZE, DMA_READ_SIZE_RESERVED0_MASK, DMA_READ_SIZE_RESERVED0_OFFSET)
#define WRITE_DMA_READ_SIZE_RESERVED0(base, val) \
    npu_reg_write_mask(base, REG_DMA_READ_SIZE, val, DMA_READ_SIZE_RESERVED0_MASK, DMA_READ_SIZE_RESERVED0_OFFSET)

/* DMA.WRITE_SIZE (Category: DMA) */
#define READ_DMA_WRITE_SIZE(base) npu_reg_read(base, REG_DMA_WRITE_SIZE)
#define WRITE_DMA_WRITE_SIZE(base, val) npu_reg_write(base, REG_DMA_WRITE_SIZE, val)
/* DMA.WRITE_SIZE.RESERVED1 (Category: DMA) */
#define DMA_WRITE_SIZE_RESERVED1_LEN        6
#define DMA_WRITE_SIZE_RESERVED1_OFFSET     0
#define DMA_WRITE_SIZE_RESERVED1_MASK_BITS  0x0000003f /* [5:0] 0b00000000000000000000000000111111 */
#define DMA_WRITE_SIZE_RESERVED1_MASK       0x0000003f /* [5:0] 0b00000000000000000000000000111111 */
#define READ_DMA_WRITE_SIZE_RESERVED1(base) \
    npu_reg_read_mask(base, REG_DMA_WRITE_SIZE, DMA_WRITE_SIZE_RESERVED1_MASK, DMA_WRITE_SIZE_RESERVED1_OFFSET)
#define WRITE_DMA_WRITE_SIZE_RESERVED1(base, val) \
    npu_reg_write_mask(base, REG_DMA_WRITE_SIZE, val, DMA_WRITE_SIZE_RESERVED1_MASK, DMA_WRITE_SIZE_RESERVED1_OFFSET)

/* DMA.WRITE_SIZE.DMA0_WRITE_SIZE (Category: DMA) */
#define DMA_WRITE_SIZE_DMA0_WRITE_SIZE_LEN        16
#define DMA_WRITE_SIZE_DMA0_WRITE_SIZE_OFFSET     6
#define DMA_WRITE_SIZE_DMA0_WRITE_SIZE_MASK_BITS  0x0000ffff /* [15:0] 0b00000000000000001111111111111111 */
#define DMA_WRITE_SIZE_DMA0_WRITE_SIZE_MASK       0x003fffc0 /* [21:6] 0b00000000001111111111111111000000 */
#define READ_DMA_WRITE_SIZE_DMA0_WRITE_SIZE(base) \
    npu_reg_read_mask(base, REG_DMA_WRITE_SIZE, DMA_WRITE_SIZE_DMA0_WRITE_SIZE_MASK, DMA_WRITE_SIZE_DMA0_WRITE_SIZE_OFFSET)
#define WRITE_DMA_WRITE_SIZE_DMA0_WRITE_SIZE(base, val) \
    npu_reg_write_mask(base, REG_DMA_WRITE_SIZE, val, DMA_WRITE_SIZE_DMA0_WRITE_SIZE_MASK, DMA_WRITE_SIZE_DMA0_WRITE_SIZE_OFFSET)

/* DMA.WRITE_SIZE.RESERVED0 (Category: DMA) */
#define DMA_WRITE_SIZE_RESERVED0_LEN        10
#define DMA_WRITE_SIZE_RESERVED0_OFFSET     22
#define DMA_WRITE_SIZE_RESERVED0_MASK_BITS  0x000003ff /* [9:0] 0b00000000000000000000001111111111 */
#define DMA_WRITE_SIZE_RESERVED0_MASK       0xffc00000 /* [31:22] 0b11111111110000000000000000000000 */
#define READ_DMA_WRITE_SIZE_RESERVED0(base) \
    npu_reg_read_mask(base, REG_DMA_WRITE_SIZE, DMA_WRITE_SIZE_RESERVED0_MASK, DMA_WRITE_SIZE_RESERVED0_OFFSET)
#define WRITE_DMA_WRITE_SIZE_RESERVED0(base, val) \
    npu_reg_write_mask(base, REG_DMA_WRITE_SIZE, val, DMA_WRITE_SIZE_RESERVED0_MASK, DMA_WRITE_SIZE_RESERVED0_OFFSET)

/* DMA.RSVD2 (Category: DMA) */
#define READ_DMA_RSVD2(base) npu_reg_read(base, REG_DMA_RSVD2)
#define WRITE_DMA_RSVD2(base, val) npu_reg_write(base, REG_DMA_RSVD2, val)
/* DMA.RSVD2.RESERVED0 (Category: DMA) */
#define DMA_RSVD2_RESERVED0_LEN        32
#define DMA_RSVD2_RESERVED0_OFFSET     0
#define DMA_RSVD2_RESERVED0_MASK_BITS  0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define DMA_RSVD2_RESERVED0_MASK       0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define READ_DMA_RSVD2_RESERVED0(base) \
    npu_reg_read_mask(base, REG_DMA_RSVD2, DMA_RSVD2_RESERVED0_MASK, DMA_RSVD2_RESERVED0_OFFSET)
#define WRITE_DMA_RSVD2_RESERVED0(base, val) \
    npu_reg_write_mask(base, REG_DMA_RSVD2, val, DMA_RSVD2_RESERVED0_MASK, DMA_RSVD2_RESERVED0_OFFSET)

/* DMA.RSVD3 (Category: DMA) */
#define READ_DMA_RSVD3(base) npu_reg_read(base, REG_DMA_RSVD3)
#define WRITE_DMA_RSVD3(base, val) npu_reg_write(base, REG_DMA_RSVD3, val)
/* DMA.RSVD3.RESERVED0 (Category: DMA) */
#define DMA_RSVD3_RESERVED0_LEN        32
#define DMA_RSVD3_RESERVED0_OFFSET     0
#define DMA_RSVD3_RESERVED0_MASK_BITS  0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define DMA_RSVD3_RESERVED0_MASK       0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define READ_DMA_RSVD3_RESERVED0(base) \
    npu_reg_read_mask(base, REG_DMA_RSVD3, DMA_RSVD3_RESERVED0_MASK, DMA_RSVD3_RESERVED0_OFFSET)
#define WRITE_DMA_RSVD3_RESERVED0(base, val) \
    npu_reg_write_mask(base, REG_DMA_RSVD3, val, DMA_RSVD3_RESERVED0_MASK, DMA_RSVD3_RESERVED0_OFFSET)

/* DMA.RSVD4 (Category: DMA) */
#define READ_DMA_RSVD4(base) npu_reg_read(base, REG_DMA_RSVD4)
#define WRITE_DMA_RSVD4(base, val) npu_reg_write(base, REG_DMA_RSVD4, val)
/* DMA.RSVD4.RESERVED0 (Category: DMA) */
#define DMA_RSVD4_RESERVED0_LEN        32
#define DMA_RSVD4_RESERVED0_OFFSET     0
#define DMA_RSVD4_RESERVED0_MASK_BITS  0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define DMA_RSVD4_RESERVED0_MASK       0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define READ_DMA_RSVD4_RESERVED0(base) \
    npu_reg_read_mask(base, REG_DMA_RSVD4, DMA_RSVD4_RESERVED0_MASK, DMA_RSVD4_RESERVED0_OFFSET)
#define WRITE_DMA_RSVD4_RESERVED0(base, val) \
    npu_reg_write_mask(base, REG_DMA_RSVD4, val, DMA_RSVD4_RESERVED0_MASK, DMA_RSVD4_RESERVED0_OFFSET)

/* DMA.RSVD5 (Category: DMA) */
#define READ_DMA_RSVD5(base) npu_reg_read(base, REG_DMA_RSVD5)
#define WRITE_DMA_RSVD5(base, val) npu_reg_write(base, REG_DMA_RSVD5, val)
/* DMA.RSVD5.RESERVED0 (Category: DMA) */
#define DMA_RSVD5_RESERVED0_LEN        32
#define DMA_RSVD5_RESERVED0_OFFSET     0
#define DMA_RSVD5_RESERVED0_MASK_BITS  0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define DMA_RSVD5_RESERVED0_MASK       0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define READ_DMA_RSVD5_RESERVED0(base) \
    npu_reg_read_mask(base, REG_DMA_RSVD5, DMA_RSVD5_RESERVED0_MASK, DMA_RSVD5_RESERVED0_OFFSET)
#define WRITE_DMA_RSVD5_RESERVED0(base, val) \
    npu_reg_write_mask(base, REG_DMA_RSVD5, val, DMA_RSVD5_RESERVED0_MASK, DMA_RSVD5_RESERVED0_OFFSET)

/* DMA.IRQ (Category: DMA) */
#define READ_DMA_IRQ(base) npu_reg_read(base, REG_DMA_IRQ)
#define WRITE_DMA_IRQ(base, val) npu_reg_write(base, REG_DMA_IRQ, val)
/* DMA.IRQ.DMA_IRQ_EN (Category: DMA) */
#define DMA_IRQ_DMA_IRQ_EN_LEN        5
#define DMA_IRQ_DMA_IRQ_EN_OFFSET     0
#define DMA_IRQ_DMA_IRQ_EN_MASK_BITS  0x0000001f /* [4:0] 0b00000000000000000000000000011111 */
#define DMA_IRQ_DMA_IRQ_EN_MASK       0x0000001f /* [4:0] 0b00000000000000000000000000011111 */
#define READ_DMA_IRQ_DMA_IRQ_EN(base) \
    npu_reg_read_mask(base, REG_DMA_IRQ, DMA_IRQ_DMA_IRQ_EN_MASK, DMA_IRQ_DMA_IRQ_EN_OFFSET)
#define WRITE_DMA_IRQ_DMA_IRQ_EN(base, val) \
    npu_reg_write_mask(base, REG_DMA_IRQ, val, DMA_IRQ_DMA_IRQ_EN_MASK, DMA_IRQ_DMA_IRQ_EN_OFFSET)

/* DMA.IRQ.RESERVED0 (Category: DMA) */
#define DMA_IRQ_RESERVED0_LEN        27
#define DMA_IRQ_RESERVED0_OFFSET     5
#define DMA_IRQ_RESERVED0_MASK_BITS  0x07ffffff /* [26:0] 0b00000111111111111111111111111111 */
#define DMA_IRQ_RESERVED0_MASK       0xffffffe0 /* [31:5] 0b11111111111111111111111111100000 */
#define READ_DMA_IRQ_RESERVED0(base) \
    npu_reg_read_mask(base, REG_DMA_IRQ, DMA_IRQ_RESERVED0_MASK, DMA_IRQ_RESERVED0_OFFSET)
#define WRITE_DMA_IRQ_RESERVED0(base, val) \
    npu_reg_write_mask(base, REG_DMA_IRQ, val, DMA_IRQ_RESERVED0_MASK, DMA_IRQ_RESERVED0_OFFSET)

/* DMA.CMD (Category: DMA) */
#define READ_DMA_CMD(base) npu_reg_read(base, REG_DMA_CMD)
#define WRITE_DMA_CMD(base, val) npu_reg_write(base, REG_DMA_CMD, val)
/* DMA.CMD.DMA_START_CMD (Category: DMA) */
#define DMA_CMD_DMA_START_CMD_LEN        5
#define DMA_CMD_DMA_START_CMD_OFFSET     0
#define DMA_CMD_DMA_START_CMD_MASK_BITS  0x0000001f /* [4:0] 0b00000000000000000000000000011111 */
#define DMA_CMD_DMA_START_CMD_MASK       0x0000001f /* [4:0] 0b00000000000000000000000000011111 */
#define READ_DMA_CMD_DMA_START_CMD(base) \
    npu_reg_read_mask(base, REG_DMA_CMD, DMA_CMD_DMA_START_CMD_MASK, DMA_CMD_DMA_START_CMD_OFFSET)
#define WRITE_DMA_CMD_DMA_START_CMD(base, val) \
    npu_reg_write_mask(base, REG_DMA_CMD, val, DMA_CMD_DMA_START_CMD_MASK, DMA_CMD_DMA_START_CMD_OFFSET)

/* DMA.CMD.RESERVED0 (Category: DMA) */
#define DMA_CMD_RESERVED0_LEN        27
#define DMA_CMD_RESERVED0_OFFSET     5
#define DMA_CMD_RESERVED0_MASK_BITS  0x07ffffff /* [26:0] 0b00000111111111111111111111111111 */
#define DMA_CMD_RESERVED0_MASK       0xffffffe0 /* [31:5] 0b11111111111111111111111111100000 */
#define READ_DMA_CMD_RESERVED0(base) \
    npu_reg_read_mask(base, REG_DMA_CMD, DMA_CMD_RESERVED0_MASK, DMA_CMD_RESERVED0_OFFSET)
#define WRITE_DMA_CMD_RESERVED0(base, val) \
    npu_reg_write_mask(base, REG_DMA_CMD, val, DMA_CMD_RESERVED0_MASK, DMA_CMD_RESERVED0_OFFSET)

/* DMA.STATUS (Category: DMA) */
#define READ_DMA_STATUS(base) npu_reg_read(base, REG_DMA_STATUS)
#define WRITE_DMA_STATUS(base, val) npu_reg_write(base, REG_DMA_STATUS, val)
/* DMA.STATUS.DMA_DONE_STATUS (Category: DMA) */
#define DMA_STATUS_DMA_DONE_STATUS_LEN        9
#define DMA_STATUS_DMA_DONE_STATUS_OFFSET     0
#define DMA_STATUS_DMA_DONE_STATUS_MASK_BITS  0x000001ff /* [8:0] 0b00000000000000000000000111111111 */
#define DMA_STATUS_DMA_DONE_STATUS_MASK       0x000001ff /* [8:0] 0b00000000000000000000000111111111 */
#define READ_DMA_STATUS_DMA_DONE_STATUS(base) \
    npu_reg_read_mask(base, REG_DMA_STATUS, DMA_STATUS_DMA_DONE_STATUS_MASK, DMA_STATUS_DMA_DONE_STATUS_OFFSET)
#define WRITE_DMA_STATUS_DMA_DONE_STATUS(base, val) \
    npu_reg_write_mask(base, REG_DMA_STATUS, val, DMA_STATUS_DMA_DONE_STATUS_MASK, DMA_STATUS_DMA_DONE_STATUS_OFFSET)

/* DMA.STATUS.RESERVED0 (Category: DMA) */
#define DMA_STATUS_RESERVED0_LEN        23
#define DMA_STATUS_RESERVED0_OFFSET     9
#define DMA_STATUS_RESERVED0_MASK_BITS  0x007fffff /* [22:0] 0b00000000011111111111111111111111 */
#define DMA_STATUS_RESERVED0_MASK       0xfffffe00 /* [31:9] 0b11111111111111111111111000000000 */
#define READ_DMA_STATUS_RESERVED0(base) \
    npu_reg_read_mask(base, REG_DMA_STATUS, DMA_STATUS_RESERVED0_MASK, DMA_STATUS_RESERVED0_OFFSET)
#define WRITE_DMA_STATUS_RESERVED0(base, val) \
    npu_reg_write_mask(base, REG_DMA_STATUS, val, DMA_STATUS_RESERVED0_MASK, DMA_STATUS_RESERVED0_OFFSET)

/* DMA.SFR_BASE_ADDR_LOW (Category: DMA) */
#define READ_DMA_SFR_BASE_ADDR_LOW(base) npu_reg_read(base, REG_DMA_SFR_BASE_ADDR_LOW)
#define WRITE_DMA_SFR_BASE_ADDR_LOW(base, val) npu_reg_write(base, REG_DMA_SFR_BASE_ADDR_LOW, val)
/* DMA.SFR_BASE_ADDR_LOW.SFR_BASE_ADDR_LOW (Category: DMA) */
#define DMA_SFR_BASE_ADDR_LOW_SFR_BASE_ADDR_LOW_LEN        32
#define DMA_SFR_BASE_ADDR_LOW_SFR_BASE_ADDR_LOW_OFFSET     0
#define DMA_SFR_BASE_ADDR_LOW_SFR_BASE_ADDR_LOW_MASK_BITS  0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define DMA_SFR_BASE_ADDR_LOW_SFR_BASE_ADDR_LOW_MASK       0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define READ_DMA_SFR_BASE_ADDR_LOW_SFR_BASE_ADDR_LOW(base) \
    npu_reg_read_mask(base, REG_DMA_SFR_BASE_ADDR_LOW, DMA_SFR_BASE_ADDR_LOW_SFR_BASE_ADDR_LOW_MASK, DMA_SFR_BASE_ADDR_LOW_SFR_BASE_ADDR_LOW_OFFSET)
#define WRITE_DMA_SFR_BASE_ADDR_LOW_SFR_BASE_ADDR_LOW(base, val) \
    npu_reg_write_mask(base, REG_DMA_SFR_BASE_ADDR_LOW, val, DMA_SFR_BASE_ADDR_LOW_SFR_BASE_ADDR_LOW_MASK, DMA_SFR_BASE_ADDR_LOW_SFR_BASE_ADDR_LOW_OFFSET)

/* DMA.SFR_BASE_ADDR_HIGH (Category: DMA) */
#define READ_DMA_SFR_BASE_ADDR_HIGH(base) npu_reg_read(base, REG_DMA_SFR_BASE_ADDR_HIGH)
#define WRITE_DMA_SFR_BASE_ADDR_HIGH(base, val) npu_reg_write(base, REG_DMA_SFR_BASE_ADDR_HIGH, val)
/* DMA.SFR_BASE_ADDR_HIGH.SFR_BASE_ADDR_HIGH (Category: DMA) */
#define DMA_SFR_BASE_ADDR_HIGH_SFR_BASE_ADDR_HIGH_LEN        32
#define DMA_SFR_BASE_ADDR_HIGH_SFR_BASE_ADDR_HIGH_OFFSET     0
#define DMA_SFR_BASE_ADDR_HIGH_SFR_BASE_ADDR_HIGH_MASK_BITS  0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define DMA_SFR_BASE_ADDR_HIGH_SFR_BASE_ADDR_HIGH_MASK       0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define READ_DMA_SFR_BASE_ADDR_HIGH_SFR_BASE_ADDR_HIGH(base) \
    npu_reg_read_mask(base, REG_DMA_SFR_BASE_ADDR_HIGH, DMA_SFR_BASE_ADDR_HIGH_SFR_BASE_ADDR_HIGH_MASK, DMA_SFR_BASE_ADDR_HIGH_SFR_BASE_ADDR_HIGH_OFFSET)
#define WRITE_DMA_SFR_BASE_ADDR_HIGH_SFR_BASE_ADDR_HIGH(base, val) \
    npu_reg_write_mask(base, REG_DMA_SFR_BASE_ADDR_HIGH, val, DMA_SFR_BASE_ADDR_HIGH_SFR_BASE_ADDR_HIGH_MASK, DMA_SFR_BASE_ADDR_HIGH_SFR_BASE_ADDR_HIGH_OFFSET)

/* DMA.SFR_ADDR_OFFS (Category: DMA) */
#define READ_DMA_SFR_ADDR_OFFS(base) npu_reg_read(base, REG_DMA_SFR_ADDR_OFFS)
#define WRITE_DMA_SFR_ADDR_OFFS(base, val) npu_reg_write(base, REG_DMA_SFR_ADDR_OFFS, val)
/* DMA.SFR_ADDR_OFFS.RESERVED0 (Category: DMA) */
#define DMA_SFR_ADDR_OFFS_RESERVED0_LEN        4
#define DMA_SFR_ADDR_OFFS_RESERVED0_OFFSET     0
#define DMA_SFR_ADDR_OFFS_RESERVED0_MASK_BITS  0x0000000f /* [3:0] 0b00000000000000000000000000001111 */
#define DMA_SFR_ADDR_OFFS_RESERVED0_MASK       0x0000000f /* [3:0] 0b00000000000000000000000000001111 */
#define READ_DMA_SFR_ADDR_OFFS_RESERVED0(base) \
    npu_reg_read_mask(base, REG_DMA_SFR_ADDR_OFFS, DMA_SFR_ADDR_OFFS_RESERVED0_MASK, DMA_SFR_ADDR_OFFS_RESERVED0_OFFSET)
#define WRITE_DMA_SFR_ADDR_OFFS_RESERVED0(base, val) \
    npu_reg_write_mask(base, REG_DMA_SFR_ADDR_OFFS, val, DMA_SFR_ADDR_OFFS_RESERVED0_MASK, DMA_SFR_ADDR_OFFS_RESERVED0_OFFSET)

/* DMA.SFR_ADDR_OFFS.SFR_ADDR_OFFS (Category: DMA) */
#define DMA_SFR_ADDR_OFFS_SFR_ADDR_OFFS_LEN        28
#define DMA_SFR_ADDR_OFFS_SFR_ADDR_OFFS_OFFSET     4
#define DMA_SFR_ADDR_OFFS_SFR_ADDR_OFFS_MASK_BITS  0x0fffffff /* [27:0] 0b00001111111111111111111111111111 */
#define DMA_SFR_ADDR_OFFS_SFR_ADDR_OFFS_MASK       0xfffffff0 /* [31:4] 0b11111111111111111111111111110000 */
#define READ_DMA_SFR_ADDR_OFFS_SFR_ADDR_OFFS(base) \
    npu_reg_read_mask(base, REG_DMA_SFR_ADDR_OFFS, DMA_SFR_ADDR_OFFS_SFR_ADDR_OFFS_MASK, DMA_SFR_ADDR_OFFS_SFR_ADDR_OFFS_OFFSET)
#define WRITE_DMA_SFR_ADDR_OFFS_SFR_ADDR_OFFS(base, val) \
    npu_reg_write_mask(base, REG_DMA_SFR_ADDR_OFFS, val, DMA_SFR_ADDR_OFFS_SFR_ADDR_OFFS_MASK, DMA_SFR_ADDR_OFFS_SFR_ADDR_OFFS_OFFSET)

/* DMA.SFR_START_ADDR (Category: DMA) */
#define READ_DMA_SFR_START_ADDR(base) npu_reg_read(base, REG_DMA_SFR_START_ADDR)
#define WRITE_DMA_SFR_START_ADDR(base, val) npu_reg_write(base, REG_DMA_SFR_START_ADDR, val)
/* DMA.SFR_START_ADDR.SFR_START_ADDR (Category: DMA) */
#define DMA_SFR_START_ADDR_SFR_START_ADDR_LEN        16
#define DMA_SFR_START_ADDR_SFR_START_ADDR_OFFSET     0
#define DMA_SFR_START_ADDR_SFR_START_ADDR_MASK_BITS  0x0000ffff /* [15:0] 0b00000000000000001111111111111111 */
#define DMA_SFR_START_ADDR_SFR_START_ADDR_MASK       0x0000ffff /* [15:0] 0b00000000000000001111111111111111 */
#define READ_DMA_SFR_START_ADDR_SFR_START_ADDR(base) \
    npu_reg_read_mask(base, REG_DMA_SFR_START_ADDR, DMA_SFR_START_ADDR_SFR_START_ADDR_MASK, DMA_SFR_START_ADDR_SFR_START_ADDR_OFFSET)
#define WRITE_DMA_SFR_START_ADDR_SFR_START_ADDR(base, val) \
    npu_reg_write_mask(base, REG_DMA_SFR_START_ADDR, val, DMA_SFR_START_ADDR_SFR_START_ADDR_MASK, DMA_SFR_START_ADDR_SFR_START_ADDR_OFFSET)

/* DMA.SFR_START_ADDR.RESERVED0 (Category: DMA) */
#define DMA_SFR_START_ADDR_RESERVED0_LEN        16
#define DMA_SFR_START_ADDR_RESERVED0_OFFSET     16
#define DMA_SFR_START_ADDR_RESERVED0_MASK_BITS  0x0000ffff /* [15:0] 0b00000000000000001111111111111111 */
#define DMA_SFR_START_ADDR_RESERVED0_MASK       0xffff0000 /* [31:16] 0b11111111111111110000000000000000 */
#define READ_DMA_SFR_START_ADDR_RESERVED0(base) \
    npu_reg_read_mask(base, REG_DMA_SFR_START_ADDR, DMA_SFR_START_ADDR_RESERVED0_MASK, DMA_SFR_START_ADDR_RESERVED0_OFFSET)
#define WRITE_DMA_SFR_START_ADDR_RESERVED0(base, val) \
    npu_reg_write_mask(base, REG_DMA_SFR_START_ADDR, val, DMA_SFR_START_ADDR_RESERVED0_MASK, DMA_SFR_START_ADDR_RESERVED0_OFFSET)

/* DMA.SFR_MOVE_CNT (Category: DMA) */
#define READ_DMA_SFR_MOVE_CNT(base) npu_reg_read(base, REG_DMA_SFR_MOVE_CNT)
#define WRITE_DMA_SFR_MOVE_CNT(base, val) npu_reg_write(base, REG_DMA_SFR_MOVE_CNT, val)
/* DMA.SFR_MOVE_CNT.RESERVED1 (Category: DMA) */
#define DMA_SFR_MOVE_CNT_RESERVED1_LEN        2
#define DMA_SFR_MOVE_CNT_RESERVED1_OFFSET     0
#define DMA_SFR_MOVE_CNT_RESERVED1_MASK_BITS  0x00000003 /* [1:0] 0b00000000000000000000000000000011 */
#define DMA_SFR_MOVE_CNT_RESERVED1_MASK       0x00000003 /* [1:0] 0b00000000000000000000000000000011 */
#define READ_DMA_SFR_MOVE_CNT_RESERVED1(base) \
    npu_reg_read_mask(base, REG_DMA_SFR_MOVE_CNT, DMA_SFR_MOVE_CNT_RESERVED1_MASK, DMA_SFR_MOVE_CNT_RESERVED1_OFFSET)
#define WRITE_DMA_SFR_MOVE_CNT_RESERVED1(base, val) \
    npu_reg_write_mask(base, REG_DMA_SFR_MOVE_CNT, val, DMA_SFR_MOVE_CNT_RESERVED1_MASK, DMA_SFR_MOVE_CNT_RESERVED1_OFFSET)

/* DMA.SFR_MOVE_CNT.SFR_MOVE_CNT (Category: DMA) */
#define DMA_SFR_MOVE_CNT_SFR_MOVE_CNT_LEN        12
#define DMA_SFR_MOVE_CNT_SFR_MOVE_CNT_OFFSET     2
#define DMA_SFR_MOVE_CNT_SFR_MOVE_CNT_MASK_BITS  0x00000fff /* [11:0] 0b00000000000000000000111111111111 */
#define DMA_SFR_MOVE_CNT_SFR_MOVE_CNT_MASK       0x00003ffc /* [13:2] 0b00000000000000000011111111111100 */
#define READ_DMA_SFR_MOVE_CNT_SFR_MOVE_CNT(base) \
    npu_reg_read_mask(base, REG_DMA_SFR_MOVE_CNT, DMA_SFR_MOVE_CNT_SFR_MOVE_CNT_MASK, DMA_SFR_MOVE_CNT_SFR_MOVE_CNT_OFFSET)
#define WRITE_DMA_SFR_MOVE_CNT_SFR_MOVE_CNT(base, val) \
    npu_reg_write_mask(base, REG_DMA_SFR_MOVE_CNT, val, DMA_SFR_MOVE_CNT_SFR_MOVE_CNT_MASK, DMA_SFR_MOVE_CNT_SFR_MOVE_CNT_OFFSET)

/* DMA.SFR_MOVE_CNT.RESERVED0 (Category: DMA) */
#define DMA_SFR_MOVE_CNT_RESERVED0_LEN        18
#define DMA_SFR_MOVE_CNT_RESERVED0_OFFSET     14
#define DMA_SFR_MOVE_CNT_RESERVED0_MASK_BITS  0x0003ffff /* [17:0] 0b00000000000000111111111111111111 */
#define DMA_SFR_MOVE_CNT_RESERVED0_MASK       0xffffc000 /* [31:14] 0b11111111111111111100000000000000 */
#define READ_DMA_SFR_MOVE_CNT_RESERVED0(base) \
    npu_reg_read_mask(base, REG_DMA_SFR_MOVE_CNT, DMA_SFR_MOVE_CNT_RESERVED0_MASK, DMA_SFR_MOVE_CNT_RESERVED0_OFFSET)
#define WRITE_DMA_SFR_MOVE_CNT_RESERVED0(base, val) \
    npu_reg_write_mask(base, REG_DMA_SFR_MOVE_CNT, val, DMA_SFR_MOVE_CNT_RESERVED0_MASK, DMA_SFR_MOVE_CNT_RESERVED0_OFFSET)

/* DMA.SFR_ID (Category: DMA) */
#define READ_DMA_SFR_ID(base) npu_reg_read(base, REG_DMA_SFR_ID)
#define WRITE_DMA_SFR_ID(base, val) npu_reg_write(base, REG_DMA_SFR_ID, val)
/* DMA.SFR_ID.SFR_ID (Category: DMA) */
#define DMA_SFR_ID_SFR_ID_LEN        16
#define DMA_SFR_ID_SFR_ID_OFFSET     0
#define DMA_SFR_ID_SFR_ID_MASK_BITS  0x0000ffff /* [15:0] 0b00000000000000001111111111111111 */
#define DMA_SFR_ID_SFR_ID_MASK       0x0000ffff /* [15:0] 0b00000000000000001111111111111111 */
#define READ_DMA_SFR_ID_SFR_ID(base) \
    npu_reg_read_mask(base, REG_DMA_SFR_ID, DMA_SFR_ID_SFR_ID_MASK, DMA_SFR_ID_SFR_ID_OFFSET)
#define WRITE_DMA_SFR_ID_SFR_ID(base, val) \
    npu_reg_write_mask(base, REG_DMA_SFR_ID, val, DMA_SFR_ID_SFR_ID_MASK, DMA_SFR_ID_SFR_ID_OFFSET)

/* DMA.SFR_ID.RESERVED0 (Category: DMA) */
#define DMA_SFR_ID_RESERVED0_LEN        16
#define DMA_SFR_ID_RESERVED0_OFFSET     16
#define DMA_SFR_ID_RESERVED0_MASK_BITS  0x0000ffff /* [15:0] 0b00000000000000001111111111111111 */
#define DMA_SFR_ID_RESERVED0_MASK       0xffff0000 /* [31:16] 0b11111111111111110000000000000000 */
#define READ_DMA_SFR_ID_RESERVED0(base) \
    npu_reg_read_mask(base, REG_DMA_SFR_ID, DMA_SFR_ID_RESERVED0_MASK, DMA_SFR_ID_RESERVED0_OFFSET)
#define WRITE_DMA_SFR_ID_RESERVED0(base, val) \
    npu_reg_write_mask(base, REG_DMA_SFR_ID, val, DMA_SFR_ID_RESERVED0_MASK, DMA_SFR_ID_RESERVED0_OFFSET)

/* DMA.SFR_DMA_STATUS (Category: DMA) */
#define READ_DMA_SFR_DMA_STATUS(base) npu_reg_read(base, REG_DMA_SFR_DMA_STATUS)
#define WRITE_DMA_SFR_DMA_STATUS(base, val) npu_reg_write(base, REG_DMA_SFR_DMA_STATUS, val)
/* DMA.SFR_DMA_STATUS.SFR_DMA_DONE_STATUS (Category: DMA) */
#define DMA_SFR_DMA_STATUS_SFR_DMA_DONE_STATUS_LEN        1
#define DMA_SFR_DMA_STATUS_SFR_DMA_DONE_STATUS_OFFSET     0
#define DMA_SFR_DMA_STATUS_SFR_DMA_DONE_STATUS_MASK_BITS  0x00000001 /* [0:0] 0b00000000000000000000000000000001 */
#define DMA_SFR_DMA_STATUS_SFR_DMA_DONE_STATUS_MASK       0x00000001 /* [0:0] 0b00000000000000000000000000000001 */
#define READ_DMA_SFR_DMA_STATUS_SFR_DMA_DONE_STATUS(base) \
    npu_reg_read_mask(base, REG_DMA_SFR_DMA_STATUS, DMA_SFR_DMA_STATUS_SFR_DMA_DONE_STATUS_MASK, DMA_SFR_DMA_STATUS_SFR_DMA_DONE_STATUS_OFFSET)
#define WRITE_DMA_SFR_DMA_STATUS_SFR_DMA_DONE_STATUS(base, val) \
    npu_reg_write_mask(base, REG_DMA_SFR_DMA_STATUS, val, DMA_SFR_DMA_STATUS_SFR_DMA_DONE_STATUS_MASK, DMA_SFR_DMA_STATUS_SFR_DMA_DONE_STATUS_OFFSET)

/* DMA.SFR_DMA_STATUS.RESERVED0 (Category: DMA) */
#define DMA_SFR_DMA_STATUS_RESERVED0_LEN        31
#define DMA_SFR_DMA_STATUS_RESERVED0_OFFSET     1
#define DMA_SFR_DMA_STATUS_RESERVED0_MASK_BITS  0x7fffffff /* [30:0] 0b01111111111111111111111111111111 */
#define DMA_SFR_DMA_STATUS_RESERVED0_MASK       0xfffffffe /* [31:1] 0b11111111111111111111111111111110 */
#define READ_DMA_SFR_DMA_STATUS_RESERVED0(base) \
    npu_reg_read_mask(base, REG_DMA_SFR_DMA_STATUS, DMA_SFR_DMA_STATUS_RESERVED0_MASK, DMA_SFR_DMA_STATUS_RESERVED0_OFFSET)
#define WRITE_DMA_SFR_DMA_STATUS_RESERVED0(base, val) \
    npu_reg_write_mask(base, REG_DMA_SFR_DMA_STATUS, val, DMA_SFR_DMA_STATUS_RESERVED0_MASK, DMA_SFR_DMA_STATUS_RESERVED0_OFFSET)

/* DMA.DMA_CTRL (Category: DMA) */
#define READ_DMA_DMA_CTRL(base) npu_reg_read(base, REG_DMA_DMA_CTRL)
#define WRITE_DMA_DMA_CTRL(base, val) npu_reg_write(base, REG_DMA_DMA_CTRL, val)
/* DMA.DMA_CTRL.DMA_CTRL_ENABLE (Category: DMA) */
#define DMA_DMA_CTRL_DMA_CTRL_ENABLE_LEN        3
#define DMA_DMA_CTRL_DMA_CTRL_ENABLE_OFFSET     0
#define DMA_DMA_CTRL_DMA_CTRL_ENABLE_MASK_BITS  0x00000007 /* [2:0] 0b00000000000000000000000000000111 */
#define DMA_DMA_CTRL_DMA_CTRL_ENABLE_MASK       0x00000007 /* [2:0] 0b00000000000000000000000000000111 */
#define READ_DMA_DMA_CTRL_DMA_CTRL_ENABLE(base) \
    npu_reg_read_mask(base, REG_DMA_DMA_CTRL, DMA_DMA_CTRL_DMA_CTRL_ENABLE_MASK, DMA_DMA_CTRL_DMA_CTRL_ENABLE_OFFSET)
#define WRITE_DMA_DMA_CTRL_DMA_CTRL_ENABLE(base, val) \
    npu_reg_write_mask(base, REG_DMA_DMA_CTRL, val, DMA_DMA_CTRL_DMA_CTRL_ENABLE_MASK, DMA_DMA_CTRL_DMA_CTRL_ENABLE_OFFSET)

/* DMA.DMA_CTRL.RESERVED0 (Category: DMA) */
#define DMA_DMA_CTRL_RESERVED0_LEN        29
#define DMA_DMA_CTRL_RESERVED0_OFFSET     3
#define DMA_DMA_CTRL_RESERVED0_MASK_BITS  0x1fffffff /* [28:0] 0b00011111111111111111111111111111 */
#define DMA_DMA_CTRL_RESERVED0_MASK       0xfffffff8 /* [31:3] 0b11111111111111111111111111111000 */
#define READ_DMA_DMA_CTRL_RESERVED0(base) \
    npu_reg_read_mask(base, REG_DMA_DMA_CTRL, DMA_DMA_CTRL_RESERVED0_MASK, DMA_DMA_CTRL_RESERVED0_OFFSET)
#define WRITE_DMA_DMA_CTRL_RESERVED0(base, val) \
    npu_reg_write_mask(base, REG_DMA_DMA_CTRL, val, DMA_DMA_CTRL_RESERVED0_MASK, DMA_DMA_CTRL_RESERVED0_OFFSET)

/* DMA.FLUSH_STATUS (Category: DMA) */
#define READ_DMA_FLUSH_STATUS(base) npu_reg_read(base, REG_DMA_FLUSH_STATUS)
#define WRITE_DMA_FLUSH_STATUS(base, val) npu_reg_write(base, REG_DMA_FLUSH_STATUS, val)
/* DMA.FLUSH_STATUS.FLUSH_STATUS (Category: DMA) */
#define DMA_FLUSH_STATUS_FLUSH_STATUS_LEN        1
#define DMA_FLUSH_STATUS_FLUSH_STATUS_OFFSET     0
#define DMA_FLUSH_STATUS_FLUSH_STATUS_MASK_BITS  0x00000001 /* [0:0] 0b00000000000000000000000000000001 */
#define DMA_FLUSH_STATUS_FLUSH_STATUS_MASK       0x00000001 /* [0:0] 0b00000000000000000000000000000001 */
#define READ_DMA_FLUSH_STATUS_FLUSH_STATUS(base) \
    npu_reg_read_mask(base, REG_DMA_FLUSH_STATUS, DMA_FLUSH_STATUS_FLUSH_STATUS_MASK, DMA_FLUSH_STATUS_FLUSH_STATUS_OFFSET)
#define WRITE_DMA_FLUSH_STATUS_FLUSH_STATUS(base, val) \
    npu_reg_write_mask(base, REG_DMA_FLUSH_STATUS, val, DMA_FLUSH_STATUS_FLUSH_STATUS_MASK, DMA_FLUSH_STATUS_FLUSH_STATUS_OFFSET)

/* DMA.FLUSH_STATUS.RESERVED0 (Category: DMA) */
#define DMA_FLUSH_STATUS_RESERVED0_LEN        31
#define DMA_FLUSH_STATUS_RESERVED0_OFFSET     1
#define DMA_FLUSH_STATUS_RESERVED0_MASK_BITS  0x7fffffff /* [30:0] 0b01111111111111111111111111111111 */
#define DMA_FLUSH_STATUS_RESERVED0_MASK       0xfffffffe /* [31:1] 0b11111111111111111111111111111110 */
#define READ_DMA_FLUSH_STATUS_RESERVED0(base) \
    npu_reg_read_mask(base, REG_DMA_FLUSH_STATUS, DMA_FLUSH_STATUS_RESERVED0_MASK, DMA_FLUSH_STATUS_RESERVED0_OFFSET)
#define WRITE_DMA_FLUSH_STATUS_RESERVED0(base, val) \
    npu_reg_write_mask(base, REG_DMA_FLUSH_STATUS, val, DMA_FLUSH_STATUS_RESERVED0_MASK, DMA_FLUSH_STATUS_RESERVED0_OFFSET)

/* DMA.AXI_CFG0 (Category: DMA) */
#define READ_DMA_AXI_CFG0(base) npu_reg_read(base, REG_DMA_AXI_CFG0)
#define WRITE_DMA_AXI_CFG0(base, val) npu_reg_write(base, REG_DMA_AXI_CFG0, val)
/* DMA.AXI_CFG0.AXI_RDMA_BURST_LENGTH (Category: DMA) */
#define DMA_AXI_CFG0_AXI_RDMA_BURST_LENGTH_LEN        9
#define DMA_AXI_CFG0_AXI_RDMA_BURST_LENGTH_OFFSET     0
#define DMA_AXI_CFG0_AXI_RDMA_BURST_LENGTH_MASK_BITS  0x000001ff /* [8:0] 0b00000000000000000000000111111111 */
#define DMA_AXI_CFG0_AXI_RDMA_BURST_LENGTH_MASK       0x000001ff /* [8:0] 0b00000000000000000000000111111111 */
#define READ_DMA_AXI_CFG0_AXI_RDMA_BURST_LENGTH(base) \
    npu_reg_read_mask(base, REG_DMA_AXI_CFG0, DMA_AXI_CFG0_AXI_RDMA_BURST_LENGTH_MASK, DMA_AXI_CFG0_AXI_RDMA_BURST_LENGTH_OFFSET)
#define WRITE_DMA_AXI_CFG0_AXI_RDMA_BURST_LENGTH(base, val) \
    npu_reg_write_mask(base, REG_DMA_AXI_CFG0, val, DMA_AXI_CFG0_AXI_RDMA_BURST_LENGTH_MASK, DMA_AXI_CFG0_AXI_RDMA_BURST_LENGTH_OFFSET)

/* DMA.AXI_CFG0.RESERVED0 (Category: DMA) */
#define DMA_AXI_CFG0_RESERVED0_LEN        23
#define DMA_AXI_CFG0_RESERVED0_OFFSET     9
#define DMA_AXI_CFG0_RESERVED0_MASK_BITS  0x007fffff /* [22:0] 0b00000000011111111111111111111111 */
#define DMA_AXI_CFG0_RESERVED0_MASK       0xfffffe00 /* [31:9] 0b11111111111111111111111000000000 */
#define READ_DMA_AXI_CFG0_RESERVED0(base) \
    npu_reg_read_mask(base, REG_DMA_AXI_CFG0, DMA_AXI_CFG0_RESERVED0_MASK, DMA_AXI_CFG0_RESERVED0_OFFSET)
#define WRITE_DMA_AXI_CFG0_RESERVED0(base, val) \
    npu_reg_write_mask(base, REG_DMA_AXI_CFG0, val, DMA_AXI_CFG0_RESERVED0_MASK, DMA_AXI_CFG0_RESERVED0_OFFSET)

/* DMA.AXI_CFG1 (Category: DMA) */
#define READ_DMA_AXI_CFG1(base) npu_reg_read(base, REG_DMA_AXI_CFG1)
#define WRITE_DMA_AXI_CFG1(base, val) npu_reg_write(base, REG_DMA_AXI_CFG1, val)
/* DMA.AXI_CFG1.AXI_WDMA_BURST_LENGTH (Category: DMA) */
#define DMA_AXI_CFG1_AXI_WDMA_BURST_LENGTH_LEN        9
#define DMA_AXI_CFG1_AXI_WDMA_BURST_LENGTH_OFFSET     0
#define DMA_AXI_CFG1_AXI_WDMA_BURST_LENGTH_MASK_BITS  0x000001ff /* [8:0] 0b00000000000000000000000111111111 */
#define DMA_AXI_CFG1_AXI_WDMA_BURST_LENGTH_MASK       0x000001ff /* [8:0] 0b00000000000000000000000111111111 */
#define READ_DMA_AXI_CFG1_AXI_WDMA_BURST_LENGTH(base) \
    npu_reg_read_mask(base, REG_DMA_AXI_CFG1, DMA_AXI_CFG1_AXI_WDMA_BURST_LENGTH_MASK, DMA_AXI_CFG1_AXI_WDMA_BURST_LENGTH_OFFSET)
#define WRITE_DMA_AXI_CFG1_AXI_WDMA_BURST_LENGTH(base, val) \
    npu_reg_write_mask(base, REG_DMA_AXI_CFG1, val, DMA_AXI_CFG1_AXI_WDMA_BURST_LENGTH_MASK, DMA_AXI_CFG1_AXI_WDMA_BURST_LENGTH_OFFSET)

/* DMA.AXI_CFG1.RESERVED0 (Category: DMA) */
#define DMA_AXI_CFG1_RESERVED0_LEN        23
#define DMA_AXI_CFG1_RESERVED0_OFFSET     9
#define DMA_AXI_CFG1_RESERVED0_MASK_BITS  0x007fffff /* [22:0] 0b00000000011111111111111111111111 */
#define DMA_AXI_CFG1_RESERVED0_MASK       0xfffffe00 /* [31:9] 0b11111111111111111111111000000000 */
#define READ_DMA_AXI_CFG1_RESERVED0(base) \
    npu_reg_read_mask(base, REG_DMA_AXI_CFG1, DMA_AXI_CFG1_RESERVED0_MASK, DMA_AXI_CFG1_RESERVED0_OFFSET)
#define WRITE_DMA_AXI_CFG1_RESERVED0(base, val) \
    npu_reg_write_mask(base, REG_DMA_AXI_CFG1, val, DMA_AXI_CFG1_RESERVED0_MASK, DMA_AXI_CFG1_RESERVED0_OFFSET)

/* DMA.AXI_CFG2 (Category: DMA) */
#define READ_DMA_AXI_CFG2(base) npu_reg_read(base, REG_DMA_AXI_CFG2)
#define WRITE_DMA_AXI_CFG2(base, val) npu_reg_write(base, REG_DMA_AXI_CFG2, val)
/* DMA.AXI_CFG2.AXI_RDMA_MO (Category: DMA) */
#define DMA_AXI_CFG2_AXI_RDMA_MO_LEN        4
#define DMA_AXI_CFG2_AXI_RDMA_MO_OFFSET     0
#define DMA_AXI_CFG2_AXI_RDMA_MO_MASK_BITS  0x0000000f /* [3:0] 0b00000000000000000000000000001111 */
#define DMA_AXI_CFG2_AXI_RDMA_MO_MASK       0x0000000f /* [3:0] 0b00000000000000000000000000001111 */
#define READ_DMA_AXI_CFG2_AXI_RDMA_MO(base) \
    npu_reg_read_mask(base, REG_DMA_AXI_CFG2, DMA_AXI_CFG2_AXI_RDMA_MO_MASK, DMA_AXI_CFG2_AXI_RDMA_MO_OFFSET)
#define WRITE_DMA_AXI_CFG2_AXI_RDMA_MO(base, val) \
    npu_reg_write_mask(base, REG_DMA_AXI_CFG2, val, DMA_AXI_CFG2_AXI_RDMA_MO_MASK, DMA_AXI_CFG2_AXI_RDMA_MO_OFFSET)

/* DMA.AXI_CFG2.RESERVED0 (Category: DMA) */
#define DMA_AXI_CFG2_RESERVED0_LEN        28
#define DMA_AXI_CFG2_RESERVED0_OFFSET     4
#define DMA_AXI_CFG2_RESERVED0_MASK_BITS  0x0fffffff /* [27:0] 0b00001111111111111111111111111111 */
#define DMA_AXI_CFG2_RESERVED0_MASK       0xfffffff0 /* [31:4] 0b11111111111111111111111111110000 */
#define READ_DMA_AXI_CFG2_RESERVED0(base) \
    npu_reg_read_mask(base, REG_DMA_AXI_CFG2, DMA_AXI_CFG2_RESERVED0_MASK, DMA_AXI_CFG2_RESERVED0_OFFSET)
#define WRITE_DMA_AXI_CFG2_RESERVED0(base, val) \
    npu_reg_write_mask(base, REG_DMA_AXI_CFG2, val, DMA_AXI_CFG2_RESERVED0_MASK, DMA_AXI_CFG2_RESERVED0_OFFSET)

/* DMA.AXI_CFG3 (Category: DMA) */
#define READ_DMA_AXI_CFG3(base) npu_reg_read(base, REG_DMA_AXI_CFG3)
#define WRITE_DMA_AXI_CFG3(base, val) npu_reg_write(base, REG_DMA_AXI_CFG3, val)
/* DMA.AXI_CFG3.AXI_WDMA_MO (Category: DMA) */
#define DMA_AXI_CFG3_AXI_WDMA_MO_LEN        4
#define DMA_AXI_CFG3_AXI_WDMA_MO_OFFSET     0
#define DMA_AXI_CFG3_AXI_WDMA_MO_MASK_BITS  0x0000000f /* [3:0] 0b00000000000000000000000000001111 */
#define DMA_AXI_CFG3_AXI_WDMA_MO_MASK       0x0000000f /* [3:0] 0b00000000000000000000000000001111 */
#define READ_DMA_AXI_CFG3_AXI_WDMA_MO(base) \
    npu_reg_read_mask(base, REG_DMA_AXI_CFG3, DMA_AXI_CFG3_AXI_WDMA_MO_MASK, DMA_AXI_CFG3_AXI_WDMA_MO_OFFSET)
#define WRITE_DMA_AXI_CFG3_AXI_WDMA_MO(base, val) \
    npu_reg_write_mask(base, REG_DMA_AXI_CFG3, val, DMA_AXI_CFG3_AXI_WDMA_MO_MASK, DMA_AXI_CFG3_AXI_WDMA_MO_OFFSET)

/* DMA.AXI_CFG3.RESERVED0 (Category: DMA) */
#define DMA_AXI_CFG3_RESERVED0_LEN        28
#define DMA_AXI_CFG3_RESERVED0_OFFSET     4
#define DMA_AXI_CFG3_RESERVED0_MASK_BITS  0x0fffffff /* [27:0] 0b00001111111111111111111111111111 */
#define DMA_AXI_CFG3_RESERVED0_MASK       0xfffffff0 /* [31:4] 0b11111111111111111111111111110000 */
#define READ_DMA_AXI_CFG3_RESERVED0(base) \
    npu_reg_read_mask(base, REG_DMA_AXI_CFG3, DMA_AXI_CFG3_RESERVED0_MASK, DMA_AXI_CFG3_RESERVED0_OFFSET)
#define WRITE_DMA_AXI_CFG3_RESERVED0(base, val) \
    npu_reg_write_mask(base, REG_DMA_AXI_CFG3, val, DMA_AXI_CFG3_RESERVED0_MASK, DMA_AXI_CFG3_RESERVED0_OFFSET)

/* DMA.AXI_CFG4 (Category: DMA) */
#define READ_DMA_AXI_CFG4(base) npu_reg_read(base, REG_DMA_AXI_CFG4)
#define WRITE_DMA_AXI_CFG4(base, val) npu_reg_write(base, REG_DMA_AXI_CFG4, val)
/* DMA.AXI_CFG4.AXI_4KB_BOUNDARY (Category: DMA) */
#define DMA_AXI_CFG4_AXI_4KB_BOUNDARY_LEN        1
#define DMA_AXI_CFG4_AXI_4KB_BOUNDARY_OFFSET     0
#define DMA_AXI_CFG4_AXI_4KB_BOUNDARY_MASK_BITS  0x00000001 /* [0:0] 0b00000000000000000000000000000001 */
#define DMA_AXI_CFG4_AXI_4KB_BOUNDARY_MASK       0x00000001 /* [0:0] 0b00000000000000000000000000000001 */
#define READ_DMA_AXI_CFG4_AXI_4KB_BOUNDARY(base) \
    npu_reg_read_mask(base, REG_DMA_AXI_CFG4, DMA_AXI_CFG4_AXI_4KB_BOUNDARY_MASK, DMA_AXI_CFG4_AXI_4KB_BOUNDARY_OFFSET)
#define WRITE_DMA_AXI_CFG4_AXI_4KB_BOUNDARY(base, val) \
    npu_reg_write_mask(base, REG_DMA_AXI_CFG4, val, DMA_AXI_CFG4_AXI_4KB_BOUNDARY_MASK, DMA_AXI_CFG4_AXI_4KB_BOUNDARY_OFFSET)

/* DMA.AXI_CFG4.RESERVED0 (Category: DMA) */
#define DMA_AXI_CFG4_RESERVED0_LEN        31
#define DMA_AXI_CFG4_RESERVED0_OFFSET     1
#define DMA_AXI_CFG4_RESERVED0_MASK_BITS  0x7fffffff /* [30:0] 0b01111111111111111111111111111111 */
#define DMA_AXI_CFG4_RESERVED0_MASK       0xfffffffe /* [31:1] 0b11111111111111111111111111111110 */
#define READ_DMA_AXI_CFG4_RESERVED0(base) \
    npu_reg_read_mask(base, REG_DMA_AXI_CFG4, DMA_AXI_CFG4_RESERVED0_MASK, DMA_AXI_CFG4_RESERVED0_OFFSET)
#define WRITE_DMA_AXI_CFG4_RESERVED0(base, val) \
    npu_reg_write_mask(base, REG_DMA_AXI_CFG4, val, DMA_AXI_CFG4_RESERVED0_MASK, DMA_AXI_CFG4_RESERVED0_OFFSET)

/* DMA.AXI_CFG5 (Category: DMA) */
#define READ_DMA_AXI_CFG5(base) npu_reg_read(base, REG_DMA_AXI_CFG5)
#define WRITE_DMA_AXI_CFG5(base, val) npu_reg_write(base, REG_DMA_AXI_CFG5, val)
/* DMA.AXI_CFG5.AXI_RDMA_QOS (Category: DMA) */
#define DMA_AXI_CFG5_AXI_RDMA_QOS_LEN        4
#define DMA_AXI_CFG5_AXI_RDMA_QOS_OFFSET     0
#define DMA_AXI_CFG5_AXI_RDMA_QOS_MASK_BITS  0x0000000f /* [3:0] 0b00000000000000000000000000001111 */
#define DMA_AXI_CFG5_AXI_RDMA_QOS_MASK       0x0000000f /* [3:0] 0b00000000000000000000000000001111 */
#define READ_DMA_AXI_CFG5_AXI_RDMA_QOS(base) \
    npu_reg_read_mask(base, REG_DMA_AXI_CFG5, DMA_AXI_CFG5_AXI_RDMA_QOS_MASK, DMA_AXI_CFG5_AXI_RDMA_QOS_OFFSET)
#define WRITE_DMA_AXI_CFG5_AXI_RDMA_QOS(base, val) \
    npu_reg_write_mask(base, REG_DMA_AXI_CFG5, val, DMA_AXI_CFG5_AXI_RDMA_QOS_MASK, DMA_AXI_CFG5_AXI_RDMA_QOS_OFFSET)

/* DMA.AXI_CFG5.RESERVED0 (Category: DMA) */
#define DMA_AXI_CFG5_RESERVED0_LEN        28
#define DMA_AXI_CFG5_RESERVED0_OFFSET     4
#define DMA_AXI_CFG5_RESERVED0_MASK_BITS  0x0fffffff /* [27:0] 0b00001111111111111111111111111111 */
#define DMA_AXI_CFG5_RESERVED0_MASK       0xfffffff0 /* [31:4] 0b11111111111111111111111111110000 */
#define READ_DMA_AXI_CFG5_RESERVED0(base) \
    npu_reg_read_mask(base, REG_DMA_AXI_CFG5, DMA_AXI_CFG5_RESERVED0_MASK, DMA_AXI_CFG5_RESERVED0_OFFSET)
#define WRITE_DMA_AXI_CFG5_RESERVED0(base, val) \
    npu_reg_write_mask(base, REG_DMA_AXI_CFG5, val, DMA_AXI_CFG5_RESERVED0_MASK, DMA_AXI_CFG5_RESERVED0_OFFSET)

/* DMA.AXI_CFG6 (Category: DMA) */
#define READ_DMA_AXI_CFG6(base) npu_reg_read(base, REG_DMA_AXI_CFG6)
#define WRITE_DMA_AXI_CFG6(base, val) npu_reg_write(base, REG_DMA_AXI_CFG6, val)
/* DMA.AXI_CFG6.AXI_WDMA_QOS (Category: DMA) */
#define DMA_AXI_CFG6_AXI_WDMA_QOS_LEN        4
#define DMA_AXI_CFG6_AXI_WDMA_QOS_OFFSET     0
#define DMA_AXI_CFG6_AXI_WDMA_QOS_MASK_BITS  0x0000000f /* [3:0] 0b00000000000000000000000000001111 */
#define DMA_AXI_CFG6_AXI_WDMA_QOS_MASK       0x0000000f /* [3:0] 0b00000000000000000000000000001111 */
#define READ_DMA_AXI_CFG6_AXI_WDMA_QOS(base) \
    npu_reg_read_mask(base, REG_DMA_AXI_CFG6, DMA_AXI_CFG6_AXI_WDMA_QOS_MASK, DMA_AXI_CFG6_AXI_WDMA_QOS_OFFSET)
#define WRITE_DMA_AXI_CFG6_AXI_WDMA_QOS(base, val) \
    npu_reg_write_mask(base, REG_DMA_AXI_CFG6, val, DMA_AXI_CFG6_AXI_WDMA_QOS_MASK, DMA_AXI_CFG6_AXI_WDMA_QOS_OFFSET)

/* DMA.AXI_CFG6.RESERVED0 (Category: DMA) */
#define DMA_AXI_CFG6_RESERVED0_LEN        28
#define DMA_AXI_CFG6_RESERVED0_OFFSET     4
#define DMA_AXI_CFG6_RESERVED0_MASK_BITS  0x0fffffff /* [27:0] 0b00001111111111111111111111111111 */
#define DMA_AXI_CFG6_RESERVED0_MASK       0xfffffff0 /* [31:4] 0b11111111111111111111111111110000 */
#define READ_DMA_AXI_CFG6_RESERVED0(base) \
    npu_reg_read_mask(base, REG_DMA_AXI_CFG6, DMA_AXI_CFG6_RESERVED0_MASK, DMA_AXI_CFG6_RESERVED0_OFFSET)
#define WRITE_DMA_AXI_CFG6_RESERVED0(base, val) \
    npu_reg_write_mask(base, REG_DMA_AXI_CFG6, val, DMA_AXI_CFG6_RESERVED0_MASK, DMA_AXI_CFG6_RESERVED0_OFFSET)

/* DMA.VIRT_RD_EN (Category: DMA) */
#define READ_DMA_VIRT_RD_EN(base) npu_reg_read(base, REG_DMA_VIRT_RD_EN)
#define WRITE_DMA_VIRT_RD_EN(base, val) npu_reg_write(base, REG_DMA_VIRT_RD_EN, val)
/* DMA.VIRT_RD_EN.VIRTUAL_READ_EN (Category: DMA) */
#define DMA_VIRT_RD_EN_VIRTUAL_READ_EN_LEN        1
#define DMA_VIRT_RD_EN_VIRTUAL_READ_EN_OFFSET     0
#define DMA_VIRT_RD_EN_VIRTUAL_READ_EN_MASK_BITS  0x00000001 /* [0:0] 0b00000000000000000000000000000001 */
#define DMA_VIRT_RD_EN_VIRTUAL_READ_EN_MASK       0x00000001 /* [0:0] 0b00000000000000000000000000000001 */
#define READ_DMA_VIRT_RD_EN_VIRTUAL_READ_EN(base) \
    npu_reg_read_mask(base, REG_DMA_VIRT_RD_EN, DMA_VIRT_RD_EN_VIRTUAL_READ_EN_MASK, DMA_VIRT_RD_EN_VIRTUAL_READ_EN_OFFSET)
#define WRITE_DMA_VIRT_RD_EN_VIRTUAL_READ_EN(base, val) \
    npu_reg_write_mask(base, REG_DMA_VIRT_RD_EN, val, DMA_VIRT_RD_EN_VIRTUAL_READ_EN_MASK, DMA_VIRT_RD_EN_VIRTUAL_READ_EN_OFFSET)

/* DMA.VIRT_RD_EN.RESERVED0 (Category: DMA) */
#define DMA_VIRT_RD_EN_RESERVED0_LEN        31
#define DMA_VIRT_RD_EN_RESERVED0_OFFSET     1
#define DMA_VIRT_RD_EN_RESERVED0_MASK_BITS  0x7fffffff /* [30:0] 0b01111111111111111111111111111111 */
#define DMA_VIRT_RD_EN_RESERVED0_MASK       0xfffffffe /* [31:1] 0b11111111111111111111111111111110 */
#define READ_DMA_VIRT_RD_EN_RESERVED0(base) \
    npu_reg_read_mask(base, REG_DMA_VIRT_RD_EN, DMA_VIRT_RD_EN_RESERVED0_MASK, DMA_VIRT_RD_EN_RESERVED0_OFFSET)
#define WRITE_DMA_VIRT_RD_EN_RESERVED0(base, val) \
    npu_reg_write_mask(base, REG_DMA_VIRT_RD_EN, val, DMA_VIRT_RD_EN_RESERVED0_MASK, DMA_VIRT_RD_EN_RESERVED0_OFFSET)

/* DMA.VIRT_RD_PTRN (Category: DMA) */
#define READ_DMA_VIRT_RD_PTRN(base) npu_reg_read(base, REG_DMA_VIRT_RD_PTRN)
#define WRITE_DMA_VIRT_RD_PTRN(base, val) npu_reg_write(base, REG_DMA_VIRT_RD_PTRN, val)
/* DMA.VIRT_RD_PTRN.VIRTUAL_READ_PATTERN (Category: DMA) */
#define DMA_VIRT_RD_PTRN_VIRTUAL_READ_PATTERN_LEN        32
#define DMA_VIRT_RD_PTRN_VIRTUAL_READ_PATTERN_OFFSET     0
#define DMA_VIRT_RD_PTRN_VIRTUAL_READ_PATTERN_MASK_BITS  0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define DMA_VIRT_RD_PTRN_VIRTUAL_READ_PATTERN_MASK       0xffffffff /* [31:0] 0b11111111111111111111111111111111 */
#define READ_DMA_VIRT_RD_PTRN_VIRTUAL_READ_PATTERN(base) \
    npu_reg_read_mask(base, REG_DMA_VIRT_RD_PTRN, DMA_VIRT_RD_PTRN_VIRTUAL_READ_PATTERN_MASK, DMA_VIRT_RD_PTRN_VIRTUAL_READ_PATTERN_OFFSET)
#define WRITE_DMA_VIRT_RD_PTRN_VIRTUAL_READ_PATTERN(base, val) \
    npu_reg_write_mask(base, REG_DMA_VIRT_RD_PTRN, val, DMA_VIRT_RD_PTRN_VIRTUAL_READ_PATTERN_MASK, DMA_VIRT_RD_PTRN_VIRTUAL_READ_PATTERN_OFFSET)


#endif // __NPU_REG_DMA_DX_L3_H
