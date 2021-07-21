#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <error.h>
#include <sys/wait.h>

int m[51][51] ;
int path[51] ;
int best_path[51];
int used[51] ;
int length = 0 ;
int min = -1 ;

int child_min = -1;
int pipes[12][2];
int city_count = 0;
pid_t pid[12];
int max_child_process = 0;
int process_count = 0;
int using_process[12] = {0,};
int child_count = 0;
char result[100] = {0x0, };
void child_travel(int i);
char child_count_str[32] = {0x0,};
int sendto=0;
int parent = 0;
void  parent_proc(int i);
void child_proc(int idx);

void parent_travel(int );
void travel(int start) {
        path[1] = start ;
        used[start] = 1 ;
        parent_travel(2) ;
        used[start] = 0 ;
}

void int_handler(int sig){
        int i, j;
        if(parent == 1){
        printf("parent handler is open\n");

        for(i=0; i<max_child_process; i++){
                if(using_process[i]==1){
                        kill(pid[i], SIGTERM);
                }
        }
        for(i=0; i<process_count; i++){
                pid_t term_process;
                term_process = wait(0x0);
                for(j=0;j<max_child_process;j++){
                        if(term_process == pid[j]){
                                parent_proc(j);
                        }
                }
        }
        printf("best minimum length : %d\n",min);
        printf("best solution : ");
        for(i=1; i<=city_count; i++){
                printf("%d ", best_path[i]);
        }
        printf("\ncount : %s\n", result);
        exit(0);
        }
}

void term_handler(int sig){
        if(parent==0){
        child_proc(sendto);
        exit(0);
}
}

void count_adder(char a[], char b[]){
        int carry=0;
        int last_carry=0;
        int a_length = strlen(a);
        int b_length = strlen(b);
        int i=0;
        int tmp;
        char inverse[100] = {0x0,};
        while((a_length>0)||(b_length>0)){
                carry = 0;
                if(a_length<=0){
                        tmp = (b[b_length-1] - '0')+last_carry;
                }
                else if(b_length<=0){
                        tmp = (a[a_length-1] - '0')+last_carry;
                }
                else{
                        tmp = (a[a_length-1]-'0')+(b[b_length-1]-'0')+last_carry;
                }
                if(tmp>=10){
                        carry = 1;
                        tmp = tmp%10;
                }
                inverse[i] = tmp+'0';
                i++;
                a_length--;
                b_length--;
                last_carry = carry;
        }
        if(last_carry ==1){
            inverse[i] = '1';
        }
        for(i= 0; i<strlen(inverse); i++){
                result[i] = inverse[strlen(inverse)-1-i];
        }
}
void child_proc(int i){
        char buf[200] = {0x0, };
        char tmp[32] = {0x0, };
        int j;
        close(pipes[i][0]);

        sprintf(buf, "%d", child_min);
        strcat(buf, " ");
        sprintf(tmp, "%d", child_count);
        strcat(buf, tmp);
        for(j=1; j<=city_count; j++){
                sprintf(tmp, "%d", best_path[j]);
                strcat(buf, " ");
                strcat(buf, tmp);
        }
        write(pipes[i][1], buf, strlen(buf));
        close(pipes[i][1]);
}

void  parent_proc(int i){
        char buf[200]={0x0,};
        char *ptr;
        char c_length[50] = {0x0,};
        int j = 1;
        close(pipes[i][1]);

        read(pipes[i][0], buf, 99);
        ptr = strtok(buf, " ");
        strcpy(c_length, ptr);
        ptr = strtok(NULL, " ");
        count_adder(result, ptr);
        ptr = strtok(NULL, " ");
        if(atoi(c_length)<min || min==-1){
                min = atoi(c_length);
                while(ptr != NULL){
                        best_path[j] = atoi(ptr);
                        ptr = strtok(NULL, " ");
                        j++;
                }
        }
}

void parent_travel(int idx) {
        int i , j;
        int k;
        if (city_count-idx == 11) {
                if(process_count == max_child_process){
                        int exit_code;
                        pid_t ch_id;
                        ch_id = wait(0x0);
                        process_count -= 1;
                        for(j = 0; j<max_child_process; j++){
                                if(ch_id == pid[j]){
                                        char tmp[32] = {0x0,};
                                        printf("%d is terminated\n", pid[j]);
                                        using_process[j] = 0;
                                        parent_proc(j);
                                        break;
                                }
                        }
                }
                for (i = 0  ; i < max_child_process; i++){
                        if(using_process[i]==0){
                                pipe(pipes[i]);
                                sendto = i;
                                using_process[i] = 1;
                                pid[i] = fork();
                                if(pid[i] ==0){
                                        parent = 0;
                                        child_min = -1;
                                        child_travel(idx);
                                        child_proc(i);
                                        exit(0);
                                }
                                else{
                                        parent = 1;
                                        printf("%d is started\n", pid[i]);
                                        process_count+=1;
                                }
                                break;
                        }
                }
        }
        else {
                for (i = 1 ; i <= city_count ; i++) {
                        if (used[i] == 0) {
                                path[idx] = i ;
                                used[i] = 1 ;
                                length += m[path[idx-1]][i] ;
                                parent_travel(idx+1) ;
                                length -= m[path[idx-1]][i] ;
                                used[i] = 0 ;
                        }
                }
        }
}

void child_travel(int idx){
        int i;
        if(idx == city_count+1){
                length+=m[path[city_count]][path[1]];
                if(child_min>length ||child_min==-1){
                        child_min = length;
                        for(i=1; i<=city_count; i++){
                                best_path[i] = path[i];
                        }
                }
                child_count++;
                length-=m[path[city_count]][path[1]];
        }
        else{
                for (i = 1 ; i <= city_count ; i++) {
                        if (used[i] == 0) {
                                        path[idx] = i ;
                                        used[i] = 1 ;
                                        length += m[path[idx-1]][i] ;
                                        child_travel(idx+1) ;
                                        length -= m[path[idx-1]][i] ;
                                        used[i] = 0 ;
                        }
                }
        }
}

int main(int argc, char *argv[]) {
        int i, j, t ;
        char* file_path = argv[1];
        char tmp;

        result[0] = '0';
        FILE* fp = fopen(file_path, "r") ;
        max_child_process = atoi(argv[2]);
        if(fp == NULL){
                printf("file path is wrong");
                return 1;
        }

        signal(SIGINT, int_handler);
        signal(SIGTERM, term_handler);

        while (fscanf(fp, "%c", &tmp) != EOF) {
                if (tmp == '\n') city_count++;
        }
        fclose(fp) ;

        fp = fopen(file_path, "r");
        for (i = 1 ; i <= city_count; i++) {
                for (j = 1 ; j <= city_count; j++) {
                        fscanf(fp, "%d", &t) ;
                        m[i][j] = t ;
                }
        }
        fclose(fp) ;
        for (i = 1  ; i <= city_count; i++) travel(i) ;
        printf("%d\n", city_count);
        for(i=0; i< strlen(result); i++){
                printf("%c", result[strlen(result)-1-i]);
        }
        printf("\n");
}
