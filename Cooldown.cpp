#include "Cooldown.h"

Cooldown::Cooldown(int duration) {
    duration_ = duration;
}

void Cooldown::Start() {
    remaining_ = duration_;
}

void Cooldown::Update() {
    if (remaining_ > 0) {
        remaining_--;
    }
}

bool Cooldown::IsReady() const {
    return remaining_ == 0;
}