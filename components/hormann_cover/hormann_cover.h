#pragma once

#include "esphome/core/component.h"
#include "esphome/components/cover/cover.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/switch/switch.h"

namespace esphome {
namespace hormann {

enum CoverTargetOperation : uint8_t {
  // order matters to match CoverOperation enum

  // stop door
  TARGET_OPERATION_IDLE = 0,
  // open door
  TARGET_OPERATION_OPEN,
  // close door
  TARGET_OPERATION_CLOSE,
  // do nothing (no action)
  TARGET_OPERATION_NONE,
};

class HormannCover : public cover::Cover, public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override;

  Trigger<> *get_actuate_trigger() const { return this->actuate_trigger_; }

  void set_open_endstop(binary_sensor::BinarySensor *open_endstop) { this->open_endstop_ = open_endstop; }
  void set_close_endstop(binary_sensor::BinarySensor *close_endstop) { this->close_endstop_ = close_endstop; }

  void set_open_duration(uint32_t open_duration) { this->open_duration_ = open_duration; }
  void set_close_duration(uint32_t close_duration) { this->close_duration_ = close_duration; }
  void set_max_duration(uint32_t max_duration) { this->max_duration_ = max_duration; }

  void open_endstop_reached();
  void open_endstop_released();

  void close_endstop_reached();
  void close_endstop_released();

  cover::CoverTraits get_traits() override;

 protected:
  void control(const cover::CoverCall &call) override;

  bool is_open_() const { return this->open_endstop_->state; }
  bool is_closed_() const { return this->close_endstop_->state; }

  CoverTargetOperation target_operation{TARGET_OPERATION_NONE};  // received action to execute

  // void start_direction_(cover::CoverOperation dir);
  void recompute_position();
  // void check_endstops_(uint32_t);
  void do_one_action();

  binary_sensor::BinarySensor *open_endstop_;   // binary sensor to detect when door is full open
  binary_sensor::BinarySensor *close_endstop_;  // binary sensor to detect when door is full closed

  Trigger<> *actuate_trigger_{new Trigger<>()};  // switch that activates the door

  uint32_t open_duration_;   // time the door needs to fully open
  uint32_t close_duration_;  // time the door needs to fully close

  uint32_t max_duration_{UINT32_MAX};

  uint32_t last_recompute_time_{0};
  uint32_t start_dir_time_{0};
  uint32_t last_publish_time_{0};
  float target_position_{0};
  uint32_t switch_activation_interval{0};
  uint32_t last_activation_{0};

  cover::CoverOperation last_dir;  // last door direction (open/close)
};

}  // namespace hormann
}  // namespace esphome