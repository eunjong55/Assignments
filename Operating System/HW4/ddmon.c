#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <pthread.h>
#include <signal.h>

#define SIZE 5

int trace = 0;

int 
pthread_mutex_lock(pthread_mutex_t * mutex)
{
	static __thread int n_malloc = 0 ; //https://gcc.gnu.org/onlinedocs/gcc-3.3/gcc/Thread-Local.html
	int i;
	n_malloc += 1 ;
	char mem[100] ={0x0,}; 
	if (mkfifo(".ddtrace", 0666)) {
		if (errno != EEXIST) {
			perror("fail to open fifo: ") ;
			exit(1) ;
		}
	}
	if(n_malloc==1 ){
		trace = 1;
  	  	void *array[SIZE];
  	  	size_t size;
  	  	char **strings;
  	  	size_t i;
  	  	size = backtrace(array, SIZE);
  	  	strings = backtrace_symbols(array, size);
  	  	//for(i = 0; i < size; i++) printf("%lu. %s\n", i, strings[i]);
		strcpy(mem, strchr(strings[1], '['));
		free(strings);
        	trace =0;
	}
if(trace==0){
//	printf("%s\n", mem);
	int fd = open(".ddtrace", O_WRONLY | O_SYNC) ;
	char s[128] ={0x0, };
	pthread_t tid = pthread_self();
	for(i=0; i<strlen(mem); i++){
		if(mem[i] == '[' || mem[i] == ']'){
			mem[i] = ' ';
		}
	}
	sprintf(s, "#1 %p %lu %s", mutex, tid, mem);
	s[strlen(s)] = 0x0;
	write(fd, s, 128);
	close(fd) ;
}
	int (*pthread_mutex_lockp)(pthread_mutex_t * mutex);	
	char * error ;
	
	pthread_mutex_lockp = dlsym(RTLD_NEXT, "pthread_mutex_lock") ;
	if ((error = dlerror()) != 0x0) 
		exit(1) ;
	
	n_malloc -=1;
	return pthread_mutex_lockp(mutex);
}
int
pthread_mutex_unlock(pthread_mutex_t * mutex)
{
	static __thread int k_malloc = 0 ; //https://gcc.gnu.org/onlinedocs/gcc-3.3/gcc/Thread-Local.html
	char mem[100] ={0x0,};  
        k_malloc += 1 ;
        if (mkfifo(".ddtrace", 0666)) {
                if (errno != EEXIST) {
                        perror("fail to open fifo: ") ;
                        exit(1) ;
                }
        }
/*
if(k_malloc==1 ){
                trace = 1;
                void *array[SIZE];
                size_t size;
                char **strings;
                size_t i;
                size = backtrace(array, SIZE);
                strings = backtrace_symbols(array, size);
                //for(i = 0; i < size; i++) printf("%lu. %s\n", i, strings[i]);
                free(strings);
                trace =0;
		strcpy(mem, strings[1]);
}*/
if(trace==0){
        int fd = open("channel", O_WRONLY | O_SYNC) ;
        char s[128] ={0x0, };
        pthread_t tid = pthread_self();
        sprintf(s, "#0 %p %lu %s ", mutex, tid, mem);
	s[strlen(s)] = 0x0;
	write(fd, s, 128);
        close(fd) ;
}
        int (*pthread_mutex_unlockp)(pthread_mutex_t * mutex);
        char * error ;

        pthread_mutex_unlockp = dlsym(RTLD_NEXT, "pthread_mutex_unlock") ;
        if ((error = dlerror()) != 0x0)
                exit(1) ;
	k_malloc -= 1;
        return pthread_mutex_unlockp(mutex);
}

