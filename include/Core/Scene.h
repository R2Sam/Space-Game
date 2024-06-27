#pragma once

class Services;

class Scene
{
protected:

	// If scene is active
	bool _active = false;

	virtual void Init() = 0;

	// Input update
	virtual void GetInputs() = 0;

public:

	virtual ~Scene() {}

	// To call when the scene is entered and exited
	virtual void Enter() = 0;
	virtual void Exit() = 0;

	// Basic update and draw
	virtual void Update() = 0;
	virtual void Draw() = 0;
};