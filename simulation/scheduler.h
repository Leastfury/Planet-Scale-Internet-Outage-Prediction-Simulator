#ifndef SCHEDULER_H
#define SCHEDULER_H

#define EVT_FAILURE  0
#define EVT_CASCADE  1
#define EVT_RECOVERY 2
#define EVT_REROUTE  3
#define EVT_OVERLOAD 4

typedef struct Event {
    int   time;             // simulation minute
    int   type;
    int   target_node;      // -1 if not applicable
    int   target_edge;      // -1 if not applicable
    char  description[128];
    struct Event* next;
} Event;

typedef struct Scheduler {
    Event* head;            // sorted linked list, head = lowest time
    int    current_time;    // current simulation minute
} Scheduler;

struct Graph;

Scheduler* init_scheduler();
void add_event(Scheduler* s, int time, int type, int target_node, int target_edge, const char* desc);
Event* pop_next_event(Scheduler* s);
void run_simulation(Scheduler* s, struct Graph* g);
void free_scheduler(Scheduler* s);

#endif
