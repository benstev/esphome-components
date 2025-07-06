#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "somfy_cover.h"
#include <EEPROMRollingCodeStorage.h>

namespace esphome {
namespace somfy {

static const char *TAG = "somfy.cover";

using namespace esphome::cover;

CoverTraits SomfyCover::get_traits() {
  auto traits = CoverTraits();
  traits.set_is_assumed_state(true);
  traits.set_supports_position(true);
  traits.set_supports_tilt(false);
  traits.set_supports_toggle(false);
  return traits;
}

void SomfyCover::dump_config() { ESP_LOGCONFIG(TAG, "somfy.cover"); }

void SomfyCover::setup() {
  pinMode(EMITTER_GPIO, OUTPUT);
  digitalWrite(EMITTER_GPIO, LOW);

  EEPROM.begin(max(4, EEPROM_RECORD_SIZE * 3));

  RollingCodeStorage *storage = new EEPROMRollingCodeStorage(this->storage_record_ * EEPROM_RECORD_SIZE);
  this->remote = new SomfyRemote(EMITTER_GPIO, this->remote_code_, storage);
}

// This will be called every time the user requests a state change.
void SomfyCover::control(const CoverCall &call) {
  if (call.get_stop()) {
    ESP_LOGI(TAG, "Control STOP <%s>", this->name_);
    this->start_direction_(COVER_OPERATION_IDLE);
    this->publish_state();
  }

  if (call.get_position().has_value()) {
    auto pos = *call.get_position();
    ESP_LOGI(TAG, "Control POSITION <%s> -> %f", this->name_, pos);
    if (pos == this->position) {
      // already at target
      if (true && (pos == COVER_OPEN || pos == COVER_CLOSED)) {
        auto op = pos == COVER_CLOSED ? COVER_OPERATION_CLOSING : COVER_OPERATION_OPENING;
        this->target_position_ = pos;
        this->start_direction_(op);
      }
    } else {
      auto op = pos < this->position ? COVER_OPERATION_CLOSING : COVER_OPERATION_OPENING;
      this->target_position_ = pos;
      this->start_direction_(op);
    }
  }
}

// This will be called by App.loop()
void SomfyCover::loop() {
  const uint32_t now = millis();

  this->recompute_position_();

  if (this->is_at_target_()) {
    if (this->target_position_ == COVER_OPEN || this->target_position_ == COVER_CLOSED) {
      this->current_operation = COVER_OPERATION_IDLE;
    } else {
      this->start_direction_(COVER_OPERATION_IDLE);
    }
    this->publish_state();
  }

  if (now - this->last_publish_time_ > 1000) {
    this->publish_state(false);
    ESP_LOGD(TAG, "Loop %s, pos: %f", this->name_, this->position);
    this->last_publish_time_ = now;
  }
}

// recalculates door position
void SomfyCover::recompute_position_() {
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

bool SomfyCover::is_at_target_() {
  switch (this->current_operation) {
    case COVER_OPERATION_OPENING:
      return this->position >= this->target_position_;
    case COVER_OPERATION_CLOSING:
      return this->position <= this->target_position_;
    case COVER_OPERATION_IDLE:
    default:
      return true;
  }
}

inline const char *CmdToString(Command v) {
  switch (v) {
    case Command::My:
      return "My";
    case Command::Up:
      return "Up";
    case Command::MyUp:
      return "MyUp";
    case Command::Down:
      return "Down";
    case Command::MyDown:
      return "MyDown";
    case Command::UpDown:
      return "UpDown";
    case Command::Prog:
      return "Prog";
    case Command::SunFlag:
      return "SunFlag";
    case Command::Flag:
      return "Flag";
    default:
      return "[UnknownCommand]";
  }
}

void SomfyCover::sendSomfyCommand(Command command) {
  ESP_LOGI(TAG, "send somfy command <%s> to <%s>", CmdToString(command), this->name_);

  this->cc1101_->xmit();
  this->remote->sendCommand(command);
  this->cc1101_->idle();
}

void SomfyCover::start_direction_(CoverOperation dir) {
  if (dir == this->current_operation && dir != COVER_OPERATION_IDLE)
    return;

  this->recompute_position_();
  switch (dir) {
    case COVER_OPERATION_IDLE:
      ESP_LOGI(TAG, "STOP <%s>", this->name_);
      sendSomfyCommand(Command::My);
      break;
    case COVER_OPERATION_OPENING:
      ESP_LOGI(TAG, "OPEN <%s>", this->name_);
      sendSomfyCommand(Command::Up);
      break;
    case COVER_OPERATION_CLOSING:
      ESP_LOGI(TAG, "CLOSE <%s>", this->name_);
      sendSomfyCommand(Command::Down);
      break;
    default:
      return;
  }

  this->current_operation = dir;
  this->last_recompute_time_ = millis();
}

void SomfyCover::program() {
  ESP_LOGI(TAG, "PROG <%s>", this->name_);
  sendSomfyCommand(Command::Prog);
}

}  // namespace somfy
}  // namespace esphome
