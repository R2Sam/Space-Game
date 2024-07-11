#include "OrbitalSimulation.h"
#include "Services.h"
#include "EventHandler.h"
#include "GameStateHandler.h"

#include "raylib.h"

#include "Log.h"

#include <cassert>
#include <array>
#include <cstring>
#include <vector>

// Unit in m
const double G = 6.674e-11;

// Unit in Km
const double GKm = 6.674e-20;

const unsigned int maxSpeed = 100000;

OrbitalSimulation::OrbitalSimulation(Services* servicesIn, const double& timeStep, const bool& km) : _services(servicesIn), _dt(timeStep), _km(km)
{
	AddSelfAsListener();
	
	_speed = 0;
}

OrbitalSimulation::~OrbitalSimulation()
{
	_services->GetEventHandler()->RemoveListener(_ptr);
}

void OrbitalSimulation::AddSelfAsListener()
{
	_services->GetEventHandler()->AddListener(_ptr);
	_services->GetEventHandler()->AddLocalListener("OrbitalSimulation", _ptr);
}

void OrbitalSimulation::OnEvent(std::shared_ptr<const Event>& event)
{
	if (std::shared_ptr<const SimulationSpeedEvent> simulationSpeedEvent = std::dynamic_pointer_cast<const SimulationSpeedEvent>(event))
	{
		SetSpeed(simulationSpeedEvent->speed);
	}
}

inline Vector3d OrbitalSimulation::CalculateAcceleration(const Vector3d& r, const double& M) const
{
	double mu;

	if (_km)
	{
		mu = GKm * M;
	}

	else
	{
		mu = G * M;
	}

	double length = r.length(); 

	assert(length > 0);

	return -r * ((mu)/(length * length * length));
}

Vector3d OrbitalSimulation::CalculateTotalAcceleration(const Vector3d& position, std::shared_ptr<OrbitalBody>& body, const std::vector<std::shared_ptr<OrbitalBody>>& bodies) const
{
	Vector3d acceleration = Vector3dZero();
	std::pair<double, std::weak_ptr<OrbitalBody>> topForce = std::make_pair(0, std::weak_ptr<OrbitalBody>());

	for (const std::shared_ptr<OrbitalBody>& otherBody : bodies)
	{
		if (body != otherBody)
		{
			Vector3d r = position - otherBody->position;

			Vector3d v = CalculateAcceleration(r, otherBody->mass);
			acceleration += v;

			float strength = v.length();
			if (topForce.first < strength)
			{
				topForce.first = strength;
				topForce.second = otherBody;
			}
		}
	}

	body->parent = topForce.second;

	return acceleration;
}

void OrbitalSimulation::RungeKutta(std::shared_ptr<OrbitalBody>& body, const std::vector<std::shared_ptr<OrbitalBody>>& bodies, const double& h)
{
    Vector3d k1v, k2v, k3v, k4v;
    Vector3d k1r, k2r, k3r, k4r;
    double halfH = h / 2;
    double sixthH = h / 6;

    k1v = CalculateTotalAcceleration(body->position, body, bodies);
    k1r = body->velocity;

    k2v = CalculateTotalAcceleration(body->position + (k1r * halfH), body, bodies);
    k2r = body->velocity + (k1v * halfH);

    k3v = CalculateTotalAcceleration(body->position + (k2r * halfH), body, bodies);
    k3r = body->velocity + (k2v * halfH);

    k4v = CalculateTotalAcceleration(body->position + (k3r * h), body, bodies);
    k4r = body->velocity + (k3v * h);

    body->posToAdd = (k1r + 2 * k2r + 2 * k3r + k4r) * sixthH;
    body->velToAdd = (k1v + 2 * k2v + 2 * k3v + k4v) * sixthH;

    if (!body->celestialBody && body->thrust != Vector3dZero())
    {
    	Vector3d thrust = body->thrust;

    	if (_km)
    	{
    		thrust *= 0.001; 
    	}

    	body->posToAdd = (thrust / body->mass) * h;
    	body->velToAdd = (thrust / body->mass) * h;
    }
}

void OrbitalSimulation::UpdateCelestialOrbits(const double& dt, std::vector<std::shared_ptr<OrbitalBody>>& celestialBodies)
{
	for(std::shared_ptr<OrbitalBody>& body : celestialBodies)
    {
        RungeKutta(body, celestialBodies, dt);
    }

    for(std::shared_ptr<OrbitalBody>& body : celestialBodies)
    {
        body->position += body->posToAdd;
        body->velocity += body->velToAdd;
    }
}

