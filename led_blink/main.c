#include "pico/stdlib.h"
#include <stdio.h>

int main() {
    stdio_usb_init(); 
    sleep_ms(5000);   
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    printf("NOW BLINK PROGRAM START\n");
    while (true) {
        printf("LED On\n");
        gpio_put(LED_PIN, 1);
        sleep_ms(3000);

        printf("LED Off\n");
        gpio_put(LED_PIN, 0);
        sleep_ms(3000);
    }
    return 0;
}