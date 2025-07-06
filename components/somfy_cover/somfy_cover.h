#pragma once

#include "esphome/core/component.h"
#include "esphome/components/cover/cover.h"
// #include "esphome/../config/components/cc1101/cc1101.h"
#include "esphome/components/cc1101/cc1101.h"

#include <SomfyRemote.h>

namespace esphome {
namespace somfy {

#define EMITTER_GPIO 5  // D1
#define EEPROM_RECORD_SIZE 2

class SomfyCover : public cover::Cover, public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const { return setup_priority::DATA; }
  void control(const cover::CoverCall &call) override;

  void set_open_duration(uint32_t open_duration) { this->open_duration_ = open_duration; }
  void set_close_duration(uint32_t close_duration) { this->close_duration_ = close_duration; }
  void set_storage_record(uint16_t record) { this->storage_record_ = record; }
  void set_remote_code(uint32_t remote_code) { this->remote_code_ = remote_code; }

  cover::CoverTraits get_traits() override;

  void set_cc1101(cc1101::CC1101 *cc) { this->cc1101_ = cc; }
  void program();

 protected:
  int storage_record_;
  uint32_t remote_code_;
  float open_duration_;
  float close_duration_;
  SomfyRemote *remote;

  uint32_t last_recompute_time_{0};
  uint32_t last_publish_time_{0};
  float target_position_{0};

  cc1101::CC1101 *cc1101_{nullptr};

  void start_direction_(cover::CoverOperation dir);
  bool is_at_target_();
  void recompute_position_();
  void sendSomfyCommand(Command command);
};

}  // namespace somfy
}  // namespace esphome