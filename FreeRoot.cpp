#include "FreeRoot.h"

void FreeRoot::Init(float samplerate) {
    oscNum = Oscillator::WAVE_SIN;
    osc.Init(samplerate);
    osc.SetAmp(0.0f);
    osc.SetWaveform(oscNum);
    //Set filter parameters
    flt.Init(samplerate);
    flt.SetFreq(10000.f);
    flt.SetRes(0.7);
}

void FreeRoot::SetDegreePressed(int degree, uint8_t joystickSector) {
    Scales::Scale_Flavors flavor;
    if (joystickSector == Scales::NUM_SCALES) {
        flavor = Scales::Maj;
    } else {
        flavor = (Scales::Scale_Flavors)joystickSector;
    }
    osc.SetFreq(mtof(root+scales.getPitch(flavor, (Scales::Scale_Degree)degree)));
    osc.SetAmp(0.4);
}

void FreeRoot::ClearPress() {
    osc.SetAmp(0);
}


void FreeRoot::SetFilterFrequencer(float freq) {
    flt.SetFreq(freq);
}


void FreeRoot::IncrementInstrument() {
    oscNum = oscNum == (Oscillator::WAVE_LAST-1) ? 0 : (oscNum+1);
    osc.SetWaveform(oscNum);
}


void FreeRoot::ShiftRoot(int shift) {
    root += shift;
}


float FreeRoot::GetSamples() {
    return flt.Process(osc.Process());;
}