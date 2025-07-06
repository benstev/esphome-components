#pragma once

#include "esphome/core/component.h"
#include <SomfyRemote.h>

#define EMITTER_GPIO 5  // D1
#define EEPROM_RECORD_SIZE 2
#define CC1101_FREQUENCY 433.42

namespace esphome {
namespace cc1101 {

class CC1101 : public Component {
 public:
  void setup() override;
  // void loop() override;
  void dump_config() override;

  void xmit();
  void idle();
  float get_setup_priority() const override { return setup_priority::HARDWARE; }
};

}  // namespace cc1101
}  // namespace esphome