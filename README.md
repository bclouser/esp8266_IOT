
Get the Code
------------
esp_mqtt (https://github.com/tuanpmt/esp_mqtt) is a submodule.
remember to clone with --recursive, or perform submodule init, submodule update, submodule sync

Build
-----
### The SDK
* You can use whichever SDK you want with minimal modifications (ideally)
* I am using opensdk built with 'make STANDALONE=y': https://github.com/pfalcon/esp-open-sdk

### Directories
#### common_modules
 These are things like device drivers(libraries), json libraries, messaging libraries,  and the like. Things that can be used across multiple projects generically.
 
#### project dirs (fan_control, projectorEpson3500, shadeControl, wifiButtons)
These are my actual projects. They are typically structured around some IoT purpose and uses a single ESP8266 chip on some module... my favorite has been NodeMCU.
These projects will link in the libraries in the 'common_modules' as specified in their individual makefile.

In order to build a project you will need to create user/secrets.h.
It should look like this. 
```c
#ifndef SECRETS
#define SECRETS

#define WIFI_SSID "ssid"
#define WIFI_PASSWD "secret"

#endif
```

There is a "build.sh" script which should work on linux as well as mac but won't require a case sensitve mounted image (thanks apple). Modify the variables in that script appropriately to your environment before performing a build.

To Build: Call the build.sh script and pass in the name of the project. 
Additionally you can pass a 2nd paramater which is passed as a target to the project's makefile.

```
/build.sh shadeControl
/build.sh shadeControl clean
```
