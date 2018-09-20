#include "api/syscall.h"
#include "api/types.h"
#include "api/print.h"

enum led_state_t {OFF, ON};

enum led_state_t green_state  = ON;
enum led_state_t orange_state = OFF;
enum led_state_t red_state    = ON;
enum led_state_t blue_state   = OFF;
enum led_state_t display_leds = ON;

device_t    leds, button;
int         desc_leds, desc_button;
uint64_t    last_isr;

void exti_button_handler ()
{
    uint64_t    clock;

    sys_get_systick(&clock, PREC_MILLI);

    /* Debounce time (in ms) */
    if (clock - last_isr > 20) {
        green_state   = (green_state == ON) ? OFF : ON;
        orange_state  = (orange_state == ON) ? OFF : ON;
        red_state     = (red_state == ON) ? OFF : ON;
        blue_state    = (blue_state == ON) ? OFF : ON;
        display_leds  = ON;
    }
    last_isr = clock;
}


int _main(uint32_t my_id)
{
    e_syscall_ret   ret;

    printf("Hello, I'm BLINKY task. My id is %x\n", my_id);

    /* Zeroing the structure to avoid improper values detected
     * by the kernel */
    memset (&leds, 0, sizeof (leds));

    strncpy (leds.name, "LEDs", sizeof (leds.name));


    /*
     * Configuring the GPIOs. Note: the related clocks are automatically set
     * by the kernel
     */

    leds.gpio_num = 4; /* Number of configured GPIO */

    leds.gpios[0].kref.port = GPIO_PD;
    leds.gpios[0].kref.pin = 12;
    leds.gpios[0].mask     = GPIO_MASK_SET_MODE | GPIO_MASK_SET_PUPD |
                             GPIO_MASK_SET_TYPE | GPIO_MASK_SET_SPEED;
    leds.gpios[0].mode     = GPIO_PIN_OUTPUT_MODE;
    leds.gpios[0].pupd     = GPIO_PULLDOWN;
    leds.gpios[0].type     = GPIO_PIN_OTYPER_PP;
    leds.gpios[0].speed    = GPIO_PIN_HIGH_SPEED;

    leds.gpios[1].kref.port = GPIO_PD;
    leds.gpios[1].kref.pin = 13;
    leds.gpios[1].mask     = GPIO_MASK_SET_MODE | GPIO_MASK_SET_PUPD |
                             GPIO_MASK_SET_TYPE | GPIO_MASK_SET_SPEED;
    leds.gpios[1].mode     = GPIO_PIN_OUTPUT_MODE;
    leds.gpios[1].pupd     = GPIO_PULLDOWN;
    leds.gpios[1].type     = GPIO_PIN_OTYPER_PP;
    leds.gpios[1].speed    = GPIO_PIN_HIGH_SPEED;

    leds.gpios[2].kref.port = GPIO_PD;
    leds.gpios[2].kref.pin = 14;
    leds.gpios[2].mask     = GPIO_MASK_SET_MODE | GPIO_MASK_SET_PUPD |
                             GPIO_MASK_SET_TYPE | GPIO_MASK_SET_SPEED;
    leds.gpios[2].mode     = GPIO_PIN_OUTPUT_MODE;
    leds.gpios[2].pupd     = GPIO_PULLDOWN;
    leds.gpios[2].type     = GPIO_PIN_OTYPER_PP;
    leds.gpios[2].speed    = GPIO_PIN_HIGH_SPEED;

    leds.gpios[3].kref.port = GPIO_PD;
    leds.gpios[3].kref.pin = 15;
    leds.gpios[3].mask     = GPIO_MASK_SET_MODE | GPIO_MASK_SET_PUPD |
                             GPIO_MASK_SET_TYPE | GPIO_MASK_SET_SPEED;
    leds.gpios[3].mode     = GPIO_PIN_OUTPUT_MODE;
    leds.gpios[3].pupd     = GPIO_PULLDOWN;
    leds.gpios[3].type     = GPIO_PIN_OTYPER_PP;
    leds.gpios[3].speed    = GPIO_PIN_HIGH_SPEED;

    ret = sys_init(INIT_DEVACCESS, &leds, &desc_leds);

    if (ret) {
        printf ("error: sys_init() %s\n", strerror(ret));
    } else {
        printf ("sys_init() - sucess\n");
    }

    memset (&button, 0, sizeof (button));
    strncpy (button.name, "BUTTON", sizeof (button.name));

    button.gpio_num = 1;
    button.gpios[0].kref.port   = GPIO_PA;
    button.gpios[0].kref.pin    = 0;
    button.gpios[0].mask        = GPIO_MASK_SET_MODE | GPIO_MASK_SET_PUPD |
                                  GPIO_MASK_SET_TYPE | GPIO_MASK_SET_SPEED |
                                  GPIO_MASK_SET_EXTI;
    button.gpios[0].mode        = GPIO_PIN_INPUT_MODE;
    button.gpios[0].pupd        = GPIO_PULLDOWN;
    button.gpios[0].type        = GPIO_PIN_OTYPER_PP;
    button.gpios[0].speed       = GPIO_PIN_LOW_SPEED;
    button.gpios[0].exti_trigger = GPIO_EXTI_TRIGGER_RISE;
    button.gpios[0].exti_handler = (user_handler_t) exti_button_handler;


    ret = sys_init(INIT_DEVACCESS, &button, &desc_button);

    if (ret) {
        printf ("error: sys_init() %s\n", strerror(ret));
    } else {
        printf ("sys_init() - sucess\n");
    }

    /*
     * Devices and ressources registration is finished
     */

    ret = sys_init(INIT_DONE);
    if (ret) {
        printf ("error INIT_DONE: %s\n", strerror(ret));
        return 1;
    }

    printf ("init done.\n");

    /*
     * Main task
     */

#ifdef TEST_MPU
    {
        uint8_t *p = (uint8_t*) 0x40004400;
        printf("%x: %x\n", p, *p);
    }
#endif

    while (1) {
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

        display_leds = (display_leds == ON) ? OFF : ON;

        sys_sleep (500, SLEEP_MODE_INTERRUPTIBLE);
    }

    return 0;
}

