# The Blinky app use case example

This application is configured with the C version of EwoK with:
```
  $ make boards/32f407disco/configs/disco_blinky_defconfig
```

This application are configured with the Ada/Spark version of EwoK with:
```
  $ make boards/32f407disco/configs/disco_blinky_ada_defconfig
```

## Purpose of the example

This is a very basic sample application on top of the EwoK microkernel for the STM32 Discovery F407.

The purpose of this sample is to show how a single application can interact with the GPIOs, in both
output mode (with the LEDs: PD12, PD13, PD14 and PD15) and input mode (with the push Button: PA0).

Two of the four user LEDs of the board are blinking (green and red, or blue and orange). When the user
pushes the user Button ('blue' button on the board), the other two LEDs start blinking and the former
ones stop.

In addition to the device init and GPIOs related syscalls, two time related syscalls are used in this
sample (`sys_get_systick` and `sys_sleep`).

## Sketch of the example

### GPIOs configuration for the LEDs

The main function of the task first initializes the four LEDs (four GPIOs in input mode) inside
the `device_t` structure associated to the LEDs:

```C
    /* Zeroing the structure to avoid improper values detected
     * by the kernel */
    memset (&leds, 0, sizeof (leds));

    strncpy (leds.name, "LEDs", sizeof (leds.name));

    leds.gpio_num = 4; /* Number of configured GPIO */

    /*
     * Configuring the LED GPIOs. Note: the related clocks are automatically set
     * by the kernel.
     * We configure 4 GPIOs here corresponding to the STM32 Discovery F407 LEDs (LD4, LD3, LD5, LD6):
     *     - PD12, PD13, PD14 and PD15 are in output mode
     * See the datasheet of the board here for more information:
     * https://www.st.com/content/ccc/resource/technical/document/user_manual/70/fe/4a/3f/e7/e1/4f/7d/DM00039084.pdf/files/DM00039084.pdf/jcr:content/translations/en.DM00039084.pdf 
     * 
     * NOTE: since we do not need an ISR handler for the LED gpios, we do not configure it (we only need to
     * synchronously set the LEDs)
     */
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
```

The `sys_init` syscall is used to initialize the device:

```C
    ret = sys_init(INIT_DEVACCESS, &leds, &desc_leds);
```

### GPIOs configuration for the Button

Then, the GPIO associated to the button is registered.

```C
    memset (&button, 0, sizeof (button));
    strncpy (button.name, "BUTTON", sizeof (button.name));
    /*
     * Configuring the Button GPIO. Note: the related clocks are automatically set
     * by the kernel.
     * We configure one GPIO here corresponding to the STM32 Discovery F407 'blue' push button (B1):
     *     - PA0 is configured in input mode
     *
     * NOTE: we need to setup an ISR handler (exti_button_handler) to asynchronously capture the button events.
     * We only focus on the button push event, we use the GPIO_EXTI_TRIGGER_RISE configuration
     * of the EXTI trigger.
     */
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
```

It is worth noticing the ISR handler registration with:

```C
button.gpios[0].exti_handler = (user_handler_t) exti_button_handler;
```
The `GPIO_EXTI_TRIGGER_RISE` configures the IRQ associated to the GPIO to be triggered
only on a rising edge (corresponding to a button push in our case).

The `sys_init` syscall is used to initialize the device:

```C
    /* Now that the button device structure is filled, use sys_init to initialize it */
    ret = sys_init(INIT_DEVACCESS, &button, &desc_button);
```

### Leaving the initialization phase

Now that all devices have been setup, it is possible to leave the initialization phase and
move forward to the nominal one using the `sys_init` syscall:


```C
    /* Devices and resources registration is finished */
    ret = sys_init(INIT_DONE);
```

Please be aware that after the `sys_init(INIT_DONE)` milestone, no further device and resource 
registration is possible.

### ISR handler

The ISR handler is quite simple, and its main task is to set the global variable `button_pushed`
to notify the main thread of the event:

```C
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

```

The only subtlety here is the *debouncing* handling inside the ISR. When a button is pushed and because of
the mechanical constraints, there is an IRQ burst before the GPIO state becomes stable (hence the 'bouncing'
qualifier). This is why we wait for 20 milliseconds before really notifying the main thread. We perform this using
an active time measurement with a millisecond precision with the `sys_get_systick` syscall. 

### Main loop in nominal phase

After the initialization phase, the main function executes a loop that waits for notifications from the ISR
through the `button_pushed`. When the Button is pushed, the LEDs couple currently blinking is switched
(from green/red to blue/orange) by modifying a global state variable.

```C
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
        ...
```

Next, the LEDs are finally driven to their current state (ON or OFF) using the `sys_cfg` syscall:

```C
...
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
...
```

The global variable `display_leds` handles the current ON or OFF state of the
LEDs couple (the other couple not blinking being forced at OFF).

In order to effectively blink, a solution would be to perform an active polling
of the current time using `sys_get_systick`. We have chosen to use a more elegant
solution where the task sleeps 500 milliseconds with a possible awakening in
case of an IRQ (a button push) using the `sys_sleep` syscall. This has the advantage
of being less CPU time consuming:

```C
        /* Sleeping for 500 ms */
        sys_sleep (500, SLEEP_MODE_INTERRUPTIBLE);
```
