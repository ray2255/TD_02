#pragma once
class Cooldown {
public:
	Cooldown(int frames);
	void Start();
	void Update();
	bool IsReady()const;
private:
	int duration_;
	int remaining_ = 0;

};
