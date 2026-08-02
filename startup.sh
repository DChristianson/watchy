#!/bin/bash
cd /opt/watchy
./watchy -f ./tom-thumb.bdf --led-gpio-mapping=adafruit-hat-pwm --led-rows=64 --led-cols=64 -y 12 -x 10
