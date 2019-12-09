/*
 * Copyright 2019 Wilfried Chauveau
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <mbed.h>
#include <DHT.h>

#ifdef MBED_CONF_APP_HTS_ADDR
#include <HTS.h>
#endif

int main() {
#ifdef MBED_CONF_APP_HTS_ADDR
    I2C i2c(PB_11, PB_10);
    i2c.frequency(400000);

    HTS221<MBED_CONF_APP_HTS_ADDR> hts(i2c);
#endif

    DHT<MBED_CONF_APP_DHT_TYPE> dht(MBED_CONF_APP_DHT_PIN);

    while(1) {
#ifdef MBED_CONF_APP_HTS_ADDR
        float temperature = 0., humidity = 0.;
        hts.read(temperature, humidity);
#endif
        float dht_temp = 0., dht_hum = 0.;
        int res = dht.read(dht_temp, dht_hum);
        
#ifdef MBED_CONF_APP_HTS_ADDR
        printf("HTS: %.2fc %.2f%%\tDHT: %d %.2fc %.2f%%\n", temperature, humidity, res, dht_temp, dht_hum);
#else
        printf("DHT: %d %.2fc %.2f%%\n", res, dht_temp, dht_hum);
#endif
        ThisThread::sleep_for(2000);
    }
}
