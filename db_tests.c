/*!---------------------------------------------------------------------------------------------------------------------
@file
@brief      <short description>
@author     Ramon Felder
@date       02.05.2019

Insert detailed description here
*///--------------------------------------------------------------------------------------------------------------------

// TODO: Move packet buffers to dedicated internal memory instead of HyperRAM

//----------------------------------------------------------------------------------------------------------------------
// Include
//----------------------------------------------------------------------------------------------------------------------

// Clib
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// Altera
#include "altera_eth_tse_regs.h"
#include "altera_avalon_tse.h"
#include "altera_msgdma.h"
#include "sys/alt_cache.h"
#include "nios2.h"

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"

// LWIP
#include "lwip/netif.h"
#include "lwip/etharp.h"
#include "lwip/init.h"
#include "netif/ethernet.h"
#include "lwip/sys.h"
#include "lwip/tcpip.h"

// Project
#include "system.h"
#include "Ringbuffer.h"
#include "eth.h"


//----------------------------------------------------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------------------------
// Types
//----------------------------------------------------------------------------------------------------------------------
typedef struct sEthRxPbuf
{
    struct pbuf_custom p;
    char payload[1520]; // 1518 without FCS, +2 padding bytes at the beginning to align IP packet to 32 bit boundary
    struct sEthRxPbuf *next;
} tEthRxPbuf;


//----------------------------------------------------------------------------------------------------------------------
// Data
//----------------------------------------------------------------------------------------------------------------------

/// MAC Address
static const uint8_t Eth_mac_addr[] = {0x00, 0x30, 0x59, 0x12, 0x34, 0x56};

/// Network interface data structure needed by LwIP
struct netif Eth_netif;

/// DMA devices for comunication with MAC
static alt_msgdma_dev *Eth_rx_sgdma_dev;
static alt_msgdma_dev *Eth_tx_sgdma_dev;


/// Oldest buffer in RX DMA queue (--> used on the next RX DMA interrupt)
static volatile tEthRxPbuf *Eth_oldest_rx_buf = NULL;

/// Newest buffer in RX DMA queue (points to NULL; make point to new buffer, when a new buffer is added)
static volatile tEthRxPbuf *Eth_newest_rx_buf = NULL;

LWIP_MEMPOOL_DECLARE(Eth_rx_mem_pool, 45, sizeof(tEthRxPbuf), "EthRxMemPool");

/// Semaphore to wake sending function (may be called from any task) when space in the TX DMA FIFO becomes available
SemaphoreHandle_t Eth_tx_sgdma_ready_sem;

/// Semaphore to tell SGDMA handler task that a TX SGDMA is finished.
SemaphoreHandle_t Eth_tx_sgdma_done_sem;

/// Task to handle Rx DMA
static TaskHandle_t Eth_sgdma_task_handle;

RINGBUFFER_DECLARE(Eth_tx_ringbuffer, 100)


//----------------------------------------------------------------------------------------------------------------------
// Function prototypes
//----------------------------------------------------------------------------------------------------------------------

// DMA
static void EthInitSgdma();
static void EthSgdmaTaskMain(void *params);
static bool EthQueueRxSgdmaDescriptor(tEthRxPbuf *pbuf);
static void EthHandleRxSgdma();
static void EthRxSgdmaCallback(void *context);
static void EthTxSgdmaCallback(void *context);
static uint32_t EthRxResponseLevel();
static bool EthRxSgdmaCanAcceptDescriptors();

// MAC
static void EthInitMac();

// LWIP
static err_t EthInitCallback(struct netif *netif);
static err_t EthLinkOutputCallback(struct netif *netif, struct pbuf *p);
static void EthFreeRxBuf(struct pbuf *p);



//----------------------------------------------------------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------------------------------------------------------

