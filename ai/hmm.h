#ifndef HMM_H
#define HMM_H

#include "../graph/topology.h"

enum HealthState {
    STATE_HEALTHY = 0,
    STATE_DEGRADED = 1,
    STATE_CRITICAL = 2,
    STATE_FAILED = 3
};

enum ObsSymbol {
    OBS_NORMAL = 0,
    OBS_HIGH_LOAD = 1,
    OBS_HIGH_LATENCY = 2,
    OBS_SEVERE = 3
};

typedef struct HMMState {
    float transition[4][4];
    float emission[4][4];
    float initial[4];
} HMMState;

typedef struct Observation {
    float latency_ratio;
    float load_ratio;
} Observation;

void init_hmm(HMMState* hmm);
int* viterbi(HMMState* hmm, Observation* obs, int length);
float forward_probability(HMMState* hmm, Observation* obs, int length);
float predict_failure_prob(HMMState* hmm, Graph* g, int node_id, int time_horizon);
int discretize_observation(Observation obs);

#endif
