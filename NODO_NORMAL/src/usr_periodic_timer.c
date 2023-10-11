
#include "usr_periodic_timer.h"
#include "periodic_timer.h"//Para acceder a las varibles utilizadas por los temporizadores.
/**
 * \brief Callback function when the timer expires,Perform tasks required to be done after particular timeouts,
* the Timer can be started again in this function to have timeouts periodically
*/
void usr_app_timer_cb(void *parameter)
{
	TimerFinaliza=1;
}