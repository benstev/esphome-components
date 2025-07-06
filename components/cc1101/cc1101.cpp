#include "esphome/core/log.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "cc1101.h"

#include <ELECHOUSE_CC1101_SRC_DRV.h>

namespace esphome {
namespace cc1101 {

static const char *TAG = "cc1101.component";

void CC1101::setup() {
  ELECHOUSE_cc1101.Init();
  ELECHOUSE_cc1101.setMHZ(CC1101_FREQUENCY);
}

void CC1101::dump_config() { ESP_LOGCONFIG(TAG, "Somfy Remote"); }

void CC1101::xmit() {
  ESP_LOGD(TAG, "enable xmit.");
  ELECHOUSE_cc1101.SetTx();
}

void CC1101::idle() {
  ESP_LOGD(TAG, "disable xmit.");
  ELECHOUSE_cc1101.setSidle();
}
}  // namespace cc1101
}  // namespace esphome