void OrbitalSimulation::UpdateNonCelestialOrbits(const double& dt, std::vector<std::shared_ptr<OrbitalBody>>& celestialBodies, std::vector<std::shared_ptr<OrbitalBody>>& nonCelestialBodies)
{
	for(std::shared_ptr<OrbitalBody>& body : nonCelestialBodies)
    {
    	RungeKutta(body, celestialBodies, dt);
    }

    for(std::shared_ptr<OrbitalBody>& body : nonCelestialBodies)
    {
        body->position += body->posToAdd;
        body->velocity += body->velToAdd;
    }
}

void OrbitalSimulation::Update()
{
	if (_speed == 0)
	{
		return;
	}

	// Cap the deltaT if the framerate goes bellow 30fps
	float deltaT = _services->deltaT;
	if (deltaT > 0.0333)
	{
		deltaT = 0.0333;

		static int i = 0;
		i++;
		if (i >= 60)
		{
			i = 0;
			LogColor("FPS bellow 30! Sim will remain at 0.0333 dt", LOG_YELLOW);
		}
	}

	unsigned int cycles = 0;

	double dt = _dt;

	unsigned short fps = 1 / deltaT;

	// Amount of updates according to the fps and speed
	float updates = (1 / (dt * fps)) * _speed;

	// If bellow 1 lower timestep so that one update is needed
	if (updates < 1)
	{	
		dt *= updates;
		updates = (1 / (dt * fps)) * _speed;
	}

	// If the updates is not an initger descrese timestep so it is
	else
	{
		float r = std::fmod(updates, 1.0);
		if ( r > 0.01)
		{
			updates +=  1 - r;
			dt = (1 / (updates * fps)) * _speed;
		}
	}

	cycles = updates;

	// Update both types of bodies
	for (int i = 0; i < cycles; i++)
	{
		UpdateCelestialOrbits(dt, _celestialBodies);
		UpdateNonCelestialOrbits(dt, _celestialBodies, _nonCelestialBodies);

		_simTime += dt;
	}
}

std::weak_ptr<OrbitalBody> OrbitalSimulation::AddBody(OrbitalBody& body)
{
	if (body.celestialBody)
	{
		_celestialBodies.push_back(std::make_shared<OrbitalBody>(body));

		return _celestialBodies[_celestialBodies.size() - 1];
	}

	_nonCelestialBodies.push_back(std::make_shared<OrbitalBody>(body));

	return _nonCelestialBodies[_nonCelestialBodies.size() - 1];
}

bool OrbitalSimulation::RemoveBody(std::weak_ptr<OrbitalBody> bodyPtr)
{
	if(!bodyPtr.lock())
	{
		return false;
	}

	if (bodyPtr.lock()->celestialBody)
	{
		for (int i = 0; i < _celestialBodies.size(); i++)
		{
			if (_celestialBodies[i] == bodyPtr.lock())
			{
				_celestialBodies.erase(_celestialBodies.begin() + i);
				return true;
			}
		}

		return false;
	}

	for (int i = 0; i < _nonCelestialBodies.size(); i++)
	{
		if (_nonCelestialBodies[i] == bodyPtr.lock())
		{
			_nonCelestialBodies.erase(_nonCelestialBodies.begin() + i);
			return true;
		}
	}

	return false;
}

std::unordered_map<std::string, std::weak_ptr<OrbitalBody>> OrbitalSimulation::GetBodies(const bool& celestialBody)
{
	std::unordered_map<std::string, std::weak_ptr<OrbitalBody>> bodies;

	if (celestialBody)
	{
		for (std::shared_ptr<OrbitalBody>& body : _celestialBodies)
		{
			bodies[body->name] = body;
		}
	}

	else
	{
		for (std::shared_ptr<OrbitalBody>& body : _nonCelestialBodies)
		{
			bodies[body->name] = body;
		}
	}

	return bodies;
}

std::vector<std::weak_ptr<OrbitalBody>> OrbitalSimulation::GetBodiesV(const bool& celestialBody)
{
	std::vector<std::weak_ptr<OrbitalBody>> bodies;

	if (celestialBody)
	{
		for (std::shared_ptr<OrbitalBody>& body : _celestialBodies)
		{
			bodies.push_back(body);
		}
	}

	else
	{
		for (std::shared_ptr<OrbitalBody>& body : _nonCelestialBodies)
		{
			bodies.push_back(body);
		}
	}

	return bodies;
}

