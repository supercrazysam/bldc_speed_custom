#include "ch.h" // ChibiOS
#include "hal.h" // ChibiOS HAL
#include "mc_interface.h" // Motor control functions
#include "hw.h" // Pin mapping on this hardware
#include "timeout.h" // To reset the timeout
static volatile bool stop_now = true;
static volatile bool is_running = false;
// Example thread
static THD_FUNCTION(example_thread, arg);
static THD_WORKING_AREA(example_thread_wa, 2048); // 2kb stack for this thread

/* 
void app_example_init(void) {

	// Start the example thread
	chThdCreateStatic(example_thread_wa, sizeof(example_thread_wa),
		NORMALPRIO, example_thread, NULL);
}*/

void app_custom_start(void) {
	stop_now = false;

        palSetPadMode(HW_UART_TX_PORT, HW_UART_TX_PIN, PAL_MODE_INPUT_PULLDOWN);   //SET TX as input
        palSetPadMode(HW_UART_RX_PORT, HW_UART_RX_PIN, PAL_MODE_INPUT_PULLDOWN);   //SET RX as input

	chThdCreateStatic(example_thread_wa, sizeof(example_thread_wa), NORMALPRIO, example_thread, NULL);
}

void app_custom_stop(void) {
	stop_now = true;
	while (is_running) {
		chThdSleepMilliseconds(1);
	}
}

 
static THD_FUNCTION(example_thread, arg) 
{
	(void)arg;
 
	chRegSetThreadName("APP_EXAMPLE");
        is_running = true;
	for(;;) {

                if (stop_now) 
                {
			is_running = false;
			return;
                }
		
			//mc_interface_set_pid_speed(pot * 10000.0);
			//mc_interface_release_motor();
		if (palReadPad(HW_UART_TX_PORT, HW_UART_TX_PIN)) 
                {
			mc_interface_set_pid_speed(600.0);
		} 
                else if (palReadPad(HW_UART_RX_PORT, HW_UART_RX_PIN)) 
                {
                        mc_interface_set_pid_speed(-600.0);
		}
                else
                {
                        mc_interface_release_motor();
                }

             
                chThdSleepMilliseconds(3);
                timeout_reset();
                
                 
		// Reset the timeout
		

                }

}
