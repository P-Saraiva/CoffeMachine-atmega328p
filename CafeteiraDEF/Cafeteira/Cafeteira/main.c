/*
 * Cafeteira.c
 *
 * Created: 30/06/2022 20:02:07
 * Author : Cafe com Açucar
 */ 

#define F_CPU 16000000UL	

// configurar BaudRate da serial
#define VELOCIDADE 9600
#define CALCULO_UBRRO (((F_CPU)/(VELOCIDADE*16UL))-1)

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include "LCD.h"

//variaveis
int segundosCafe = 8;
int segundosLeite = 10;
int segundosChocolate = 10;
float valorArrecadado = 0.00;

volatile int Quantidades[6];
volatile int aux = 0;
volatile int bebidaSelecionada = 0;
volatile char FLAG_Select = 0;

volatile int segundos = 0;

volatile char ShowAbastecimento = 0;
volatile char firstTimeOff = 0;
volatile char firstTime = 0;
//volatile int zeraFLAG = 0;

enum estados {Desligado, Ligado, ComCopo, PreparoBebida, Reabastecimento};
enum estados _estado;

/*------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------*/
// ----------------------  Funcões da Serial ----------------------
// Toda a mensagem foi transmitida
uint8_t uartTxOk(void)
{
	return (UCSR0A & (1<<UDRE0));
}

// Memoriza toda a mensagem para enviar
void uartTx(uint8_t dado)
{
	UDR0 = dado;
}

// Transmite Serial
void transmiteStringSerial(char *p)
{
	while(*p != '\0')  // \0 significa o final de uma string
	{
		// Verifica se o buffer de transmissao esta vazio
		// se UDRE0 = 1, está vazio
		while(!(UCSR0A & (1<<UDRE0)));
		// Associa a memoria com o valor do ponteiro.
		UDR0 = *p;
		p++;
		
	}
}

// Recebe Serial
ISR(USART_RX_vect)
{
	char ReceiveByte;
	ReceiveByte = UDR0;
	transmiteStringSerial("Você enviou: ");
	while(!uartTxOk());
	uartTx(ReceiveByte);
	transmiteStringSerial("\r\n");
	UDR0 = ReceiveByte;
}

// Timer 1
ISR(TIMER1_COMPA_vect){
	segundos++;
}

