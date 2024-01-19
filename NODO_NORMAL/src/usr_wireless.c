#include "usr_wireless.h"
#include "wireless_config.h"
#include "math.h"
#include "periodic_timer.h"
///////////////  Variables used in uplink.  //////////////////////////////////////
uint16_t ID_S;// Stores the source address of the device.  
uint8_t NB[4];// Stores the battery level of the device.
strc_NS listNS[7];// List with signal levels of nearby nodes.
int DatosNSIniciales=0; //Allows to start or restart the list with signal level values.
int cargarDir=0;//Helps to load the destination address to be used by the node for the initial default route.
uint16_t ID_DEFAULT; // Stores default identifier.
uint16_t SRC_ADDR_;// Variable to modify the source address from the main code.
int iniTx=0;// Helps to control the sending of an uplink frame.
// Variables used to add new values of received energy levels. 
int lengListNS=0;
int cont_NS=0; 
int longitudTramaRX=0; 
////////////////  Variables used in downlink.  ///////////////////////////////////
uint8_t IDS_FRAMES[20]; // List of frame identifiers sent by the PC.
int longitud_IDTramas=0;// Helps to add new frame identifiers sent by the PC.
int ctrReTX=0; // Helps to control the sending of an downlink frame.
tiempoDuracionTimer=0;// Variable to assign the duration of the timer used to send broadcast.
////////////////  Additional control /////////////////////////////////////////////
TimerFinaliza=0;//
int ctrtem=0;
////////////////  Floating Number Conversion Functions  //////////////////////////
void reverse(char* str, int len);
int intToStr(int x, char str[], int d);
void ftoa(float n, char* res, int afterpoint);

void usr_wireless_app_task(void)
{
	////////////////  Load initial source address. //////////////////////////
	SRC_ADDR_=0x0001;
	SRC_ADDR_ = CCPU_ENDIAN_TO_LE16(SRC_ADDR_);
	tal_pib_set(macShortAddress, (pib_value_t *)&SRC_ADDR_);
	ID_S=SRC_ADDR_; //
	////////////////  Load initial destination address. //////////////////////////
	if (cargarDir==0)
	{
		DST_ADDR_=0x0002;
		ID_DEFAULT=DST_ADDR_;
		cargarDir=1;
	}
	////////////////  Initialize storage of signal levels. //////////////////////////
	if (DatosNSIniciales==0)
	{
		lengListNS=1;	
	}
	////////////////  Receive signal from node pushbutton.  //////////////////////////
	if (!ioport_get_pin_level(GPIO_PUSH_BUTTON_0))
	{
		delay_ms(100);
		////////////////// Get battery level. ///////////////////////////////
		char charBateria[20]; 
		float floatBateria=get_bat_sensor_data();
		ftoa(floatBateria, charBateria, 4);
		NB[0]=(uint8_t)charBateria[0];
		NB[1]=(uint8_t)charBateria[1];
		NB[2]=(uint8_t)charBateria[2];
		NB[3]=(uint8_t)charBateria[3];	
		////////////////// Initialize next conditional. ///////////////////////////////
		LED_Toggle(LED2);
		lengListNS=cont_NS;
		iniTx=1;
	}
	///////////////////// Build uplink frame to be sent.  /////////////////////
	else if (iniTx==1)
	{ 	
		iniTx=0;
		int indice=0;
		uint8_t TRAMA[50];// Uplink  frame.
		//-------SOURCE IDENTIFIER (ID_S)---------------------------------------//
		TRAMA[0]=(uint8_t)(ID_S >> 8);
		TRAMA[1]=(uint8_t)ID_S;
		//-------BATTERY LEVEL (NB)-------------------------------------------//	
		TRAMA[2]=NB[0];
		TRAMA[3]=NB[1];
		TRAMA[4]=NB[2];		
		TRAMA[5]=NB[3];	
			
		for (int i=0; i<lengListNS; i++)
		{	
		//------- SIGNAL LEVEL (NS)--------------------------------------------//
			TRAMA[indice+6]=(uint8_t)(listNS[i].NS >> 8);
			TRAMA[indice+7]=(uint8_t)listNS[i].NS;
		//-------SIGNAL LEVEL IDENTIFIER (ID_NS)--------------------------//
			TRAMA[indice+8]=(uint8_t)(listNS[i].ID_DIR_NS >> 8);
			TRAMA[indice+9]=(uint8_t)listNS[i].ID_DIR_NS;
			
			indice=indice+4;
		}
		//////////////////  Transmission of the uplink frame. ////////////////////////////
		TRAMA[(lengListNS*4)+6]=0xFE;
		DST_ADDR_=ID_DEFAULT;
		transmit_sample_frame(TRAMA, (lengListNS*4)+7);	
	}
	//////////////////  Re-transmission of the uplink frame. ////////////////////////////
	if (ctrReTX==1)
	{
		DST_ADDR_=ID_DEFAULT;		
		transmit_sample_frame(trama_recibida.CargaUtil_802_15_4, longitudTramaRX);
		ctrReTX=0;
	} 
	//////////////////  Timer configuration. ////////////////////////////////////////////////////////////
	if (ctrReTX==2)
	{
		int tiempo=(int)SRC_ADDR_;
		tiempo=tiempo*1;
		tiempoDuracionTimer=tiempo;
		start_timer1();
		ctrtem=1;	
		ctrReTX=0;
	}
	//////////////////  Dowlink frame retransmission (Broadcast). (Broadcast) ///////////////////////////
	if (TimerFinaliza==1 && ctrtem==1)
	{
		TimerFinaliza=0;
		ctrtem=0;
		stop_timer1();
		DST_ADDR_=0xFFFF;
		transmit_sample_frame(trama_recibida.CargaUtil_802_15_4, 5);
	}
	delay_ms(10);
}

