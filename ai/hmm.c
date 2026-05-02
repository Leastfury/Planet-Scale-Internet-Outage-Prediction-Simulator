#include "hmm.h"
#include <stdio.h>
#include <stdlib.h>

void init_hmm(HMMState* hmm) {
    // Initial probabilities
    hmm->initial[STATE_HEALTHY] = 0.9f;
    hmm->initial[STATE_DEGRADED] = 0.08f;
    hmm->initial[STATE_CRITICAL] = 0.02f;
    hmm->initial[STATE_FAILED] = 0.0f;
    
    // Transition probabilities A[from][to]
    float A[4][4] = {
        {0.95f, 0.04f, 0.01f, 0.00f}, // HEALTHY
        {0.10f, 0.70f, 0.15f, 0.05f}, // DEGRADED
        {0.05f, 0.10f, 0.50f, 0.35f}, // CRITICAL
        {0.00f, 0.00f, 0.00f, 1.00f}  // FAILED (absorbing)
    };
    
    // Emission probabilities B[state][symbol]
    float B[4][4] = {
        {0.80f, 0.10f, 0.08f, 0.02f}, // HEALTHY produces mostly NORMAL
        {0.20f, 0.40f, 0.30f, 0.10f}, // DEGRADED
        {0.05f, 0.20f, 0.25f, 0.50f}, // CRITICAL
        {0.00f, 0.00f, 0.00f, 1.00f}  // FAILED produces SEVERE
    };
    
    for (int i=0; i<4; i++) {
        for (int j=0; j<4; j++) {
            hmm->transition[i][j] = A[i][j];
            hmm->emission[i][j] = B[i][j];
        }
    }
}

int discretize_observation(Observation obs) {
    int high_load = obs.load_ratio > 0.8f ? 1 : 0;
    int high_lat = obs.latency_ratio > 1.5f ? 1 : 0;
    if (high_load && high_lat) return OBS_SEVERE;
    if (high_lat) return OBS_HIGH_LATENCY;
    if (high_load) return OBS_HIGH_LOAD;
    return OBS_NORMAL;
}

int* viterbi(HMMState* hmm, Observation* obs, int length) {
    if (length <= 0) return NULL;
    
    // Support standard C99 VLA
    float (*v)[4] = malloc(sizeof(float) * length * 4);
    int (*path)[4] = malloc(sizeof(int) * length * 4);
    
    int sym0 = discretize_observation(obs[0]);
    for (int s = 0; s < 4; s++) {
        v[0][s] = hmm->initial[s] * hmm->emission[s][sym0];
        path[0][s] = 0;
    }
    
    for (int t = 1; t < length; t++) {
        int sym = discretize_observation(obs[t]);
        for (int s = 0; s < 4; s++) {
            float max_prob = -1.0f;
            int max_state = 0;
            for (int s_prev = 0; s_prev < 4; s_prev++) {
                float prob = v[t-1][s_prev] * hmm->transition[s_prev][s] * hmm->emission[s][sym];
                if (prob > max_prob) {
                    max_prob = prob;
                    max_state = s_prev;
                }
            }
            v[t][s] = max_prob;
            path[t][s] = max_state;
        }
    }
    
    float max_prob = -1.0f;
    int best_last_state = 0;
    for (int s = 0; s < 4; s++) {
        if (v[length-1][s] > max_prob) {
            max_prob = v[length-1][s];
            best_last_state = s;
        }
    }
    
    int* opt_path = (int*)malloc(sizeof(int) * length);
    opt_path[length-1] = best_last_state;
    for (int t = length - 1; t > 0; t--) {
        opt_path[t-1] = path[t][opt_path[t]];
    }
    
    free(v);
    free(path);
    return opt_path;
}

float forward_probability(HMMState* hmm, Observation* obs, int length) {
    if (length <= 0) return 0.0f;
    
    float (*alpha)[4] = malloc(sizeof(float) * length * 4);
    
    int sym0 = discretize_observation(obs[0]);
    for (int s = 0; s < 4; s++) {
        alpha[0][s] = hmm->initial[s] * hmm->emission[s][sym0];
    }
    
    for (int t = 1; t < length; t++) {
        int sym = discretize_observation(obs[t]);
        for (int s = 0; s < 4; s++) {
            float sum = 0.0f;
            for (int s_prev = 0; s_prev < 4; s_prev++) {
                sum += alpha[t-1][s_prev] * hmm->transition[s_prev][s];
            }
            alpha[t][s] = sum * hmm->emission[s][sym];
        }
    }
    
    float total_prob = 0.0f;
    for (int s = 0; s < 4; s++) {
        total_prob += alpha[length-1][s];
    }
    
    free(alpha);
    return total_prob;
}

float predict_failure_prob(HMMState* hmm, Graph* g, int node_id, int time_horizon) {
    if (node_id < 0 || node_id >= g->node_count) return 0.0f;
    Node* n = &g->nodes[node_id];
    if (!n->is_active) return 1.0f;
    
    Observation obs[1];
    obs[0].load_ratio = (n->capacity > 0) ? ((float)n->current_load / n->capacity) : 0.0f;
    obs[0].latency_ratio = 1.0f;
    if (n->risk_score > 50.0f) obs[0].latency_ratio = 2.0f;
    
    float alpha[4];
    int sym = discretize_observation(obs[0]);
    float total_prob = 0.0f;
    for (int s = 0; s < 4; s++) {
        alpha[s] = hmm->initial[s] * hmm->emission[s][sym];
        total_prob += alpha[s];
    }
    
    if (total_prob > 0.0f) {
        for (int s = 0; s < 4; s++) alpha[s] /= total_prob;
    } else {
        alpha[STATE_HEALTHY] = 1.0f;
        alpha[STATE_DEGRADED] = 0.0f;
        alpha[STATE_CRITICAL] = 0.0f;
        alpha[STATE_FAILED] = 0.0f;
    }
    
    float current_dist[4];
    float next_dist[4];
    for(int i=0; i<4; i++) current_dist[i] = alpha[i];
    
    for (int t = 0; t < time_horizon; t++) {
        for (int j = 0; j < 4; j++) next_dist[j] = 0.0f;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                next_dist[j] += current_dist[i] * hmm->transition[i][j];
            }
        }
        for (int j = 0; j < 4; j++) current_dist[j] = next_dist[j];
    }
    
    return current_dist[STATE_FAILED] + current_dist[STATE_CRITICAL] * 0.5f;
}
