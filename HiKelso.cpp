#include "daisy_pod.h"
#include "daisysp.h"
#include "HiKelso_Types.h"
#include "HiKelso_Controls.h"
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
int                 chord[NUM_CHORDS][NUM_DEGREES];
int                 chordNum = 0;
int                 majorFrequencies[8] = {0, 2, 4, 5, 7, 9, 11, 12};
int                 frequencies[8] = {0, 2, 4, 5, 7, 9, 11, 12};
Chord_Flavors       defaultChord[8] = {Maj, Minor, Minor, Maj, Maj, Minor, Dim, Maj};
int                 root = 48;
float               tickFrequency;
HiKelso_Controls    controls;
Free_State          freePlayState = FREE_CHORD;
Step_State          seqState = STEP_PAUSE;

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

    for(int i = 0; i < 4; i++)
    {
        osc[i].SetFreq(mtof(notes[i]));
    }

    // Audio Loop
    for(size_t i = 0; i < size; i += 2)
    {
        float sig = 0;
        if (seqState == STEP_EDIT || seqState == STEP_PLAY) {
            NextSamples(sig);
        }
        for(int i = 0; i < 4; i++)
        {
            sig += osc[i].Process();
        }

        out[i]     = sig;
        out[i + 1] = sig;
    }
}

void InitSynth(float samplerate)
{
    oscNum = Oscillator::WAVE_SIN;
    for(int i = 0; i < 4; i++)
    {
        osc[i].Init(samplerate);
        osc[i].SetAmp(0.0f);
        osc[i].SetWaveform(oscNum);
        notes[i] = 60;
    }
}

void InitChords()
{
    //set thirds
    for(int i = 0; i < 8; i++)
    {
        //every other chord, maj third, Minor third
        chord[i][THIRD] = 3 + ((i + 1) % 2);
    }
    //set fifths
    // perfect 5th
    chord[Maj][FIFTH] = chord[Minor][FIFTH] = chord[Maj7][FIFTH] = chord[min7][FIFTH] = chord[dom7][FIFTH] = 7;
    // diminished 5th
    chord[Dim][FIFTH] = chord[dim7][FIFTH] = 6;
    // augmented 5th
    chord[Aug][FIFTH] = 8;

    //set sevenths
    // triads (octave since triad has no 7th)
    chord[Maj][SEVENTH] = chord[Minor][SEVENTH] = chord[Aug][SEVENTH] = chord[Dim][SEVENTH] = 12;
    // major 7th
    chord[Maj7][SEVENTH] = 11;
    // minor 7th
    chord[min7][SEVENTH] = chord[dom7][SEVENTH] = 10;
    // diminished 7th
    chord[dim7][SEVENTH] = 9;
}

int main(void)
{
    float samplerate;

    // Init everything
    hw.Init();
    hw.SetAudioBlockSize(4);
    samplerate = hw.AudioSampleRate();
    controls.Init(&hw);
    InitSynth(samplerate);
    InitChords();
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
        if (seqState == STEP_EDIT) {
            controls.ToggleSeqLED();
            System::Delay(200);
            hw.PrintLine("seq edit");
        }
        // else {
        //     hw.PrintLine("SeqState: %d", seqState);
        //     System::Delay(200);
        // } 
        
        if (freePlayState == FREE_ROOT) {
            controls.ToggleFreePlayLED();
            System::Delay(200);
            hw.PrintLine("free root");
        }
    }
}

