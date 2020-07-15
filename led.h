#pragma once

#define RED_LED    RPI_V2_GPIO_P1_36
#define GREEN_LED  RPI_V2_GPIO_P1_38
#define BLUE_LED   RPI_V2_GPIO_P1_40
#define COMMON_LED RPI_V2_GPIO_P1_35

int led_init(void);

void led_close(void);

void led_red(int on);

void led_green(int on);

void led_blue(int on);

void led_blink(int on);

