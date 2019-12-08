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

#ifndef MBED_DHT_H
#define MBED_DHT_H

#include "mbed.h"

// 1 sync + 40 bits + 1 end mark
#define DHT_TIME_SAMPLE_COUNT 42

enum eType {
    DHT11     = 11,
    SEN11301P = 11,
    RHT01     = 11,
    DHT22     = 22,
    AM2302    = 22,
    SEN51035P = 22,
    RHT02     = 22,
    RHT03     = 22
};

enum eError {
    ERROR_NONE = 0,
    ERROR_BUS_BUSY =1,
    ERROR_SYNC_TIMEOUT = 4,
    ERROR_DATA_TIMEOUT =5 ,
    ERROR_CHECKSUM = 6,
} ;

template<eType type>
class DHT {

public:

    DHT(PinName pin): _pin(pin) {}
    eError read(float &temperature, float &humidity) {
        uint32_t bitTimes[DHT_TIME_SAMPLE_COUNT] = {0};
        uint32_t i = 0;
        uint32_t start = us_ticker_read();
        DigitalInOut io(_pin);
        io.output();
        io = 1;

        uint32_t end = start + 250;
        do {
            if (us_ticker_read() > 250) {
                return ERROR_BUS_BUSY;
            }
        } while (!io);

        io = 0;
        ThisThread::sleep_for(2);

        core_util_critical_section_enter();
        io = 1;
        io.input();
        bitTimes[0] = us_ticker_read();
        // wait for up to N ms after last bit time.
        bool prev = true;
        while (((us_ticker_read() - bitTimes[(i==0)?0:(i - 1)]) < (20*1000)) && (i < DHT_TIME_SAMPLE_COUNT)) {
            bool _new = io == 1;
            if (_new != prev) {
                if (!_new) {
                    bitTimes[i] = us_ticker_read();
                    i++;
                }
                prev = _new;
            }
        }
        core_util_critical_section_exit();

        if (i == 0) {
            return ERROR_SYNC_TIMEOUT;
        } else if (i != DHT_TIME_SAMPLE_COUNT) {
            return ERROR_DATA_TIMEOUT;
        }

        uint8_t data[5];
        {
            uint8_t *ptr = data;
            for (uint32_t i = 2; i < DHT_TIME_SAMPLE_COUNT; ) {
                for (uint32_t j = 0; j < 8; j++, i++) {
                    *ptr <<= 1;
                    int32_t diff = bitTimes[i] - bitTimes[i-1];
                    //printf("%u %d\n", i, diff);
                    if (diff >= 100) {
                        *ptr |= 1;
                    }
                }
                ptr++;
            }
        }

        //printf("%02hhX%02hhX%02hhX%02hhX%02hhX\n", data[0], data[1], data[2], data[3], data[4]);
        if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
            return ERROR_CHECKSUM;
        }

        temperature = calc_temp(data);
        humidity = calc_hum(data);
        return ERROR_NONE;
    }
    ~DHT();

private:
    PinName _pin;
    float calc_hum(uint8_t data[5]) {
        switch (type) {
            case DHT11:
                return data[0];
            case DHT22:
                return (((data[0]) << 8) | data[1]) / 256.;
        }
        return NAN;
    }
    float calc_temp(uint8_t data[5]) {
        switch (type) {
            case DHT11:
                return data[2];
            case DHT22:
                return (((data[2]) << 8) | data[3]) / 256.;
        }
        return NAN;
    }
};

#endif
