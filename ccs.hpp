#pragma once

#include <cstdint>

enum class CC : uint8_t
{
    // http://nickfever.com/music/midi-cc-list
    BankSelect = 0,
    Modulation = 1,
    BreathControl = 2,
    FootControl = 4,
    PortamentoTime = 5,
    Volume     = 7,
    Balance    = 8,
    Pan        = 10,
    Expression = 11,
    EffectController1 = 12,
    EffectController2 = 13,
    DamperPedalSwitch = 64,
    SustainPedalSwitch = 64,
    PortamentoSwitch = 65,
    SostenutoSwitch  = 66,
    SoftPedalSwitch  = 67,
    LegatoSwitch = 68,
    Hold2 = 69,

    // Custom
    FilterReso = 71,
    Release    = 72,
    Attack     = 73,
    FilterFreq = 74 ,
    Reverb     = 91,
};