void EthInit()
{
    Eth_tx_sgdma_ready_sem = xSemaphoreCreateBinary();
    Eth_tx_sgdma_done_sem = xSemaphoreCreateCounting(0xFFFFFFFF, 0);

    ip4_addr_t ipaddr, netmask, default_gateway;

    LWIP_MEMPOOL_INIT(Eth_rx_mem_pool);

    tcpip_init(NULL, NULL);

    IP4_ADDR(&ipaddr, 192,168,1,20);
    IP4_ADDR(&netmask, 255, 255, 255, 0);
    IP4_ADDR(&default_gateway, 0, 0, 0, 0);

    LOCK_TCPIP_CORE();
    {
        netif_add(
                &Eth_netif,
                &ipaddr,
                &netmask,
                &default_gateway,
                NULL,
                &EthInitCallback,
                tcpip_input
        );

        netif_set_default(&Eth_netif);
        netif_set_link_up(&Eth_netif);
        netif_set_up(&Eth_netif);
    }
    UNLOCK_TCPIP_CORE();

    xTaskCreate(
        &EthSgdmaTaskMain,
        "EthSgdma",
        1024*8,
        NULL,
        2,
        &Eth_sgdma_task_handle
    );
    //xTaskNotifyGive(Eth_sgdma_task_handle);
}

//----------------------------------------------------------------------------------------------------------------------

static err_t EthInitCallback(struct netif *netif)
{
    netif->hwaddr_len = ETHARP_HWADDR_LEN;
    memcpy(netif->hwaddr, Eth_mac_addr, ETHARP_HWADDR_LEN);
    netif->mtu = 1500;
    netif->name[0] = 'e';
    netif->name[1] = 'n';
    netif->num = 0;
    netif->output = etharp_output;
    netif->linkoutput = &EthLinkOutputCallback;
    netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
    //netif->hostname = "ER12";

    EthInitMac();

    EthInitSgdma();

    return ERR_OK;
}


//----------------------------------------------------------------------------------------------------------------------

static void EthInitMac()
{
    uint32_t temp;

    // --- DWORD Offset 0x00 - 0x17: Base configuration ---

    // Command Config
    IOWR_ALTERA_TSEMAC_CMD_CONFIG(ETH_BASE,
            ALTERA_TSEMAC_CMD_TX_ENA_MSK
            |ALTERA_TSEMAC_CMD_RX_ENA_MSK
            //|ALTERA_TSEMAC_CMD_PROMIS_EN_MSK      // Promiscuous for Debugging only
            //|ALTERA_TSEMAC_CMD_PAD_EN_MSK         // not available in small MAC
            //|ALTERA_TSEMAC_CMD_LOOPBACK_MSK       // not available in small MAC
            //|ALTERA_TSEMAC_CMD_CNT_RESET_MSK
    );

    // MAC Address
    temp = *(uint32_t*)(Eth_mac_addr);
    IOWR_ALTERA_TSEMAC_MAC_0(ETH_BASE, temp);
    temp = (*(uint32_t*)(Eth_mac_addr + 4)) & 0xFFFF;
    IOWR_ALTERA_TSEMAC_MAC_1(ETH_BASE, temp);

    // Max frame length
    IOWR_ALTERA_TSEMAC_FRM_LENGTH(ETH_BASE, 1518);

    // PHY address for MDIO access
    IOWR_ALTERA_TSEMAC_MDIO_ADDR0(ETH_BASE, 1);
    IOWR_ALTERA_TSEMAC_MDIO_ADDR1(ETH_BASE, 1);

    // --- DWORD Offset 0x18 - 0x38: Statistics ---

    // --- DWORD Offset 0x3A: Transmit command ---
    IOWR_ALTERA_TSEMAC_TX_CMD_STAT(ETH_BASE, ALTERA_TSEMAC_TX_CMD_STAT_TXSHIFT16_MSK);

    // --- DWORD Offset 0x3B: Receive command ---
    IOWR_ALTERA_TSEMAC_RX_CMD_STAT(ETH_BASE, ALTERA_TSEMAC_RX_CMD_STAT_RXSHIFT16_MSK);

    // --- DWORD Offset 0x40 - 0x7F: Multicast Hash Table ---
    // --- DWORD Offset 0x80 - 0x9F: MDIO 0 mapped ---
    // --- DWORD Offset 0xA0 - 0xBF: MDIO 1 mapped ---
    // --- DWORD Offset 0xC0 - 0xC7: Supplementary Address ---
    // --- DWORD Offset 0xD0 - 0xD6: IEEE 1588v2 Feature ---
}

