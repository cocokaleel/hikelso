#ifndef SCALES_H
#define SCALES_H

class Scales {
    public:
        enum Scale_Flavors {
            Maj, Minor, Aug, Dim,
            Maj7, min7, dom7,
            dim7, NUM_SCALES
        };

        enum Scale_Degree {
            FIRST = 0,
            SECOND,
            THIRD,
            FOURTH,
            FIFTH,
            SIXTH,
            SEVENTH,
            EIGHTH,
            NUM_DEGREES
        };
        Scales() {
            // all first degrees are 0 different from root
            for (int i = 0; i < NUM_SCALES; i++) {
                relative_pitches[i][FIRST] = 0;
                relative_pitches[i][SECOND] = 2;
                relative_pitches[i][FOURTH] = 5;
                relative_pitches[i][SIXTH] = 9;
                relative_pitches[i][EIGHTH] = 12;
            }

            //set thirds
            relative_pitches[Maj][THIRD] = relative_pitches[Maj7][THIRD] = relative_pitches[dom7][THIRD] = relative_pitches[Aug][THIRD] = 4;
            relative_pitches[Minor][THIRD] = relative_pitches[Dim][THIRD] = relative_pitches[min7][THIRD] = relative_pitches[dim7][THIRD] = 3;
            
            //set fifths
            // perfect 5th
            relative_pitches[Maj][FIFTH] = relative_pitches[Minor][FIFTH] = relative_pitches[Maj7][FIFTH] = relative_pitches[min7][FIFTH] = relative_pitches[dom7][FIFTH] = 7;
            // diminished 5th
            relative_pitches[Dim][FIFTH] = relative_pitches[dim7][FIFTH] = 6;
            // augmented 5th
            relative_pitches[Aug][FIFTH] = 8;

            //set sevenths
            // triads (octave since triad has no 7th)
            relative_pitches[Maj][SEVENTH] = relative_pitches[Minor][SEVENTH] = relative_pitches[Aug][SEVENTH] = relative_pitches[Dim][SEVENTH] = 11;
            // major 7th
            relative_pitches[Maj7][SEVENTH] = 11;
            // minor 7th
            relative_pitches[min7][SEVENTH] = relative_pitches[dom7][SEVENTH] = 10;
            // diminished 7th
            relative_pitches[dim7][SEVENTH] = 9;
        }

        int getPitch(Scale_Flavors flavor, Scale_Degree degree) {
            if (flavor >= NUM_SCALES) { //default to major scale (joystick optimized)
                return relative_pitches[Maj][degree];
            }
            return relative_pitches[flavor][degree];
        }

        Scale_Flavors getDefaultFlavor(Scale_Degree degree) {
            return defaultFlavors[degree];
        }
    private:
        const Scale_Flavors defaultFlavors[8] = {Maj, Minor, Minor, Maj, Maj, Minor, Dim, Maj};
        int relative_pitches[NUM_SCALES][NUM_DEGREES];
};

#endif