void UpdateChord()
{
    uint8_t joystickSector = controls.GetJoystickAngleNumber();
    chordNum = (joystickSector >= 8 && activeDegree < 8) ? defaultChord[activeDegree] : joystickSector;

    // if the joystick is moved during free root playing, change the third, fifth, and seventh to the chord degrees
    if (freePlayState == FREE_ROOT && joystickSector <= 8) {

        frequencies[2] = chord[chordNum][THIRD];
        frequencies[4] = chord[chordNum][FIFTH];

        switch(chordNum) {
            case Maj7:
            case min7:
            case dim7:
            case dom7:
                frequencies[6] = chord[chordNum][SEVENTH];
            break;
            default:
                frequencies[6] = chord[Maj7][SEVENTH];
            break;
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

    uint8_t activeButton = controls.GetActiveDegreeButton();
    if (activeDegree != activeButton) {
        controls.SetDegreeLED(activeDegree, false);
        activeDegree = activeButton;
        if (activeButton < 8) {
            controls.SetDegreeLED(activeDegree, true);
            // a button other than the active button is pressed
            if (freePlayState == FREE_CHORD) {
                for(int i = 0; i < 4; i++) // turn on all oscs
                {
                    osc[i].SetAmp(0.1);
                }
            } else if (freePlayState == FREE_ROOT) {
                osc[0].SetAmp(0.4); // turn on root osc
            }
        } else { // no button is actively pressed
            for(int i = 0; i < 4; i++) // turn off all oscs
            {
                osc[i].SetAmp(0);
            }
        }
    }
}

void ProcessNewInstrumentButton() {
    if (controls.NewInstrumentRequested()) {
        oscNum = oscNum == (Oscillator::WAVE_LAST-1) ? 0 : (oscNum+1);
        for(int i = 0; i < 4; i++)
        {
            osc[i].SetWaveform(oscNum);
        }
    }
}

void ProcessMode() {
    if (controls.GetFreePlaySwitchPressed()) {
        if (freePlayState == FREE_INACTIVE) { // free play pressed from seq mode 
            controls.TurnOffAllLEDs();
            freePlayState = FREE_CHORD;
            controls.SetFreePlayLED(true);
            hw.PrintLine("free chord!");
            if (seqState == STEP_EDIT) {
                hw.PrintLine("seq paused");
                seqState = STEP_PAUSE;
            }
        } else { // free play re-pressed
            freePlayState = (Free_State)(freePlayState + 1);
            if (freePlayState == FREE_INACTIVE) {
                freePlayState = FREE_CHORD;
                controls.SetFreePlayLED(true);
            }
        }
        if (freePlayState == FREE_CHORD) {
            frequencies[2] = chord[Maj7][THIRD];
            frequencies[4] = chord[Maj7][FIFTH];
            frequencies[6] = chord[Maj7][SEVENTH];
        }
    } else if (controls.GetSeqSwitchPressed()) {
        seqStep = 0;

        if (freePlayState != FREE_INACTIVE) { // seq mode pressed from free play
            controls.SetFreePlayLED(false);
            seqState = STEP_PLAY;
            freePlayState = FREE_INACTIVE;
            hw.PrintLine("seq play");
        } else { // seq mode re-pressed
            controls.TurnOffAllLEDs();
            
            seqState = (Step_State)(seqState + 1);
            if (seqState >= STEP_MAX) seqState = (Step_State)0;

            switch(seqState) {
                case STEP_PLAY:
                    hw.PrintLine("step play");
                break;
                case STEP_EDIT:
                    hw.PrintLine("step edit");
                break;
                case STEP_PAUSE:
                    hw.PrintLine("step paused");
                break;
                default:
                    hw.PrintLine("OH NO");
                break;
            }
        }
        controls.SetSeqLED(false);
        if (seqState != STEP_PAUSE) {
            controls.SetDegreeLED(seqStep, true);
        }
        if (seqState != STEP_EDIT) {
            editCycle = false;
        } else {
            editCycle = seqActive[seqStep];
        }
        if (seqState == STEP_PLAY) {
            controls.SetSeqLED(true);
        }
    }
}

void UpdateSequencerParams() {
    if (seqState == STEP_EDIT) {
        seqPitches[seqStep] += controls.GetEncoderIncrement();
        seqOsc.SetFreq(mtof(seqPitches[seqStep]));
    } else if (seqState == STEP_PLAY) {
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
    if (freePlayState!= FREE_INACTIVE) {
        SetVolumeAndDegree();
        UpdateChord();
        // shift root
        root += controls.GetEncoderIncrement();

        if (activeDegree < 8) {
            freq = root+frequencies[activeDegree];
            notes[0] = freq;
            notes[1] = freq + chord[chordNum][0];
            notes[2] = freq + chord[chordNum][1];
            notes[3] = freq + chord[chordNum][2];
        }
    }
    if (freePlayState == FREE_INACTIVE) {
        UpdateSequencerParams();
        if (seqState == STEP_EDIT) {
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

    if(tick.Process() && seqState == STEP_PLAY)
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