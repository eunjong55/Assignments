#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

int cycle_check();
void show_edges();

pthread_t t[10] ={0x0, };
int t_info[10][10];
pthread_mutex_t * node[10] = {0x0, };
int edge[10][10];

//int visited[10]={0,};
//int checked[10] = {0,};
int result[10];

int 
main (int argc, char* argv[])
{
	int fd = open(".ddtrace", O_RDONLY | O_SYNC) ;
	int i;
	
	for(i=0;i<10; i++){
		memset(t_info[i], -1, sizeof(int)*10);
	}	
	while (1) {
		char s[128] ;
		int len ;
		char mem[100]={0x0,};
		char addr[100] = {0x0,};
		if ((len = read(fd, s, 128)) == -1)
			break ;
		if (len > 0){
			//printf("%s\n", s);	
			unsigned long tid;
                        pthread_mutex_t * mid;
                        int isLock;
	
				sscanf(s, "#%d %p %lu %s", &isLock, &mid, &tid, mem);
//				printf("%s\n", mem);
				int t_index;
				int m_index;
				int t_exist = 0;
				int m_exist = 0;
				if(isLock ==1){
					for(i=0; i<10; i++){
						if(t[i] == tid){
							t_index = i;
							t_exist = 1;
							break;
						}
					}
					if(t_exist == 0){
						for(i=0; i<10; i++){
							if(t[i] == 0x0){
								t_index = i;
								t[i] = tid;
								break;
							}	
						}
					}
				
					for(i=0; i<10; i++){
                        		        if(node[i] == mid){
                        		                m_index = i;
                        		                m_exist = 1;
                        		                break;
                        		        }
                        		}
                        		if(m_exist == 0){
                        		        for(i=0; i<10; i++){
                        		                if(node[i] == 0x0){
                        		                        m_index = i;
                        		                        node[i] = mid;
                        		                        break;
                        		                }
                       		 	        }
                        		}
                        		for(i=0; i<10; i++){
						if(t_info[t_index][i] != -1){
							edge[t_info[t_index][i]][m_index] = 1;
						}
					}
					for(i=0; i<10; i++){
						if(t_info[t_index][i] == -1){
							t_info[t_index][i] = m_index;
							break;
						}
					}
//					show_edges();
//					memset(visited, 0, sizeof(int)*10);
//					memset(checked, 0, sizeof(int)*10);
					if(cycle_check()==1){
						printf("dead lock occurred\n");
						for(i=0; i<10; i++){
							if(result[i]==-1){
								break;
							}
                                                        printf("mutex address: %p \n", node[result[i]]);
                                                	for(int l=0; l<10; l++){
                                                        if(t[l] != 0x0){
                                                        for(int k=0; k<10; k++){
                                                                if(t_info[l][k]== result[i]){
								printf("thread : %lu \n", t[l]);
								}
                                                        }
                                                        }
                                                }
						}
						char info[100] ={0x0, };
						char command[100] ={0x0,};
						FILE *p = NULL;
						sprintf(command, "addr2line -e %s %s", argv[1], mem);
						p = popen(command, "r");
						fgets(info, sizeof(info), p);
						printf("DEADLOCK OCCURED BY : %s\n", info);
						exit(0);
					}
				}
				else{
					for(i=0; i<10; i++){
						if(node[i] == mid){
							m_index = i;
							break;
						}
					}
					for(i=0; i<10; i++){
						if(t[i] == tid){
							t_index = i;
							break;
						}
					}
					for(i=0; i<10; i++){
						edge[i][m_index] = 0;
					}
					for(i=0; i<10; i++){
						if(t_info[t_index][i] == m_index){
							t_info[t_index][i] = -1;
						}
					}
//					show_edges();
				}
		}
	}
	close(fd) ;
	return 0 ;
}
int dfs(int now, int start, int visit[], int path[], int count){
	int i;
	if(visit[now] ==1){
		if(now==start){
			memcpy(result, path,sizeof(int)*10);
			return 1;
		}
		return 0;
	}

	visit[now] = 1;
	for(i=0; i<10; i++){
		if(edge[now][i] == 1){
			path[count] = i;
			if(dfs(edge[now][i], start, visit, path, count+1)==1){
return 1;
}
			path[count] = -1;
		}
	}
	return 0;
}
int cycle_check(){
	int i;
	memset(result, -1, sizeof(int)*10);
	for(i=0; i<10; i++){
		if(node[i] != 0x0){
			int visit[10] = {0,};
			int path[10];
			memset(path, -1, sizeof(int)*10);
			path[0] = i;
			if(dfs(i, i, visit, path, 1)==1){
				return 1;	
			}
		}
	}
	return 0;
}/*
int cycle_check(){
	int i;
	if(visited[here]){
		return 1;
	}
	if(checked[here]){
		return 0;
	}
	checked[here] = 1;
	visited[here] = 1;
	for(i=0; i<10; i++){
		if(edge[here][i] ==1){
			if(cycle_check(i)==1) return 1;
		}
	}
	visited[here] = 0;
	return 0;
}*/
void show_edges(){
	int i, j;

	for(i=0; i<10; i++){
		printf("\n");
		for(j=0; j<10; j++){
			printf("%d ", edge[i][j]);
		}
	}
	printf("\n");			
}