// Interrupcao INT0 -- SELECT --
ISR(INT0_vect){
	
	if(FLAG_Select == 1)
	{
		FLAG_Select = 0;
		
		if (_estado == ComCopo || _estado == Ligado){
			aux++;
			switch(aux){
				case 1:
				set_bit(PORTC, D0); //7 segmentos 1
				clr_bit(PORTC, D1);
				clr_bit(PORTC, D2);
				clr_bit(PORTC, D3);
				
				// LCD
				cmd_LCD(0x80,0);
				escreve_LCD("1-Cafe Puro         ");
				cmd_LCD(0xC0,0);
				escreve_LCD("Valor R$1,50          ");
				
				bebidaSelecionada = 1;
				break;
				
				case 2:
				clr_bit(PORTC, D0); //7 segmentos 2
				set_bit(PORTC, D1);
				clr_bit(PORTC, D2);
				clr_bit(PORTC, D3);
				
				// LCD
				cmd_LCD(0x80,0);
				escreve_LCD("2-Cafe+Leite          ");
				cmd_LCD(0xC0,0);
				escreve_LCD("Valor R$2,00              ");
				
				bebidaSelecionada = 2;
				break;
				
				case 3:
				set_bit(PORTC, D0); //7 segmentos 3
				set_bit(PORTC, D1);
				clr_bit(PORTC, D2);
				clr_bit(PORTC, D3);
				
				// LCD
				cmd_LCD(0x80,0);
				escreve_LCD("3-Cafe+Leite+Chocolate        ");
				cmd_LCD(0xC0,0);
				escreve_LCD("Valor R$2,75                     ");
				
				bebidaSelecionada = 3;
				break;
				
				case 4:
				clr_bit(PORTC, D0); //7 segmentos 4
				clr_bit(PORTC, D1);
				set_bit(PORTC, D2);
				clr_bit(PORTC, D3);
				
				// LCD
				cmd_LCD(0x80,0);
				escreve_LCD("4-Leite                 ");
				cmd_LCD(0xC0,0);
				escreve_LCD("Valor R$1,00               ");
				
				bebidaSelecionada = 4;
				
				break;
				case 5:
				set_bit(PORTC, D0); //7 segmentos 5
				clr_bit(PORTC, D1);
				set_bit(PORTC, D2);
				clr_bit(PORTC, D3);
				
				// LCD
				cmd_LCD(0x80,0);
				escreve_LCD("5-Leite+Chocolate               ");
				cmd_LCD(0xC0,0);
				escreve_LCD("Valor R$1,80               ");
				
				bebidaSelecionada = 5;
				break;
				
				case 6:
				clr_bit(PORTC, D0); //7 segmentos 6
				set_bit(PORTC, D1);
				set_bit(PORTC, D2);
				clr_bit(PORTC, D3);
				
				// LCD
				cmd_LCD(0x80,0);
				escreve_LCD("6-Chocolate         ");
				cmd_LCD(0xC0,0);
				escreve_LCD("Valor R$1,50           ");
				
				bebidaSelecionada = 6;
				break;
				
				case 7:
				set_bit(PORTC, D0); //7 segmentos 7
				set_bit(PORTC, D1);
				set_bit(PORTC, D2);
				clr_bit(PORTC, D3);
				
				// LCD
				cmd_LCD(0x80,0);
				escreve_LCD("7-Leite/Chocolate          ");
				cmd_LCD(0xC0,0);
				escreve_LCD("Valor R$1,80          ");
				
				bebidaSelecionada = 7;
				aux = 0;
				break;
			}
		}
	}
	else
		FLAG_Select = 1;
	
}

int VerificaIngredientes(){
	if ((segundosCafe < 6) && (segundosCafe > 3)) //print lcd sem cafe puro
		return 1;
	if (segundosCafe == 2) //sem cafe com chocolate, cafe com leite e cafe puro
		return 2;
	if (segundosCafe < 2) //sem cafe com leite e chocolate, cafe com chocolate, cafe com leite e cafe puro
		return 3;
	if ((segundosLeite < 6) && (segundosLeite > 3)) //sem leite puro
		return 4;
	if (segundosLeite == 2) //sem cafe com leite, leite com chocolate e leite puro
		return 5;
	if (segundosLeite < 2) //sem cafe com leite e chocolate, leite com chocolate, cafe com leite e leite puro
		return 6;
	if ((segundosChocolate < 6) && (segundosChocolate > 3)) //sem chocolate puro
		return 7;
	if (segundosChocolate == 2) //sem cafe com chocolate, leite com chocolate e chocolate puro
		return 8;
	if (segundosChocolate < 2) //sem cafe com leite e chocolate, leite com chocolate, cafe com chocolate e chocolate puro
		return 9;
	
	return 0;
}

