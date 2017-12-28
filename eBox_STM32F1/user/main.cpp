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
#include "button.h"

Si4432	rf(&PG8,&PG6,&PG7,&spi2);
Button btn(&PE4, 1);

Led     led_net(&PB5,0);
Led     led_echo(&PE5,0);
Timer timer1(TIM1);
Timer timer2(TIM2);


///////////////////////////////////
uint8_t     recv_buf[1024] = {0};
uint16_t    len = 0;
uint32_t    count = 0;

uint8_t rbuf[512];
uint8_t wbuf[512];
uint8_t uart1_rx_buf[512];
uint8_t uart1_rx_buf_len=0;
uint8_t uart1_rx_buf_c[512];
uint8_t uart1_rx_buf_c_len=0;
uint8_t module_si4432=1;
uint8_t timer1_flag=0;
uint8_t timer2_flag=0;

//????????
void RF_TxData(uint8_t *protno,uint8_t len_t)
{
	uint8_t TxBuf_RF[200];
	//?????? TxBuf_RF??
	
  for(int i=0;i<len_t;i++)
  {
    TxBuf_RF[i]=protno[i];
  }

	//FIFO??
	rf.write(TxBuf_RF,len_t);

	//?????????,??????????????????????????CRC????????
	rf.SpiWriteRegister(0x05, 0x03);
	rf.SpiWriteRegister(0x06, 0x00);
	//?????????????;
	rf.set_rx_mode();
}

void rx_event()
{
    uint8_t c;
    
    timer2.start();
    c = uart1.read(); 
    //uart1.write(c);
    if(uart1_rx_buf_len<200)
    {
      uart1_rx_buf[uart1_rx_buf_len]=c;
    }
    uart1_rx_buf_len++;
    
    
    
    
}
void t1it()
{
    timer1_flag=1; 
    //uart1.printf("timer1 event!\r\n");
}
void t2it()
{
    timer2_flag=1;
    timer2.stop();
    //uart1.printf("timer2 event!,%d\r\n",uart1_rx_buf_len);
    
      for(int jj=0;jj<=uart1_rx_buf_len;jj++)
        {
          uart1_rx_buf_c[jj]=uart1_rx_buf[jj];
        }
    
    uart1_rx_buf_c_len=uart1_rx_buf_len;
    uart1_rx_buf_len=0;
}
void si4432_clr_sleep_mode(void)
{
	rf.SpiWriteRegister(0x07, 0x01);//ready
	//clear FIFO, no LDC
	rf.SpiWriteRegister(0x08, 0x03);
	rf.SpiWriteRegister(0x08, 0x00);

	rf.SpiReadRegister(0x03);//clr int
	rf.SpiReadRegister(0x04);//clr int
	rf.SpiWriteRegister(0x05, 0x12);//en rx fifoint
	rf.SpiWriteRegister(0x06, 0xc0);//
	rf.SpiWriteRegister(0x07, 0x20);
	rf.SpiWriteRegister(0x08, 0x04); 

}
void si4432_send_wake(void)
{
  
	rf.SpiWriteRegister(0x07, 0x01);//ready mode
		
	rf.SpiWriteRegister(0x08, 0x00);//clear fifo
	rf.SpiWriteRegister(0x06, 0x00);//interrrupt disable
	rf.SpiWriteRegister(0x32, 0x00);//header control
	//rf.SpiWriteRegister(0x33, 0x02); /*????2,?????????*/
	rf.SpiWriteRegister(0x33, 0x03); /*????2,?????????*/
	rf.SpiWriteRegister(0x34, 0xFF); /*?????,??:4bit*/
	rf.SpiReadRegister(0x03);
	rf.SpiReadRegister(0x04);
	rf.SpiWriteRegister(0x72, 0x48);//send preq
	rf.SpiWriteRegister(0x3E, 1);//send data length
	rf.SpiWriteRegister(0x7F, 0x55);
	rf.SpiWriteRegister(0x05, 0x84);//enable intrrupt
	//rf.SpiWriteRegister(0x06, 0x00);
	rf.SpiWriteRegister(0x07, 0x09);//send
//        while((ITSTATUS&0x04)!=0x04){
//        ITSTATUS=RF4432_ReadReg(INTERRUPT_STATUS_1);
//        };	   //??????????2??1,???0???
        
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
  led_echo.begin();
  //
  timer1.begin(1);    
  timer1.attach(t1it);
  timer1.interrupt(ENABLE);
  timer1.start();
  //
  timer2.begin(100);    
  timer2.attach(t2it);
  timer2.interrupt(ENABLE);
  //timer2.start();
	

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
		
    btn.loop();
    if(btn.release())
        {
            
          if( 1==module_si4432 )
            {
              module_si4432=1;
              led_echo.on();
              led_net.off();
              //uart1.printf("\r\nclick event!,echo_module");
            }
            else if(2==module_si4432)
            {
              module_si4432=0;
              led_net.off();
              led_echo.off();
              uart1.printf("\r\nclick event!,rx_module");
            }
            else
            {
              module_si4432=1;
              led_net.on();
              led_echo.off();
              uart1.printf("\r\nclick event!£¬tx_module");
            }
            uart1.printf("click event!\r\n");
            
            
            
        }
        
    //send 0xff
    if(timer2_flag==1&&module_si4432==1&&timer1_flag==1)
    {
      timer2_flag=0;      
      timer1_flag=0;
      si4432_clr_sleep_mode();
      for(int i=0;i<5;i++)
      {
        si4432_send_wake();
        delay_ms(250);
      }
      
      RF_TxData(uart1_rx_buf_c,uart1_rx_buf_c_len);
    } 
    
    
		//????????
		if(rf.read_irq() == 0)
		{
			//??????????,????????,???????
			rf.set_idle_mode();
			uint8_t read_len = rf.read(rbuf);
			//uart1.printf("read data length:%d\n",read_len);
      //uart1.printf("read data :");
      for(i=0;i<read_len;i++)
      {
        uart1.write(rbuf[i]);
      }
      //uart1.printf("\n");
      rf.set_rx_mode();
      if(2==module_si4432)
      {
        RF_TxData(rbuf,read_len);
      }
		}
    
    
	}
}



