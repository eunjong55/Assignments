#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

typedef struct {
	pthread_cond_t enqueue_cv;
	pthread_cond_t dequeue_cv;
	
	pthread_mutex_t lock;
	int ** prefix ;
	int capacity;
	int num;
	int front;
	int rear;
} bounded_buffer;

typedef struct{
	pthread_mutex_t mutex;
	pthread_cond_t writers_cv;
	pthread_cond_t readers_cv;

	int to_read;
	int to_write;
	int reading;
	int writing;
} rwlock_t ;

bounded_buffer * buf = 0x0;
bounded_buffer * canceled_prefix_buffer = 0x0;
rwlock_t * l[8];

int max_count = 0;
int city_count = 0;
int ** map = 0x0;

int * global_best_path = 0x0;
int global_min= -1;
int global_count= 0;
int terminated_thread_count = 0;
int reenque_flag = 0;
int end_produce_flag = 0;

pthread_t prod;
pthread_t cons[8];

int * thread_best_path[8];
int thread_min[8] = {-1, -1, -1, -1, -1, -1, -1,-1};
int thread_total_count[8]={0,};
int thread_subtask_count[8]={0, };
int * thread_prefix[8];
int thread_sub_count[8] = {0, };

void prod_travel(int idx, int path[], int used[]);
void cons_travel(int idx, int path[], int used[], int write_to);
void * producer(void * ptr);
void * consumer(void * ptr);
void bounded_buffer_init(bounded_buffer * buf, int capacity);
void bounded_buffer_enqueue(bounded_buffer * buf, int * prefix);
int * bounded_buffer_dequeue(bounded_buffer * buf);
void rwlock_init(rwlock_t * l);
void reader_lock(rwlock_t * l);
void reader_unlock(rwlock_t * l);
void writer_lock(rwlock_t * l);
void writer_unlock(rwlock_t * l);
int calc_cost(int path[]);
int make_map(char* file_path);
void show_map();
void show_path(int path_arr[]);
void int_handler(int sig);
void clean_up(void *arg);

int main(int argc, char *argv[]){
	int i;
	max_count = atoi(argv[2]);
	
	signal(SIGINT, int_handler);

	if(make_map(argv[1])==-1){//argv[1] is file path
		return -1;
	}

	global_best_path = (int *)malloc(sizeof(int)*city_count);
	
	for(i=0; i<8; i++){
		thread_best_path[i] = (int *)malloc(sizeof(int)*city_count);
		thread_prefix[i] = (int *)malloc(sizeof(int)*city_count);
	}

	buf = malloc(sizeof(bounded_buffer));
	canceled_prefix_buffer = malloc(sizeof(bounded_buffer));

	bounded_buffer_init(buf, 10);
	bounded_buffer_init(canceled_prefix_buffer, 100);

	for(i=0; i<8; i++){
		l[i] = (rwlock_t *)malloc(sizeof(rwlock_t));
		rwlock_init(l[i]);
	}

	pthread_create(&(prod), 0x0, producer, 0x0);

	for(i=0; i<max_count; i++){
		pthread_create(&(cons[i]), 0x0, consumer, 0x0);
	}

	while(end_produce_flag==0){
		char command[100] ={0x0, };
		printf("\nenter the command (stat, threads, num N(N is number)\n");
		scanf("%s", command);
		if(strcmp(command, "stat")==0){
			for(i =0;i<max_count; i++){
                                reader_lock(l[i]);
                        }
			long current_count = terminated_thread_count;
			for(i=0; i<max_count; i++){
				current_count+=thread_total_count[i];
				if(global_min ==-1||global_min>thread_min[i]){
					global_min = thread_min[i];
					memcpy(global_best_path, thread_best_path[i], sizeof(int)*city_count);
				}
			}
			printf("cheked route count : %lu\nbest cost : %d\n", current_count, global_min);
			printf("best path : ");
			show_path(global_best_path);
			for(i =0;i<max_count; i++){
                                reader_unlock(l[i]);
                        }
		}
		else if(strcmp(command, "threads")==0){
                        for(i =0;i<max_count; i++){
                                reader_lock(l[i]);
                        }
                        for(i=0; i<max_count; i++){
				printf("thread id : %lu, checked route in this subtask : %d, processed subtask count : %d\n", cons[i], thread_subtask_count[i], thread_sub_count[i]);
			}
			for(i =0;i<max_count; i++){
                                reader_unlock(l[i]);
                        }
                }
                else if(strcmp(command, "num")==0){
			int number;
			int temp = max_count;
			scanf("%d", &number);
			if(number>max_count){
				max_count = number;
				for(i=temp; i<max_count; i++){
					thread_min[i] = -1;
					thread_sub_count[i]=0;
					thread_total_count[i] = 0;
					pthread_create(&(cons[i]), 0x0, consumer, 0x0);
				}
			}
			else if(number<max_count){
				max_count = number;
				int j=0;
				for(i=max_count; i<temp; i++){
                                        pthread_cancel(cons[i]);
                                }
                                for(i=max_count; i<temp; i++){
                                        pthread_join(cons[i], 0x0);
                                }
				for(i=max_count; i<temp; i++){
					int * reenque_prefix = (int *)malloc(sizeof(int)*city_count);
					memcpy(reenque_prefix, thread_prefix[i], sizeof(int)*city_count);
					bounded_buffer_enqueue(canceled_prefix_buffer, reenque_prefix);
					terminated_thread_count += (thread_total_count[i]-thread_subtask_count[i]);
				}
			}
		}
		else{
				printf("wrong command\n");
                }
	}
	for(i=0; i<max_count; i++){
		pthread_join(cons[i], 0x0);
	}

	pthread_join(prod, 0x0);
	
	for(i =0;i<max_count; i++){
        	reader_lock(l[i]);
        }
        long current_count = terminated_thread_count;
        for(i=0; i<max_count; i++){
        	current_count+=thread_total_count[i];
		if(global_min ==-1||global_min>thread_min[i]){
                	global_min = thread_min[i];
                        memcpy(global_best_path, thread_best_path[i], sizeof(int)*city_count);
                }
        }
        printf("cheked route count : %lu\nbest cost : %d\n", current_count, global_min);
        printf("best path : ");
        show_path(global_best_path);
        for(i =0;i<max_count; i++){
  	      reader_unlock(l[i]);
        }
	exit(0);
}