// Interrupcao INT1 -- ENTER --
ISR(INT1_vect){	
	char msg[70];
	if (_estado == Reabastecimento){
		
		// Envio de LOG para Serial
		sprintf(msg,"Modo reabastecimento finalizado \r\n");
		transmiteStringSerial(msg);
		
		_estado = Ligado;
		return;
	}

	if (rd_bit(PINB, S_COPO)> 0){
		int var = VerificaIngredientes();
		
		sprintf(msg,"%d",var);
		transmiteStringSerial(msg);
		
		FLAG_Select = 0;
		switch(var)
		{
			case 0: //tudo ok
				// Envio de LOG para Serial

				sprintf(msg,"Todos os ingredientes disponiveis. Preparando bebida \r\n");
				transmiteStringSerial(msg);
				
				_estado = PreparoBebida;
			break;
			case 1:
				if(bebidaSelecionada == 1){
					// LCD
					cmd_LCD(0x80,0);
					escreve_LCD("Nao ha cafe puro         ");
					cmd_LCD(0xC0,0);
					escreve_LCD("                  ");
					
					// Envio de LOG para Serial

					sprintf(msg,"Nao ha cafe disponivel. Aguardando cliente selecionar outro cafe \r\n");
					transmiteStringSerial(msg);
					}else{
					// Envio de LOG para Serial
	
					sprintf(msg,"Ha estouque para fazer o cafe puro. Preparando o cafe \r\n");
					transmiteStringSerial(msg);
					_estado = PreparoBebida;
				}
			break;
			case 2:
				if((bebidaSelecionada == 1) || (bebidaSelecionada  == 2) || (bebidaSelecionada == 7)){
					// LCD
					cmd_LCD(0x80,0);
					escreve_LCD("Nao ha cafe puro         ");
					cmd_LCD(0xC0,0);
					escreve_LCD("                  ");
					
					// Envio de LOG para Serial

					sprintf(msg,"Nao ha cafe disponivel. Aguardando cliente selecionar outro cafe \r\n");
					transmiteStringSerial(msg);
					}else{
					// Envio de LOG para Serial
					
					sprintf(msg,"Ha estouque para fazer o cafe puro. Preparando o cafe \r\n");
					transmiteStringSerial(msg);
					_estado = PreparoBebida;
				}
				break;
			case 3:
				if((bebidaSelecionada == 1) || (bebidaSelecionada  == 2) || (bebidaSelecionada  == 3) || (bebidaSelecionada == 7)){
				// LCD
				cmd_LCD(0x80,0);
				escreve_LCD("Nao ha cafe puro         ");
				cmd_LCD(0xC0,0);
				escreve_LCD("                  ");
				
				// Envio de LOG para Serial

				sprintf(msg,"Nao ha cafe disponivel. Aguardando cliente selecionar outro cafe \r\n");
				transmiteStringSerial(msg);
				}else{
				// Envio de LOG para Serial
				
				sprintf(msg,"Ha estouque para fazer o cafe puro. Preparando o cafe \r\n");
				transmiteStringSerial(msg);
				_estado = PreparoBebida;
			}
			break;
			case 4:
				if(bebidaSelecionada == 4){
					// LCD
					cmd_LCD(0x80,0);
					escreve_LCD("Sem Leite Puro");
					cmd_LCD(0xC0,0);
					escreve_LCD("                 ");
					
					// Envio de LOG para Serial
		
					sprintf(msg,"Nao ha estouque de leite. Aguardando usuario selecionar outro cafe \r\n");
					transmiteStringSerial(msg);
					}else{
					// Envio de LOG para Serial
		
					sprintf(msg,"Ha estouque para fazer o pedido. Preparando a bedida  \r\n");
					transmiteStringSerial(msg);
					_estado = PreparoBebida;
				}
			break;
			case 5:
				if((bebidaSelecionada == 2) || (bebidaSelecionada  == 4) || (bebidaSelecionada == 5)){
					// LCD
					cmd_LCD(0x80,0);
					escreve_LCD("Sem Leite Puro");
					cmd_LCD(0xC0,0);
					escreve_LCD("                 ");
					
					// Envio de LOG para Serial
					
					sprintf(msg,"Nao ha estouque de leite. Aguardando usuario selecionar outro cafe \r\n");
					transmiteStringSerial(msg);
					}else{
					// Envio de LOG para Serial
					
					sprintf(msg,"Ha estouque para fazer o pedido. Preparando a bedida  \r\n");
					transmiteStringSerial(msg);
					_estado = PreparoBebida;
				}
				break;
			case 6:
				if((bebidaSelecionada == 2) || (bebidaSelecionada  == 3) || (bebidaSelecionada == 4)|| (bebidaSelecionada == 5)){
					// LCD
					cmd_LCD(0x80,0);
					escreve_LCD("Sem Leite Puro");
					cmd_LCD(0xC0,0);
					escreve_LCD("                 ");
					
					// Envio de LOG para Serial
					
					sprintf(msg,"Nao ha estouque de leite. Aguardando usuario selecionar outro cafe \r\n");
					transmiteStringSerial(msg);
					}else{
					// Envio de LOG para Serial
					
					sprintf(msg,"Ha estouque para fazer o pedido. Preparando a bedida  \r\n");
					transmiteStringSerial(msg);
					_estado = PreparoBebida;
				}
				break;
			case 7:
				if(bebidaSelecionada == 6){
					// LCD
					cmd_LCD(0x80,0);
					escreve_LCD("Sem Choc Puro");
					cmd_LCD(0xC0,0);
					escreve_LCD("                          ");
					
					// Envio de LOG para Serial

					sprintf(msg,"Nao ha estouque de chocolate. Aguardando usuario selecionar outro pedido \r\n");
					transmiteStringSerial(msg);
					}else{
						// Envio de LOG para Serial
			
					sprintf(msg,"Ha estouque para fazer o pedido. Preparando a bedida \r\n");
					transmiteStringSerial(msg);
					_estado = PreparoBebida;
				}
			break;
			case 8:
				if((bebidaSelecionada == 5) || (bebidaSelecionada  == 6) || (bebidaSelecionada == 7)){
					// LCD
					cmd_LCD(0x80,0);
					escreve_LCD("Sem Choc Puro");
					cmd_LCD(0xC0,0);
					escreve_LCD("                          ");
					
					// Envio de LOG para Serial

					sprintf(msg,"Nao ha estouque de chocolate. Aguardando usuario selecionar outro pedido \r\n");
					transmiteStringSerial(msg);
					}else{
					// Envio de LOG para Serial
					
					sprintf(msg,"Ha estouque para fazer o pedido. Preparando a bedida \r\n");
					transmiteStringSerial(msg);
					_estado = PreparoBebida;
				}
				break;
			case 9:
				if((bebidaSelecionada == 3) || (bebidaSelecionada  == 5) || (bebidaSelecionada == 6)|| (bebidaSelecionada == 7)){
					// LCD
					cmd_LCD(0x80,0);
					escreve_LCD("Sem Choc Puro");
					cmd_LCD(0xC0,0);
					escreve_LCD("                          ");
					
					// Envio de LOG para Serial

					sprintf(msg,"Nao ha estouque de chocolate. Aguardando usuario selecionar outro pedido \r\n");
					transmiteStringSerial(msg);
					}else{
					// Envio de LOG para Serial
					
					sprintf(msg,"Ha estouque para fazer o pedido. Preparando a bedida \r\n");
					transmiteStringSerial(msg);
					_estado = PreparoBebida;
				}
				break;
		}
		return;
	}
	
	if (FLAG_Select){
		
		//LCD
		cmd_LCD(0x80,0);
		escreve_LCD("MODO REBASTECIMENTO");
		cmd_LCD(0xC0,0);
		escreve_LCD("                    ");
		
		// Envio de LOG para Serial
		sprintf(msg,"Entrando no modo reabastecimento  \r\n");
		transmiteStringSerial(msg);
		ShowAbastecimento = 0;
		
		_estado = Reabastecimento;
		return;
	}
}

