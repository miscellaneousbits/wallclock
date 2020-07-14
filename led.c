
#include <bcm2835.h>

#define RED_LED    RPI_V2_GPIO_P1_36
#define GREEN_LED  RPI_V2_GPIO_P1_38
#define BLUE_LED   RPI_V2_GPIO_P1_40
#define COMMON_LED RPI_V2_GPIO_P1_35

int led_init(void)
{
    if (!bcm2835_init())
        return -1;
    bcm2835_gpio_fsel(RED_LED, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(GREEN_LED, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(BLUE_LED, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(COMMON_LED, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(COMMON_LED, HIGH);
    bcm2835_gpio_write(RED_LED, HIGH);
    bcm2835_gpio_write(GREEN_LED, HIGH);
    bcm2835_gpio_write(BLUE_LED, HIGH);
    return 0;
}

void led_close(void)
{
    bcm2835_close();
}

void led_red(void)
{
    bcm2835_gpio_write(RED_LED, LOW);
    bcm2835_gpio_write(GREEN_LED, HIGH);
    bcm2835_gpio_write(BLUE_LED, HIGH);
}

void led_green(void)
{
    bcm2835_gpio_write(RED_LED, HIGH);
    bcm2835_gpio_write(GREEN_LED, LOW);
    bcm2835_gpio_write(BLUE_LED, HIGH);
}

void led_blue(void)
{
    bcm2835_gpio_write(RED_LED, HIGH);
    bcm2835_gpio_write(GREEN_LED, HIGH);
    bcm2835_gpio_write(BLUE_LED, LOW);
}

