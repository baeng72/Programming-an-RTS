#pragma once
#include <functional>
enum class EventType{WindowResize,WindowClose,KeyPressed,KeyReleased,KeyTyped,MouseButtonPressed,MouseButtonReleased,MouseScrolled,MouseMoved};

struct Event {
	EventType type;
	bool handled{ false };
	Event(EventType t) :type(t) { }
};

struct WindowResizeEvent : public Event {
	uint32_t width;
	uint32_t height;
	WindowResizeEvent(uint32_t w, uint32_t h) :Event(EventType::WindowResize),width(w), height(h) {

	}
};
struct WindowCloseEvent : public Event {
	WindowCloseEvent() :Event(EventType::WindowClose) {}
};
struct KeyPressedEvent : public Event {
	int key;
	bool repeat{ false };
	KeyPressedEvent(int k, bool r) :Event(EventType::KeyPressed),key(k), repeat(r) {}
};
struct KeyReleasedEvent : public Event {
	int key;
	KeyReleasedEvent(int k):Event(EventType::KeyReleased), key(k) {}
};
struct KeyTypedEvent : public Event {
	int key;
	KeyTypedEvent(int k) :Event(EventType::KeyTyped), key(k) {}
};
struct MouseButtonPressedEvent : public Event {
	int button;
	MouseButtonPressedEvent(int b) :Event(EventType::MouseButtonPressed), button(b) {}
};
struct MouseButtonReleasedEvent : public Event {
	int button;
	MouseButtonReleasedEvent(int b) :Event(EventType::MouseButtonReleased), button(b) {}
};
struct MouseScrolledEvent : public Event {	
	float xOffset, yOffset;
	MouseScrolledEvent(float xo,float yo) :Event(EventType::MouseScrolled), xOffset(xo),yOffset(yo) {}
};
struct MouseMovedEvent : public Event {	
	float xPos, yPos;
	MouseMovedEvent(float xp,float yp) :Event(EventType::MouseMoved), xPos(xp), yPos(yp) {}
};



class EventDispatcher {
	Event& _ev;
	
public:
	EventDispatcher(Event& ev):_ev(ev) {

	
	}
	
	template<typename T,typename F>
	bool Dispatch(EventType type,const F& func) {
		if (type == _ev.type) {
			_ev.handled |= func(static_cast<T&>(_ev));
			return true;
		}
		return false;
	}
	
};