/*------------------------------------------------------------------------------------*/


void setValvula(int ingrediente, int condicao){
	switch(ingrediente)
	{
		case 1:	// cafe
			if (condicao == 1)
				set_bit(PORTB, CAFE);
			else
				clr_bit(PORTB, CAFE);
		break;
		
		case 2:	// leite
			if (condicao == 1)
				set_bit(PORTB, LEITE);
			else
				clr_bit(PORTB, LEITE);
		break;
		
		case 3:	// chocolate
			if (condicao == 1)
				set_bit(PORTB, CHOCO);
			else
				clr_bit(PORTB, CHOCO);
		break;
	}
}

void setMotor(int condicao){
	if (condicao == 1)
		TCCR2B = 0b00000100; // prescaler de 64 Liga Motor
	if (condicao == 0)
	{
		TCNT2  = 0;    //zera temporizador
		TCCR2B = 0;    // prescaler de 0 Desliga Motor
	}
}

/*------------------------------------------------------------------------------------*/

// Maquina de estados

void desligado(){
	clr_bit(PORTC, LED_ONOFF);
	FLAG_Select = 0;
	
	cmd_LCD(0x80,0);
	escreve_LCD("Desligada         ");
	cmd_LCD(0xC0,0);
	escreve_LCD("                 ");
	if(firstTimeOff == 0)
	{
		// Envio de LOG para Serial
		char msg[50];
		sprintf(msg,"Máquina Desligada \r\n");
		transmiteStringSerial(msg);
		firstTimeOff = 1;
	}
	
	if (rd_bit(PINC, BT_ONOFF) > 0 ){
		firstTime = 0;
		_estado = Ligado;
		return;
	}
}

