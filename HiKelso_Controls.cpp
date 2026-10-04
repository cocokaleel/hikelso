#include "HiKelso_Controls.h"
#define M_PI 3.14159265358979323846
#include <math.h>

void HiKelso_Controls::InitDegreeButtons() {
    degreeButtons[0].Init(D19);
    degreeButtons[1].Init(D21);
    degreeButtons[2].Init(D3);
    degreeButtons[3].Init(D5);
    degreeButtons[4].Init(D28);
    degreeButtons[5].Init(D26);
    degreeButtons[6].Init(D24);
    degreeButtons[7].Init(D2);
}

void HiKelso_Controls::InitDegreeLEDs() {
    degreeLEDs[0].Init(D20, GPIO::Mode::OUTPUT, GPIO::Pull::NOPULL, GPIO::Speed::LOW);
    degreeLEDs[1].Init(D22, GPIO::Mode::OUTPUT, GPIO::Pull::NOPULL, GPIO::Speed::LOW);
    degreeLEDs[2].Init(D4, GPIO::Mode::OUTPUT, GPIO::Pull::NOPULL, GPIO::Speed::LOW);
    degreeLEDs[3].Init(D6, GPIO::Mode::OUTPUT, GPIO::Pull::NOPULL, GPIO::Speed::LOW);
    degreeLEDs[4].Init(D27, GPIO::Mode::OUTPUT, GPIO::Pull::NOPULL, GPIO::Speed::LOW);
    degreeLEDs[5].Init(D25, GPIO::Mode::OUTPUT, GPIO::Pull::NOPULL, GPIO::Speed::LOW);
    degreeLEDs[6].Init(D23, GPIO::Mode::OUTPUT, GPIO::Pull::NOPULL, GPIO::Speed::LOW);
    degreeLEDs[7].Init(D1, GPIO::Mode::OUTPUT, GPIO::Pull::NOPULL, GPIO::Speed::LOW);

    seqLED.Init(D11, GPIO::Mode::OUTPUT, GPIO::Pull::NOPULL, GPIO::Speed::LOW);
    freeLED.Init(D13, GPIO::Mode::OUTPUT, GPIO::Pull::NOPULL, GPIO::Speed::LOW);
}

void HiKelso_Controls::InitModeSwitches() {
    freePlaySwitch.Init(D12);
    seqModeSwitch.Init(D10);
}

void HiKelso_Controls::InitRootButtons() {
    rootEncoder.Init(D8, D7, D9);
}

void HiKelso_Controls::SetFreePlayLED(bool on) {
    freeLED.Write(on);
}

void HiKelso_Controls::SetSeqLED(bool on) {
    seqLED.Write(on);
}

void HiKelso_Controls::ToggleFreePlayLED() {
    freeLED.Toggle();
}

void HiKelso_Controls::ToggleSeqLED() {
    seqLED.Toggle();
}

void HiKelso_Controls::TurnOffAllLEDs() {
    TurnOffDegreeLEDs();
    freeLED.Write(false);
    seqLED.Write(false);
}

void HiKelso_Controls::TurnOffDegreeLEDs() {
    for (int i = 0; i < 8; i++) {
        degreeLEDs[i].Write(false);
    }
}

void HiKelso_Controls::Init(DaisySeed *hw) {
    hardware = hw;
    InitModeSwitches();
    InitDegreeButtons();
    InitDegreeLEDs();
    InitRootButtons();
    InitADCs();
}

uint8_t HiKelso_Controls::GetJoystickAngleNumber() {
    float floatX = hardware->adc.GetFloat(0) - 0.498993; //TODO: grab these calib values on startup
    float floatY = hardware->adc.GetFloat(1) - 0.486191;

    if (floatX < 0.3 && floatX > -0.3 && floatY < 0.3 && floatY > -0.3) {
        return 8;
    } else {
        return 8*(atan2(floatX, floatY) + M_PI)/(2*M_PI);
    }
}


void HiKelso_Controls::InitADCs() {
    //Configure pin 21 as an ADC input. This is where we'll read the knob.
    adcInputs[0].InitSingle(D16);//joystick x
    adcInputs[1].InitSingle(D15);//joystick y
    adcInputs[2].InitSingle(D17);

    //Initialize the adc with the config we just made
    hardware->adc.Init(adcInputs, 3);
    filterControl.Init(hardware->adc.GetPtr(2), hardware->AudioSampleRate());
    filterFreqParam.Init(filterControl, 100, 10000, filterFreqParam.LOGARITHMIC);
}


bool HiKelso_Controls::NewInstrumentRequested() {
    // rootEncoder.Debounce(); // Debounce likely not needed because happens when checking increment
    return rootEncoder.FallingEdge();

    // changeInstSwitch.Debounce();
    // return changeInstSwitch.FallingEdge();
}

bool HiKelso_Controls::GetFreePlaySwitchPressed() {
    freePlaySwitch.Debounce();
    return freePlaySwitch.FallingEdge();
}

int32_t HiKelso_Controls::GetEncoderIncrement() {

    rootEncoder.Debounce();
    return rootEncoder.Increment();
}


void HiKelso_Controls::SetDegreeLED(uint8_t i, bool on) {
    degreeLEDs[i].Write(on);   
}


bool HiKelso_Controls::ProcessDegreeButtons() {
    bool newPress = false;
    for (int i = 0; i < 8; i++) {
        degreeButtons[i].Debounce();
        if (degreeButtons[i].RisingEdge()) {
            activeDegreeButton = i;
            newPress = true;
        } 
        if (degreeButtons[i].FallingEdge()) {
            // degreeLEDs[i].Write(false);
            if (activeDegreeButton == i) {
                // it is possible for both of these functions to fire in the same for loop, so only turn off amp if the active degree is still the one getting turned off
                activeDegreeButton = 8;
            }
        }
    }
    return newPress;
}

uint8_t HiKelso_Controls::GetActiveDegreeButton() {
    return activeDegreeButton;
}

float HiKelso_Controls::GetFilterFrequency() {
    k2 = filterControl.Process();

    if(abs(k2 - oldk2) > 0.00001)
    {
        filterFreq = filterFreqParam.Process();
    }

    return filterFreq;
}

bool HiKelso_Controls::GetSeqSwitchPressed() {
    seqModeSwitch.Debounce();
    return seqModeSwitch.RisingEdge();
}