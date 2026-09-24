
namespace HiKelso {
    enum Seq_State {
        SEQ_PLAY,
        SEQ_EDIT,
        SEQ_PAUSE,
        SEQ_MAX
    };

    enum Free_State {
        FREE_CHORD = 0,
        FREE_ROOT,
        FREE_INACTIVE,
    };

    enum Mode {
        SEQ,
        FREE
    };

    enum ModeButtonEvent{
        SEQ_EVENT,
        FREE_EVENT
    };
}

using namespace HiKelso;

class HiKelso_State {
    public:
        void UpdateState(ModeButtonEvent buttonEvent);
        HiKelso::Mode mode = FREE;
        Free_State free = FREE_CHORD;
        Seq_State seq = SEQ_PAUSE;
    private: 
};