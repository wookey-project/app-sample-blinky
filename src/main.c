#include "libc/syscall.h"
#include "libc/types.h"
#include "libc/stdio.h"
#include "libc/string.h"

#include "generated/led_blue.h"
#include "generated/led_red.h"
#include "generated/led_green.h"
#include "generated/led_orange.h"
#include "generated/button.h"

/*
 * Simple example of a single user task that uses GPIOs.
 * Four GPIOs are used in output mode (the LEDs) and one GPIO in
 * input mode (the push Button). It sets an Interrupt Service Routine (ISR) to
 * handle the button push events.
 *
 * Note:
 *   By default, the debug USART TX pin is on GPIO PB6 (this is set in the
 *   kernel and tranparent to user applications).
 */

typedef enum {OFF = 0, ON = 1} led_state_t;

/* Leds state */
led_state_t green_state  = ON;
led_state_t orange_state = OFF;
led_state_t red_state    = ON;
led_state_t blue_state   = OFF;

/* Blink state */
led_state_t display_leds = ON;

/* This variable handles the button push events. It is shared between the ISR
 * handler code and the main thread, hence the 'volatile' qualifier.  */
volatile bool   button_pushed = false;

device_t    leds, button;
int         desc_leds, desc_button;

/* Last interrupt in milliseconds */
uint64_t    last_isr;

/*
 * User defined ISR to execute when the blue button (gpio PA0) on the STM32
 * discovery board is pressed.
 * Note:  ISRs can use only a restricted set of syscalls. More info on kernel
 *        sources (Ada/ewok-syscalls-handler.adb or syscalls-handler.c)
 *
 * Because of possible 'bouncing' issues when the button is pressed, one must
 * take care of IRQ bursts. Hence the usage of sys_get_systick to wait at least
 * 20 milliseconds before notifying that the button is pushed (this is a very
 * basic way of handling the debouncing, and is only here as an example!).
 */
void exti_button_handler ()
{
    uint64_t        clock;
    e_syscall_ret   ret;

    /* Syscall to get the elapsed cpu time since the board booted */
    ret = sys_get_systick(&clock, PREC_MILLI);

    if (ret == SYS_E_DONE) {
	    /* Debounce time (in ms) */
	    if (clock - last_isr < 20) {
	        last_isr = clock;
	        return;
	    }
    }

    last_isr = clock;
    button_pushed = true;
}


