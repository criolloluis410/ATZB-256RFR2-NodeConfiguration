
#ifndef USR_WIRELESS_H_
#define USR_WIRELESS_H_

#include "wireless_api.h"
#include "tfa.h"

#define max_dato 101
typedef struct
{
	uint8_t longitud;						// Longitud de Trama
	uint16_t fcf;							// fcf
	uint8_t num_sec;						// Numero de secuencia
	uint16_t d_PAN;							// Direccion PAN
	uint16_t d_dstn;						// Direccion ORIGEN
	uint16_t d_orgn;    					// Direccion DESTINO
	uint8_t CargaUtil_802_15_4[max_dato];           // Payload
	uint16_t fcs;                           // frame control secquence
} trama_ieee_802_15_42;

trama_ieee_802_15_42 trama_recibida;
//////////////////////////////////////////////////////////////////////////
struct Strc_NS
{
	uint16_t NS;	// Nivel de se�al
	uint16_t ID_DIR_NS;	// Direccion origen de la se�al
};
typedef struct Strc_NS strc_NS;


/**
* \brief This function needs to be edited by the user for adding application tasks
*/
void usr_wireless_app_task(void);

/**
* \brief This function needs to be edited by the user for adding  tasks when a frame is received
* \param frame pointer to the received frame
*/
void usr_frame_received_cb(frame_info_t *frame);

/**
* \brief This function needs to be edited by the user for adding  tasks when a frame is transmitted
* \param status  Status of frame transmission i.e MAC_SUCCESS,MAC_NO_ACK,CHANNEL_ACCESS_FAILURE etc
* \param frame pointer to the transmitted frame
*/
void usr_frame_transmitted_cb(retval_t status, frame_info_t *frame);

///////////////////////////////////////////////////////////////////////

float get_bat_sensor_data(void);

/*Reverses a float variable*/
float reverse_float( const float float_val);

/* Converts milli Volt into Volt*/
float convert_mv_to_v(uint16_t float_val);


#endif /* USER_WIRELESS_H_ */