#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;
using namespace seed;

class SequencerSound {
    public:
        virtual void Init(float samplerate);
        virtual void Trigger(bool overrideRunning, int step);
        virtual float GetSample();
        virtual void SetFilter(float freq);
        virtual void IncrementQuality(int increment, int step);
        void ToggleActive(uint8_t step) {active[step] = !active[step];}
        bool GetActive(uint8_t step) {return active[step];}
    private:
        bool active[8] = {false};
};

class SequencerSine : public SequencerSound {
    public:
        SequencerSine(){}

        void Init(float samplerate) {
            for (int i = 0; i < 8; i++) {
                frequencies[i] = 60;
            }

            flt.Init(samplerate);

            //Set filter parameters
            flt.SetFreq(10000.f);
            flt.SetRes(0.7);
            seqOsc.Init(samplerate);
            env.Init(samplerate);
            //Osc parameters
            seqOsc.SetWaveform(seqOsc.WAVE_TRI);
            seqOsc.SetFreq(mtof(60));

            //Envelope parameters
            env.SetTime(ADENV_SEG_ATTACK, 0.02);
            env.SetTime(ADENV_SEG_DECAY, 0.5);
            env.SetMin(0.0);
            env.SetMax(0.8);
        }
        
        void IncrementQuality(int increment, int step) {
            frequencies[step] += increment;
        }

        void Trigger(bool overrideRunning, int step) {
            seqOsc.SetFreq(mtof(frequencies[step]));

            if (overrideRunning) {
                env.Trigger();
            } else if (!env.IsRunning()) {
                env.Trigger();
            }
        }
        float GetSample() {
            seqOsc.SetAmp(env.Process());
            return flt.Process(seqOsc.Process());
        }
        void SetFilter(float freq) {
            flt.SetFreq(freq);
        }
    private:
        Oscillator          seqOsc;
        AdEnv               env;
        MoogLadder          flt;
        int                 frequencies[8];
};

class SequencerBassDrum : public SequencerSound {
    public:
        SequencerBassDrum(){}

        void Init(float samplerate) {
            flt.Init(samplerate);

            //Set filter parameters
            flt.SetFreq(10000.f);
            flt.SetRes(0.7);
            //Initialize oscillator for kickdrum
            osc.Init(samplerate);
            osc.SetWaveform(Oscillator::WAVE_TRI);
            osc.SetAmp(1);

            //This envelope will control the kick oscillator's pitch
            //Note that this envelope is much faster than the volume
            kickPitchEnv.Init(samplerate);
            kickPitchEnv.SetTime(ADENV_SEG_ATTACK, .01);
            kickPitchEnv.SetTime(ADENV_SEG_DECAY, .05);
            kickPitchEnv.SetMax(400);
            kickPitchEnv.SetMin(50);

            //This one will control the kick's volume
            kickVolDecayLength = 1.0;
            kickVolEnv.Init(samplerate);
            kickVolEnv.SetTime(ADENV_SEG_ATTACK, .01);
            kickVolEnv.SetTime(ADENV_SEG_DECAY, kickVolDecayLength);
            kickVolEnv.SetMax(1);
            kickVolEnv.SetMin(0);
        }
        void IncrementQuality(int increment, int step) {
            kickVolDecayLength *= pow(0.97, increment);
            kickVolEnv.SetTime(ADENV_SEG_DECAY, kickVolDecayLength);
        }
        void Trigger(bool overrideRunning, int step) {
            if (overrideRunning) {
                kickVolEnv.Trigger();
                kickPitchEnv.Trigger();
            } else if (!kickVolEnv.IsRunning()) {
                kickVolEnv.Trigger();
                kickPitchEnv.Trigger();
            }
        }
        float GetSample() {
            //Apply the pitch envelope to the kick
            osc.SetFreq(kickPitchEnv.Process());
            //Set the kick volume to the envelope's output
            osc.SetAmp(kickVolEnv.Process());
            //Process the next oscillator sample
            return flt.Process(osc.Process());
        }
        void SetFilter(float freq) {
            flt.SetFreq(freq);
        }

    private:
        AdEnv kickVolEnv, kickPitchEnv;
        float       kickVolDecayLength;
        Oscillator osc;
        MoogLadder          flt;
};

class SequencerSnare : public SequencerSound {
    public:
        SequencerSnare(){}
        void Init(float samplerate) {
            flt.Init(samplerate);

            //Set filter parameters
            flt.SetFreq(10000.f);
            flt.SetRes(0.7);
            //Initialize noise
            noise.Init();

            //Initialize envelopes, this one's for the snare amplitude
            snareDecayTime = 0.2;
            snareEnv.Init(samplerate);
            snareEnv.SetTime(ADENV_SEG_ATTACK, .01);
            snareEnv.SetTime(ADENV_SEG_DECAY, snareDecayTime);
            snareEnv.SetMax(1);
            snareEnv.SetMin(0);
        }
        void IncrementQuality(int increment, int step) {
            snareDecayTime *= pow(0.97, increment);
            snareEnv.SetTime(ADENV_SEG_DECAY, snareDecayTime);
        }
        void Trigger(bool overrideRunning, int step) {
            if (overrideRunning) {
                snareEnv.Trigger();
            } else if (!snareEnv.IsRunning()) {
                snareEnv.Trigger();
            }
        }
        float GetSample() {return flt.Process(noise.Process() * snareEnv.Process());}
        void SetFilter(float freq) {
            flt.SetFreq(freq);
        }
    private:
        AdEnv snareEnv;
        WhiteNoise noise;
        MoogLadder          flt;
        float   snareDecayTime = 0.2;
};