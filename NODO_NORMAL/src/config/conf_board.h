
#ifndef CONF_BOARD_H
#define CONF_BOARD_H

// pulsadores con su respectivo puerto
#define GPIO_PUSH_BUTTON_0	IOPORT_CREATE_PIN(PORTE, 5)	//Definici�n bot�n PUSH y PE 5/ Nodo ATZB: PORT E_0 //Nodo RCB: PORT E_5
#define GPIO_PUSH_BUTTON_ON_BOARD GPIO_PUSH_BUTTON_0	//Definici�n bot�n GPIO_PUSH_BUTTON_0 como GPIO_PUSH_BUTTON_0/
//diodos led con su respectivo puerto
#define LED0		IOPORT_CREATE_PIN(PORTD, 6)	/*Definici�n LED0 y PE2*/ //Nodo ATZB: PORT D_6, G_2, E_2
#define LED1		IOPORT_CREATE_PIN(PORTG, 2) /*Definici�n LED0 y PE3*/ //Nodo RCB: PORT E_2, E_3, E_4
#define LED2		IOPORT_CREATE_PIN(PORTE, 2) /*Definici�n LED0 y PE3*/
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/
/*Asignaci�n de funcionalidades de LEDs*/
#define LED_Off(led_gpio)		ioport_set_pin_level(led_gpio, 1)
#define LED_On(led_gpio)		ioport_set_pin_level(led_gpio, 0)
#define LED_Toggle(led_gpio)	ioport_toggle_pin_level(led_gpio)

#endif // CONF_BOARD_H
