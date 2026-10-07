#include "Sequencer.h"

void Sequencer::Init(float samplerate) {
    seqStep = 0;
    
    tickFrequency   = 3.f;

    tick.Init(3, samplerate);

    soundLine[SOUND_SINE] = new SequencerSine;
    soundLine[SOUND_BASS] = new SequencerBassDrum;
    soundLine[SOUND_SNARE] = new SequencerSnare;
    soundLine[SOUND_SAW] = new SequencerSaw;

    for(int i = 0; i < SOUND_MAX; i++)
    {
        soundLine[i]->Init(samplerate);
    }

    activeEditingSoundLine = 0;
}

void Sequencer::ProcessEdit() { //Triggers no matter what - lights display if step on or off
    soundLine[activeEditingSoundLine]->Trigger(false, seqStep);
}

bool Sequencer::ProcessMetronome() {
    if (tick.Process()) {
        seqStep++;
        seqStep %= 8;

        for (int i = 0; i < SOUND_MAX; i++) {
            if (soundLine[i]->GetActive(seqStep)) {
                soundLine[i]->Trigger(true, seqStep);
            }
        }
        return true;
    }
    return false;
}

float Sequencer::GetSample() {
    float sig = 0;

    for (int i = 0; i < SOUND_MAX; i++) {
        // calculate this 0.25 from SOUND_MAX
        sig += 0.25 * soundLine[i]->GetSample();
    }

    return sig;
}

uint8_t Sequencer::GetActiveStep() {
    return seqStep;
}

void Sequencer::SetActiveSeqStep(uint8_t buttonPressed) {
    if (seqStep != buttonPressed && soundLine[activeEditingSoundLine]->GetActive(buttonPressed)) {
        // first press of already active button should not toggle it off, just select it
        seqStep = buttonPressed;
    } else {
        // otherwise, select the new button (doesn't matter if it's already set) and flip it
        seqStep = buttonPressed;
        soundLine[activeEditingSoundLine]->ToggleActive(seqStep); // flip if the step is seqActive
    }
}

void Sequencer::TurnOnEditMode() {
    editCycle = true;
}


bool Sequencer::GetStepActive(uint8_t step) {
    return soundLine[activeEditingSoundLine]->GetActive(step);
}

void Sequencer::IncrementTickFrequency(int increment) {
    tickFrequency += increment;
    if (tickFrequency < 1) {
        tickFrequency = 1.0;
    }
    tick.SetFreq(tickFrequency);
}

void Sequencer::IncrementActivePitch(int increment) {
    soundLine[activeEditingSoundLine]->IncrementQuality(increment, seqStep);
}

void Sequencer::SetFilterFrequencer(float freq) {
    soundLine[activeEditingSoundLine]->SetFilter(freq);
}

void Sequencer::SetEditCycle(bool newEditCycle) {
    editCycle = newEditCycle;
}


void Sequencer::IncrementInstrument() {
    activeEditingSoundLine++;
    if (activeEditingSoundLine >= SOUND_MAX) {
        activeEditingSoundLine = SOUND_SINE;
    }
}