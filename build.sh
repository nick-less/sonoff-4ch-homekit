#!/bin/sh
docker run -v "$(pwd)"/src/:/home/sdkbuilder/esp-homekit/src  -it cross/esp8266
