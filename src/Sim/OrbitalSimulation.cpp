#include "OrbitalSimulation.h"
#include "Services.h"
#include "EventHandler.h"
#include "GameStateHandler.h"

#include "raylib.h"

#include "Log.h"

#include <cassert>
#include <array>
#include <cmath>
#include <cstring>
#include <algorithm>

// Unit in m
const double G = 6.67430e-11;

// Unit in Km
const double GKm = 6.67430e-20;

const unsigned int maxSpeed = 100e3;

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

Vector3d OrbitalSimulation::CalculateTotalAcceleration(const Vector3d& position, OrbitalBody& body, std::deque<CelestialBody>& bodies)
{
	Vector3d acceleration = Vector3dZero();
	std::pair<double, CelestialBody*> topForce = std::make_pair(0, nullptr);

	for (CelestialBody& celestialBody : bodies)
	{
		if (body.name != celestialBody.name)
		{
			Vector3d r = position - celestialBody.position;

			Vector3d v = CalculateAcceleration(r, celestialBody.mass);
			acceleration += v;

			float strength = v.length();
			if (topForce.first < strength && body.mass < celestialBody.mass)
			{
				topForce.first = strength;
				topForce.second = &celestialBody;
			}
		}
	}

	body.parent = topForce.second;

	return acceleration;
}

void OrbitalSimulation::RungeKutta(OrbitalBody& body, std::deque<CelestialBody>& bodies, const double& h)
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

    if (body.thrust != Vector3dZero())
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

void OrbitalSimulation::CalculateOrbitalParamaters(CelestialBody* body)
{
	if (body->parent)
	{
		Vector3d position = body->position - body->parent->position;
		Vector3d velocity = body->velocity - body->parent->velocity;

		double mu = G * body->parent->mass;

		if (_km)
		{
			mu = GKm * body->parent->mass;
		}

	    Vector3d h = position.cross(velocity);
	    double h_mag = h.length();

	    Vector3d e = ((velocity.cross(h) / mu) - position.normalize());
	    double e_mag = e.length();
	    body->eccentricity = e_mag;

	    double en = ((velocity.length() * velocity.length()) / 2.0) - (mu / position.length());
	    body->semiMajorAxis = -(mu / (2.0 * en));

	    body->inclination = std::acos(h.z / h_mag);

	    Vector3d n = {-h.y, h.x, 0};
	    double n_mag = n.length();

	    body->longitudeAscendingNode = std::atan2(n.y, n.x);
	    if (body->longitudeAscendingNode < 0)
	    {
	        body->longitudeAscendingNode += 2.0 * PI;
	    }
	    if (body->longitudeAscendingNode >= 2.0 * PI)
	    {
	    	body->longitudeAscendingNode -= 2.0 * PI;
	    }

	    body->argumentOfPeriapsis = std::acos(std::clamp(n.dot(e) / (n_mag * body->eccentricity), -1.0, 1.0));
	    if (e.z < 0)
	    {
	        body->argumentOfPeriapsis = 2.0 * PI - body->argumentOfPeriapsis;
	    }

	    body->trueAnomaly = std::acos(std::clamp(e.dot(position) / (body->eccentricity * position.length()), -1.0, 1.0));
	    if (position.dot(velocity) < 0)
	    {
	        body->trueAnomaly = 2.0 * PI - body->trueAnomaly;
	    }

	    Log(body->name << " " << body->semiMajorAxis << " " << body->eccentricity << " " << body->inclination << " " << body->argumentOfPeriapsis << " " << body->longitudeAscendingNode << " " << body->trueAnomaly);
	}
}

