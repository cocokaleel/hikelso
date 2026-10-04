#include "daisy_pod.h"
#include "daisysp.h"
#include "Scales.h"
using namespace daisy;
using namespace daisysp;
using namespace seed;

class FreeRoot {
    public:
        void Init(float samplerate);
        void IncrementInstrument();
        void ShiftRoot(int shift);
        void SetDegreePressed(int degree, uint8_t joystickSector);
        void ClearPress();
        float GetSamples();
        void SetFilterFrequencer(float freq);

    private:
        Oscillator          osc;
        uint8_t             oscNum;
        int                 note;
        int                 chordNum = 0;
        int                 root = 48;
        Scales              scales;
        MoogLadder          flt;
};