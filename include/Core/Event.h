#pragma once
#include <memory>

class Event {
public:
    virtual ~Event() = default;
};

class EventListener {
public:
    EventListener* _ptr;

    virtual void AddSelfAsListener() = 0;
    virtual void OnEvent(std::shared_ptr<const Event>& event) = 0;

    EventListener() : _ptr(this) {};
    ~EventListener() { _ptr = nullptr;}
};