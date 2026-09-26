#include "FreeChord.h"

void FreeChord::Init(float samplerate) {
    oscNum = Oscillator::WAVE_SIN;
    for(int i = 0; i < 4; i++)
    {
        osc[i].Init(samplerate);
        osc[i].SetAmp(0.0f);
        osc[i].SetWaveform(oscNum);
    }
}

void FreeChord::IncrementInstrument() {
    oscNum = oscNum == (Oscillator::WAVE_LAST-1) ? 0 : (oscNum+1);
    for(int i = 0; i < 4; i++)
    {
        osc[i].SetWaveform(oscNum);
    }
}

void FreeChord::SetDegreePressed(int degree, uint8_t joystickSector) {
    Scales::Scale_Flavors flavor;
    int degreePitchShift = scales.getPitch(Scales::Maj7, (Scales::Scale_Degree)degree);
    if (joystickSector == Scales::NUM_SCALES) { //if joystick is centered, play default flavor for degree
        flavor = scales.getDefaultFlavor((Scales::Scale_Degree)degree);
    } else { // otherwise play selected flavor for degree
        flavor = (Scales::Scale_Flavors) joystickSector;
    }
    // set each 1 3 5 to correct pitch
    osc[0].SetFreq(mtof(root + degreePitchShift + scales.getPitch(flavor, Scales::FIRST)));
    osc[1].SetFreq(mtof(root + degreePitchShift + scales.getPitch(flavor, Scales::THIRD)));
    osc[2].SetFreq(mtof(root + degreePitchShift + scales.getPitch(flavor, Scales::FIFTH)));

    // if 7th chord, set seventh, otherwise set to 8ths
    switch (flavor) {
        case (Scales::Maj7):
        case (Scales::min7):
        case (Scales::dom7):
        case (Scales::dim7):
            osc[3].SetFreq(mtof(root + degreePitchShift + scales.getPitch(flavor, Scales::SEVENTH)));
            break;
        default:
            osc[3].SetFreq(mtof(root + degreePitchShift + scales.getPitch(flavor, Scales::EIGHTH)));
    }

    for(int i = 0; i < 4; i++) // turn on all oscs
    {
        osc[i].SetAmp(0.1);
    }
}

void FreeChord::ClearPress() {
    for(int i = 0; i < 4; i++) // turn on all oscs
    {
        osc[i].SetAmp(0);
    }
}


void FreeChord::ShiftRoot(int shift) {
    root += shift;
}


float FreeChord::GetSamples() {
    return osc[0].Process() + osc[1].Process() + osc[2].Process() + osc[3].Process();
}