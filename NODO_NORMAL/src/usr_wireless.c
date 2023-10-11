#include "usr_wireless.h"
#include "wireless_config.h"
#include "math.h"
#include "periodic_timer.h"// Permite utilizar temporizadores.
// Datos Compartidos
uint16_t ID_DEFAULT;

// Datos de Subida
uint16_t ID_S;// Almacena la direcion fuente o origen (source) del dispositivo  
uint8_t NB[4];// Nivel de bateria del dispositivo
strc_NS listaNS[7];// Lista con niveles de senal de los nodos proximos, la cantidad de elementos no deberia exeder del numero de nodos
int longitudListaNS=0;// Contiene la cantidad de niveles de energia almacenados en el nodo
int cont_NS=0;// Ayuda a agregar nuevos valores de niveles de energia recibidos 
uint16_t SRC_ADDR_;// Variable para modificar la direccion fuente desde el codigo principal
int longitudTramaRX=0; 
 
// Datos de Bajada
uint8_t IDS_TRAMAS[20]; // Lista de identidicadores de tramas enviadas por el PC, la cantidad de elementos no deberia exeder del numero de nodos
int longitud_IDTramas=0;// Ayuda a agregar nuevos identidicadores de tramas enviados por el PC
int ctrReTX=0; // 
int tiempoTX=0; //
//uint16_t ID_DEFAULT_NUEVO;// Almacena el nuevo identificador por defecto

// Control adicional
//int count_1=0; //
int iniTx=0;// Permite enviar una trama de subida
int DatosNSIniciales=0; // Ayuda a cargar datos iniciales de nivel de señal???????
int cargarDir=0;// // Ayuda a cargar la direcion por defecto inicial
tiempoDuracionTimer=0;//Variable para asignar la duraci�n del temporizador de un estado especifico
//int agregarTimer=0;//Variable para activar el temporizador cuando sea necesario
TimerFinaliza=0;// Variable para indicar que el tiempo del temporizador finalizo
//Variable de pruebas
int crtfor=0;
int ctrtem=0;
int contUltimaTX=0;
uint8_t listacrtfor[1];
//// Funciones para comvercion de numeros flotantes /////////////////////////////////////////////////////////////////////////////////////////
void reverse(char* str, int len);
int intToStr(int x, char str[], int d);
void ftoa(float n, char* res, int afterpoint);

