#include "OrbitalSimulation.h"
#include "Services.h"
#include "EventHandler.h"
#include "GameStateHandler.h"

#include "raylib.h"

#include "Log.h"

#include <cassert>
#include <array>
#include <cstring>
#include <thread>
#include <deque>

// Unit in m
const double G = 6.674e-11;

// Unit in Km
const double GKm = 6.674e-20;

const unsigned int maxSpeed = 100000;

const std::tm epoch = {0, 0, 0, 1, 0, 120, -1};

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

Vector3d OrbitalSimulation::CalculateTotalAcceleration(const Vector3d& position, OrbitalBody& body, const std::vector<OrbitalBody>& bodies) const
{
	Vector3d acceleration = Vector3dZero();
	std::pair<double, std::string> topForce = std::make_pair(0, "");

	for (const OrbitalBody& otherBody : bodies)
	{
		if (body.name != otherBody.name)
		{
			Vector3d r = position - otherBody.position;

			Vector3d v = CalculateAcceleration(r, otherBody.mass);
			acceleration += v;

			float strength = v.length();
			if (topForce.first < strength && body.mass < otherBody.mass)
			{
				topForce.first = strength;
				topForce.second = otherBody.name;
			}
		}
	}

	body.parent = topForce.second;

	return acceleration;
}

void OrbitalSimulation::RungeKutta(OrbitalBody& body, const std::vector<OrbitalBody>& bodies, const double& h)
{
    Vector3d k1v, k2v, k3v, k4v;
    Vector3d k1r, k2r, k3r, k4r;
    double halfH = h / 2;
    double sixthH = h / 6;

    k1v = CalculateTotalAcceleration(body.position, body, bodies);
    k1r = body.velocity;

    k2v = CalculateTotalAcceleration(body.position + (k1r * halfH), body, bodies);
    k2r = body.velocity + (k1v * halfH);

    k3v = CalculateTotalAcceleration(body.position + (k2r * halfH), body, bodies);
    k3r = body.velocity + (k2v * halfH);

    k4v = CalculateTotalAcceleration(body.position + (k3r * h), body, bodies);
    k4r = body.velocity + (k3v * h);

    body.position += (k1r + 2 * k2r + 2 * k3r + k4r) * sixthH;
    body.velocity += (k1v + 2 * k2v + 2 * k3v + k4v) * sixthH;

    if (!body.celestialBody && body.thrust != Vector3dZero())
    {
    	Vector3d thrust = body.thrust;

    	if (_km)
    	{
    		thrust *= 0.001; 
    	}

    	body.position += (thrust / body.mass) * h;
    	body.velocity += (thrust / body.mass) * h;
    }
}