std::pair<std::unordered_map<std::string, std::shared_ptr<OrbitalBody>>, std::unordered_map<std::string, std::shared_ptr<OrbitalBody>>> OrbitalSimulation::BodiesAtTime(std::vector<std::weak_ptr<OrbitalBody>>& ptrs, const double& time)
{
	std::pair<std::vector<std::shared_ptr<OrbitalBody>>, std::vector<std::shared_ptr<OrbitalBody>>> bodies;

	std::vector<std::shared_ptr<OrbitalBody>> nonCelestialBodies;
	nonCelestialBodies.reserve(ptrs.size());

	for (std::weak_ptr<OrbitalBody> weak : ptrs)
	{
		if (weak.lock())
		{
			nonCelestialBodies.push_back(weak.lock());
		}
	}

	bodies.first.reserve(_celestialBodies.size());
	for (std::shared_ptr<OrbitalBody>& body : _celestialBodies)
	{
		bodies.first.push_back(std::make_shared<OrbitalBody>(*body));
	}

	bodies.second.reserve(_nonCelestialBodies.size());
	for (std::shared_ptr<OrbitalBody>& body : nonCelestialBodies)
	{
		bodies.second.push_back(std::make_shared<OrbitalBody>(*body));
	}

	std::pair<std::unordered_map<std::string, std::shared_ptr<OrbitalBody>>, std::unordered_map<std::string, std::shared_ptr<OrbitalBody>>> output;

	if (time > _simTime)
	{
		float updates = (time - _simTime) / _dt;

		float r = std::fmod(updates, 1.0);
		if ( r > 0.01)
		{
			updates -= r;
		}

		unsigned int cycles = updates;

		for (int i = 0; i < cycles; i++)
		{
			UpdateCelestialOrbits(_dt, bodies.first);
			UpdateNonCelestialOrbits(_dt, bodies.first, bodies.second);
		}
	}

	else if (time < _simTime)
	{
		float updates = (_simTime - time) / _dt;

		float r = std::fmod(updates, 1.0);
		if ( r > 0.01)
		{
			updates -= r;
		}

		unsigned int cycles = updates;

		for (int i = 0; i < cycles; i++)
		{
			UpdateCelestialOrbits(-_dt, bodies.first);
			UpdateNonCelestialOrbits(-_dt, bodies.first, bodies.second);
		}
	}

	for (std::shared_ptr<OrbitalBody>& body : bodies.first)
	{
		output.first.insert(std::make_pair(body->name, std::move(body)));
	}

	for (std::shared_ptr<OrbitalBody>& body : bodies.second)
	{
		output.second.insert(std::make_pair(body->name, std::move(body)));
	}

	return output;
}

std::pair<std::vector<std::shared_ptr<OrbitalBody>>, std::vector<std::shared_ptr<OrbitalBody>>> OrbitalSimulation::BodiesAtTimeV(std::vector<std::weak_ptr<OrbitalBody>>& ptrs, const double& time)
{
	std::pair<std::vector<std::shared_ptr<OrbitalBody>>, std::vector<std::shared_ptr<OrbitalBody>>> bodies;

	std::vector<std::shared_ptr<OrbitalBody>> nonCelestialBodies;
	nonCelestialBodies.reserve(ptrs.size());

	for (std::weak_ptr<OrbitalBody> weak : ptrs)
	{
		if (weak.lock())
		{
			nonCelestialBodies.push_back(weak.lock());
		}
	}

	bodies.first.reserve(_celestialBodies.size());
	for (std::shared_ptr<OrbitalBody>& body : _celestialBodies)
	{
		bodies.first.push_back(std::make_shared<OrbitalBody>(*body));
	}

	bodies.second.reserve(_nonCelestialBodies.size());
	for (std::shared_ptr<OrbitalBody>& body : nonCelestialBodies)
	{
		bodies.second.push_back(std::make_shared<OrbitalBody>(*body));
	}

	if (time > _simTime)
	{
		float updates = (time - _simTime) / _dt;

		float r = std::fmod(updates, 1.0);
		if ( r > 0.01)
		{
			updates -= r;
		}

		unsigned int cycles = updates;

		for (int i = 0; i < cycles; i++)
		{
			UpdateCelestialOrbits(_dt, bodies.first);
			UpdateNonCelestialOrbits(_dt, bodies.first, bodies.second);
		}
	}

	else if (time < _simTime)
	{
		float updates = (_simTime - time) / _dt;

		float r = std::fmod(updates, 1.0);
		if ( r > 0.01)
		{
			updates -= r;
		}

		unsigned int cycles = updates;

		for (int i = 0; i < cycles; i++)
		{
			UpdateCelestialOrbits(-_dt, bodies.first);
			UpdateNonCelestialOrbits(-_dt, bodies.first, bodies.second);
		}
	}

	return bodies;
}