int _main(uint32_t my_id)
{
    e_syscall_ret   ret;

    printf("Hello, I'm BLINKY task. My id is %x\n", my_id);

    /* Zeroing the structure to avoid improper values detected by the kernel */
    memset (&leds, 0, sizeof (leds));

    strncpy (leds.name, "LEDs", sizeof (leds.name));

    /*
     * Configuring the LED GPIOs. Note: the related clocks are automatically set
     * by the kernel.
     * We configure 4 GPIOs here corresponding to the STM32 Discovery F407 LEDs
     * (LD4, LD3, LD5, LD6):
     *     - PD12, PD13, PD14 and PD15 are in output mode
     * See the datasheet of the board here for more information:
     * https://www.st.com/content/ccc/resource/technical/document/user_manual/70/fe/4a/3f/e7/e1/4f/7d/DM00039084.pdf/files/DM00039084.pdf/jcr:content/translations/en.DM00039084.pdf
     *
     * NOTE: since we do not need an ISR handler for the LED gpios, we do not
     * configure it (we only need to synchronously set the LEDs)
     */

    /* Number of configured GPIO */
    leds.gpio_num = 4;

    leds.gpios[0].kref.port = led_blue_dev_infos.gpios[LED_BLUE].port;
    leds.gpios[0].kref.pin = led_blue_dev_infos.gpios[LED_BLUE].pin;
    leds.gpios[0].mask     = GPIO_MASK_SET_MODE | GPIO_MASK_SET_PUPD |
                             GPIO_MASK_SET_TYPE | GPIO_MASK_SET_SPEED;
    leds.gpios[0].mode     = GPIO_PIN_OUTPUT_MODE;
    leds.gpios[0].pupd     = GPIO_PULLDOWN;
    leds.gpios[0].type     = GPIO_PIN_OTYPER_PP;
    leds.gpios[0].speed    = GPIO_PIN_HIGH_SPEED;


    leds.gpios[1].kref.port = led_red_dev_infos.gpios[LED_RED].port;
    leds.gpios[1].kref.pin = led_red_dev_infos.gpios[LED_RED].pin;
    leds.gpios[1].mask     = GPIO_MASK_SET_MODE | GPIO_MASK_SET_PUPD |
                             GPIO_MASK_SET_TYPE | GPIO_MASK_SET_SPEED;
    leds.gpios[1].mode     = GPIO_PIN_OUTPUT_MODE;
    leds.gpios[1].pupd     = GPIO_PULLDOWN;
    leds.gpios[1].type     = GPIO_PIN_OTYPER_PP;
    leds.gpios[1].speed    = GPIO_PIN_HIGH_SPEED;


    leds.gpios[2].kref.port = led_orange_dev_infos.gpios[LED_ORANGE].port;
    leds.gpios[2].kref.pin = led_orange_dev_infos.gpios[LED_ORANGE].pin;
    leds.gpios[2].mask     = GPIO_MASK_SET_MODE | GPIO_MASK_SET_PUPD |
                             GPIO_MASK_SET_TYPE | GPIO_MASK_SET_SPEED;
    leds.gpios[2].mode     = GPIO_PIN_OUTPUT_MODE;
    leds.gpios[2].pupd     = GPIO_PULLDOWN;
    leds.gpios[2].type     = GPIO_PIN_OTYPER_PP;
    leds.gpios[2].speed    = GPIO_PIN_HIGH_SPEED;

    leds.gpios[3].kref.port = led_green_dev_infos.gpios[LED_GREEN].port;
    leds.gpios[3].kref.pin = led_green_dev_infos.gpios[LED_GREEN].pin;
    leds.gpios[3].mask     = GPIO_MASK_SET_MODE | GPIO_MASK_SET_PUPD |
                             GPIO_MASK_SET_TYPE | GPIO_MASK_SET_SPEED;
    leds.gpios[3].mode     = GPIO_PIN_OUTPUT_MODE;
    leds.gpios[3].pupd     = GPIO_PULLDOWN;
    leds.gpios[3].type     = GPIO_PIN_OTYPER_PP;
    leds.gpios[3].speed    = GPIO_PIN_HIGH_SPEED;



    /* Now that the leds device structure is filled, use sys_init to initialize
     * it */
    ret = sys_init(INIT_DEVACCESS, &leds, &desc_leds);

    if (ret) {
        printf ("error: sys_init() %s\n", strerror(ret));
    } else {
        printf ("sys_init() - success\n");
    }

    /*
     * Configuring the Button GPIO. Note: the related clocks are automatically
     * set by the kernel.
     * We configure one GPIO here corresponding to the STM32 Discovery F407
     * 'blue' push button (B1):
     *     - PA0 is configured in input mode
     *
     * NOTE:
     *   We need to setup an ISR handler (exti_button_handler) to
     *   asynchronously capture the button events. We only focus on the button
     *   push event, we use the GPIO_EXTI_TRIGGER_RISE configuration of the
     *   EXTI trigger.
     */

    memset (&button, 0, sizeof (button));
    strncpy (button.name, "BUTTON", sizeof (button.name));

    button.gpio_num = 1;
    button.gpios[0].kref.port   = button_dev_infos.gpios[BUTTON].port;
    button.gpios[0].kref.pin    = button_dev_infos.gpios[BUTTON].pin;
    button.gpios[0].mask        = GPIO_MASK_SET_MODE | GPIO_MASK_SET_PUPD |
                                  GPIO_MASK_SET_TYPE | GPIO_MASK_SET_SPEED |
                                  GPIO_MASK_SET_EXTI;
    button.gpios[0].mode        = GPIO_PIN_INPUT_MODE;
    button.gpios[0].pupd        = GPIO_PULLDOWN;
    button.gpios[0].type        = GPIO_PIN_OTYPER_PP;
    button.gpios[0].speed       = GPIO_PIN_LOW_SPEED;
    button.gpios[0].exti_trigger = GPIO_EXTI_TRIGGER_RISE;
    button.gpios[0].exti_lock    = GPIO_EXTI_UNLOCKED;
    button.gpios[0].exti_handler = (user_handler_t) exti_button_handler;

    /* Now that the button device structure is filled, use sys_init to
     * initialize it */
    ret = sys_init(INIT_DEVACCESS, &button, &desc_button);

    if (ret) {
        printf ("error: sys_init() %s\n", strerror(ret));
    } else {
        printf ("sys_init() - sucess\n");
    }

    /* Devices and resources registration is finished */
    ret = sys_init(INIT_DONE);
    if (ret) {
        printf ("error INIT_DONE: %s\n", strerror(ret));
        return 1;
    }

    printf ("init done.\n");

    /*
     * Main task: the main task is a while loop to toggle two of the four LEDs
     * on the board. When the user pushes the 'blue' button of the board, the
     * other two LEDs begin to toggle.
     */

    while (1) {

        if (button_pushed == true) {
            printf ("button has been pressed\n");

            /* Change leds state */
            green_state   = (green_state == ON) ? OFF : ON;
            orange_state  = (orange_state == ON) ? OFF : ON;
            red_state     = (red_state == ON) ? OFF : ON;
            blue_state    = (blue_state == ON) ? OFF : ON;

            /* Show leds */
            display_leds  = ON;

            button_pushed = false;
        }

        if (display_leds == ON) {
            ret = sys_cfg(CFG_GPIO_SET, (uint8_t) leds.gpios[0].kref.val, green_state);
            if (ret != SYS_E_DONE) {
                printf ("sys_cfg(): failed\n");
                return 1;
            }

            ret = sys_cfg(CFG_GPIO_SET, (uint8_t) leds.gpios[1].kref.val, orange_state);
            if (ret != SYS_E_DONE) {
                printf ("sys_cfg(): failed\n");
                return 1;
            }

            ret = sys_cfg(CFG_GPIO_SET, (uint8_t) leds.gpios[2].kref.val, red_state);
            if (ret != SYS_E_DONE) {
                printf ("sys_cfg(): failed\n");
                return 1;
            }

            ret = sys_cfg(CFG_GPIO_SET, (uint8_t) leds.gpios[3].kref.val, blue_state);
            if (ret != SYS_E_DONE) {
                printf ("sys_cfg(): failed\n");
                return 1;
            }
        } else {
            ret = sys_cfg(CFG_GPIO_SET, (uint8_t) leds.gpios[0].kref.val, 0);
            if (ret != SYS_E_DONE) {
                printf ("sys_cfg(): failed\n");
                return 1;
            }
            ret = sys_cfg(CFG_GPIO_SET, (uint8_t) leds.gpios[1].kref.val, 0);
            if (ret != SYS_E_DONE) {
                printf ("sys_cfg(): failed\n");
                return 1;
            }
            ret = sys_cfg(CFG_GPIO_SET, (uint8_t) leds.gpios[2].kref.val, 0);
            if (ret != SYS_E_DONE) {
                printf ("sys_cfg(): failed\n");
                return 1;
            }
            ret = sys_cfg(CFG_GPIO_SET, (uint8_t) leds.gpios[3].kref.val, 0);
            if (ret != SYS_E_DONE) {
                printf ("sys_cfg(): failed\n");
                return 1;
            }
        }

        /* Make the leds blink */
        display_leds = (display_leds == ON) ? OFF : ON;

        /* Sleeping for 500 ms */
        sys_sleep (500, SLEEP_MODE_INTERRUPTIBLE);
    }

    return 0;
}

