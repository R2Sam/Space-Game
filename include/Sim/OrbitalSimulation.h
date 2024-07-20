#pragma once
#include "Event.h"

#include "MyRaylib.h"

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <atomic>
#include <thread>

class Services;

class OrbitalBody
{
public:

	std::string name;
	bool celestialBody;

	Vector3d position;
	Vector3d velocity;

	std::string parent = "Null";

	Vector3d thrust = Vector3dZero();

	double mass;
	float radius;

	OrbitalBody(const std::string& nameIn, const bool& celestialBodyIn, const Vector3d& positionIn, const Vector3d& velocityIn, const double& massIn, const float& radiusIn) : name(nameIn), celestialBody(celestialBodyIn), position(positionIn), velocity(velocityIn), mass(massIn), radius(radiusIn) {}
};

class OrbitalSimulation : public EventListener
{
private:

	// Ptr of global services
	Services* _services;

	std::vector<std::shared_ptr<OrbitalBody>> _celestialBodies;
	std::vector<std::shared_ptr<OrbitalBody>> _nonCelestialBodies;

	std::unordered_set<std::string> _celestialBodiesNames;
	std::unordered_set<std::string> _nonCelestialBodiesNames;

	// Threads
	std::vector<std::thread> _threads;
	std::atomic<int> _threadNumber = 4;

	std::atomic<int> _done = _threadNumber * 2;
	std::atomic<int> _ready = 0;

	double _dt;
	unsigned int _speed;

	double _simTime = 0;
	bool _km;

	void AddSelfAsListener() override;
	void OnEvent(std::shared_ptr<const Event>& event) override;

	// Calculate acceleration and then numerically integrate
	inline Vector3d CalculateAcceleration(const Vector3d& r, const double& M) const;
	Vector3d CalculateTotalAcceleration(const Vector3d& position, OrbitalBody& body, const std::vector<OrbitalBody>& bodies) const;
	void RungeKutta(OrbitalBody& body, const std::vector<OrbitalBody>& bodies, const double& h);

	void UpdateOrbits(std::vector<OrbitalBody>& bodies, std::vector<OrbitalBody>& celestialBodies, const std::atomic<double>& dt, const std::atomic<int>& steps, std::atomic<int>& done,  std::atomic<int>& ready, std::atomic<bool>& start);

public:

	OrbitalSimulation(Services* servicesIn, const double& timeStep, const bool& km);
	~OrbitalSimulation();

	void Update();

	void ResetThreads();

	std::weak_ptr<OrbitalBody> AddBody(OrbitalBody& body);
	bool RemoveBody(std::weak_ptr<OrbitalBody> bodyPtr);

	// Get current bodies ptrs
	std::unordered_map<std::string, std::weak_ptr<OrbitalBody>> GetBodies(const bool& celestialBody);
	std::vector<std::weak_ptr<OrbitalBody>> GetBodiesV(const bool& celestialBody);

	// Get time since sim start in s
	double GetTime() const;
	std::string GetDate() const;

	// Input bools that will control speed
	void SpeedControl(bool& increse, bool& decrese);

	// Get and set current sim speed
	unsigned int GetSpeed() const;
	void SetSpeed(const unsigned int& speed);

	// Get unit type and set
	bool GetKm() const;
	void SetKm(const bool& km);

	// Save and load bodies
	bool SaveBodiesToFile(const std::string& path);
	bool LoadBodiesFromFile(const std::string& path);
};

class SimulationSpeedEvent : public Event
{
public: 

	unsigned int speed;

	SimulationSpeedEvent(const unsigned int& speedIn) : speed(speedIn) {}
};