//----------------------------------------------------------------------------------------------------------------------


static err_t EthLinkOutputCallback(struct netif *netif, struct pbuf *p)
{
    int rc;
    int len;
    alt_msgdma_standard_descriptor descriptor;
    alt_u32 flags;
    bool first_fragment = true;

    // for each buffer in the chain
    while (p)
    {
        flags = ALTERA_MSGDMA_DESCRIPTOR_CONTROL_EARLY_TERMINATION_IRQ_MASK
              | ALTERA_MSGDMA_DESCRIPTOR_CONTROL_TRANSFER_COMPLETE_IRQ_MASK;

        len = p->len;

        if(first_fragment)
        {
            // generate Start-Of-Packet for first fragment only
            flags |= ALTERA_MSGDMA_DESCRIPTOR_CONTROL_GENERATE_SOP_MASK;

            // first fragment starts with 2 padding bytes
            len += 2;

            first_fragment = false;
        }

        // generate End-Of-Packet for last fragment only
        if(p->next == NULL)
        {
            flags |= ALTERA_MSGDMA_DESCRIPTOR_CONTROL_GENERATE_EOP_MASK;
        }

        // create DMA descriptor for sending
        rc = alt_msgdma_construct_standard_mm_to_st_descriptor (
            Eth_tx_sgdma_dev,
            &descriptor,
            p->payload,
            len,
            flags
        );
        if (rc != 0)
        {
            return ERR_VAL;
        }

        // flush cache
        alt_dcache_flush (p->payload, len);

        while(Ringbuffer_is_full(&Eth_tx_ringbuffer))
        {
            xSemaphoreTake(Eth_tx_sgdma_ready_sem, portMAX_DELAY);
        }

        // ------------------------ critical section ------------------------
        portENTER_CRITICAL();
        {
            // start DMA transfer
            rc = alt_msgdma_standard_descriptor_async_transfer (
                Eth_tx_sgdma_dev,
                &descriptor
             );

            // on success, add it to the buffer queue
            // Must be added before the "DMA complete" interrupt occurs, thus the critical section.
            if(rc == 0)
            {
                Ringbuffer_push(&Eth_tx_ringbuffer, (void*)p);
                pbuf_ref(p);
                p = p->next;
            }
        }
        portEXIT_CRITICAL();
        // ------------------------ end of critical section ------------------------

        // if DMA FIFO full: keep trying
        if (rc == -ENOSPC) // DMA FIFO full
        {
            xSemaphoreTake(Eth_tx_sgdma_ready_sem, portMAX_DELAY);
        }
        else if(rc != 0)
        {
            return ERR_TIMEOUT;
        }
    }

    return ERR_OK;
}


//----------------------------------------------------------------------------------------------------------------------

static void EthInitSgdma()
{
    // reset SGDMAs
    IOWR_ALTERA_MSGDMA_CSR_CONTROL(ETH_RX_DMA_CSR_BASE, ALTERA_MSGDMA_CSR_RESET_MASK);
    IOWR_ALTERA_MSGDMA_CSR_CONTROL(ETH_TX_DMA_CSR_BASE, ALTERA_MSGDMA_CSR_RESET_MASK);
    while (IORD_ALTERA_MSGDMA_CSR_CONTROL(ETH_RX_DMA_CSR_BASE) & ALTERA_MSGDMA_CSR_RESET_MASK);
    while (IORD_ALTERA_MSGDMA_CSR_CONTROL(ETH_TX_DMA_CSR_BASE) & ALTERA_MSGDMA_CSR_RESET_MASK);

    Eth_rx_sgdma_dev = alt_msgdma_open(ETH_RX_DMA_CSR_NAME);
    Eth_tx_sgdma_dev = alt_msgdma_open(ETH_TX_DMA_CSR_NAME);

    if((Eth_rx_sgdma_dev == NULL) || (Eth_tx_sgdma_dev == NULL))
    {
        NIOS2_BREAK();
    }

    alt_msgdma_register_callback(
        Eth_rx_sgdma_dev,
        &EthRxSgdmaCallback,
        0,
        NULL
     );

    alt_msgdma_register_callback(
        Eth_tx_sgdma_dev,
        &EthTxSgdmaCallback,
        0,
        NULL
     );
}



