#include "daisy_pod.h"
#include "daisysp.h"
#include "HiKelso_Controls.h"
#include "HiKelso_State.h"
#include "FreeChord.h"
#include "FreeRoot.h"

// #include "moogladder.h"

using namespace daisy;
using namespace daisysp;
using namespace seed;


enum Chord_Flavors {
    Maj, Minor, Aug, Dim,
    Maj7, min7, dom7,
    dim7, NUM_CHORDS
};

enum Chord_Degree {
    THIRD,
    FIFTH,
    SEVENTH,
    NUM_DEGREES
};

DaisySeed           hw;
int                 freq      = 0;
Oscillator          seqOsc;
AdEnv               env;
Metro               tick;
MoogLadder          flt;
int                 activeDegree = 8;
Oscillator          osc[4];
uint8_t             oscNum;
int                 notes[4];
int                 chordNum = 0;
float               tickFrequency;
HiKelso_Controls    controls;
HiKelso_State       state;
FreeChord           chordMachine;
FreeRoot            rootMachine;
uint8_t             joystickSector;

bool    editCycle;
uint8_t seqStep;
uint8_t wave;
float   dec[8];
int     seqPitches[8];
bool    seqActive[8];
float   env_out;

void NextSamples(float &sig);
void UpdateControls();
void ProcessNewInstrumentButton();
void SetVolumeAndDegree();

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
            NextSamples(sig);
        }
        if (state.free == FREE_CHORD) {
            sig += chordMachine.GetSamples();
        }
        if (state.free == FREE_ROOT) {
            sig += rootMachine.GetSamples();
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

    // Start the controls
    controls.Start();
    hw.StartAudio(AudioCallback);

    controls.SetFreePlayLED(true);
    hw.StartLog();

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

void SetActiveSeqStep() {
    bool newPress = controls.ProcessDegreeButtons();

    uint8_t activeButton = controls.GetActiveDegreeButton();
    if (newPress) {
        if (seqStep == activeButton) { //indicates a re-press
            seqActive[seqStep] = !seqActive[seqStep]; // flip if the step is seqActive
            editCycle = seqActive[seqStep]; // align edit cycle with the seqActive level
        } else if (activeButton < 8) { // not a repress, but the button is valid (button is pressed)
            controls.SetDegreeLED(seqStep, false);
            seqStep = activeButton;
            controls.SetDegreeLED(seqStep, true);
            editCycle = seqActive[seqStep]; // align edit cycle with the seqActive level
        }
    }
}

void SetVolumeAndDegree() {
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
    editCycle = false;
    seqStep = 0;
    if (state.free == FREE_CHORD) {
        controls.SetFreePlayLED(true);
    }
    if (state.seq == SEQ_PLAY) {
        controls.SetSeqLED(true);
        controls.SetDegreeLED(seqStep, true);
    }
    if (state.seq == SEQ_EDIT) {
        editCycle = seqActive[seqStep];
        controls.SetDegreeLED(seqStep, true);
    }
}

void UpdateSequencerParams() {
    if (state.seq == SEQ_EDIT) {
        seqPitches[seqStep] += controls.GetEncoderIncrement();
        seqOsc.SetFreq(mtof(seqPitches[seqStep]));
    } else if (state.seq == SEQ_PLAY) {
        tickFrequency += controls.GetEncoderIncrement();
        if (tickFrequency < 1) {
            tickFrequency = 1.0;
        }
        tick.SetFreq(tickFrequency);
    }
    flt.SetFreq(controls.GetFilterFrequency());
}

void UpdateControls()
{
    ProcessMode();
    if (state.mode == FREE) {
        SetVolumeAndDegree();
    }
    if (state.mode == SEQ) {
        UpdateSequencerParams();
        if (state.seq == SEQ_EDIT) {
            SetActiveSeqStep();
        }
    }

    ProcessNewInstrumentButton();
}


void NextSamples(float &sig)
{
    env_out = env.Process();
    seqOsc.SetAmp(env_out);
    sig = seqOsc.Process();
    sig = flt.Process(sig);

    if(tick.Process() && state.seq == SEQ_PLAY)
    {
        controls.SetDegreeLED(seqStep, false);
        seqStep++;
        seqStep %= 8;
        controls.SetDegreeLED(seqStep, true);
        if(seqActive[seqStep])
        {
            env.Trigger();
        }
    }

    if(seqActive[seqStep])
    {
        env.SetTime(ADENV_SEG_DECAY, dec[seqStep]);
        seqOsc.SetFreq(mtof(seqPitches[seqStep]));
    }
    if(!env.IsRunning() && editCycle)
    {
        env.Trigger();
    }
}