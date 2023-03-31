/*
 * host.h
 */

// Added JOB_FILE_UPLOAD_MIDDLE
enum host_job_type {
    JOB_SEND_PKT_ALL_PORTS,
    JOB_PING_SEND_REQ,
    JOB_PING_SEND_REPLY,
    JOB_PING_WAIT_FOR_REPLY,
    JOB_FILE_UPLOAD_SEND,
    JOB_FILE_UPLOAD_RECV_START,
    JOB_FILE_UPLOAD_RECV_END,
    JOB_FILE_UPLOAD_RECV_MIDDLE,
    JOB_FORWARD_PACKET
};

struct host_job {
    enum host_job_type type;
    struct packet *packet;
    int in_port_index;
    int out_port_index;
    char fname_download[100];
    char fname_upload[100];
    int ping_timer;
    int file_upload_dst;
    struct host_job *next;
};


struct job_queue {
    struct host_job *head;
    struct host_job *tail;
    int occ;
};

void job_q_init(struct job_queue *j_q);

void job_q_add(struct job_queue *j_q, struct host_job *j);

int job_q_num(struct job_queue *j_q);

struct host_job *job_q_remove(struct job_queue *j_q);

_Noreturn void host_main(int host_id);
