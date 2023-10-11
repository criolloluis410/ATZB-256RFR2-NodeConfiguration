
uint16_t DST_ADDR_;// Variable para almacenar el identificador para alcanzar al siguiente nodo.

#ifndef WL_WIZARD_CONFIG_H_
#define WL_WIZARD_CONFIG_H_


//Transeiver Configuration
#define TRANSMITTER_ENABLED

#define DEFAULT_PAN_ID          0xCAFE

#define SRC_ADDR				0x0000
#define SRC_PAN_ID		        0xCAFE

#define CHANNEL_TRANSMIT_RECEIVE		12
#define CHANNEL_PAGE_TRANSMIT_RECEIVE	0

#define DST_ADDR		DST_ADDR_
#define DST_PAN_ID      0xCAFE
#define ACK_REQ			0
#define FRAME_RETRY		0
#define CSMA_MODE		CSMA_UNSLOTTED

#define ENABLE_ANTENNA_1                          (1)
#define ENABLE_ANTENNA_2                          (2)

#define ANT_SELECT					ENABLE_ANTENNA_2
#define ENABLE_ANTENNA_DIVERSITY	false

#define FRAME_OVERHEAD          (11)


#endif /* WL_WIZARD_CONFIG_H_ */