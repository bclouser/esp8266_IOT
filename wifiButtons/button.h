#ifndef _BUTTON_H_
#define _BUTTON_H_


#define BUTTON1_IO_MUX     PERIPHS_IO_MUX_GPIO0_U
#define BUTTON1_IO_NUM     0
#define BUTTON1_IO_FUNC    FUNC_GPIO0

#define BUTTON2_IO_MUX     PERIPHS_IO_MUX_GPIO4_U
#define BUTTON2_IO_NUM     4
#define BUTTON2_IO_FUNC    FUNC_GPIO4

#define BUTTON3_IO_MUX     PERIPHS_IO_MUX_GPIO5_U
#define BUTTON3_IO_NUM     5
#define BUTTON3_IO_FUNC    FUNC_GPIO5

#define BUTTON4_IO_MUX     PERIPHS_IO_MUX_MTMS_U
#define BUTTON4_IO_NUM     14
#define BUTTON4_IO_FUNC    FUNC_GPIO14

#define NUMBER_OF_BUTTONS 4

void initButtons(void);
void startBlinkTimer(void);
void stopBlinkTimer(void);

#endif