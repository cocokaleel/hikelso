#include "daisy_pod.h"
#include "daisysp.h"
#include "Scales.h"

using namespace daisy;
using namespace daisysp;
using namespace seed;

class FreeChord {
    public:
        void Init(float samplerate);
        void IncrementInstrument();
        void ShiftRoot(int shift);
        void SetDegreePressed(int degree, uint8_t joystickSector);
        void ClearPress();
        float GetSamples();

    private:
        Oscillator          osc[4];
        uint8_t             oscNum;
        int                 root = 48;
        Scales              scales;
};