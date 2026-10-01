#include "Sequencer.h"

void Sequencer::Init(float samplerate) {

    tickFrequency   = 3.f;

    seqOsc.Init(samplerate);
    env.Init(samplerate);
    tick.Init(3, samplerate);
    flt.Init(samplerate);


    //Osc parameters
    seqOsc.SetWaveform(seqOsc.WAVE_TRI);

    //Envelope parameters
    env.SetTime(ADENV_SEG_ATTACK, 0.02);
    env.SetMin(0.0);
    env.SetMax(0.8);

    //Set filter parameters
    flt.SetFreq(10000.f);
    flt.SetRes(0.7);


    for(int i = 0; i < 8; i++)
    {
        dec[i]    = .5;
        seqActive[i] = true;
        seqPitches[i]  = 60;
    }
}

void Sequencer::ProcessEdit() {
    if(!env.IsRunning() && editCycle)
    {
        env.Trigger();
    }
}

bool Sequencer::ProcessMetronome() {
    if (tick.Process()) {
        seqStep++;
        seqStep %= 8;

        if(seqActive[seqStep])
        {
            env.Trigger();
        }
        if(seqActive[seqStep])
        {
            env.SetTime(ADENV_SEG_DECAY, dec[seqStep]);
            seqOsc.SetFreq(mtof(seqPitches[seqStep]));
        }
        return true;
    }
    return false;
}

float Sequencer::GetSample() {
    float sig;

    seqOsc.SetAmp(env.Process());
    sig = seqOsc.Process();
    sig = flt.Process(sig);

    return sig;
}

uint8_t Sequencer::GetActiveStep() {
    return seqStep;
}

void Sequencer::SetActiveSeqStep(uint8_t buttonPressed) {
    if (seqStep == buttonPressed) { //indicates a re-press
        seqActive[seqStep] = !seqActive[seqStep]; // flip if the step is seqActive
        editCycle = seqActive[seqStep]; // align edit cycle with the seqActive level
    } else if (buttonPressed < 8) { // not a repress, but the button is valid (button is pressed)
        seqStep = buttonPressed;
        editCycle = seqActive[seqStep]; // align edit cycle with the seqActive level
    }
}

void Sequencer::TurnOnEditMode() {
    editCycle = seqActive[seqStep];
}

void Sequencer::IncrementTickFrequency(int increment) {
    tickFrequency += increment;
    if (tickFrequency < 1) {
        tickFrequency = 1.0;
    }
    tick.SetFreq(tickFrequency);
}

void Sequencer::IncrementActivePitch(int increment) {
    seqPitches[seqStep] += increment;
    seqOsc.SetFreq(mtof(seqPitches[seqStep]));
}

void Sequencer::SetFilterFrequencer(float freq) {
    flt.SetFreq(freq);
}

void Sequencer::SetEditCycle(bool newEditCycle) {
    editCycle = newEditCycle;
}


void Sequencer::IncrementInstrument() {
    
}