
#include <bcm2835.h>

#define RED_LED    RPI_V2_GPIO_P1_36
#define GREEN_LED  RPI_V2_GPIO_P1_38
#define BLUE_LED   RPI_V2_GPIO_P1_40
#define COMMON_LED RPI_V2_GPIO_P1_35
#define PWM_CHAN   1

int led_init(void)
{
    if (!bcm2835_init())
        return -1;
    bcm2835_gpio_fsel(RED_LED, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(GREEN_LED, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(BLUE_LED, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(RED_LED, HIGH);
    bcm2835_gpio_write(GREEN_LED, HIGH);
    bcm2835_gpio_write(BLUE_LED, HIGH);
    bcm2835_gpio_fsel(COMMON_LED, BCM2835_GPIO_FSEL_ALT5);
    bcm2835_pwm_set_clock(BCM2835_PWM_CLOCK_DIVIDER_1);
    bcm2835_pwm_set_mode(PWM_CHAN, 1, 1);
    bcm2835_pwm_set_range(PWM_CHAN, 8191);
    bcm2835_pwm_set_data(PWM_CHAN, 8191);

    return 0;
}

void led_close(void)
{
    bcm2835_gpio_write(RED_LED, HIGH);
    bcm2835_gpio_write(GREEN_LED, HIGH);
    bcm2835_gpio_write(BLUE_LED, HIGH);
    bcm2835_close();
}

void led_blink(int on)
{
    bcm2835_pwm_set_data(PWM_CHAN, on ? 2048 : 8191);
}

void led_red(int on)
{
    bcm2835_gpio_write(RED_LED, on ? LOW : HIGH);
}

void led_green(int on)
{
    bcm2835_gpio_write(GREEN_LED, on ? LOW : HIGH);
}

void led_blue(int on)
{
    bcm2835_gpio_write(BLUE_LED, on ? LOW : HIGH);
}