void prod_travel(int idx, int path[], int used[]){
	int i;
	if(city_count-idx == 11){
		while(canceled_prefix_buffer->num>0){
			bounded_buffer_enqueue(buf, bounded_buffer_dequeue(canceled_prefix_buffer));
		}
		int * subtask = (int *) malloc(sizeof(int)*city_count);
		memcpy(subtask, path, sizeof(int)*city_count);
		bounded_buffer_enqueue(buf, subtask);
	}
	else{
		for(i=0; i<city_count; i++){
			if(used[i] == 0){
				path[idx] = i;
				used[i] = 1;
				prod_travel(idx+1, path, used);
				used[i] = 0;
			}
		}
	}
}

void cons_travel(int idx, int path[], int used[], int write_to){
	int i;
        if(idx == city_count){
		int cost = 0;
		writer_lock(l[write_to]);
		thread_subtask_count[write_to]+=1;
		thread_total_count[write_to]+=1;
		cost = calc_cost(path);
		if((thread_min[write_to]==-1)||(cost < thread_min[write_to])){
			thread_min[write_to] = cost;
			memcpy(thread_best_path[write_to], path, sizeof(int)*city_count);
		}
		writer_unlock(l[write_to]);
		pthread_testcancel();
	}
        else{
                for(i=0; i<city_count; i++){
                        if(used[i] == 0){
                                path[idx] = i;
                                used[i] = 1;
                                cons_travel(idx+1, path, used, write_to);
                                used[i] = 0;
                        }
                }
        }
}

void * producer(void * ptr){
        int * path =(int *) calloc(city_count, sizeof(int));
	int * used =(int *) calloc(city_count, sizeof(int));
	
	prod_travel(0, path, used);
	
	end_produce_flag = 1;
	
	return 0x0;
}

void * consumer(void * ptr){
	int i;
	int write_to;
	pthread_t tid = pthread_self();
	for(i=0; i<max_count; i++){
		if(cons[i] == tid){
			write_to = i;
			break;
		}
	}
	while(end_produce_flag==0){
		int idx = city_count-11;
		int * used = (int *)malloc(sizeof(int)*city_count);
		int * prefix = bounded_buffer_dequeue(buf);
		writer_lock(l[write_to]);
		memcpy(thread_prefix[write_to] , prefix, sizeof(int)*city_count);
		writer_unlock(l[write_to]);
		thread_subtask_count[write_to] = 0;
		for(i=0; i<city_count-11; i++){
			used[prefix[i]] = 1;
		}
		cons_travel(idx, prefix, used, write_to);
		writer_lock(l[write_to]);
		thread_sub_count[write_to]+=1;
		writer_unlock(l[write_to]);
	}
}

void bounded_buffer_init(bounded_buffer * buf, int capacity){
	pthread_cond_init(&(buf->enqueue_cv), 0x0);
	pthread_cond_init(&(buf->dequeue_cv), 0x0);
	pthread_mutex_init(&(buf->lock), 0x0);
	buf->capacity = capacity;
	buf->prefix = (int**) calloc(capacity, sizeof(int *));
	buf->num = 0;
	buf->front = 0;
	buf->rear = 0;
}

