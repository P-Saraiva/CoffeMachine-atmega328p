#ifndef _DEF_PRINCIPAIS_H
#define _DEF_PRINCIPAIS_H

#define F_CPU 16000000UL	//define a frequencia do microcontrolador - 16MHz

// configurar BaudRate da serial
#define VELOCIDADE 9600
#define CALCULO_UBRRO (((F_CPU)/(VELOCIDADE*16UL))-1)

#include <avr/io.h> 	    //definições do componente especificado
#include <util/delay.h>		//biblioteca para o uso das rotinas de _delay_ms e _delay_us()
#include <avr/pgmspace.h>   //para o uso do PROGMEM, gravação de dados na memória flash

//Definições de macros para o trabalho com bits

#define	set_bit(y,bit)	(y|=(1<<bit))	//coloca em 1 o bit x da variável Y
#define	clr_bit(y,bit)	(y&=~(1<<bit))	//coloca em 0 o bit x da variável Y
#define cpl_bit(y,bit) 	(y^=(1<<bit))	//troca o estado lógico do bit x da variável Y
#define rd_bit(y,bit) 	(y&(1<<bit))	//retorna 0 ou 1 conforme leitura do bit

// Definicoes Gerais
#define FALSE 0
#define TRUE 1

//Serial
#define RXD PIND0
#define TXD PORTD1

//seleção
#define SELECT PIND2
#define ENTER PIND3

//LCD
#define LCD_D4 PORTD4
#define LCD_D5 PORTD5
#define LCD_D6 PORTD6
#define LCD_D7 PORTD7
#define LCD_RS PORTB0
#define LCD_E PORTB1

//Bebidas
#define CAFE PORTB2
#define MOTOR PORTB3
#define LEITE PORTB4
#define CHOCO PORTB5

//sensor copo
#define S_COPO PINB7

//7 seg
#define D0 PORTC0
#define D1 PORTC1
#define D2 PORTC2
#define D3 PORTC3

//botao on-off
#define BT_ONOFF PINC4
#define LED_ONOFF PORTC5

#endif
