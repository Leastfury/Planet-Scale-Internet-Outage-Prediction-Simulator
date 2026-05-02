#ifndef QLEARNING_H
#define QLEARNING_H

#include "../graph/topology.h"

enum QAction {
    ACTION_REROUTE = 0,
    ACTION_ACTIVATE_BACKUP = 1,
    ACTION_THROTTLE = 2,
    ACTION_INCREASE_CAP = 3,
    ACTION_DO_NOTHING = 4
};

typedef struct QLearning {
    float Q[1000][5];
    float learning_rate;
    float discount;
    float epsilon;
} QLearning;

void init_qlearning(QLearning* ql);
int discretize_state(Graph* g);
int select_action(QLearning* ql, int state, int is_training);
void update_q(QLearning* ql, int state, int action, float reward, int next_state);
void train_episodes(Graph* g, int episodes);
void save_qtable(QLearning* ql, const char* filename);
void load_qtable(QLearning* ql, const char* filename);

#endif
