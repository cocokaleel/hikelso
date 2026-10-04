#include "daisy_pod.h"
#include "daisysp.h"
#include "HiKelso_Controls.h"
#include "HiKelso_State.h"
#include "FreeChord.h"
#include "FreeRoot.h"
#include "Sequencer.h"


using namespace daisy;
using namespace daisysp;
using namespace seed;

DaisySeed           hw;
int                 activeDegree = 8;
uint8_t             joystickSector;
HiKelso_Controls    controls;
HiKelso_State       state;
FreeChord           chordMachine;
FreeRoot            rootMachine;
Sequencer           seqMachine;

void UpdateControls();
void ProcessNewInstrumentButton();
void FreePlayControls();
void SequencerControls();
void ProcessMode();

static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    UpdateControls();

    // Audio Loop
    for(size_t i = 0; i < size; i += 2)
    {
        float sig = 0;
        if (state.seq == SEQ_EDIT || state.seq == SEQ_PLAY) {
            sig += 0.5 * seqMachine.GetSample();
        }
        if (state.free == FREE_CHORD) {
            sig += 0.5 * chordMachine.GetSamples();
        } else if (state.free == FREE_ROOT) {
            sig += 0.5 * rootMachine.GetSamples();
        }

        out[i]     = sig;
        out[i + 1] = sig;
    }
}

int main(void)
{
    float samplerate;

    // Init everything
    hw.Init();
    hw.SetAudioBlockSize(4);
    samplerate = hw.AudioSampleRate();
    controls.Init(&hw);
    chordMachine.Init(samplerate);
    rootMachine.Init(samplerate);
    seqMachine.Init(samplerate);

    // Start the controls
    controls.Start();
    hw.StartAudio(AudioCallback);

    controls.SetFreePlayLED(true);

    while(1) {
        if (state.seq == SEQ_EDIT) {
            controls.ToggleSeqLED();
            System::Delay(200); 
        }
        if (state.free == FREE_ROOT) {
            controls.ToggleFreePlayLED();
            System::Delay(200);
        }
    }
}

void FreePlayControls() {
    controls.ProcessDegreeButtons();
    uint8_t newJoystickSector = controls.GetJoystickAngleNumber();
    uint8_t activeButton = controls.GetActiveDegreeButton();

        // shift root
    int encoderInc = controls.GetEncoderIncrement();
    if (activeDegree != activeButton || newJoystickSector != joystickSector || encoderInc != 0) {
        rootMachine.ShiftRoot(encoderInc);
        chordMachine.ShiftRoot(encoderInc);
        
        controls.SetDegreeLED(activeDegree, false);
        activeDegree = activeButton;
        joystickSector = newJoystickSector;
        if (activeButton < 8) {
            controls.SetDegreeLED(activeDegree, true);
            // a button other than the active button is pressed
            if (state.free == FREE_CHORD) {
                chordMachine.SetDegreePressed(activeDegree, joystickSector);
            } else if (state.free == FREE_ROOT) {
                rootMachine.SetDegreePressed(activeDegree, joystickSector);
            }
        } else { // no button is actively pressed
            if (state.free == FREE_CHORD) {
                chordMachine.ClearPress();
            } else if (state.free == FREE_ROOT) {
                rootMachine.ClearPress();
            }
        }
    }

    // set filter
    if (state.free == FREE_CHORD) {
        chordMachine.SetFilterFrequencer(controls.GetFilterFrequency());

    } else if (state.free == FREE_ROOT) {
        rootMachine.SetFilterFrequencer(controls.GetFilterFrequency());
    }
}

void ProcessNewInstrumentButton() {
    if (controls.NewInstrumentRequested()) {
        if (state.free == FREE_CHORD) {
            chordMachine.IncrementInstrument();
        } else if (state.free == FREE_ROOT) {
            rootMachine.IncrementInstrument();
        }
    }
}

void ProcessMode() {
    if (controls.GetFreePlaySwitchPressed()) {
        state.UpdateState(FREE_EVENT);
    } else if (controls.GetSeqSwitchPressed()) {
        state.UpdateState(SEQ_EVENT);
    } else {
        return;
    }

    // only if one of the buttons is pressed (implies mode change)
    controls.TurnOffAllLEDs();
    seqMachine.SetEditCycle(false);
    if (state.free == FREE_CHORD) {
        controls.SetFreePlayLED(true);
    }
    if (state.seq == SEQ_PLAY) {
        controls.SetSeqLED(true);
        controls.SetDegreeLED(seqMachine.GetActiveStep(), true);
    }
    if (state.seq == SEQ_EDIT) {
        seqMachine.TurnOnEditMode();
        controls.SetDegreeLED(seqMachine.GetActiveStep(), true);
    }
}

void SequencerControls() {
    if (state.seq == SEQ_EDIT) {
        if (controls.ProcessDegreeButtons()) { //true when a new press detected
            uint8_t activeButton = controls.GetActiveDegreeButton();
            controls.TurnOffDegreeLEDs();
            controls.SetDegreeLED(activeButton, true);
            seqMachine.SetActiveSeqStep(activeButton);
        }
        seqMachine.IncrementActivePitch(controls.GetEncoderIncrement());
        seqMachine.ProcessEdit();
    } else if (state.seq == SEQ_PLAY) {
        seqMachine.IncrementTickFrequency(controls.GetEncoderIncrement());
    }
    seqMachine.SetFilterFrequencer(controls.GetFilterFrequency());
}

void UpdateControls()
{
    ProcessMode();
    if (state.mode == FREE) {
        FreePlayControls();
    }
    if (state.mode == SEQ) {
        SequencerControls();
    }

    uint8_t prevSeqLed = seqMachine.GetActiveStep();
    if (state.seq == SEQ_PLAY && seqMachine.ProcessMetronome()) {
        controls.SetDegreeLED(prevSeqLed, false);
        controls.SetDegreeLED(seqMachine.GetActiveStep(), true);
    }

    ProcessNewInstrumentButton();
}