#include "seizure_tflite.h"

#if ENABLE_ML
Eloquent::TF::Sequential<TF_NUM_OPS, ML_ARENA_SIZE> gMl;
#endif
