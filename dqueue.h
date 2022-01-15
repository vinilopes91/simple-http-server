#define SUCCESS 1
#define OUT_OF_MEMORY -1
#define ELEM_NOT_FOUND -2
#define INVALID_NULL_POINTER -3
#define NOT_FULL -4

typedef struct dqueue Queue;

Queue* queue_create();
void queue_free(Queue* qu);

int queue_push(Queue *qu, int client_socket);
int queue_pop(Queue *qu, int* client_socket);

int queue_empty(Queue* qu);