void OrbitalSimulation::UpdateCelestialBody(std::deque<CelestialBody>& bodies, const double dt)
{
	for (CelestialBody& body : bodies)
	{
		if (body.parent)
		{
		    double mu = G * body.parent->mass;

		    if (_km)
		    {
		    	mu = GKm * body.parent->mass;
		    }

		    double n = sqrt(mu / pow(body.semiMajorAxis, 3));
		    double E0 = 2.0 * atan(sqrt((1.0 - body.eccentricity) / (1.0 + body.eccentricity)) * tan(body.trueAnomaly / 2.0));
			double M0 = E0 - body.eccentricity * sin(E0);
		    double M = M0 + n * dt;
		   	if (M > 2.0 * PI)
		    {
		    	M -= 2.0 * PI;
		    }

		    double E = M;
		    double deltaE = 1e-9;
		    for (int i = 0; i < 100; i++)
		    {
		    	double delta = E - body.eccentricity * sin(E) - M;

		    	if (std::fabs(delta) < deltaE)
		    	{
		    		break;
		    	}

		    	E -= delta / (1.0 - body.eccentricity * cos(E));
		    }

		    body.trueAnomaly = 2.0 * std::atan2(sqrt(1.0 + body.eccentricity) * sin(E / 2.0), sqrt(1.0 - body.eccentricity) * cos(E / 2.0));

		    double r = body.semiMajorAxis * (1.0 - body.eccentricity * body.eccentricity) / (1.0 + body.eccentricity * cos(body.trueAnomaly));
		    double x_orbital = r * cos(body.trueAnomaly);
		    double y_orbital = r * sin(body.trueAnomaly);

		    double cosOmega = cos(body.longitudeAscendingNode);
		    double sinOmega = sin(body.longitudeAscendingNode);
		    double cosi = cos(body.inclination);
		    double sini = sin(body.inclination);
		    double cosw = cos(body.argumentOfPeriapsis);
		    double sinw = sin(body.argumentOfPeriapsis);

		    Vector3d position = body.parent->position;

		    position.x += (cosOmega * cosw - sinOmega * sinw * cosi) * x_orbital + (-cosOmega * sinw - sinOmega * cosw * cosi) * y_orbital;
		    position.y += (sinOmega * cosw + cosOmega * sinw * cosi) * x_orbital + (-sinOmega * sinw + cosOmega * cosw * cosi) * y_orbital;
		    position.z += (sinw * sini) * x_orbital + (cosw * sini) * y_orbital;

		    body.position = position;

		    double v = sqrt(mu * (2.0 / r - 1.0 / body.semiMajorAxis));
		    double h = sqrt(mu * body.semiMajorAxis * (1.0 - body.eccentricity * body.eccentricity));

		    Vector3d orbitalVelocity = {-v * sin(body.trueAnomaly), v * (body.eccentricity + cos(body.trueAnomaly)), 0.0};

		    Vector3d rotatedOrbitalVelocity = {orbitalVelocity.x * cosw - orbitalVelocity.y * sinw, orbitalVelocity.x * sinw + orbitalVelocity.y * cosw, orbitalVelocity.z};
		    Vector3d rotatedInclinedVelocity = {rotatedOrbitalVelocity.x, rotatedOrbitalVelocity.y * cosi - rotatedOrbitalVelocity.z * sini, rotatedOrbitalVelocity.y * sini + rotatedOrbitalVelocity.z * cosi};

		    Vector3d velocity = body.parent->velocity;

		    velocity.x += rotatedInclinedVelocity.x * cosOmega - rotatedInclinedVelocity.y * sinOmega;
		    velocity.y += rotatedInclinedVelocity.x * sinOmega + rotatedInclinedVelocity.y * cosOmega;
    		velocity.z += rotatedInclinedVelocity.z;

		    body.velocity = velocity;
	    }
	}
}