std::vector<std::vector<std::pair<std::string, Vector3d>>> OrbitalSimulation::GetCelestialBodiesPos(const double& time, const int& resolution)
{
	std::vector<std::vector<std::pair<std::string, Vector3d>>> positions;

	if (time != 0 && time > _simTime && resolution > 0)
	{
		positions.reserve(_celestialBodies.size());

		std::vector<std::shared_ptr<OrbitalBody>> celestialBodies;
		celestialBodies.reserve(_celestialBodies.size());

		for (std::shared_ptr<OrbitalBody>& body : _celestialBodies)
		{
			celestialBodies.push_back(std::make_shared<OrbitalBody>(*body));
			positions.push_back(std::vector<std::pair<std::string, Vector3d>>{});
		}

		float updates = (time - _simTime) / _dt;

		float r = std::fmod(updates, 1.0);
		if ( r > 0.01)
		{
			updates -= r;
		}

		unsigned int cycles = updates;

		for (int i = 0; i < cycles; i++)
		{
			UpdateCelestialOrbits(_dt, celestialBodies);

			if (i % resolution == 0)
			{
				for (int i = 0; i < celestialBodies.size(); i++)
				{
					positions[i].push_back(std::make_pair(celestialBodies[i]->name, celestialBodies[i]->position));
				}
			}
		}
	}

	return positions;
}

std::pair<std::vector<std::vector<std::pair<std::string, Vector3d>>>, std::vector<std::vector<std::pair<std::string, Vector3d>>>> OrbitalSimulation::GetBodiesPos(std::vector<std::weak_ptr<OrbitalBody>>& ptrs, const double& time, const int& resolution)
{
	std::pair<std::vector<std::vector<std::pair<std::string, Vector3d>>>, std::vector<std::vector<std::pair<std::string, Vector3d>>>> positions;

	std::vector<std::shared_ptr<OrbitalBody>> nonCelestialBodies;
	nonCelestialBodies.reserve(ptrs.size());

	for (std::weak_ptr<OrbitalBody> weak : ptrs)
	{
		if (weak.lock())
		{
			nonCelestialBodies.push_back(weak.lock());
		}
	}

	if (!nonCelestialBodies.empty() && time != 0 && time > _simTime && resolution > 0)
	{
		positions.first.reserve(_celestialBodies.size());

		std::vector<std::shared_ptr<OrbitalBody>> celestialBodies;
		celestialBodies.reserve(_celestialBodies.size());

		for (std::shared_ptr<OrbitalBody>& body : _celestialBodies)
		{
			celestialBodies.push_back(std::make_shared<OrbitalBody>(*body));
			positions.first.push_back(std::vector<std::pair<std::string, Vector3d>>{});
		}

		float updates = (time - _simTime) / _dt;

		float r = std::fmod(updates, 1.0);
		if ( r > 0.01)
		{
			updates -= r;
		}

		unsigned int cycles = updates;

		for (int i = 0; i < cycles; i++)
		{
			UpdateCelestialOrbits(_dt, celestialBodies);
			UpdateNonCelestialOrbits(_dt, celestialBodies, nonCelestialBodies);

			if (i % resolution == 0)
			{
				for (int i = 0; i < celestialBodies.size(); i++)
				{
					positions.first[i].push_back(std::make_pair(celestialBodies[i]->name, celestialBodies[i]->position));
				}

				for (int i = 0; i < nonCelestialBodies.size(); i++)
				{
					positions.second[i].push_back(std::make_pair(nonCelestialBodies[i]->name, nonCelestialBodies[i]->position));
				}
			}
		}
	}

	return positions;
}

double OrbitalSimulation::GetTime() const
{
	return _simTime;
}

void OrbitalSimulation::SpeedControl(bool& increse, bool& decrese)
{

	std::array<unsigned int, 10> speeds = {0, 1, 4, 10, 30, 100, 300, 1000, 2000, 5000};
	static unsigned int speedIndex = 0;

	if (increse)
	{
		if (speedIndex < speeds.size() - 1)
		{
			speedIndex++;
			_speed = speeds[speedIndex];
		}
	}

	if (decrese)
	{
		if (speedIndex > 0)
		{
			speedIndex--;
			_speed = speeds[speedIndex];
		}
	}
}