void bounded_buffer_enqueue(bounded_buffer * buf, int * path){
	pthread_mutex_lock(&(buf->lock));
        while(buf->num == buf->capacity){
                pthread_cond_wait(&(buf->enqueue_cv), &(buf->lock));
        }
        buf->prefix[buf->rear] = path;
	buf->rear = (buf->rear+1) % buf->capacity;
	buf->num += 1;
	pthread_cond_signal(&(buf->dequeue_cv));
	pthread_mutex_unlock(&(buf->lock));
}

int * bounded_buffer_dequeue(bounded_buffer * buf){
	int* r= (int *)malloc(sizeof(int)*city_count);

	pthread_mutex_lock(&(buf->lock));
	
	while(buf->num == 0){
		pthread_cond_wait(&(buf->dequeue_cv), &(buf->lock));
	}
	memcpy(r , buf->prefix[buf->front], city_count*sizeof(int));
	buf->front = (buf->front +1)% buf->capacity;
	buf->num -=1;
	pthread_cond_signal(&(buf->enqueue_cv));

	pthread_mutex_unlock(&(buf->lock));

	return r;
}

void rwlock_init(rwlock_t * l){
	l->to_read = 0;
	l->to_write = 0;
	l->reading = 0;
	l->writing = 0;
	pthread_mutex_init(&(l->mutex), 0x0);
	pthread_cond_init(&(l->writers_cv), 0x0);
	pthread_cond_init(&(l->readers_cv), 0x0);
}

void writer_lock(rwlock_t * l){
	pthread_mutex_lock(&(l->mutex));
	l->to_write += 1;
	while(l->reading>0 || l->to_read > 0){
		pthread_cond_wait(&(l->writers_cv), &(l->mutex));
	}
	l->to_write -= 1;
	l->writing += 1;
	pthread_mutex_unlock(&(l->mutex));
}

void writer_unlock(rwlock_t * l){
	pthread_mutex_lock(&(l->mutex));
	l->writing -= 1;
	if(l->to_read>0){
		pthread_cond_signal(&(l->readers_cv));
	}
	pthread_mutex_unlock(&(l->mutex));
}

void reader_lock(rwlock_t * l){
        pthread_mutex_lock(&(l->mutex));
        l->to_read += 1;
        while(l->reading > 0||l->writing >0){
                pthread_cond_wait(&(l->readers_cv), &(l->mutex));
        }
        l->to_read -= 1;
        l->reading += 1;
        pthread_mutex_unlock(&(l->mutex));
}

void reader_unlock(rwlock_t * l){
	pthread_mutex_lock(&(l->mutex));
	l->reading -= 1;
	if(l->to_read> 0){
		pthread_cond_signal(&(l->readers_cv));
	}
	else if(l->to_write>0){
		pthread_cond_signal(&(l->writers_cv));
	}
	pthread_mutex_unlock(&(l->mutex));
}

int calc_cost(int path[]){
	int cost = 0;
	int i;
	for(i=0; i<city_count-1; i++){
		cost+=map[path[i]][path[i+1]];
	}
	cost+=map[path[i]][path[0]];
	return cost;
}

int make_map(char* file_path){
	FILE* fp = fopen(file_path, "r");
	int i,j,k;
	char tmp;
	if(fp==NULL){
		printf("file path is wrong\n");
		return -1;
	}
	while(fscanf(fp, "%c", &tmp) != EOF){
		if(tmp == '\n') city_count++;
	}
	map = (int**) malloc (sizeof(int*) *city_count);
	for(i=0; i<city_count; i++){
		map[i] = (int*) malloc (sizeof(int) * city_count);
	}
	fp = fopen(file_path, "r");
	for(i=0; i< city_count; i++){
		for(j=0; j< city_count; j++){
			fscanf(fp, "%d", &k);
			map[i][j] = k;
		}	
	}
	fclose(fp);
	return 0;
}

void show_map(){
	int i,j;
	printf("city count : %d\n", city_count);
	for(i=0; i<city_count; i++){
		for(j=0; j<city_count; j++){
			printf("%d ", map[i][j]);
		}
		printf("\n");
	}
}

void show_path(int path_arr[]){
	int i;
	for(i=0; i<city_count; i++){
		printf("%d ",path_arr[i]+1);
	}
	printf("%d", path_arr[0]+1);
	printf("\n");
}
void int_handler(int sig){
	int i;
	printf("terminated.\n");
	for(i =0;i<max_count; i++){
        	reader_lock(l[i]);
	}
	long current_count = terminated_thread_count;
	for(i=0; i<max_count; i++){
        	current_count+=thread_total_count[i];
        	if(global_min ==-1||global_min>thread_min[i]){
                	global_min = thread_min[i];
			memcpy(global_best_path, thread_best_path[i], sizeof(int)*city_count);
                }
	}
	printf("cheked route count : %lu\nbest cost : %d\n", current_count, global_min);
        printf("best path : ");
	show_path(global_best_path);
        for(i =0;i<max_count; i++){
        	reader_unlock(l[i]);
        }
	exit(0);
}
void clean_up(void *arg)
{
    printf("Thread cancel Clean_up function\n");
}