void OrbitalSimulation::UpdateOrbitalBody(std::deque<std::shared_ptr<OrbitalBody>>& bodies, std::deque<CelestialBody>& celestialBodies, const double dt)
{
	for (std::shared_ptr<OrbitalBody>& body : bodies)
	{
		RungeKutta(*body, celestialBodies, dt);
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
	if (deltaT > 0.066666)
	{
		deltaT = 0.066666;

		static int i = 0;
		i++;
		if (i >= 60)
		{
			i = 0;
			LogColor("FPS bellow 15! Sim will remain at 0.06 dt", LOG_YELLOW);
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

	std::deque<CelestialBody> celestialBodies = _celestialBodies;

	for (int i = 0; i < updates; i++)
	{
		UpdateOrbitalBody(_orbitalBodies, _celestialBodies, dt);
		UpdateCelestialBody(celestialBodies, dt);
	}

	_celestialBodies = celestialBodies;

	_simTime += dt * updates;
}

CelestialBody* OrbitalSimulation::AddCelestialBody(const CelestialBody& body)
{
	CelestialBody* pointer = nullptr;

	auto it = _celestialBodiesMap.find(body.name);
	if (it == _celestialBodiesMap.end())
	{
		_celestialBodies.push_back(body);
		pointer = &_celestialBodies[_celestialBodies.size() - 1];
		_celestialBodiesMap[body.name] = pointer;
	}

	return pointer;
}

std::weak_ptr<OrbitalBody> OrbitalSimulation::AddOrbitalBody(const OrbitalBody& body)
{
	std::weak_ptr<OrbitalBody> pointer;

	auto it = _orbitalBodiesMap.find(body.name);
	if (it == _orbitalBodiesMap.end())
	{
		_orbitalBodies.push_back(std::make_shared<OrbitalBody>(body));
		pointer = _orbitalBodies[_orbitalBodies.size() - 1];
		_orbitalBodiesMap[body.name] = pointer;
	}

	return pointer;
}

bool OrbitalSimulation::RemoveOrbitalBody(std::weak_ptr<OrbitalBody>& bodyPtr)
{
	if (auto ptr = bodyPtr.lock())
	{
		auto mapit = _orbitalBodiesMap.find(ptr->name);
		if (mapit != _orbitalBodiesMap.end())
		{
			_orbitalBodiesMap.erase(ptr->name);

			auto dequeit = std::find(_orbitalBodies.begin(), _orbitalBodies.end(), ptr);
			if (dequeit != _orbitalBodies.end())
			{
				_orbitalBodies.erase(dequeit);
				return true;
			}
		}
	}

	return false;
}

std::vector<CelestialBody*> OrbitalSimulation::GetCelestialBodies()
{
	std::vector<CelestialBody*> vector;
	vector.reserve(_celestialBodies.size());

	for (CelestialBody& body : _celestialBodies)
	{
		vector.push_back(&body);
	}

	return vector;
}

std::unordered_map<std::string, CelestialBody*> OrbitalSimulation::GetCelestialBodiesMap()
{
	return _celestialBodiesMap;
}

std::vector<std::weak_ptr<OrbitalBody>> OrbitalSimulation::GetOrbitalBodies()
{
	std::vector<std::weak_ptr<OrbitalBody>> vector;
	vector.reserve(_orbitalBodies.size());

	for (std::shared_ptr<OrbitalBody>& body : _orbitalBodies)
	{
		vector.push_back(body);
	}

	return vector;
}

std::unordered_map<std::string, std::weak_ptr<OrbitalBody>> OrbitalSimulation::GetOrbitalBodiesMap()
{
	return _orbitalBodiesMap;
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
		for (CelestialBody& body : _celestialBodies)
		{
			body.position *= 1000;
			body.velocity *= 1000;
			body.radius *= 1000;
		}

		for (std::shared_ptr<OrbitalBody>& body : _orbitalBodies)
		{
			body->position *= 1000;
			body->velocity *= 1000;
		}
	}

	else
	{
		for (CelestialBody& body : _celestialBodies)
		{
			body.position *= 0.001;
			body.velocity *= 0.001;
			body.radius *= 0.001;
		}

		for (std::shared_ptr<OrbitalBody>& body : _orbitalBodies)
		{
			body->position *= 0.001;
			body->velocity *= 0.001;
		}
	}

	_km = km;
}

bool OrbitalSimulation::SaveBodiesToFile(const std::string& path)
{
    std::string output;

    output += "--Date:" + SecondsToDate(_simTime, epoch) + "\n";

    output += "--CelestialBodies\n";

    for (const CelestialBody& body : _celestialBodies)
    {
        output += "--Name:" + body.name;

        output += "--Parent:";

        bool parent = false;
        if (body.parent)
        {
	        auto it = _celestialBodiesMap.find(body.parent->name);
	        if (it != _celestialBodiesMap.end())
	        {
	        	output += body.parent->name;
	        	parent = true;
	        }
	    }
		
		if (!parent)
        {
        	output += "Null";
        }
        
        Vector3d pos = body.position;
        output += "--Position:";

        if (parent)
        {
	        auto it = _celestialBodiesMap.find(body.parent->name);
	        if (it != _celestialBodiesMap.end())
			{
				pos -= it->second->position;
			}
		}

        output += DoubleToRoundedString(pos.x, std::numeric_limits<double>::max_digits10) + "," + DoubleToRoundedString(pos.y, std::numeric_limits<double>::max_digits10) + "," + DoubleToRoundedString(pos.z, std::numeric_limits<double>::max_digits10);
 
 		Vector3d vel = body.velocity;       
        output += "--Velocity:";

        if (parent)
        {
	        auto it = _celestialBodiesMap.find(body.parent->name);
	        if (it != _celestialBodiesMap.end())
			{
				vel -= it->second->velocity;
			}
		}

        output += DoubleToRoundedString(vel.x, std::numeric_limits<double>::max_digits10) + "," + DoubleToRoundedString(vel.y, std::numeric_limits<double>::max_digits10) + "," + DoubleToRoundedString(vel.z, std::numeric_limits<double>::max_digits10);
        
        output += "--Mass:" + DoubleToRoundedString(body.mass, std::numeric_limits<double>::max_digits10);
        
        output += "--Radius:" + DoubleToRoundedString(body.radius, std::numeric_limits<double>::max_digits10);

        output += "--SemiMajorAxis:" +  DoubleToRoundedString(body.semiMajorAxis, std::numeric_limits<double>::max_digits10);

        output += "--Eccentricity:" +  DoubleToRoundedString(body.eccentricity, std::numeric_limits<double>::max_digits10);

        output += "--Inclination:" +  DoubleToRoundedString(body.inclination, std::numeric_limits<double>::max_digits10);

        output += "--ArgumentOfPeriapsis:" +  DoubleToRoundedString(body.argumentOfPeriapsis, std::numeric_limits<double>::max_digits10);

        output += "--LongitudeAscendingNode:" +  DoubleToRoundedString(body.longitudeAscendingNode, std::numeric_limits<double>::max_digits10);

        output += "--TrueAnomaly:" +  DoubleToRoundedString(body.trueAnomaly, std::numeric_limits<double>::max_digits10);
        
        output += "---\n";
    }

    output += "--OrbitalBodies\n";

    for (const std::shared_ptr<OrbitalBody>& body : _orbitalBodies)
    {
        output += "--Name:" + body->name;

        output += "--Parent:";

        bool parent = false;
        if (body->parent)
        {
	        auto it = _celestialBodiesMap.find(body->parent->name);
	        if (it != _celestialBodiesMap.end())
	        {
	        	output += body->parent->name;
	        	parent = true;
	        }
	    }

        if(!parent)
        {
        	output += "Null";
        }
        
       	Vector3d pos = body->position;
        output += "--Position:";

        if (parent)
        {
	        auto it = _celestialBodiesMap.find(body->parent->name);
	        if (it != _celestialBodiesMap.end())
			{
				pos -= it->second->position;
			}
		}

        output += DoubleToRoundedString(pos.x, std::numeric_limits<double>::max_digits10) + "," + DoubleToRoundedString(pos.y, std::numeric_limits<double>::max_digits10) + "," + DoubleToRoundedString(pos.z, std::numeric_limits<double>::max_digits10);
 
 		Vector3d vel = body->velocity;       
        output += "--Velocity:";

        if (parent)
        {
	        auto it = _celestialBodiesMap.find(body->parent->name);
	        if (it != _celestialBodiesMap.end())
			{
				vel -= it->second->position;
			}
		}

        output += DoubleToRoundedString(vel.x, std::numeric_limits<double>::max_digits10) + "," + DoubleToRoundedString(vel.y, std::numeric_limits<double>::max_digits10) + "," + DoubleToRoundedString(vel.z, std::numeric_limits<double>::max_digits10);
        
        output += "--Mass:" + DoubleToRoundedString(body->mass, std::numeric_limits<double>::max_digits10);
        
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
	bool celestialBody = true;

	Vector3d pos = Vector3dZero();
	Vector3d vel = Vector3dZero();

	double mass = 0;
	double radius = 0;

	double semiMajorAxis = 0;
    double eccentricity = 0;
    double inclination = 0;
    double argumentOfPeriapsis = 0;
    double longitudeAscendingNode = 0;
    double trueAnomaly = 0;

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

		if (buffer == "--CelestialBodies")
		{
			buffer.clear();

			celestialBody = true;
		}

		if (buffer == "--OrbitalBodies")
		{
			buffer.clear();

			celestialBody = false;
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

		if (buffer == "--SemiMajorAxis:")
		{
			for (int ii = i + 1; ii < fileLength; ii++)
			{
				// Ending of a section with "--"
				if (fileText[ii] == '-' && fileText[ii + 1] == '-')
				{
					semiMajorAxis = std::stod(numberS);

					buffer.clear();

					numberS.clear();

					i = ii - 1;

					break;
				}

				numberS += fileText[ii];
			}
		}

		if (buffer == "--Eccentricity:")
		{
			for (int ii = i + 1; ii < fileLength; ii++)
			{
				// Ending of a section with "--"
				if (fileText[ii] == '-' && fileText[ii + 1] == '-')
				{
					eccentricity = std::stod(numberS);

					buffer.clear();

					numberS.clear();

					i = ii - 1;

					break;
				}

				numberS += fileText[ii];
			}
		}

		if (buffer == "--Inclination:")
		{
			for (int ii = i + 1; ii < fileLength; ii++)
			{
				// Ending of a section with "--"
				if (fileText[ii] == '-' && fileText[ii + 1] == '-')
				{
					inclination = std::stod(numberS);

					buffer.clear();

					numberS.clear();

					i = ii - 1;

					break;
				}

				numberS += fileText[ii];
			}
		}

		if (buffer == "--ArgumentOfPeriapsis:")
		{
			for (int ii = i + 1; ii < fileLength; ii++)
			{
				// Ending of a section with "--"
				if (fileText[ii] == '-' && fileText[ii + 1] == '-')
				{
					argumentOfPeriapsis = std::stod(numberS);

					buffer.clear();

					numberS.clear();

					i = ii - 1;

					break;
				}

				numberS += fileText[ii];
			}
		}

		if (buffer == "--LongitudeAscendingNode:")
		{
			for (int ii = i + 1; ii < fileLength; ii++)
			{
				// Ending of a section with "--"
				if (fileText[ii] == '-' && fileText[ii + 1] == '-')
				{
					longitudeAscendingNode = std::stod(numberS);

					buffer.clear();

					numberS.clear();

					i = ii - 1;

					break;
				}

				numberS += fileText[ii];
			}
		}

		if (buffer == "--TrueAnomaly:")
		{
			for (int ii = i + 1; ii < fileLength; ii++)
			{
				// Ending of a section with "--"
				if (fileText[ii] == '-' && fileText[ii + 1] == '-')
				{
					trueAnomaly = std::stod(numberS);

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
				auto it = _celestialBodiesMap.find(parent);
		        if (it != _celestialBodiesMap.end())
		        {
						pos += it->second->position;
						vel += it->second->velocity;
				}
			}

			if (celestialBody)
			{
				auto it = _celestialBodiesMap.find(name);
	        	if (it != _celestialBodiesMap.end())
	        	{
	        		it->second->position = pos;
	        		it->second->velocity = vel;

	        		it->second->semiMajorAxis = semiMajorAxis;
	        		it->second->eccentricity = eccentricity;
	        		it->second->inclination = inclination;
	        		it->second->argumentOfPeriapsis = argumentOfPeriapsis;
	        		it->second->longitudeAscendingNode = longitudeAscendingNode;
	        		it->second->trueAnomaly = trueAnomaly;
	        	}

				else
				{
					CelestialBody body(name, pos, vel, mass, radius);

					if (parent != "Null")
					{
						auto it = _celestialBodiesMap.find(parent);
		        		if (it != _celestialBodiesMap.end())
		        		{
		        			body.parent = it->second;
		        		}
					}

					CelestialBody* ptr = AddCelestialBody(body);

					if (semiMajorAxis > 0)
					{
						ptr->semiMajorAxis = semiMajorAxis;
		        		ptr->eccentricity = eccentricity;
		        		ptr->inclination = inclination;
		        		ptr->argumentOfPeriapsis = argumentOfPeriapsis;
		        		ptr->longitudeAscendingNode = longitudeAscendingNode;
		        		ptr->trueAnomaly = trueAnomaly;
					}
					else
					{
						CalculateOrbitalParamaters(ptr);
					}
				}
			}

			else
			{
				auto it = _orbitalBodiesMap.find(name);
	        	if (it != _orbitalBodiesMap.end())
	        	{
	        		if (auto ptr = it->second.lock())
	        		{
		        		ptr->position = pos;
		        		ptr->velocity = vel;
		        	}
		        }

				else
				{
					OrbitalBody body(name, pos, vel, mass);

					if (parent != "Null")
					{
						auto it = _celestialBodiesMap.find(parent);
		        		if (it != _celestialBodiesMap.end())
		        		{
		        			body.parent = it->second;
		        		}
					}

					AddOrbitalBody(body);
				}
			}

	 		name.clear();
	 		parent.clear();
	 		buffer.clear();

	 		pos = Vector3dZero();
			vel = Vector3dZero();

			mass = 0;
			radius = 0;

			semiMajorAxis = 0;
		    eccentricity = 0;
		    inclination = 0;
		    argumentOfPeriapsis = 0;
		    longitudeAscendingNode = 0;
		    trueAnomaly = 0;
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