void OrbitalSimulation::Step(const double& stepSize)
{
	if (_speed != 0 || stepSize == 0)
	{
		return;
	}

	float time = _simTime + stepSize;
	float updates = (time - _simTime) / _dt;

	float dt;

	if (stepSize > 0)
	{
		dt = _dt;
	}

	else
	{
		dt = -_dt;
	}

	float r = std::fmod(updates, 1.0);
	if ( r > 0.01)
	{
		updates -= r;
	}

	unsigned int cycles = updates;

	for (int i = 0; i < cycles; i++)
	{
		UpdateCelestialOrbits(dt, _celestialBodies);
		UpdateNonCelestialOrbits(dt, _celestialBodies, _nonCelestialBodies);
	}

	if ( r > 0.01)
	{

		dt *= r;

		UpdateCelestialOrbits(dt, _celestialBodies);
		UpdateNonCelestialOrbits(dt, _celestialBodies, _nonCelestialBodies);
	}
}

unsigned int OrbitalSimulation::GetSpeed() const
{
	return _speed;
}

void OrbitalSimulation::SetSpeed(const unsigned int& speed)
{
	if (speed <= maxSpeed)
	{
		_speed = speed;
	}

	else
	{
		_speed = maxSpeed;
	}
}

bool OrbitalSimulation::GetKm() const 
{
	return _km;
}

void OrbitalSimulation::SetKm(const bool& km)
{
	if (km == _km)
	{
		return;
	}

	if (_km)
	{
		for (std::shared_ptr<OrbitalBody>& body : _celestialBodies)
		{
			body->position *= 1000;
			body->velocity *= 1000;
			body->radius *= 1000;
		}

		for (std::shared_ptr<OrbitalBody>& body : _nonCelestialBodies)
		{
			body->position *= 1000;
			body->velocity *= 1000;
			body->radius *= 1000;
		}
	}

	else
	{
		for (std::shared_ptr<OrbitalBody>& body : _celestialBodies)
		{
			body->position *= 0.001;
			body->velocity *= 0.001;
			body->radius *= 0.001;
		}

		for (std::shared_ptr<OrbitalBody>& body : _nonCelestialBodies)
		{
			body->position *= 0.001;
			body->velocity *= 0.001;
			body->radius *= 0.001;
		}
	}

	_km = km;
}

bool OrbitalSimulation::SaveBodiesToFile(const std::string& path)
{
    std::string output;

    for (const std::shared_ptr<OrbitalBody>& body : _celestialBodies)
    {
        output += "--Name:" + body->name;
        
        output += "--CelestialBody:" + std::to_string(true);
        
        Vector3d pos = body->position;
        output += "--Position:";
        output += std::to_string(pos.x) + "," + std::to_string(pos.y) + "," + std::to_string(pos.z);
 
 		Vector3d vel = body->velocity;       
        output += "--Velocity:";
        output += std::to_string(vel.x) + "," + std::to_string(vel.y) + "," + std::to_string(vel.z);
        
        output += "--Mass:" + std::to_string(body->mass);
        
        output += "--Radius:" + std::to_string(body->radius);
        
        output += "---\n";
    }

    for (const std::shared_ptr<OrbitalBody>& body : _nonCelestialBodies)
    {
        output += "--Name:" + body->name;
        
        output += "--CelestialBody:" + std::to_string(false);
        
        Vector3d pos = body->position;
        output += "--Position:";
        output += std::to_string(pos.x) + "," + std::to_string(pos.y) + "," + std::to_string(pos.z);
 
 		Vector3d vel = body->velocity;       
        output += "--Velocity:";
        output += std::to_string(vel.x) + "," + std::to_string(vel.y) + "," + std::to_string(vel.z);
        
        output += "--Mass:" + std::to_string(body->mass);
        
        output += "--Radius:" + std::to_string(body->radius);
        
        output += "---\n";
    }

    std::vector<char> writableOutput(output.begin(), output.end());
    writableOutput.push_back('\0');

    return SaveFileText(path.c_str(), writableOutput.data());
}

