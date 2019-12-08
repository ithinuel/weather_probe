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

#ifndef __HTS_H__
#define __HTS_H__

#include "mbed.h"

template <int addr>
class HTS221 {
    I2C &_i2c;

    int16_t _h0_out;
    float _hcoef;
    float _h0_rH;

    int16_t _t0_out;
    float _tcoef;
    float _t0_degc;

    void write_byte(uint8_t reg, uint8_t value) {
        char buf[] { reg, value };
        _i2c.write(addr, buf, 2);
    }
    uint8_t read_byte(uint8_t reg) {
        char buf[1];
        _i2c.write(addr, (char*)&reg, 1);
        _i2c.read(addr, buf, 1);
        return (uint8_t)buf[0];
    }
    void read_array(uint8_t reg, uint8_t *buf, uint32_t len) {
        char cmd = 0x80 | reg;
        _i2c.write(addr, &cmd, 1); // Set sub register & auto increment.
        _i2c.read(addr, (char *)buf, len);  // read the single-byte who am I register
    }

    public:
        HTS221(I2C &i2c): _i2c(i2c) {
            // who am I
            //printf("ID: 0x%02X\n", read_byte(0xF));

            // set up average configuration
            write_byte(0x10, 0x3F);

            // read 16bytes for calib
            uint8_t buf[16];
            read_array(0x30, buf, 16);

            _h0_rH = buf[0] / 2.;
            _h0_out = (buf[7] << 8) | buf[6];
            float h1_rH = buf[1] / 2.;
            int16_t h1_out = (buf[0xB] << 8) | buf[0xA];
            _hcoef = (h1_rH - _h0_rH) / ((float)(h1_out - _h0_out));

            _t0_degc = (((buf[5] & 0x3) << 8) | buf[2]) / 8.;
            _t0_out = (buf[0xD] << 8) | buf[0xC];
            float t1_degc = (((buf[5] & 0xC) << 6) | buf[3]) / 8.;
            int16_t t1_out = (buf[0xF] << 8) | buf[0xE];
            _tcoef = (t1_degc - _t0_degc) / ((float)(t1_out - _t0_out));

            //printf("Calib: h0 %.2f=%d h1 %.2f=%d: %f\n", _h0_rH, _h0_out, h1_rH, h1_out, _hcoef);
            //printf("Calib: t0 %.2f=%d t1 %.2f=%d: %f\n", _t0_degc, _t0_out, t1_degc, t1_out, _tcoef);

            write_byte(0x21, 0x00);
            write_byte(0x20, 0x85);
        }

        void read(float &temperature, float &humidity) {
            uint8_t buf[4];

            read_array(0x28, buf, 4);

            humidity = ((int16_t)((buf[1] << 8) | buf[0]) - _h0_out) * _hcoef + _h0_rH;
            temperature = ((int16_t)((buf[3] << 8) | buf[2]) - _t0_out) * _tcoef + _t0_degc;
        }

        ~HTS221() {
            write_byte(0x20, 0x00);
        }
};

#endif /* __HTS_H__ */
