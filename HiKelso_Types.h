
enum Step_State {
    STEP_PLAY,
    STEP_EDIT,
    STEP_PAUSE,
    STEP_MAX //not a real type, just for indexing
};

enum Free_State {
    FREE_CHORD = 0,
    FREE_ROOT,
    FREE_INACTIVE
};