void usr_wireless_app_task(void)
{

	SRC_ADDR_=0x0001;// Identificador de direccion fuente(source)
	SRC_ADDR_ = CCPU_ENDIAN_TO_LE16(SRC_ADDR_);
	tal_pib_set(macShortAddress, (pib_value_t *)&SRC_ADDR_);// Asignacon de direccion fuente
	ID_S=SRC_ADDR_; // Almacenamiento de direccion fuente previamente asignada
	if (cargarDir==0)
	{
		DST_ADDR_=0x0002;// Almacenamiento del identificador para alcanzar al siguiente nodo, se ingresa un valor inicial.
		ID_DEFAULT=DST_ADDR_;
		cargarDir=1;
	}
	//Datos Iniciales 
	if (DatosNSIniciales==0)
	{
		longitudListaNS=1;	
	}

	if (!ioport_get_pin_level(GPIO_PUSH_BUTTON_0))
	{
		delay_ms(10); //Retardo para que funcione el pulsador
		longitudListaNS=cont_NS;//Cambia de valor cuando es deshabilitado el if anterior
		//longitudListaNS=7;
		iniTx=1;
		// Obtener nivel de Bateria ////////////////////////////////////////////////////////////
		char charBateria[20]; // Almacena el nivel de la bateria comvertido en una cadena de caracteres
		float floatBateria=get_bat_sensor_data();// Obtiene en nivel de bateria
		ftoa(floatBateria, charBateria, 4);// Combierte el nivel de bateria de float a una cadena de caracteres hexadecimales
		NB[0]=(uint8_t)charBateria[0];// almacena los cuatro primeros valores de la cadena en el vector NB
		NB[1]=(uint8_t)charBateria[1];
		NB[2]=(uint8_t)charBateria[2];
		NB[3]=(uint8_t)charBateria[3];	
	}
	else if (iniTx==1)
	{ 	/////////////////////////// Construccion de la trama de subida a ser enviada  ///////////////////////////////////////////////
		iniTx=0;
		//count_1=1;
		int indice=0;
		uint8_t TRAMA[50];//Trama de subida
		//-------IDENTIFICADOR FUENTE---------------------------------------//
		TRAMA[0]=(uint8_t)(ID_S >> 8);
		TRAMA[1]=(uint8_t)ID_S;
		//-------NIVEL DE BATERIA-------------------------------------------//	
		TRAMA[2]=NB[0];
		TRAMA[3]=NB[1];
		TRAMA[4]=NB[2];		
		TRAMA[5]=NB[3];	
			
		for (int i=0; i<longitudListaNS; i++)
		{	
		//-------NIVEL DE SEÑAL--------------------------------------------//
			TRAMA[indice+6]=(uint8_t)(listaNS[i].NS >> 8);
			TRAMA[indice+7]=(uint8_t)listaNS[i].NS;
		//-------IDENTIFICADOR DEL NIVEL DE SE�AL--------------------------//
			TRAMA[indice+8]=(uint8_t)(listaNS[i].ID_DIR_NS >> 8);
			TRAMA[indice+9]=(uint8_t)listaNS[i].ID_DIR_NS;
			
			indice=indice+4;
		}
		TRAMA[(longitudListaNS*4)+6]=0xFE;// CH de parada
		DST_ADDR_=ID_DEFAULT;
		transmit_sample_frame(TRAMA, (longitudListaNS*4)+7);	
	
		//crtfor=0;
	}
	//////////////////////////////////////////////////////////////////////////////
	if (ctrReTX==1)
	{
		DST_ADDR_=ID_DEFAULT;		
		transmit_sample_frame(trama_recibida.CargaUtil_802_15_4, longitudTramaRX);
		ctrReTX=0;
	} 
	if (ctrReTX==2)
	{
		int tiempo=(int)SRC_ADDR_;
		tiempo=tiempo*1;
		tiempoDuracionTimer=tiempo;
		start_timer1();
		ctrtem=1;	
		ctrReTX=0;
	}
	if (TimerFinaliza==1 && ctrtem==1)
	{
		TimerFinaliza=0;
		ctrtem=0;
		stop_timer1();
		DST_ADDR_=0xFFFF;/* siguiente salto */
		transmit_sample_frame(trama_recibida.CargaUtil_802_15_4, 5);
	}

	delay_ms(10);
}

