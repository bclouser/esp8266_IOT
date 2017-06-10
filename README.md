What is
-------
A framework for esp8266 projects
I wanted all my esp8266 projects in one place so I could leverage common libraries and a common build system.

Get the Code
------------
esp_mqtt (https://github.com/tuanpmt/esp_mqtt) is a submodule in '''common_modules/mqtt'''

Third party modules should be submoduled into '''common_modules''' and subsequent build target added to '''common_modules''' makefile

Build
-----
In order to build you will need to create '''user/secrets.h'''
It should look like this. 
```c
#ifndef SECRETS
#define SECRETS

#define WIFI_SSID "ssid"
#define WIFI_PASSWD "secret"

#endif
```

There is a "buildForMac.sh" script which should work on linux as well but won't require a case sensitve mounted image (thanks apple). Modify the variables in that script appropriately to your environment and it should build.