void usr_frame_received_cb(frame_info_t *frame)
{		
		//////////////////  Frame reception.  ////////////////////////////////////////////////////////////
		memset(&trama_recibida,0,sizeof(trama_recibida));
		memcpy(&trama_recibida,frame->mpdu,sizeof(trama_recibida));
		//////////////////  Condidicon to identify an uplink frame. /////////////////////////////////////	
		if (trama_recibida.d_dstn==SRC_ADDR_)
		{
			longitudTramaRX=0;  
			for (int l = 0; l < 50; l++)
			{
				uint8_t ultimoCH=trama_recibida.CargaUtil_802_15_4[l];
				if (ultimoCH==0xFE)
				{
					longitudTramaRX=l+1;
				}
			}
			ctrReTX=1;
		}
		//////////////////  Condidiconical to identify a dowlink frame.  ///////////////////////////////
		else if(trama_recibida.d_dstn==0xFFFF) 	
		{	
			////////////////////////// Energy Level Storage. ///////////////////////////////////////
			int coincedencia_ID_NS=0;
			int longitudListaNS_1=7;
			LED_Toggle(LED0);
			///////////// Verify that the Energy Level has not been previously stored. ///////////// 
			for (int i=0; i<longitudListaNS_1; i++)
			{
				if (listNS[i].ID_DIR_NS==trama_recibida.d_orgn)
				{
					coincedencia_ID_NS=1;
				}
			}
			/////////////// Energy level extraction and aggregation. ///////////////////////////////////
			if (coincedencia_ID_NS==0)
			{
				uint8_t *payload_ptr=frame->mpdu;
				uint8_t mpdu_len =payload_ptr[0]+2;
				uint8_t potencia_trama=payload_ptr[mpdu_len];
				LED_Toggle(LED2);
				listNS[cont_NS].NS=potencia_trama;
				listNS[cont_NS].ID_DIR_NS=trama_recibida.d_orgn;
				cont_NS++;
				DatosNSIniciales=1;			
			}
			//////////////////  Extraction of the final destination identifier.  ///////////////////////////////
			uint8_t DF1=trama_recibida.CargaUtil_802_15_4[0];
			uint8_t DF2=trama_recibida.CargaUtil_802_15_4[1];
			uint16_t ID_DF;
			ID_DF=DF1*0x100;
			ID_DF=ID_DF+DF2; 
			/////////////////// Configuration of new default identifier. /////////////////////////////
			if (ID_S==ID_DF) 
			{				
				uint8_t ID_DEFAULT_Rx1 = trama_recibida.CargaUtil_802_15_4[2];
				uint8_t ID_DEFAULT_Rx2 = trama_recibida.CargaUtil_802_15_4[3];	
				uint16_t ID_DEFAULT_NUEVO;		
				ID_DEFAULT_NUEVO=ID_DEFAULT_Rx1*0x100;
				ID_DEFAULT_NUEVO=ID_DEFAULT_NUEVO+ID_DEFAULT_Rx2;
				ID_DEFAULT=ID_DEFAULT_NUEVO;
			}
			///////////////////// Preventing loops by broadcast transmission.  /////////////////////////////	
			int coincedencia_IDTrama=0;
			uint8_t idTramaRx=trama_recibida.CargaUtil_802_15_4[4];
			for(int i=0; i<=longitud_IDTramas; i++)
			{
				if (IDS_FRAMES[i]==idTramaRx)
				{
					coincedencia_IDTrama=1;
				}					
			}
			if (coincedencia_IDTrama==0)
			{
				IDS_FRAMES[longitud_IDTramas]=idTramaRx;
				ctrReTX=2;
				longitud_IDTramas++;
			}				
		}
		///////////////////////  Remove data from the receive buffer. /////////////////////////////////////
		delay_ms(0);
		bmm_buffer_free(frame->buffer_header);
}

void usr_frame_transmitted_cb(retval_t status, frame_info_t *frame)
{

}
// Combine a number into a string of chars.
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
	while (i < d)
	str[i++] = '0';
	
	reverse(str, i);
	str[i] = '\0';
	return i;
}
	// convert integer part to string
void ftoa(float n, char* res, int afterpoint)
{
	int ipart = (int)n;
	float fpart = n - (float)ipart;
	int i = intToStr(ipart, res, 0);
	if (afterpoint != 0) {
		res[i] = '.'; // add dot
		fpart = fpart * pow(10, afterpoint);
		
		intToStr((int)fpart, res + i + 1, afterpoint);
	}
}
///////////////// Functions that get the battery level of the node. ///////////////////////////////////////////////////////////////////7
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

// Converts milli Volt into Volt
float convert_mv_to_v(uint16_t float_val)
{
	return (float_val * (1.0/1000));
}

