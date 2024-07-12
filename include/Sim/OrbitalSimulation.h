#pragma once
#include "Event.h"

#include "MyRaylib.h"

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

class Services;

class OrbitalBody
{
public:

	std::string name;
	bool celestialBody;

	Vector3d position;
	Vector3d velocity;

	Vector3d posToAdd;
	Vector3d velToAdd;

	std::weak_ptr<OrbitalBody> parent;

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

	double _dt;
	unsigned int _speed;

	double _simTime = 0;
	bool _km;

	void AddSelfAsListener() override;
	void OnEvent(std::shared_ptr<const Event>& event) override;

	// Calculate acceleration and then numerically integrate
	inline Vector3d CalculateAcceleration(const Vector3d& r, const double& M) const;
	Vector3d CalculateTotalAcceleration(const Vector3d& position, std::shared_ptr<OrbitalBody>& body, const std::vector<std::shared_ptr<OrbitalBody>>& bodies) const;
	void RungeKutta(std::shared_ptr<OrbitalBody>& body, const std::vector<std::shared_ptr<OrbitalBody>>& bodies, const double& h);

	void UpdateCelestialOrbits(const double& dt, std::vector<std::shared_ptr<OrbitalBody>>& celestialBodies);
	void UpdateNonCelestialOrbits(const double& dt, std::vector<std::shared_ptr<OrbitalBody>>& celestialBodies, std::vector<std::shared_ptr<OrbitalBody>>& nonCelestialBodies);

public:

	OrbitalSimulation(Services* servicesIn, const double& timeStep, const bool& km);
	~OrbitalSimulation();

	void Update();

	std::weak_ptr<OrbitalBody> AddBody(OrbitalBody& body);
	bool RemoveBody(std::weak_ptr<OrbitalBody> bodyPtr);

	// Get current bodies ptrs
	std::unordered_map<std::string, std::weak_ptr<OrbitalBody>> GetBodies(const bool& celestialBody);
	std::vector<std::weak_ptr<OrbitalBody>> GetBodiesV(const bool& celestialBody);

	// Get bodies at a certain instance in the future or past
	std::pair<std::unordered_map<std::string, std::shared_ptr<OrbitalBody>>, std::unordered_map<std::string, std::shared_ptr<OrbitalBody>>> BodiesAtTime(std::vector<std::weak_ptr<OrbitalBody>>& ptrs, const double& time);
	std::pair<std::vector<std::shared_ptr<OrbitalBody>>, std::vector<std::shared_ptr<OrbitalBody>>> BodiesAtTimeV(std::vector<std::weak_ptr<OrbitalBody>>& ptrs, const double& time);

	// Get future positions
	std::vector<std::vector<std::pair<std::string, Vector3d>>> GetCelestialBodiesPos(const double& time, const int& resolution);
	std::pair<std::vector<std::vector<std::pair<std::string, Vector3d>>>, std::vector<std::vector<std::pair<std::string, Vector3d>>>> GetBodiesPos(std::vector<std::weak_ptr<OrbitalBody>>& ptrs, const double& time, const int& resolution);

	// Get time since sim start in s
	double GetTime() const;
	std::string GetDate() const;

	// Input bools that will control speed
	void SpeedControl(bool& increse, bool& decrese);

	// Step
	void Step(const double& stepSize);

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