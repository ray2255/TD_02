#pragma once

class Cooldown {
public:
    Cooldown(int frames);

    void Start();
    void Update();
    bool IsReady() const;

private:
    int duration_ = 0;
    int remaining_ = 0;
};