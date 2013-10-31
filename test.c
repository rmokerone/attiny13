#define F_CPU 4.8E6
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

volatile unsigned char light = 0;
volatile unsigned int adc_data;
volatile char time_ok;
volatile char mark = 1;
unsigned char led = 0x0f;

void Init_IO(void);
void display_led_mode1 (char num); 
void display_led_mode2 (void);
void Init_timer_led (void);
void Init_timer_adc (void);
void Init_ADC (void);

int main(void) 
{
    unsigned int adc_value = 0;
    Init_IO ();
    Init_timer_adc ();
    Init_ADC ();
    while (1)
    {
	if (time_ok)
	{
	    time_ok = 0;
	    adc_value = (unsigned long) adc_data * 1100 / 1024;
	    adc_value = adc_value / 10;
	}
	display_led_mode1 (adc_value);
    }
}

void Init_IO (void)
{
   DDRB = 0x0f;
   PORTB = 0X0F;
}

void display_led_mode1 (char num)
{
    char i;
    if (num > 4)
	num = 4;
    
    for (i = 0; i <= num; i++)
    {
	PORTB = (led >> i); 
	_delay_ms (30); 
    } 
    led = 0x0f;
}
   
void display_led_mode2 (void)
{
    Init_timer_led();   
}

//led模式寄存器初始化
void Init_timer_led ()
{
    
    TCCR0A = (1<<WGM01)|(1<<WGM00); 
    TCCR0B = (1<<CS01)|(1<<CS00);
    TCNT0 = 0X00;
    OCR0B = 0x00;
    TIMSK0 |= (1<<OCF0B)|(1<<TOIE0);
    sei();
}


ISR (TIM0_COMPB_vect)
{
    PORTB = 0x00;
}


ISR (TIM0_OVF_vect)
{
    PORTB = 0Xff;
    if (mark == 1)
        OCR0B = light++;
    else
        OCR0B = light--;
    if ((light == 0xff) || (light == 0x00))
	mark = ~mark;
}

void Init_timer_adc ()
{
    TCCR0A = (1<<WGM01); //CTC模式
    TCCR0B = (1<<CS01)|(1<<CS00);
    OCR0A  = 0X96; //控制取样频率 2ms每次
    TIMSK0 = (1<<OCIE0A);
}

void Init_ADC (void)
{
    ADMUX  = (1<<REFS0)|(1<<MUX1); //选用片内基准电压
    ADCSRA = (1<<ADEN)|(1<<ADSC)|(1<<ADATE)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1);
    ADCSRB = (1<<ADTS1)|(1<<ADTS0); //触发源选用T/C比较匹配A
    sei();

}

//ctc比较匹配中断
ISR (TIM0_COMPA_vect)
{
     time_ok = 1;
}

//ADc转换结束中断
ISR (ADC_vect)
{
     adc_data = ADCW;
}

