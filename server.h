#ifndef EE367L_LAB05_SERVER_H
#define EE367L_LAB05_SERVER_H

#define NAMING_TABLE_SIZE 256

struct server_job_queue {
    struct server_job *head;
    struct server_job *tail;
    int occ;
};

//void job_q_init(struct server_job_queue *j_q);

//int job_q_num(struct server_job_queue *j_q);

_Noreturn void server_main(int host_id);

#endif //EE367L_LAB05_SERVER_H
