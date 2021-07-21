#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

void GenSeq(char* s, int n);
const int num_alphabet = 5;
void PrintSeq(char* s, int n);
int LCS(char* X, char* Y, int i, int j);
int LCS_Length(char* X, char* Y, int m, int n);
int** c;
int** b; // 0: '↖', -1: '↑', 1: '←'
void PRINT_LCS(char* X, int i, int j);
int FindMax(int a, int b);
void PRINT_ALL_LCSs(char* X, int i, int j, int lcs_len);
void find_ALL_LCSs(char* X, int x, int y, int len, char* str);
int GetCurrentUsec();
char** lcs_dic;
int lcs_count = 0;
int lcs_len = 0;


int main(void) { 
  int i = 15; // length of X
  int j = 15; // length of Y

  char* X, * Y; // sequences
  X = (char*)malloc(sizeof(char)*i);
  Y = (char*)malloc(sizeof(char)*j);

  c = (int**)malloc(sizeof(int*)*(i+1));
  for (int k=0; k<i+1; k++) c[k] = (int*)malloc(sizeof(int)*(j+1));
  b = (int**)malloc(sizeof(int*)*(i+1));
  for (int k=0; k<i+1; k++) b[k] = (int*)malloc(sizeof(int)*(j+1));

  srand(time(NULL));
  GenSeq(X, i);
  GenSeq(Y, j);
  
  PrintSeq(X, i);
  PrintSeq(Y, j);

  int t1, t2, result;

  // Divide and Conquer
  t1 = GetCurrentUsec();
  result = LCS(X, Y, i, j);
  t2 = GetCurrentUsec();
  printf("Then length of LCS is %d.\n", result);
  printf("Divide and Conqure took %d usec.\n", t2-t1);

  // Dynaic Programming
  t1 = GetCurrentUsec();
  result = LCS_Length(X, Y, i, j);
  t2 = GetCurrentUsec();
  printf("Then length of LCS is %d.\n", result);
  printf("Dynamic Programming took %d usec.\n", t2-t1);
  
  printf("<");
  PRINT_LCS(X, i, j);
  printf(">\n\n");
  
  // additional point: see the assignment description
  PRINT_ALL_LCSs(X, i, j, result);

  return 0;
}

//////////////////////////////
//  write your answer below //
//////////////////////////////
/*

for i = 2, j = 2
Divide and Conquer took : 0 usec.
Dynamic Programming took : 0 usec.

for i = 5, j = 5
Divide and Conquer took : 2.3 usec.
Dynamic Programming took : 0.6 usec.

for i = 10, j = 10
Divide and Conquer took : 880 usec.
Dynamic Programming took : 2.7 usec.

for i = 15, j = 15
Divide and Conquer took : 110243 usec.
Dynamic Programming took : 6 usec.

*/
//////////////////////////////


int LCS(char* X, char* Y, int i, int j)
{
  int c;
  if ((i==0) || (j==0))
    c = 0;
  else if (X[i-1] == Y[j-1])
    c = LCS(X, Y, i-1, j-1) + 1;
  else
    c = FindMax(LCS(X, Y, i-1, j), LCS(X, Y, i, j-1));
  return c;
}

int FindMax(int a, int b)
{
  if (a >= b) 
    return a;
  else
    return b;
}

int LCS_Length(char* X, char* Y, int m, int n)
{
  int i, j;
  for (i=0; i<=m; i++) c[i][0] = 0;
  for (j=0; j<=n; j++) c[0][j] = 0;
  // implement your code here
  for(i=1; i<= m; i++){
    for(j=1; j<= n; j++){
      if(X[i-1] == Y[j-1]){
        c[i][j] = c[i-1][j-1] +1;
        b[i][j] = 0;
      }
      else{
        if(c[i-1][j] >= c[i][j-1]){
          c[i][j] = c[i-1][j];
          b[i][j] = -1;
        }
        else{
          c[i][j] = c[i][j-1];
          b[i][j] = 1;
        }
      }
    }
  }
  return c[m][n];
}

void PRINT_LCS(char* X, int i, int j)
{
  if ((i==0) || (j==0)) return;
  if (b[i][j] == 0)
  {
    PRINT_LCS(X, i-1, j-1);
    printf("%c, ", X[i-1]);
  }
  else if (b[i][j] == -1)
    PRINT_LCS(X, i-1, j);
  else if (b[i][j] == 1)
    PRINT_LCS(X, i, j-1);
}

int GetCurrentUsec()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_usec;
}

void GenSeq(char* s, int n)
{
  for (int i=0; i<n; i++)
  {
    s[i] = (char)((random() % num_alphabet)+ 'A');
  }
  return;
}

void PrintSeq(char* s, int n)
{
  printf("<");
  for (int i=0; i<n; i++)
  {
    printf("%c, ", s[i]);
  }
  printf(">\n");
}
// 0: '↖', -1: '↑', 1: '←'
void PRINT_ALL_LCSs(char* X, int i, int j, int result){
  lcs_len = result;
  char* str = (char*)malloc(sizeof(char)*lcs_len);
  find_ALL_LCSs(X, i, j, lcs_len, str);
  printf("==========PRINT_ALL_LCSs==========\n");
  for(int k=0; k<lcs_count; k++){
    printf("<");
    for(int l = 0; l<lcs_len; l++){
      printf("%c, ", lcs_dic[k][l]);
    }
    printf(">\n");
  }
}

void find_ALL_LCSs(char* X, int x, int y, int len, char* str){
  if(len==0){
    if(lcs_count==0){//no element in lcs_dic
      lcs_dic = (char **)malloc((lcs_count+1)*sizeof(char *));
      lcs_dic[0] = (char *)malloc(sizeof(char)*lcs_len);
      strcat(lcs_dic[0], str);
      lcs_count++;
    }
    else{
      for(int i = 0; i<lcs_count; i++){//duplicate check
        if(strcmp(lcs_dic[i], str)==0){
          return;
        }
      }
      lcs_dic = (char **)realloc(lcs_dic, (lcs_count+1)*sizeof(char *));
      lcs_dic[lcs_count] = (char *)malloc(sizeof(char)*lcs_len);
      strcat(lcs_dic[lcs_count], str);
      lcs_count++;
    }
    return;
  }
  if((x==0)||(y==0)){
    return;
  }
  if((b[x][y]!=0)&&(c[x-1][y] == c[x][y-1])){
    find_ALL_LCSs(X, x-1, y, len, str);
    find_ALL_LCSs(X, x, y-1, len, str);
  }
  else if(b[x][y]==1){
    find_ALL_LCSs(X, x, y-1, len, str);
  }
  else if(b[x][y]==-1){
    find_ALL_LCSs(X, x-1, y, len, str);
  }
  else if(b[x][y]==0){
    str[len-1] = X[x-1];
    find_ALL_LCSs(X, x-1, y-1, len-1, str);
  }
}

