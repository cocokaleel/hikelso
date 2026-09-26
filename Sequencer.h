#include "daisy_pod.h"
#include "daisysp.h"
// #include "moogladder.h"

using namespace daisy;
using namespace daisysp;
using namespace seed;

class Sequencer {
    public:
        void Init(float samplerate);
        void IncrementInstrument();
        float GetSample();
        uint8_t GetActiveStep();
        bool ProcessMetronome();
        void ProcessEdit();
        void SetActiveSeqStep(uint8_t buttonPressed);
        void IncrementTickFrequency(int increment);
        void IncrementActivePitch(int increment);
        void SetFilterFrequencer(float freq);
        void SetEditCycle(bool newEditCycle);
        void TurnOnEditMode();
    private:
        Oscillator          seqOsc;
        AdEnv               env;
        Metro               tick;
        bool    editCycle;
        uint8_t seqStep;
        uint8_t wave;
        float   dec[8];
        int     seqPitches[8];
        bool    seqActive[8];
        MoogLadder          flt;
        float               tickFrequency;
};