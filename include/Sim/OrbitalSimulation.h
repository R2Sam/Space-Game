#pragma once
#include "Event.h"

#include "MyRaylib.h"

#include <string>
#include <memory>
#include <vector>
#include <deque>
#include <thread>
#include <unordered_map>

class Services;

class CelestialBody
{
public:

	std::string name;

	Vector3d position;
	Vector3d velocity;

	CelestialBody* parent = nullptr;

	double mass;
	double radius;

	double semiMajorAxis;
    double eccentricity;
    double inclination;
    double argumentOfPeriapsis;
    double longitudeAscendingNode;
    double trueAnomaly;

    CelestialBody(const std::string& nameIn, const Vector3d& positionIn, const Vector3d& velocityIn, const double& massIn, const double& radiusIn) : name(nameIn), position(positionIn), velocity(velocityIn), mass(massIn), radius(radiusIn) {}
};

class OrbitalBody
{
public:

	std::string name;

	Vector3d position;
	Vector3d velocity;

	CelestialBody* parent = nullptr;

	Vector3d thrust = Vector3dZero();

	double mass;

	OrbitalBody(const std::string& nameIn, const Vector3d& positionIn, const Vector3d& velocityIn, const double& massIn) : name(nameIn), position(positionIn), velocity(velocityIn), mass(massIn) {}
};

class OrbitalSimulation : public EventListener
{
private:

	// Ptr of global services
	Services* _services;

	std::deque<CelestialBody> _celestialBodies;
	std::unordered_map<std::string, CelestialBody*> _celestialBodiesMap;

	std::deque<std::shared_ptr<OrbitalBody>> _orbitalBodies;
	std::unordered_map<std::string, std::weak_ptr<OrbitalBody>> _orbitalBodiesMap;

	// Threads
	std::vector<std::thread> _threads;

	double _dt;
	unsigned int _speed;

	double _simTime = 0;
	bool _km;

	void AddSelfAsListener() override;
	void OnEvent(std::shared_ptr<const Event>& event) override;

	// Calculate acceleration and then numerically integrate
	inline Vector3d CalculateAcceleration(const Vector3d& r, const double& M) const;
	Vector3d CalculateTotalAcceleration(const Vector3d& position, OrbitalBody& body, std::deque<CelestialBody>& bodies);
	void RungeKutta(OrbitalBody& body, std::deque<CelestialBody>& bodies, const double& h);

	void CalculateOrbitalParamaters(CelestialBody* body);

	void UpdateCelestialBodies(std::deque<CelestialBody>& bodies, const double dt);
	void UpdateOrbitalBodies(std::deque<std::shared_ptr<OrbitalBody>>& bodies, std::deque<CelestialBody>& celestialBodies, const double dt);

public:

	OrbitalSimulation(Services* servicesIn, const double& timeStep, const bool& km);
	~OrbitalSimulation();

	void Update();

	void ResetThreads();

	// Add and remove bodies
	CelestialBody* AddCelestialBody(const CelestialBody& body);
	std::weak_ptr<OrbitalBody> AddOrbitalBody(const OrbitalBody& body);
	bool RemoveOrbitalBody(std::weak_ptr<OrbitalBody>& bodyPtr);

	// Get bodies
	std::vector<CelestialBody*> GetCelestialBodies();
	std::unordered_map<std::string, CelestialBody*> GetCelestialBodiesMap();

	std::vector<std::weak_ptr<OrbitalBody>> GetOrbitalBodies();
	std::unordered_map<std::string, std::weak_ptr<OrbitalBody>> GetOrbitalBodiesMap();

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