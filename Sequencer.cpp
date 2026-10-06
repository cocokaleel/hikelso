#include "Sequencer.h"

void Sequencer::Init(float samplerate) {

    tickFrequency   = 3.f;

    tick.Init(3, samplerate);

    soundLine[SOUND_SINE] = new SequencerSine;
    soundLine[SOUND_BASS] = new SequencerBassDrum;
    soundLine[SOUND_SNARE] = new SequencerSnare;

    for(int i = 0; i < SOUND_MAX; i++)
    {
        soundLine[i]->Init(samplerate);
    }

    activeEditingSoundLine = 0;
}

void Sequencer::ProcessEdit() {
    if(editCycle)
    {
        soundLine[activeEditingSoundLine]->Trigger(false, seqStep);
    }
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
        // calculate this 0.33 from SOUND_MAX
        sig += 0.33 * soundLine[i]->GetSample();
    }

    return sig;
}

uint8_t Sequencer::GetActiveStep() {
    return seqStep;
}

void Sequencer::SetActiveSeqStep(uint8_t buttonPressed) {
    if (seqStep == buttonPressed) { //indicates a re-press
        soundLine[activeEditingSoundLine]->ToggleActive(seqStep); // flip if the step is seqActive
        editCycle = soundLine[activeEditingSoundLine]->GetActive(seqStep); // align edit cycle with the seqActive level
            
    } else if (buttonPressed < 8) { // not a repress, but the button is valid (button is pressed)
        seqStep = buttonPressed;
        editCycle = soundLine[activeEditingSoundLine]->GetActive(seqStep); // align edit cycle with the seqActive level
    }
}

void Sequencer::TurnOnEditMode() {
    editCycle = soundLine[activeEditingSoundLine]->GetActive(seqStep);
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