void ligado()
{
	char msg[70];
	set_bit(PORTC, LED_ONOFF);

	
	EIMSK = 0b00000011;	// Habilita mascara das interrupções int0 e int1
	if(firstTime == 0)
	{
		// LCD
		cmd_LCD(0x80,0);
		escreve_LCD("Máquina Ligada");
		cmd_LCD(0xC0,0);
		escreve_LCD("Escolha a bebida:");
		
		// Envio de LOG para Serial
		sprintf(msg,"Máquina Ligada. Aguardando cliente selecionar pedido \r\n");
		transmiteStringSerial(msg);
		firstTime = 1;
	} 
	
	if (rd_bit(PINC, BT_ONOFF) ==0 ){
		//Muda estado para Desligado
		firstTimeOff = 0;
		_estado = Desligado;
	}
	
	if(rd_bit(PINB, S_COPO) > 0){
		// Envio de LOG para Serial
		sprintf(msg,"Maquina Estado - Com copo. Aguardando o botao ENTER ser pressionado. \r\n");
		transmiteStringSerial(msg);
		//Muda estado para comCopo
		_estado = ComCopo;
	}
}

void comCopo(){
	;
	if(rd_bit(PINB, S_COPO) == 0 )
	{
		_estado = Ligado;
	}
}

void preparoBebida(){
	TCCR1B = 0b00001101;   // prescaler de 1024: CS12 = 1 e CS10 = 1
	int tempoPreparo = 0;
	
	// LCD
	cmd_LCD(0x80,0);
	escreve_LCD("Preparando          ");
	cmd_LCD(0xC0,0);
	escreve_LCD("Aguarde...          ");
	
	segundos = 0;
	switch(bebidaSelecionada)
	{
		case 1: 
		//print no lcd cafe puro
			
			setMotor(1);	//liga o motor
			while(segundos < 3){}; //espera 3s
			setMotor(0); //desliga o motor
			
			segundos = 0; //zera o parametro de tempo
		
			tempoPreparo = 6;
			setValvula(1,1); //Abre valvula cafe
			
			// Balanceio
			segundosCafe -= tempoPreparo;
			Quantidades[0] += 1;
			valorArrecadado+= 1.50;
		break;

		case 2:
			setMotor(1);	//liga o motor
			while(segundos < 3){}; //espera 3s
			setMotor(0); //desliga o motor
			segundos = 0; //zera o parametro de tempo
		
			tempoPreparo = 3;
			setValvula(2,1); //Abre valvula leite
			setValvula(1,1); //Abre valvula cafe
			
			segundosCafe -= tempoPreparo;
			segundosLeite -= tempoPreparo;
			Quantidades[1] += 1;
			valorArrecadado+= 2.00;
		break;

		case 3:
			setMotor(1);	//liga o motor
			while(segundos < 3){}; //espera 3s
			setMotor(0); //desliga o motor
			segundos = 0; //zera o parametro de tempo
		
			tempoPreparo = 2;
			setValvula(3,1); //Abre valvula chocolate
			setValvula(2,1); //Abre valvula leite
			setValvula(1,1); //Abre valvula cafe
			
			segundosCafe -= tempoPreparo;
			segundosLeite -= tempoPreparo;
			segundosChocolate -= tempoPreparo;
			Quantidades[2] += 1;
			valorArrecadado+= 2.75;
		break;

		case 4:
			tempoPreparo = 6;
			setValvula(2,1); //Abre valvula leite
			
			segundosLeite -= tempoPreparo;
			Quantidades[3] += 1;
			valorArrecadado+= 1.00;
		break;

		case 5:
			tempoPreparo = 3;
			setValvula(3,1); //Abre valvula chocolate
			setValvula(2,1); //Abre valvula leite
			
			segundosLeite -= tempoPreparo;
			segundosChocolate -= tempoPreparo;
			Quantidades[4] += 1;
			valorArrecadado+= 1.80;
		break;

		case 6:
			tempoPreparo = 6;
			setValvula(3,1); //Abre valvula chocolate
			
			segundosChocolate -= tempoPreparo;
			Quantidades[5] += 1;
			valorArrecadado+= 1.50;
		break;

		case 7:
			setMotor(1);	//liga o motor
			while(segundos < 3){}; //espera 3s
			setMotor(0); //desliga o motor
			segundos = 0; //zera o parametro de tempo
		
			tempoPreparo = 3;
			setValvula(3,1); //Abre valvula chocolate
			setValvula(1,1); //Abre valvula cafe
			
			segundosCafe -= tempoPreparo;
			segundosChocolate -= tempoPreparo;
			Quantidades[6] += 1;
			valorArrecadado+= 2.10;
		break;
	}

	// Espera tempo de passar as bebidas
	while(segundos < tempoPreparo){};
	
	// Fecha todas as valvulas
	setValvula(1,0);
	setValvula(2,0);
	setValvula(3,0);
	
	TCCR1B = 0; //para o timer
	TCCR1B |= (1 << WGM12); //zera o preset e matem o modo ctc
	segundos = 0;

	// LCD
	cmd_LCD(0x80,0);
	escreve_LCD("Bebida Pronta!    ");
	cmd_LCD(0xC0,0);
	escreve_LCD("Retire seu Copo    ");

	// Envio de LOG para Serial
	char msg[80];
	sprintf(msg,"Bebida pronta. \r\n");
	transmiteStringSerial(msg);

	while(rd_bit(PORTD,SELECT));
	// LCD
	cmd_LCD(0x80,0);
	escreve_LCD("Aperte Select p/");
	cmd_LCD(0xC0,0);
	escreve_LCD("Escolher a bebida");

	_estado = Ligado;
}

