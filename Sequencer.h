#include "daisy_pod.h"
#include "daisysp.h"
#include "SequencerSounds.h"
// #include "moogladder.h"

using namespace daisy;
using namespace daisysp;
using namespace seed;

class Sequencer {
    enum SoundIndex {
        SOUND_SINE,
        SOUND_BASS,
        SOUND_SNARE,
        SOUND_MAX
    };

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
        Metro               tick;
        bool                editCycle;
        uint8_t             seqStep;
        SequencerSound      soundLine[3];
        uint8_t             activeEditingSoundLine;
        MoogLadder          flt;
        float               tickFrequency;
};