void OrbitalSimulation::UpdateOrbits(std::vector<OrbitalBody>& bodies, std::vector<OrbitalBody>& celestialBodies, const std::atomic<double>& dt, const std::atomic<int>& steps, std::atomic<int>& done, std::atomic<int>& ready, std::atomic<bool>& start)
{
	while (true)
	{
		while (done.load(std::memory_order_acquire) != 0)
		{
			std::this_thread::yield();
		}

		ready.fetch_add(1, std::memory_order_release);

		while (!start.load(std::memory_order_acquire))
		{
			std::this_thread::yield();
		}

		if (bodies.size())
		{
			for (int i = 0; i < steps; i++)
			{
				for (OrbitalBody& body : bodies)
				{
					RungeKutta(body, celestialBodies, dt);
				}
			}
		}

		done.fetch_add(1, std::memory_order_release);
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

	const static int threadNumber = 4;
	static std::vector<std::thread> threads;

	static std::atomic<int> steps = updates;
	steps = updates;
	static std::atomic<double> timeStep = dt;
	timeStep = dt;

	static std::atomic<int> done = threadNumber * 2;
	static std::atomic<int> ready = 0;
	static std::atomic<bool> start = false;

	static std::deque<std::vector<OrbitalBody>> celestialBodies(threadNumber);
	static std::deque<std::vector<OrbitalBody>> nonCelestialBodies(threadNumber);

	static std::deque<std::vector<OrbitalBody>> allCelestialBodies1(threadNumber);
	static std::deque<std::vector<OrbitalBody>> allCelestialBodies2(threadNumber);

	static bool initialReserve = false;

	if (!initialReserve)
	{
		initialReserve = true;

		for (int i = 0; i < threadNumber; i++)
		{
			threads.push_back(std::thread(&OrbitalSimulation::UpdateOrbits, this, std::ref(celestialBodies[i]), std::ref(allCelestialBodies1[i]), std::ref(timeStep), std::ref(steps), std::ref(done), std::ref(ready), std::ref(start)));
		}

		for (int i = 0; i < threadNumber; i++)
		{
			threads.push_back(std::thread(&OrbitalSimulation::UpdateOrbits, this, std::ref(nonCelestialBodies[i]), std::ref(allCelestialBodies2[i]), std::ref(timeStep), std::ref(steps), std::ref(done), std::ref(ready), std::ref(start)));
		}
	}

	static int celestialBodyCount = 0;
	static int nonCelestialBodyCount = 0;

	if (celestialBodyCount != _celestialBodies.size())
	{
		celestialBodyCount = _celestialBodies.size();
		int count = celestialBodyCount / threadNumber;
		int remainder = celestialBodyCount % threadNumber;

		int bodyCount = 0;

		for (int i = 0; i < celestialBodies.size(); i++)
		{
			celestialBodies[i].clear();

			if (i == celestialBodies.size() - 1)
			{
				celestialBodies[i].reserve(count + remainder);

				for (int j = 0; j < (count + remainder); j++)
				{
					OrbitalBody body = *_celestialBodies[bodyCount];
					celestialBodies[i].push_back(body);

					bodyCount++;
				}
			}

			else
			{
				celestialBodies[i].reserve(count);

				for (int j = 0; j < count; j++)
				{
					OrbitalBody body = *_celestialBodies[bodyCount];
					celestialBodies[i].push_back(body);

					bodyCount++;
				}
			}
		}

		for (std::vector<OrbitalBody>& v : allCelestialBodies1)
		{
			v.clear();
			v.reserve(_celestialBodies.size());

			for (std::shared_ptr<OrbitalBody>& body : _celestialBodies)
			{
				v.push_back(*body);
			}
		}

		for (std::vector<OrbitalBody>& v : allCelestialBodies2)
		{
			v.clear();
			v.reserve(_celestialBodies.size());

			for (std::shared_ptr<OrbitalBody>& body : _celestialBodies)
			{
				v.push_back(*body);
			}
		}
	}

	for (std::vector<OrbitalBody>& v : allCelestialBodies1)
	{
		for (int i = 0; i < _celestialBodies.size(); i++)
		{
			v[i] = *_celestialBodies[i];
		}
	}

	for (std::vector<OrbitalBody>& v : allCelestialBodies2)
	{
		for (int i = 0; i < _celestialBodies.size(); i++)
		{
			v[i] = *_celestialBodies[i];
		}
	}

	if (nonCelestialBodyCount != _nonCelestialBodies.size())
	{
		nonCelestialBodyCount = _nonCelestialBodies.size();
		int count = nonCelestialBodyCount / threadNumber;
		int remainder = nonCelestialBodyCount % threadNumber;

		int bodyCount = 0;

		for (int i = 0; i < nonCelestialBodies.size(); i++)
		{
			nonCelestialBodies[i].clear();

			if (i == nonCelestialBodies.size() - 1)
			{
				nonCelestialBodies[i].reserve(count + remainder);

				for (int j = 0; j < (count + remainder); j++)
				{
					OrbitalBody body = *_nonCelestialBodies[bodyCount];
					nonCelestialBodies[i].push_back(body);

					bodyCount++;
				}
			}

			else
			{
				nonCelestialBodies[i].reserve(count);

				for (int j = 0; j < count; j++)
				{
					OrbitalBody body = *_nonCelestialBodies[bodyCount];
					nonCelestialBodies[i].push_back(body);

					bodyCount++;
				}
			}
		}
	}

	start.store(false, std::memory_order_release);
	ready.store(0, std::memory_order_release);
	done.store(0, std::memory_order_release);
	while (ready.load(std::memory_order_acquire) < threadNumber * 2 )
	{
		std::this_thread::yield();
	}
	start.store(true, std::memory_order_release);
	while (done.load(std::memory_order_acquire) < threadNumber * 2 )
	{
		std::this_thread::yield();
	}

	int bodyCount = 0;
	for (int i = 0; i < celestialBodies.size(); i++)
	{
		for (int j = 0; j < celestialBodies[i].size(); j++)
		{
			assert(celestialBodies[i][j].name == _celestialBodies[bodyCount]->name);
			*_celestialBodies[bodyCount] = celestialBodies[i][j];
			bodyCount++;
		}
	}

	bodyCount = 0;
	for (int i = 0; i < nonCelestialBodies.size(); i++)
	{
		for (int j = 0; j < nonCelestialBodies[i].size(); j++)
		{
			assert(nonCelestialBodies[i][j].name == _nonCelestialBodies[bodyCount]->name);
			*_nonCelestialBodies[bodyCount] = nonCelestialBodies[i][j];
			bodyCount++;
		}
	}

	_simTime += dt * updates;
}

std::weak_ptr<OrbitalBody> OrbitalSimulation::AddBody(OrbitalBody& body)
{
	if (body.celestialBody)
	{
		auto it = _celestialBodiesNames.find(body.name);
		if (it != _celestialBodiesNames.end())
		{
			LogColor("Body: " << body.name << " is already in the sim", LOG_YELLOW);
			return std::weak_ptr<OrbitalBody>();
		}

		_celestialBodies.push_back(std::make_shared<OrbitalBody>(body));
		_celestialBodiesNames.insert(body.name);

		return _celestialBodies[_celestialBodies.size() - 1];
	}

	auto it = _nonCelestialBodiesNames.find(body.name);
	if (it != _nonCelestialBodiesNames.end())
	{
		LogColor("Body: " << body.name << " is already in the sim", LOG_YELLOW);
		return std::weak_ptr<OrbitalBody>();
	}

	_nonCelestialBodies.push_back(std::make_shared<OrbitalBody>(body));
	_nonCelestialBodiesNames.insert(body.name);

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
				_celestialBodiesNames.erase(bodyPtr.lock()->name);
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
			_nonCelestialBodiesNames.erase(bodyPtr.lock()->name);
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

double OrbitalSimulation::GetTime() const
{
	return _simTime;
}

std::string OrbitalSimulation::GetDate() const
{
	return SecondsToDate(_simTime, epoch);
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

    output += "--Date:" + SecondsToDate(_simTime, epoch) + "\n";

    for (const std::shared_ptr<OrbitalBody>& body : _celestialBodies)
    {
        output += "--Name:" + body->name;

        output += "--Parent:";

        bool parent = false;
        auto it = _celestialBodiesNames.find(body->parent);
        if (it != _celestialBodiesNames.end())
        {
        	output += body->parent;
        	parent = true;
        }

        else
        {
        	output += "Null";
        }
        
        output += "--CelestialBody:" + std::to_string(true);
        
        Vector3d pos = body->position;
        output += "--Position:";

        if (parent)
        {
	        for (int i = 0; i < _celestialBodies.size(); i++)
			{
				if (_celestialBodies[i]->name == body->parent)
				{
					pos -= _celestialBodies[i]->position;
				}
			}
		}

        output += std::to_string(pos.x) + "," + std::to_string(pos.y) + "," + std::to_string(pos.z);
 
 		Vector3d vel = body->velocity;       
        output += "--Velocity:";

        if (parent)
        {
	        for (int i = 0; i < _celestialBodies.size(); i++)
			{
				if (_celestialBodies[i]->name == body->parent)
				{
					vel -= _celestialBodies[i]->velocity;
				}
			}
		}

        output += std::to_string(vel.x) + "," + std::to_string(vel.y) + "," + std::to_string(vel.z);
        
        output += "--Mass:" + std::to_string(body->mass);
        
        output += "--Radius:" + std::to_string(body->radius);
        
        output += "---\n";
    }

    for (const std::shared_ptr<OrbitalBody>& body : _nonCelestialBodies)
    {
        output += "--Name:" + body->name;

        output += "--Parent:";

        bool parent = false;
        auto it = _nonCelestialBodiesNames.find(body->parent);
        if (it != _nonCelestialBodiesNames.end())
        {
        	output += body->parent;
        	parent = true;
        }

        else
        {
        	output += "Null";
        }
        
        
        output += "--CelestialBody:" + std::to_string(false);
        
       	Vector3d pos = body->position;
        output += "--Position:";

        if (parent)
        {
	        for (int i = 0; i < _celestialBodies.size(); i++)
			{
				if (_celestialBodies[i]->name == body->parent)
				{
					pos -= _celestialBodies[i]->position;
				}
			}
		}

        output += std::to_string(pos.x) + "," + std::to_string(pos.y) + "," + std::to_string(pos.z);
 
 		Vector3d vel = body->velocity;       
        output += "--Velocity:";

        if (parent)
        {
	        for (int i = 0; i < _celestialBodies.size(); i++)
			{
				if (_celestialBodies[i]->name == body->parent)
				{
					vel -= _celestialBodies[i]->velocity;
				}
			}
		}

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

	std::string date;

	std::string name;
	std::string parent;
	bool CelestialBody;

	Vector3d pos;
	Vector3d vel;

	double mass;
	double radius;

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

		if (buffer == "--Date:")
		{
			for (int ii = i + 1; ii < fileLength; ii++)
			{
				// Ending of a section with "--"
				if (fileText[ii] == '-' && fileText[ii + 1] == '-')
				{
					buffer.clear();

					i = ii - 1;

					break;					
				}

				date += fileText[ii];
			}
		}

		if (buffer == "--Name:")
		{
			for (int ii = i + 1; ii < fileLength; ii++)
			{
				// Ending of a section with "--"
				if (fileText[ii] == '-' && fileText[ii + 1] == '-')
				{
					buffer.clear();

					i = ii - 1;

					break;					
				}

				name += fileText[ii];
			}
		}

		if (buffer == "--Parent:")
		{
			for (int ii = i + 1; ii < fileLength; ii++)
			{
				// Ending of a section with "--"
				if (fileText[ii] == '-' && fileText[ii + 1] == '-')
				{
					buffer.clear();

					i = ii - 1;

					break;					
				}

				parent += fileText[ii];
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

					numberS.clear();

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

					numberS.clear();

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

					numberS.clear();

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

					numberS.clear();

					i = ii - 1;

					break;
				}

				numberS += fileText[ii];
			}
		}

		if (buffer == "---")
		{
			if (parent != "Null")
			{
				for (int i = 0; i < _celestialBodies.size(); i++)
				{
					if (_celestialBodies[i]->name == parent)
					{
						pos += _celestialBodies[i]->position;
						vel += _celestialBodies[i]->velocity;
						break;
					}
				}
			}

			OrbitalBody body(name, CelestialBody, pos, vel, mass, radius);
			body.parent = parent;
 			AddBody(body);

	 		name.clear();
	 		parent.clear();
	 		buffer.clear();
		}
	}

	_simTime = DateToSeconds(date, epoch);

	if (_simTime < 0)
	{
		Log("Bad Date: " + date);
		_simTime = 0;
	}

	UnloadFileText(fileText);
	return true;
}