static void EthSgdmaTaskMain(void *params)
{
    tEthRxPbuf *next_pbuf = NULL;

    while(1)
    {
        // Handle incoming packets, if any
        while(EthRxResponseLevel())
        {
            EthHandleRxSgdma();
        }

        // Release buffer of sent packets
        while(pdTRUE == xSemaphoreTake(Eth_tx_sgdma_done_sem, 0)) // non-blocking
        {
            struct pbuf* p;
            if(Ringbuffer_pop(&Eth_tx_ringbuffer, (void*) &p))
            {
                pbuf_free(p);
            }
            else
            {
                NIOS2_BREAK();
            }

            // The sending task might be waiting for free space in the SGDMA descriptor queue,
            // so wake it up.
            xSemaphoreGive(Eth_tx_sgdma_ready_sem);
        }

        // Add more DMA descriptors to Rx queue until the queue is full or no more buffers are available
        while(EthRxSgdmaCanAcceptDescriptors())
        {
            // get a new buffer if necessary
            if(next_pbuf == NULL)
            {
                next_pbuf = LWIP_MEMPOOL_ALLOC(Eth_rx_mem_pool);
            }

            // if we have a free buffer
            if(next_pbuf)
            {
                // add it to the DMA queue
                if(EthQueueRxSgdmaDescriptor(next_pbuf))
                {
                    next_pbuf = NULL;
                    continue;
                }
            }
            break;
        }

        // go to sleep until something happens on the Rx DMA
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}


static bool EthQueueRxSgdmaDescriptor(tEthRxPbuf *pbuf)
{
    alt_msgdma_standard_descriptor descriptor;
    int rc;

    // Create DMA descriptor
    rc = alt_msgdma_construct_standard_st_to_mm_descriptor (
        Eth_rx_sgdma_dev,
        &descriptor,
        (void*)pbuf->payload,
        sizeof(pbuf->payload),
        ALTERA_MSGDMA_DESCRIPTOR_CONTROL_END_ON_EOP_MASK
        | ALTERA_MSGDMA_DESCRIPTOR_CONTROL_TRANSFER_COMPLETE_IRQ_MASK
        | ALTERA_MSGDMA_DESCRIPTOR_CONTROL_EARLY_TERMINATION_IRQ_MASK
    );

    // return with error if the DMA descriptor cannot be created
    if(rc)
    {
        NIOS2_BREAK();
        return false;
    }

    // flush cache to prepare for DMA
    alt_dcache_flush(pbuf->payload, sizeof(pbuf->payload));

    // add to DMA queue
    rc = alt_msgdma_standard_descriptor_async_transfer (
        Eth_rx_sgdma_dev,
        &descriptor
     );
    if (rc)
    {
        NIOS2_BREAK();
        return false;
    }

    // add to buffer queue
    pbuf->next = NULL;
    if(Eth_oldest_rx_buf == NULL)
    {
        Eth_oldest_rx_buf = pbuf;
    }
    else
    {
        Eth_newest_rx_buf->next = pbuf;
    }
    Eth_newest_rx_buf = pbuf;

    return true;
}



static void EthHandleRxSgdma()
{
    tEthRxPbuf *pbuf;
    err_t rc;

    // read oldest response and automatically remove it from the response FIFO
    uint32_t length = IORD_ALTERA_MSGDMA_RESPONSE_ACTUAL_BYTES_TRANSFERRED(ETH_RX_DMA_RESPONSE_BASE);
    uint32_t status = IORD_ALTERA_MSGDMA_RESPONSE_ERRORS_REG(ETH_RX_DMA_RESPONSE_BASE);
    int errors = (status & ALTERA_MSGDMA_RESPONSE_ERROR_MASK) >> ALTERA_MSGDMA_RESPONSE_ERROR_OFFSET;
    bool early_termination = status && ALTERA_MSGDMA_RESPONSE_EARLY_TERMINATION_MASK;

    if(Eth_oldest_rx_buf == NULL)
    {
        NIOS2_BREAK();
        return;
    }

    (void)errors;
    (void)early_termination;

    if(errors || early_termination || (length < 4))
    {
        NIOS2_BREAK();
    }

    pbuf = (tEthRxPbuf*) Eth_oldest_rx_buf;
    Eth_oldest_rx_buf = pbuf->next;

    struct pbuf* p = pbuf_alloced_custom(PBUF_RAW,
        length,
        PBUF_REF,
        &pbuf->p,
        pbuf->payload + 2,
        sizeof(pbuf->payload) - 2
    );
    pbuf->p.custom_free_function = &EthFreeRxBuf;

    if(pbuf->p.pbuf.ref != 1)
    {
        NIOS2_BREAK();
    }

    if (pbuf->p.pbuf.next != NULL)
    {
        NIOS2_BREAK();
    }

    // Pass to LwIP. The buffer will be released by the stack.
    // If the stack does not accept the buffer, release it immediately
    rc = Eth_netif.input(p, &Eth_netif);
    if(rc != ERR_OK)
    {
        NIOS2_BREAK();
        pbuf_free(p);
    }
}


static void EthRxSgdmaCallback(void *context)
{
    BaseType_t task_woken = pdFALSE;
    vTaskNotifyGiveFromISR(Eth_sgdma_task_handle, &task_woken);
    portEND_SWITCHING_ISR(task_woken);
}


static void EthTxSgdmaCallback(void *context)
{
    BaseType_t task_woken = pdFALSE;
    xSemaphoreGiveFromISR(Eth_tx_sgdma_done_sem, &task_woken);
    vTaskNotifyGiveFromISR(Eth_sgdma_task_handle, &task_woken);
    portEND_SWITCHING_ISR(task_woken);
}


static uint32_t EthRxResponseLevel()
{
    uint32_t level = IORD_ALTERA_MSGDMA_CSR_RESPONSE_FILL_LEVEL(ETH_RX_DMA_CSR_BASE);
    level &= ALTERA_MSGDMA_CSR_RESPONSE_FILL_LEVEL_MASK;
    level >>= ALTERA_MSGDMA_CSR_RESPONSE_FILL_LEVEL_OFFSET;
    return level;
}


static bool EthRxSgdmaCanAcceptDescriptors()
{
    return !(IORD_ALTERA_MSGDMA_CSR_STATUS(ETH_RX_DMA_CSR_BASE) & ALTERA_MSGDMA_CSR_DESCRIPTOR_BUFFER_FULL_MASK);
}

//----------------------------------------------------------------------------------------------------------------------

static void EthFreeRxBuf(struct pbuf *p)
{
    SYS_ARCH_DECL_PROTECT(old_level);
    tEthRxPbuf* buf = (tEthRxPbuf*)p;
    // invalidate data cache here - lwIP and/or application may have written into buffer!
    // (invalidate is faster than flushing, and no one needs the correct data in the buffer)
    alt_dcache_flush(buf->payload, sizeof(buf->payload));
    SYS_ARCH_PROTECT(old_level);
    LWIP_MEMPOOL_FREE(Eth_rx_mem_pool, buf);
    SYS_ARCH_UNPROTECT(old_level);

    // the SGDMA might be waiting for buffers, so wake it up when a buffer becomes free
    xTaskNotifyGive(Eth_sgdma_task_handle);
}
