//------------------------------------------------------------------------------
// Include
//------------------------------------------------------------------------------

// Clib
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>

// Altera
#include <alt_types.h>
#include <altera_avalon_pio_regs.h>
#include <Irda.h>
#include <system.h>

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"

// Project
#include "Vt100.h"
#include "Irda.h"


//------------------------------------------------------------------------------
// Macros
//------------------------------------------------------------------------------
#define IRDA_STRINGBUFFER_LEN 30


//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
typedef struct
{
    char data[IRDA_STRINGBUFFER_LEN];
    int start;
    int len;
} tIrdaStringBuffer;


//------------------------------------------------------------------------------
// Variables
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Function prototypes
//------------------------------------------------------------------------------
static void IrdaStringBufferAdd(tIrdaStringBuffer *buf, char c);
static void IrdaStringBufferInit(tIrdaStringBuffer *buf);
static void IrdaStringBufferPrint(const tIrdaStringBuffer *buf);
static void IrdaShowBuffers(const tIrdaStringBuffer *send_buf, const tIrdaStringBuffer *recv_buf);
static void IrdaUpdateEnabled(bool enabled);


//------------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------------
void IrdaMain()
{
    int fd;
    tIrdaStringBuffer send_buffer;
    tIrdaStringBuffer recv_buffer;
    bool enabled = true;

    // print title
    printf(VT100_HOME VT100_BOLD "IRDA Test" VT100_NORMAL);

    // open IRDA UART, cancel on failure
    fd = open(IRDA_UART_NAME, O_NONBLOCK | O_RDWR);
    if (fd < 0) {
        Vt100SetCursor(3, 1);
        printf("Error: open returned %d, errno=%d\r\n", fd, errno);
        printf("Press any key to return.");
        getchar();
        return;
    }

    // Initialize system and show initial data
    Vt100SetCursor(10, 1);
    printf("Press ESC to cancel\r\n"
           "Type anything to send.");
    IrdaStringBufferInit(&send_buffer);
    IrdaStringBufferInit(&recv_buffer);
    IrdaShowBuffers(&send_buffer, &recv_buffer);
    IrdaUpdateEnabled(enabled);

    // Main loop
    Vt100SetInputNonBlocking();
    bool changeflag = false;
    while(1)
    {
        char c = getchar();

        // ESC: return
        if(c == VT100_KEY_ESC)
        {
            break;
        }

        // Enter: toggle IRDA UART enable
        else if(c == 13)
        {
            enabled = !enabled;
            changeflag = true;
        }

        // Other keys: send via IRDA UART
        else if(c >= 0)
        {
            write(fd, &c, 1);
            IrdaStringBufferAdd(&send_buffer, c);
            changeflag = true;
        }

        // Byte received from IRDA UART
        else if (read(fd, &c, 1) == 1)
        {
            IrdaStringBufferAdd(&recv_buffer, c);
            changeflag = true;
        }

        // Else: wait for a short amount of time
        else
        {
            if(changeflag)
            {
                IrdaUpdateEnabled(enabled);
                IrdaShowBuffers(&send_buffer, &recv_buffer);
                changeflag = false;
            }
            vTaskDelay(portMS_TO_TICKS(100));
        }
    }


#if 0

    irda = fopen(IRDA_UART_NAME, "w");
    if (irda == NULL)
    {
        printf("Cannot open IRDA\n");
    }

    else
    {
        IOWR_ALTERA_AVALON_PIO_DATA(IRDA_SD_BASE, 0);
        fprintf(irda, "IRDA is enabled\n");
        vTaskDelay(1e3);

        IOWR_ALTERA_AVALON_PIO_DATA(IRDA_SD_BASE, 1);
        fprintf(irda, "IRDA is disabled, this should not be received\n");
        vTaskDelay(1e3);

        IOWR_ALTERA_AVALON_PIO_DATA(IRDA_SD_BASE, 0);


        fclose(irda);
    }

    printf("Press any key to return");
    getchar();

    // open UART
    fd = open(IRDA_UART_NAME, O_NONBLOCK | O_RDWR);
    if (fd < 0) {
        printf("open returned %d, errno=%d\n", fd, errno);
        return;
    }

    while (1) {
        char b;
        do {
            rc = read(fd, &b, 1);
        } while (rc != 1);
        printf("%c", b);
    }

#endif

    // close UART
    if(fd)
    {
        close(fd);
    }

}







static void IrdaStringBufferAdd(tIrdaStringBuffer *buf, char c)
{
    int write_pos = (buf->start + buf->len);
    if(write_pos >= IRDA_STRINGBUFFER_LEN)
    {
        write_pos -= IRDA_STRINGBUFFER_LEN;
    }
    buf->data[write_pos] = c;
    if(buf->len < IRDA_STRINGBUFFER_LEN)
    {
        buf->len++;
    }
    else
    {
        buf->start++;
        if(buf->start == IRDA_STRINGBUFFER_LEN)
        {
            buf->start = 0;
        }
    }
}

static void IrdaStringBufferInit(tIrdaStringBuffer *buf)
{
    buf->len = 0;
    buf->start = 0;
}


static void IrdaStringBufferPrint(const tIrdaStringBuffer *buf)
{
    int pos = buf->start;
    for(int i = 0; i < buf->len; i++)
    {
        putchar(buf->data[pos++]);
        if(pos == IRDA_STRINGBUFFER_LEN) {
            pos -= IRDA_STRINGBUFFER_LEN;
        }
    }
}

static void IrdaShowBuffers(const tIrdaStringBuffer *send_buf, const tIrdaStringBuffer *recv_buf)
{
    Vt100SetCursor(3, 1);
    printf("Send    : \033[0K");
    IrdaStringBufferPrint(send_buf);
    printf("\r\n");
    printf("Receive : \033[0K");
    IrdaStringBufferPrint(recv_buf);
    printf("\r\n");
}


static void IrdaUpdateEnabled(bool enabled)
{
    Vt100SetCursor(6, 1);
    if(enabled)
    {
        IOWR_ALTERA_AVALON_PIO_DATA(IRDA_SD_BASE, 0);
        printf("IRDA transceiver enabled (press Enter to toggle) ");
    }
    else
    {
        IOWR_ALTERA_AVALON_PIO_DATA(IRDA_SD_BASE, 1);
        printf("IRDA transceiver disabled (press Enter to toggle)");
    }
}
