#include "EventHandler.h"
#include <algorithm>

EventHandler::EventHandler()
{
	
}

EventHandler::~EventHandler()
{

}

void EventHandler::ProcessEvent(std::unique_ptr<const Event> event)
{
	std::shared_ptr<const Event> eventPtr = std::move(event);

	// Go through listeners and send event also add dead ones to list
	for (int i = 0; i < _listeners.size(); i++)
	{
		_listeners[i]->OnEvent(eventPtr);
	}
}

void EventHandler::ProcessEventLocal(const std::string name, std::unique_ptr<const Event> event)
{
	// Go through the queue and send the event to all relevant listeners
	auto listeners = _localListeners.find(name);
	if (listeners != _localListeners.end())
	{

		std::shared_ptr<const Event> eventPtr = std::move(event);

		// Go through listeners and send event also add dead ones to list
		for (int i = 0; i < listeners->second.size(); i++)
		{

			listeners->second[i]->OnEvent(eventPtr);
		}
	}
}

void EventHandler::RemoveLocalListener(EventListener* listener)
{
	for (auto& localListenerPair : _localListeners)
	{

		auto& listeners = localListenerPair.second;

		auto it = std::find(listeners.begin(), listeners.end(), listener);

		if (it != listeners.end())
		{
			listeners.erase(it);
		}
	}
}

void EventHandler::AddEvent(std::unique_ptr<const Event> event)
{
	// Add the event to the next queue
	ProcessEvent(std::move(event));
}

void EventHandler::AddListener(EventListener* listener)
{
	// Add listner to the list
	_listeners.push_back(listener);
}

void EventHandler::RemoveListener(EventListener* listener)
{
	auto it = std::find(_listeners.begin(), _listeners.end(), listener);

	if (it != _listeners.end())
	{
		_listeners.erase(it);
	}

	RemoveLocalListener(listener);
}

void EventHandler::AddLocalEvent(const std::string name, std::unique_ptr<const Event> event)
{
	// Add the event to the correct next queue
	ProcessEventLocal(name, std::move(event));
}

void EventHandler::AddLocalListener(const std::string name, EventListener* listener)
{
	// Add listner to the correct list
	_localListeners[name].push_back(listener);
}