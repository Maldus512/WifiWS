
extern "C" {
#include "user_interface.h"
}

#include <ESP8266WiFi.h>

os_timer_t myTimer;

bool tickOccured;


// start of timerCallback
void timerCallback(void *pArg) {
      tickOccured = true;
} // End of timerCallback

void user_init(void) {
    tickOccured = false;
      os_timer_setfn(&myTimer, timerCallback, NULL);
      os_timer_arm(&myTimer, 1000, true);
} // End of user_init