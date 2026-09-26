#include "daisysp.h"
#include "daisy_pod.h"

using namespace daisy;
using namespace daisysp;
using namespace seed;

class HiKelso_Controls {
    public:
        HiKelso_Controls(){}
        ~HiKelso_Controls(){};

        void Init(DaisySeed *hw);

        void Start() {
            hardware->adc.Start();
        }

        bool NewInstrumentRequested();

        /**
         * @brief Returns the sector that the joystick is currently present in
         * 
         * @return 0-7 sectors, 8=default
         */
        uint8_t GetJoystickAngleNumber();

        /**
         * @brief Return how much the encoder has incremented
         * 
         * @return int32_t 
         */
        int32_t GetEncoderIncrement();

        /**
         * @brief processes the degree button presses
         * @return true if a new button is pressed
         */
        bool ProcessDegreeButtons();

        /**
         * @brief Return if the free play type switch was pressed
         * 
         * @return true 
         * @return false 
         */
        bool GetFreePlaySwitchPressed();

        bool GetSeqSwitchPressed();

        /**
         * @brief Get the number of the active degree button
         * 
         * @return uint8_t 8 if no active button, 0-7 if one of the active buttons
         */
        uint8_t GetActiveDegreeButton();

        void SetDegreeLED(uint8_t i, bool on);

        void TurnOffAllLEDs();
        void TurnOffDegreeLEDs();

        float GetFilterFrequency();

        void SetFreePlayLED(bool on);
        void SetSeqLED(bool on);

        void ToggleFreePlayLED();

        void ToggleSeqLED();

    private:
        void InitDegreeButtons();
        void InitDegreeLEDs();
        void InitChangeInstSwitch();
        void InitRootButtons();
        void InitADCs();

        AdcChannelConfig adcInputs[3]; //joystick adc 0-1, filterPot 2
        uint8_t     activeDegreeButton = 8;
        Switch      degreeButtons[8];
        GPIO        degreeLEDs[8];
        GPIO        seqLED;
        GPIO        freeLED;
        Encoder     rootEncoder;
        Switch      changeInstSwitch; 
        Switch      freePlaySwitch;
        Switch      seqModeSwitch;
        AnalogControl   filterControl;
        Parameter  filterFreqParam;
        DaisySeed   *hardware;
        float k2, oldk2 = 0.f;
        float filterFreq = 10000;
};