/*
file   : *.cpp
author : shentq
version: V1.0
date   : 2015/7/5

Copyright 2015 shentq. All Rights Reserved.
*/

//STM32 RUN IN eBox
#include "ebox.h"
#include "wstring.h"
#include "bsp.h"
#include "si4432.h"

Si4432	rf(&PG8,&PG6,&PG7,&spi2);
Button btn(&PA8, 0);

Led     led_net(&PB5,0);


///////////////////////////////////
uint8_t     recv_buf[1024] = {0};
uint16_t    len = 0;
uint32_t    count = 0;

uint8_t rbuf[512];
uint8_t wbuf[512];
uint8_t module_si4432=0;


//????????
void RF_TxData(uint8_t protno)
{
	uint8_t TxBuf_RF[32];
	//?????? TxBuf_RF??
	TxBuf_RF[0] = 0x5A;			//??
	TxBuf_RF[1] = 0xA5;			//??
	TxBuf_RF[2] = 6;	 	//???
	TxBuf_RF[3] = protno;			//
	TxBuf_RF[4] = 0xAA;			//??
	TxBuf_RF[4] = 0xBB;			//??

	//FIFO??
	rf.write(TxBuf_RF,5+len);

	//?????????,??????????????????????????CRC????????
	rf.SpiWriteRegister(0x05, 0x03);
	rf.SpiWriteRegister(0x06, 0x00);
	//?????????????;
	rf.set_rx_mode();
}

void rx_event()
{
    uint8_t c;
    c = uart1.read();
    uart1.write(c);
    if(c==0xa5)
    {
      module_si4432=1;
      led_net.on();
    }
    else if(c==0xaa)
    {
      module_si4432=0;
      led_net.off();
    }
    
    
}
void setup()
{
	ebox_init();
	uart1.begin(115200);
  uart1.attach(rx_event,RxIrq);
  uart1.interrupt(RxIrq,ENABLE);
	uart1.printf("-------------------------------\r\n");
	//flash.begin(1);
	rf.begin(2);
	btn.begin();
	led_net.begin();
	

}
int main(void)
{
	int ret,i;
  uint8_t tmp=0;
	uint16_t tx_cnt = 0;

	uint32_t last_time= 0;
	uint32_t last_rt_time = 0;
	uint32_t last_get_time = 0;
	uint32_t last_login_time = 0;
	setup();

	//si4432????
	rf.start();
	//si4432????
	rf.set_config();
	//si4432???????
	rf.set_rx_mode();

	while(1)
	{
		
    
    //send 0xff
    
    
    if (module_si4432==1)
    {
    tmp=rf.SpiReadRegister(0X01);
    uart1.printf("reg01 is %x\n",tmp);
    tmp=rf.read_rssi();
    uart1.printf("RSSI is %x\n",tmp);
    RF_TxData(0xff);
    uart1.printf("SEND DATA is %x\n",0XFF);
    delay_ms(1000);
    }
		//????????
		if(rf.read_irq() == 0)
		{
			//??????????,????????,???????
			rf.set_idle_mode();
			uint8_t read_len = rf.read(rbuf);
			uart1.printf("read data length:%d\n",read_len);
      uart1.printf("read data :");
      for(i=0;i<read_len;i++)
      {
        uart1.printf("%x,",rbuf[i]);
      }
      uart1.printf("\n");
      rf.set_rx_mode();
		}
    
	}
}