void reabastecimento(){
	if(ShowAbastecimento == 0)
	{
		char msg[70];
		
		// Bebida 1 Valor arrecadado
		float ValorTotalBebida1 = 0;
		ValorTotalBebida1 = Quantidades[0] * (1.5);
		int ValorRealBebida1 = 0;
		ValorRealBebida1 = (int)ValorTotalBebida1;
		float transitorio1 = (ValorTotalBebida1 - ValorTotalBebida1);
		int ValoresCentavos1 = (int)(transitorio1 * 100);
		
		sprintf(msg,"%d",Quantidades[0]);
		transmiteStringSerial(msg);
		sprintf(msg," Xicaras de Cafe Puro. Valor arrecadado com esta bebida: ");
		transmiteStringSerial(msg);
		sprintf(msg,"%d",ValorRealBebida1);
		transmiteStringSerial(msg);
		sprintf(msg,",");
		transmiteStringSerial(msg);
		sprintf(msg,"%d",ValoresCentavos1);
		transmiteStringSerial(msg);
		sprintf(msg,"\r\n");
		transmiteStringSerial(msg);
		
		// Bebida 1 Valor arrecadado
		float ValorTotalBebida2 = 0;
		ValorTotalBebida2 = Quantidades[1] * (1.5);
		int ValorRealBebida2 = 0;
		ValorRealBebida2 = (int)ValorTotalBebida2;
		float transitorio2 = (ValorTotalBebida2 - ValorTotalBebida2);
		int ValoresCentavos2 = (int)(transitorio2 * 100);
		
		sprintf(msg,"%d",Quantidades[1]);
		transmiteStringSerial(msg);
		sprintf(msg," Xicaras de Cafe com Leite. Valor arrecadado com esta bebida: ");
		transmiteStringSerial(msg);
		sprintf(msg,"%d",ValorRealBebida2);
		transmiteStringSerial(msg);
		sprintf(msg,",");
		transmiteStringSerial(msg);
		sprintf(msg,"%d",ValoresCentavos2);
		transmiteStringSerial(msg);
		sprintf(msg,"\r\n");
		transmiteStringSerial(msg);
		
		//reabastece
		segundosCafe = 720;
		segundosLeite = 300;
		segundosChocolate = 420;
		
		// Envio do valor total arrecadado pela serial
		int ValoresReal = 0;
		ValoresReal = (int)valorArrecadado;
		float transitorio = (valorArrecadado - ValoresReal);
		int ValoresCentavos = (int)(transitorio * 100);
		// Envio de LOG para Serial
		
		sprintf(msg,"O valor total arrecadado foi: \r\n");
		transmiteStringSerial(msg);
		sprintf(msg,"%d",ValoresReal);
		transmiteStringSerial(msg);
		sprintf(msg,",");
		transmiteStringSerial(msg);
		sprintf(msg,"%d",ValoresCentavos);
		transmiteStringSerial(msg);
		sprintf(msg,"\r\n");
		transmiteStringSerial(msg);
		
		//zera valor arrecadado
		valorArrecadado = 0.00;
		ShowAbastecimento = 1;
	}
}

