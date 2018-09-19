#include "midi.hpp"
#include "utils.hpp"
#include "debug.hpp"
#include <RtMidi.h>
#include <map>
#include <mutex>

static std::unique_ptr<RtMidiIn> midi;

struct CallbackStore
{
	uint8_t mask;
	Midi::Callback callback;
};

static std::mutex callbackMutex;

static std::map<Midi::Handle, CallbackStore> callbacks;

static void midiCallback( double timeStamp, std::vector<unsigned char> *message, void *userData);

Midi::Handle Midi::RegisterCallback(uint8_t event, Callback const & fun)
{
	std::lock_guard<decltype(callbackMutex)> _lock(callbackMutex);
	static Handle maxHandle = 0;

	Handle h = ++maxHandle;
	callbacks.emplace(h, CallbackStore { event, fun });
	return h;
}

void Midi::UnregisterCallback(Handle h)
{
	std::lock_guard<decltype(callbackMutex)> _lock(callbackMutex);
	callbacks.erase(h);
}

void Midi::Initialize(nlohmann::json const & config)
{
	midi = std::make_unique<RtMidiIn>(RtMidi::UNSPECIFIED, "Midi-V");
	midi->setCallback(midiCallback, nullptr);

    Log() << "Available midi ports:";
    unsigned int const midiPortCount = midi->getPortCount();
    for(unsigned int i = 0; i < midiPortCount; i++)
    {
        Log() << "[" << i << "] " << midi->getPortName(i);
    }

    // Windows does not support virtual midi ports by-api
#ifndef MIDIV_WINDOWS
    if(Utils::get(config, "useVirtualMidi", true))
    {
        midi->openVirtualPort("Visualization Input");
    }
    else
#endif
    {
        midi->openPort(Utils::get(config, "midiPort", 0), "Visualization Input");
    }
	midi->ignoreTypes(true, true, true); // no time, no sense, no sysex
}

void midiCallback( double timeStamp, std::vector<unsigned char> * message, void * /*userData*/)
{
	Midi::Message msg;
	msg.event = message->at(0) & 0xF0;
	msg.channel = message->at(0) & 0x0F;
	msg.timestamp = timeStamp;
	std::copy(
		message->begin() + 1, message->end(),
		std::back_inserter(msg.payload));

	std::lock_guard<std::mutex> _lock(callbackMutex);

	for(auto const & _cb : callbacks)
	{
		auto const & cb = _cb.second;
		if((cb.mask == 0x00) || (cb.mask == msg.event))
			cb.callback(msg);
	}
}