void usr_frame_received_cb(frame_info_t *frame)
{		
		memset(&trama_recibida,0,sizeof(trama_recibida));// Elimina informacion previa de la estructura de recepcion 
		memcpy(&trama_recibida,frame->mpdu,sizeof(trama_recibida));// copia la informacion de la memoria fuente(buffer) al detino
		//bmm_buffer_free(frame->buffer_header);//Elimina los datos del buffer, evita superpocicion.
			
		if (trama_recibida.d_dstn==SRC_ADDR_) // Si se cumple esta condicion la trama es de subida
		{
			// Obtencion de la longitud de la trama 
			//int longitudTramaRX=0;
			longitudTramaRX=0;  
			for (int l = 0; l < 50; l++)
			{
				uint8_t ultimoCH=trama_recibida.CargaUtil_802_15_4[l];
				if (ultimoCH==0xFE)
				{
					longitudTramaRX=l+1;
				}
			}
			//Re-transmision de la trama recibida atraves de la ID_DEFAULT
			ctrReTX=1;
			//transmit_sample_frame(trama_recibida.CargaUtil_802_15_4, longitudTramaRX);	
		}
		else if(trama_recibida.d_dstn==0xFFFF) // Si se cumple esta condicion la trama es de bajada		
		{	
			// Almacenar Niveles de Energia ////////////////////////////////////////////////////////////
			int coincedencia_ID_NS=0;
			//int longitudListaNS_1=sizeof(listaNS);
			int longitudListaNS_1=7;
			LED_Toggle(LED0);
			// Berificar que el Nivel de energia no ha sido almacenado previamente  
			for (int i=0; i<longitudListaNS_1; i++)
			{
				if (listaNS[i].ID_DIR_NS==trama_recibida.d_orgn)
				{
					//crtfor++;
					coincedencia_ID_NS=1;// Si esta condicion se cumple, el NS de la trama rx ya se enuentra en la lista de niveles de energia
					//LED_Toggle(LED1);// 
				}
			}
			if (coincedencia_ID_NS==0)// Si esta condicion se cumple, el NS de la trama rx se debe extraer del buffer y almacenarlo  
			{
				// Extraccion del nivel de energia
				uint8_t *payload_ptr=frame->mpdu;
				uint8_t mpdu_len =payload_ptr[0]+2;
				uint8_t potencia_trama=payload_ptr[mpdu_len];
				//Agregacion de nuevos datos de NS
				LED_Toggle(LED2);
				listaNS[cont_NS].NS=potencia_trama;
				listaNS[cont_NS].ID_DIR_NS=trama_recibida.d_orgn;
				cont_NS++;
				DatosNSIniciales=1;			
			}
			//Extraccion de la direccion de destino final( ID_DF) de la trama de bajada
			uint8_t DF1=trama_recibida.CargaUtil_802_15_4[0];
			uint8_t DF2=trama_recibida.CargaUtil_802_15_4[1];
			//Combinacion de los dos primeros bytes para obtener una dirreccion 
			uint16_t ID_DF;
			ID_DF=DF1*0x100;// Se recorren 8 bits a la izquierda
			ID_DF=ID_DF+DF2; //Se completan los bits restantes
			
			if (ID_S==ID_DF) //Se comprueba que la direccion de destino final pertenece al nodo o no
			{				
				// Nueva Direccion por Defecto ID_DEFAULT almacenada en una trama de bajada
				uint8_t ID_DEFAULT_Rx1 = trama_recibida.CargaUtil_802_15_4[2];
				uint8_t ID_DEFAULT_Rx2 = trama_recibida.CargaUtil_802_15_4[3];	
				uint16_t ID_DEFAULT_NUEVO;// Almacena el nuevo identificador por defecto			
				ID_DEFAULT_NUEVO=ID_DEFAULT_Rx1*0x100;// Se recorren 8 bits a la izquierda
				ID_DEFAULT_NUEVO=ID_DEFAULT_NUEVO+ID_DEFAULT_Rx2;// Se completan los bits restantes
				// Agregacion del nuevo identificador por defecto
				ID_DEFAULT=ID_DEFAULT_NUEVO;
				//cargarDir=1;// Inavilita la asignacion de ID_DEF desde la funcion principal
			}
			// Prevencion de bucles
			int coincedencia_IDTrama=0;
			uint8_t idTramaRx=trama_recibida.CargaUtil_802_15_4[4];
			for(int i=0; i<=longitud_IDTramas; i++)
			{
				if (IDS_TRAMAS[i]==idTramaRx)
				{
					coincedencia_IDTrama=1;// Indica que se encontro el ID_FRAME 
				}					
			}
			if (coincedencia_IDTrama==0)
			{
				IDS_TRAMAS[longitud_IDTramas]=idTramaRx;// Se agrega un nuevo Identificador
				//Re-transmision bajada
				//transmit_sample_frame(trama_recibida.CargaUtil_802_15_4, 5);	
				ctrReTX=2;
				longitud_IDTramas++;
			}				
		}
		delay_ms(0);
		bmm_buffer_free(frame->buffer_header);//Elimina los datos del buffer, evita superpocicion.
}

void usr_frame_transmitted_cb(retval_t status, frame_info_t *frame)
{

}
///////////////////////////////////////////////////////////////////////////////////

void reverse(char* str, int len)
{
	int i = 0, j = len - 1, temp;
	while (i < j) {
		temp = str[i];
		str[i] = str[j];
		str[j] = temp;
		i++;
		j--;
	}
}

int intToStr(int x, char str[], int d)
{
	int i = 0;
	while (x) {
		str[i++] = (x % 10) + '0';
		x = x / 10;
	}
	
	// If number of digits required is more, then
	// add 0s at the beginning
	while (i < d)
	str[i++] = '0';
	
	reverse(str, i);
	str[i] = '\0';
	return i;
}

void ftoa(float n, char* res, int afterpoint)
{
	// Extract integer part
	int ipart = (int)n;
	
	// Extract floating part
	float fpart = n - (float)ipart;
	
	// convert integer part to string
	int i = intToStr(ipart, res, 0);
	
	// check for display option after point
	if (afterpoint != 0) {
		res[i] = '.'; // add dot
		
		// Get the value of fraction part upto given no.
		// of points after dot. The third parameter
		// is needed to handle cases like 233.007
		fpart = fpart * pow(10, afterpoint);
		
		intToStr((int)fpart, res + i + 1, afterpoint);
	}
}
////////////////////////////////////////////////////////////////////////////////////7
float get_bat_sensor_data(void)
{
	float bat_voltage;
	bat_voltage = reverse_float(convert_mv_to_v(tfa_get_batmon_voltage()));
	return bat_voltage;
}