/*------------------------------------------------------------------------------------*/

//Main
int main(void){
	//Setup

	//Define entradas e saídas dos pinos, sem pullup
	DDRB = 0b00111111;
	DDRC = 0b00101111; 
	DDRD = 0b11110010;
	
	//configurar a serial para 9600 bits/s
	UBRR0H = CALCULO_UBRRO >> 8;
	UBRR0L = CALCULO_UBRRO;
	
	//configura a serial p trabalhar com 8 bits, sem paridade, 1 stop bit ... 8N1
	UCSR0C = (1<<UCSZ00) | (1<<UCSZ01);
	
	// Habilita a transmissao, recepcao por interrupcao serial
	UCSR0B = 0b10011000;
	
	// Timer 1 1s
	TCCR1A = 0;	//limpa registrador
	TCCR1B = 0;	//limpa registrador
	TCNT1  = 0;	//zera temporizado
	
	OCR1A = 0x3D09;           // carrega registrador de comparação: 16MHz/1024/1Hz = 15625 = 0X3D09
	TCCR1B |= (1 << WGM12);   // modo CTC prescaler de 0
	TIMSK1 |= (1 << OCIE1A);  // habilita interrupção por igualdade de comparação
	
	// Timer 2 1kHz
	TCCR2A = 0;	//limpa registrador
	TCCR2B = 0;	//limpa registrador
	TCNT2  = 0;	//zera temporizador
	
	OCR2A = 125;           // carrega registrador de comparação: 16MHz/2*64/1kHz = 125
	TCCR2A = (1 << COM2A0)|(1 << WGM21); // modo CTC
	TCCR2B = 0;    // prescaler de 0

	// Interrupções Externas
	EICRA = 0b00001001; // Habilita interrupções int1 e int0  = enter e select
	
	sei(); // Habilita interrupções globais
		
	set_bit(PORTC, D0); //7 segmentos 1
	bebidaSelecionada = 1;
	
	// Inicia LCD
	inic_LCD_4bits();
		
	// Envio de LOG para Serial
	char msg[50];
	sprintf(msg,"Simulacao Proteus Iniciada. Realizando Setup\r\n");
	transmiteStringSerial(msg);
	
	Quantidades[0] = 0;
	Quantidades[1] = 0;
	Quantidades[2] = 0;
	Quantidades[3] = 0;
	Quantidades[4] = 0;
	Quantidades[5] = 0;
	Quantidades[6] = 0;
	
	_estado = Desligado; // Estado inicial

	// Loop
	while(1){
		switch (_estado){
			case Desligado:
				desligado();			
			break;
		
			case Ligado:
				ligado();
			break;

			case ComCopo:
				comCopo();
			break;
			
			case PreparoBebida:
				preparoBebida();
			break;
			
			case Reabastecimento:
				reabastecimento();
			break;
		}
	}
}

