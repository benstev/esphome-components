#include "esphome/core/log.h"
#include "hormann_cover.h"
#include "esphome/core/application.h"

namespace esphome {
namespace hormann {

static const char *TAG = "hormann.cover";

using namespace esphome::cover;

CoverTraits HormannCover::get_traits() {
  auto traits = CoverTraits();
  traits.set_is_assumed_state(false);
  traits.set_supports_position(true);
  traits.set_supports_tilt(false);
  return traits;
}

float HormannCover::get_setup_priority() const { return setup_priority::DATA; }

void HormannCover::dump_config() { ESP_LOGCONFIG(TAG, "hormann.cover"); }

void HormannCover::setup() {
  if (this->is_open_()) {
    this->position = COVER_OPEN;
  } else if (this->is_closed_()) {
    this->position = COVER_CLOSED;
  } else {
    // door neither closed nor open
    // assume door its ad middle position
    this->position = 0.5f;
  }
  this->publish_state(false);
}

// This will be called every time the user requests a state change.
void HormannCover::control(const CoverCall &call) {
  if (call.get_position().has_value()) {
    // get requested position
    float pos = *call.get_position();

    ESP_LOGD("CustomGarageCover", "Position command received: %0.2f", pos);

    if (pos != this->position) {
      // not at target
      // calculate target operation
      this->target_operation = pos < this->position ? TARGET_OPERATION_CLOSE : TARGET_OPERATION_OPEN;
    }
  }
  if (call.get_stop()) {
    ESP_LOGD("CustomGarageCover", "Stop command received.");

    if (this->current_operation != COVER_OPERATION_IDLE) {
      this->target_operation = TARGET_OPERATION_IDLE;
    }
  }

  // ------------

  // if (call.get_position().has_value()) {
  //   auto pos = *call.get_position();

  //   ESP_LOGD(TAG, "Position command received: %0.2f.", pos);

  //   if (pos != this->position) {
  //     // not at target

  //     auto op = pos < this->position ? COVER_OPERATION_CLOSING : COVER_OPERATION_OPENING;
  //     this->start_direction_(op);
  //     this->publish_state(false);
  //   }
  // }

  // if (call.get_stop()) {
  //   ESP_LOGD(TAG, "Stop command received.");
  //   this->start_direction_(COVER_OPERATION_IDLE);
  //   this->publish_state(false);
  // }
}

// This will be called by App.loop()
void HormannCover::loop() {
  const uint32_t now = App.get_loop_component_start_time();

  this->recompute_position();

  // perform one action if target operation different than current operation
  if (this->target_operation != TARGET_OPERATION_NONE &&
      (static_cast<uint8_t>(this->target_operation) != static_cast<uint8_t>(this->current_operation))) {
    // only activate door if time greater than activation interval
    if (now - this->last_activation_ > switch_activation_interval) {
      this->do_one_action();
      this->last_activation_ = now;
      this->last_publish_time_ = now;
    }

  } else if (static_cast<uint8_t>(this->target_operation) == static_cast<uint8_t>(this->current_operation)) {
    // target reached, set target as None.
    ESP_LOGD("CustomGarageCover", "Target operation reached");
    this->target_operation = TARGET_OPERATION_NONE;
  }

  // send current position every second
  if (this->current_operation != COVER_OPERATION_IDLE && (now - this->last_publish_time_) > 1000) {
    this->publish_state(false);
    this->last_publish_time_ = now;
  }

if (now - this->start_dir_time_ > this->max_duration_) {
    ESP_LOGD(TAG, "Max duration reached. Cover motion must have been interrupted manually.");
    this->start_dir_time_ = now;
  }

  //---------

  // const uint32_t now = App.get_loop_component_start_time();

  // this->check_endstops_(now);

  // // recompute position every loop cycle

  // if (this->current_operation == COVER_OPERATION_IDLE)
  //   return;

  // this->recompute_position();

  // // send current position every second
  // if (this->current_operation != COVER_OPERATION_IDLE && (now - this->last_publish_time_) > 1000) {
  //   this->publish_state(false);
  //   this->last_publish_time_ = now;
  // }
}

void HormannCover::open_endstop_reached() {
  float dur = (millis() - this->start_dir_time_) / 1e3f;
  ESP_LOGD(TAG, "Open endstop reached. Took %.1fs.", dur);
  // ESP_LOGD(TAG, "Open endstop reached.");

  this->target_operation = TARGET_OPERATION_NONE;

  this->current_operation = COVER_OPERATION_IDLE;
  this->position = COVER_OPEN;
  this->last_dir = COVER_OPERATION_OPENING;
  this->publish_state(false);
}

void HormannCover::open_endstop_released() {
  ESP_LOGD(TAG, "Open endstop released.");
  this->current_operation = COVER_OPERATION_CLOSING;
  this->last_dir = COVER_OPERATION_CLOSING;
  this->publish_state(false);
}

void HormannCover::close_endstop_reached() {
  float dur = (millis() - this->start_dir_time_) / 1e3f;
  ESP_LOGD(TAG, "Close endstop reached. Took %.1fs.", dur);
  // ESP_LOGD(TAG, "Close endstop reached.");

  this->target_operation = TARGET_OPERATION_NONE;

  this->current_operation = COVER_OPERATION_IDLE;
  this->position = COVER_CLOSED;
  this->last_dir = COVER_OPERATION_CLOSING;
  this->publish_state(false);
}

void HormannCover::close_endstop_released() {
  ESP_LOGD(TAG, "Close endstop released.");
  this->current_operation = COVER_OPERATION_OPENING;
  this->last_dir = COVER_OPERATION_OPENING;
  this->publish_state(false);
}

// void HormannCover::check_endstops_(uint32_t now) {
//   if (this->is_open_() && this->is_closed_()) {
//     ESP_LOGE(TAG, "Error: both endstops fired simultaneously.");
//   }

//   if (this->is_open_()) {
//     if (this->current_operation == COVER_OPERATION_OPENING) {
//       float dur = (now - this->start_dir_time_) / 1e3f;
//       ESP_LOGD(TAG, "Open endstop reached. Took %.1fs.", dur);
//     }

//     this->current_operation = COVER_OPERATION_IDLE;
//     if (this->position != COVER_OPEN) {
//       this->position = COVER_OPEN;
//       this->publish_state(false);
//     }

//   } else if (this->is_closed_()) {
//     if (this->current_operation == COVER_OPERATION_CLOSING) {
//       float dur = (now - this->start_dir_time_) / 1e3f;
//       ESP_LOGD(TAG, "Close endstop reached. Took %.1fs.", dur);
//     }

//     this->current_operation = COVER_OPERATION_IDLE;
//     if (this->position != COVER_CLOSED) {
//       this->position = COVER_CLOSED;
//       this->publish_state(false);
//     }

//   } else if (now - this->start_dir_time_ > this->max_duration_) {
//     ESP_LOGE(TAG, "Max duration reached. Stopping cover.");
//     this->start_direction_(COVER_OPERATION_IDLE);
//     this->publish_state(false);
//   }
// }

// void HormannCover::start_direction_(CoverOperation dir) {
//   if (dir == this->current_operation)
//     return;

//   this->recompute_position();

//   ESP_LOGD(TAG, "Send gate actuate signal.");
//   this->actuate_trigger_->trigger();

//   this->current_operation = dir;

//   const uint32_t now = millis();
//   this->start_dir_time_ = now;
//   this->last_recompute_time_ = now;
// }

void HormannCover::recompute_position() {
  if (this->current_operation == COVER_OPERATION_IDLE)
    return;

  float dir;
  float action_dur;
  switch (this->current_operation) {
    case COVER_OPERATION_OPENING:
      dir = 1.0f;
      action_dur = this->open_duration_;
      break;
    case COVER_OPERATION_CLOSING:
      dir = -1.0f;
      action_dur = this->close_duration_;
      break;
    default:
      return;
  }

  const uint32_t now = millis();
  this->position += dir * (now - this->last_recompute_time_) / action_dur;
  this->position = clamp(this->position, 0.0f, 1.0f);

  this->last_recompute_time_ = now;
}

// Actuate the door switch and update state
void HormannCover::do_one_action() {
  ESP_LOGD(TAG, "Actuate gate.");
  this->actuate_trigger_->trigger();

  if (this->current_operation == COVER_OPERATION_OPENING || this->current_operation == COVER_OPERATION_CLOSING) {
    this->current_operation = COVER_OPERATION_IDLE;
  } else {
    if (this->last_dir == COVER_OPERATION_OPENING) {
      this->current_operation = COVER_OPERATION_CLOSING;
      this->last_dir = COVER_OPERATION_CLOSING;
    } else {
      this->current_operation = COVER_OPERATION_OPENING;
      this->last_dir = COVER_OPERATION_OPENING;
    }
  }
  this->publish_state(false);

  this->start_dir_time_ = millis();
}

}  // namespace hormann
}  // namespace esphome