/*Reverses a float variable*/
float reverse_float( const float float_val )
{
	#if UC3
	float retuVal;
	char *floatToConvert = (char *)&float_val;
	char *returnFloat = (char *)&retuVal;
	/* swap the bytes into a temporary buffer */
	returnFloat[0] = floatToConvert[3];
	returnFloat[1] = floatToConvert[2];
	returnFloat[2] = floatToConvert[1];
	returnFloat[3] = floatToConvert[0];
	return retuVal;
	#else
	return float_val; //nothing to be done for Little Endian System
	#endif
}

/* Converts milli Volt into Volt*/
float convert_mv_to_v(uint16_t float_val)
{
	return (float_val * (1.0/1000));
}

////////////////////////////////////////////////////////////////////////////////////////
/*
uint8_t Digit[10][8] =
{
{ 1,1,1,1,1,1,0,0 },    // 0
{ 0,1,1,0,0,0,0,0 },    // 1
{ 1,1,0,1,1,0,1,0 },    // 2
{ 1,1,1,1,0,0,1,0 },    // 3
{ 0,0,1,0,0,1,1,0 },    // 4
{ 1,0,1,1,0,1,1,0 },    // 5
{ 1,0,1,1,1,1,1,0 },    // 6
{ 1,1,1,0,0,0,0,0 },    // 7
{ 1,1,1,1,1,1,1,0 },    // 8
{ 1,1,1,0,0,1,1,0 }     // 9
};*/
	////////////////////////////segunda forma //////////////////////////////////////////////
	/*int contadorDeEnvio=longitudListaNS;
	int indice2=0;
	if (indice2<longitudListaNS)
	{
		if (indice2<longitudListaNS-1){
			
			uint8_t TRAMA[9]={
				(uint8_t)(ID_S >> 8), (uint8_t)ID_S, 
				(uint8_t)(NB >> 8), (uint8_t)NB, 
				(uint8_t)(listaNS[indice2].NS >> 8), (uint8_t)listaNS[indice2].NS, 
				(uint8_t)(listaNS[indice2].ID_NS >> 8), (uint8_t)listaNS[indice2].ID_NS,
				'\n'};
			transmit_sample_frame(TRAMA, 9);
		}
		else if(indice2==longitudListaNS-1){
			
			uint8_t TRAMA[10]={
				(uint8_t)(ID_S >> 8), (uint8_t)ID_S,
				(uint8_t)(NB >> 8), (uint8_t)NB,
				(uint8_t)(listaNS[indice2].NS >> 8), (uint8_t)listaNS[indice2].NS,
				(uint8_t)(listaNS[indice2].ID_NS >> 8), (uint8_t)listaNS[indice2].ID_NS,
				'\n',0xFE};
			transmit_sample_frame(TRAMA, 10);
		}
		delay_ms(200); //Retardo para que funcione el pulsador
		indice2++;		
	}*/
	//////////////////////////////////////////////////////////////////////////
	////////////////////pruebas///////////////////////////////////////////////
				/////////////////////////////////////////////////////////////////
				//LED_Toggle(LED0);
				//DST_ADDR_=0xFFFF;/* siguiente salto */
				//trama_recibida.CargaUtil_802_15_4[5]=(uint8_t)(trama_recibida.d_orgn >> 8);
				//trama_recibida.CargaUtil_802_15_4[6]=(uint8_t)trama_recibida.d_orgn;
				//trama_recibida.CargaUtil_802_15_4[7]=coincedencia_ID_NS;
				//trama_recibida.CargaUtil_802_15_4[8]=longitudListaNS_1;
				//trama_recibida.CargaUtil_802_15_4[9]=(uint8_t)(listaNS[0].ID_DIR_NS>> 8);
				//trama_recibida.CargaUtil_802_15_4[10]=(uint8_t)listaNS[0].ID_DIR_NS;
				//
				//trama_recibida.CargaUtil_802_15_4[11]=(uint8_t)(listaNS[1].ID_DIR_NS>> 8);
				//trama_recibida.CargaUtil_802_15_4[12]=(uint8_t)listaNS[1].ID_DIR_NS;
//
				//trama_recibida.CargaUtil_802_15_4[13]=(uint8_t)(listaNS[2].ID_DIR_NS>> 8);
				//trama_recibida.CargaUtil_802_15_4[14]=(uint8_t)listaNS[2].ID_DIR_NS;
				//transmit_sample_frame(trama_recibida.CargaUtil_802_15_4, 15);
				//delay_ms(0);
				//////////////////////////////////////////////////////////////////