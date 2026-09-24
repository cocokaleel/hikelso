#include "HiKelso_State.h"

using namespace HiKelso;

void HiKelso_State::UpdateState(ModeButtonEvent buttonEvent) {
    Mode requestedMode;
    if (buttonEvent == SEQ_EVENT) {
        requestedMode = SEQ;
    } else {
        requestedMode = FREE;
    }
    // if button clicked isn't current mode, switch mode
    if (requestedMode != mode) {
        if (buttonEvent == FREE_EVENT) {
            mode = FREE;
            free = FREE_CHORD;
            if (seq == SEQ_EDIT) {
                seq = SEQ_PLAY;
            }
        }
        if (buttonEvent == SEQ_EVENT) {
            mode = SEQ;
            free = FREE_INACTIVE;
            seq = SEQ_PLAY;
        }
        return;
    }


    // button event is mode
    switch(buttonEvent) {
        case FREE_EVENT:
            if (free == FREE_CHORD) {free = FREE_ROOT;}
            else if (free == FREE_ROOT) {free = FREE_CHORD;}
            else {free = FREE_ROOT;} //error case, but recover 
            break;
        case SEQ_EVENT:
            seq = (Seq_State)(seq+1);
            if (seq == SEQ_MAX) {seq = (Seq_State)0;}
            break;
    }
}