bool OrbitalSimulation::LoadBodiesFromFile(const std::string& path)
{
	if (!FileExists(path.c_str()))
	{
		return false;
	}

	char* fileText = LoadFileText(path.c_str());
	int fileLength = strlen(fileText);

	std::string buffer;
	std::string numberS;
	double numbers[3];
	int vectorFill = 0;

	std::string name;
	bool CelestialBody;

	Vector3d pos;
	Vector3d vel;

	double mass;
	double radius;

	_simTime = 0;

	_celestialBodies.clear();
	_celestialBodies.shrink_to_fit();

	_nonCelestialBodies.shrink_to_fit();
	_nonCelestialBodies.clear();

	for (int i = 0; i < fileLength; i++)
	{
		if (fileText[i] == '\n')
		{
			continue;
		}

		buffer += fileText[i];

		if (buffer == "--Name:")
		{
			for (int ii = i + 1; ii < fileLength; ii++)
			{
				// Ending of a section with "--"
				if (fileText[ii] == '-' && fileText[ii + 1] == '-')
				{
					buffer.clear();
					buffer.shrink_to_fit();

					i = ii - 1;

					break;					
				}

				name += fileText[ii];
			}
		}

		if (buffer == "--CelestialBody:")
		{
			if (fileText[i+1] == '0')
			{
				CelestialBody = false;
			}

			else
			{
				CelestialBody = true;
			}

			buffer.clear();
			buffer.shrink_to_fit();

			// We skip ahead since we alredy looked one forward
			i++;
		}

		if (buffer == "--Position:")
		{
			for (int ii = i + 1; ii < fileLength; ii++)
			{
				if (fileText[ii] == '-' && fileText[ii + 1] == '-')
				{
					// If we reach here thus we haven't "--" then the data is in error
					if (vectorFill == 3)
					{
						Log("Out of range");
						UnloadFileText(fileText);
						return false;
					}

					numbers[vectorFill] = std::stod(numberS);

					pos = {numbers[0], numbers[1], numbers[2]};

					buffer.clear();
					buffer.shrink_to_fit();

					numberS.clear();
					numberS.shrink_to_fit();

					vectorFill = 0;

					i = ii - 1;

					break;
				}

				if (fileText[ii] == ',')
				{
					// If we reach here thus we haven't "--" then the data is in error
					if (vectorFill == 3)
					{
						Log("Out of range");
						UnloadFileText(fileText);
						return false;
					}

					numbers[vectorFill] = std::stod(numberS);
					vectorFill++;

					numberS.clear();
					numberS.shrink_to_fit();

					continue;
				}

				numberS += fileText[ii];
			}
		}

		if (buffer == "--Velocity:")
		{
			for (int ii = i + 1; ii < fileLength; ii++)
			{
				if (fileText[ii] == '-' && fileText[ii + 1] == '-')
				{
					// If we reach here thus we haven't "--" then the data is in error
					if (vectorFill == 3)
					{
						Log("Out of range");
						UnloadFileText(fileText);
						return false;
					}

					numbers[vectorFill] = std::stod(numberS);

					vel = {numbers[0], numbers[1], numbers[2]};

					buffer.clear();
					buffer.shrink_to_fit();

					numberS.clear();
					numberS.shrink_to_fit();

					vectorFill = 0;

					i = ii - 1;

					break;
				}

				if (fileText[ii] == ',')
				{
					// If we reach here thus we haven't "--" then the data is in error
					if (vectorFill == 3)
					{
						Log("Out of range");
						UnloadFileText(fileText);
						return false;
					}

					numbers[vectorFill] = std::stod(numberS);
					vectorFill++;

					numberS.clear();
					numberS.shrink_to_fit();

					continue;
				}

				numberS += fileText[ii];
			}
		}

		if (buffer == "--Mass:")
		{
			for (int ii = i + 1; ii < fileLength; ii++)
			{
				// Ending of a section with "--"
				if (fileText[ii] == '-' && fileText[ii + 1] == '-')
				{
					mass = std::stod(numberS);

					buffer.clear();
					buffer.shrink_to_fit();

					numberS.clear();
					numberS.shrink_to_fit();

					i = ii - 1;

					break;
				}

				numberS += fileText[ii];
			}
		}

		if (buffer == "--Radius:")
		{
			for (int ii = i + 1; ii < fileLength; ii++)
			{
				// Ending of a section with "--"
				if (fileText[ii] == '-' && fileText[ii + 1] == '-')
				{
					radius = std::stod(numberS);

					buffer.clear();
					buffer.shrink_to_fit();

					numberS.clear();
					numberS.shrink_to_fit();

					i = ii - 1;

					break;
				}

				numberS += fileText[ii];
			}
		}

		if (buffer == "---")
		{
			OrbitalBody body(name, CelestialBody, pos, vel, mass, radius);
	 		AddBody(body);

	 		name.clear();
	 		buffer.clear();
		}
	}

	UnloadFileText(fileText);
	return true;
}