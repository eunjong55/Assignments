#include <stdio.h>
#include <float.h>
#include <stdlib.h>

struct list {
  int vertex_index;
  double weight;
  struct list* next;
};

int compare(const void *a, const void *b)    // 오름차순 비교 함수 구현
{
    int num1 = *(int *)a;   
    int num2 = *(int *)b;    

    if (num1 < num2)    
        return 1;      
    
    if (num1 > num2)    
        return -1;      
    
    return 0;    
}

struct list* newList(int vertex_index, double weight) 
{ 
    struct list* temp = (struct list*)malloc(sizeof(struct list)); 
    // Dynamically allocated memory still stays allocated until the main program terminates completely
    temp->vertex_index = vertex_index; 
    temp->weight = weight; 
    temp->next = NULL;
    return temp; 
}

enum v_color {
	WHITE, // 0
	GRAY, // 1
	BLACK // 2
};

struct DFS_vertex
{
  int index;
  int d; // discovery time
	int f; // finish time
	int c; // color: 
	struct DFS_vertex* pi; // index of parent vertex
};

// struct DFS_vertex* newDFSVertex(int index)
// {
//   struct DFS_vertex* temp = (struct DFS_vertex*)malloc(sizeof(struct DFS_vertex));
//   temp->index = index;
//   temp->d = -1;
//   temp->f = -1;
//   temp->c = WHITE;
//   temp->pi = NULL;
//   return temp;
// }

struct list* CreateArrayOfAdjList(int n, double G[n][n]);
void PrintAdjList(int vertex_index, struct list* Adj_array);

void GetTransposeOfG(int n, double G[n][n], double GT[n][n]);

int DFS_VISIT(int n, struct list* Adj_array, struct DFS_vertex* DFS_vertices, int u, int time);
struct DFS_vertex* DFS(int n, struct list* Adj_array, int source_index, int* indexOrder);
void printDFS(int n, struct DFS_vertex* DFS_vertices);
void print_scc(int n, struct list* Adj_array, int source_index, int* indexOrder);

void SCC(int n, double G[n][n]){
  int * f_arr = (int *)malloc(sizeof(int)*n);
  int * indexOrder = (int *)malloc(sizeof(int)*n);
  double GT[n][n];
  GetTransposeOfG(n, G, GT);
  struct list* Adj_array_G = CreateArrayOfAdjList(n, G);
  struct list* Adj_array_GT = CreateArrayOfAdjList(n, GT);
  int i, j;
  
  int source_vertex = 6;
  struct DFS_vertex* DFS_G = DFS(n, Adj_array_G, source_vertex, NULL);
  for (int i = 0; i<n; i++)
	{
		f_arr[i] = DFS_G[i].f;
	}
	qsort(f_arr, n, sizeof(int), compare);
	for (int i = 0; i<n; i++)
	{
	  for(int j=0; j<n; j++){
	   if(DFS_G[j].f == f_arr[i]){
	    indexOrder[i] = DFS_G[j].index;
	    break;
	    }
	  }
	}
	print_scc(n, Adj_array_GT, indexOrder[0], indexOrder);
}

int main(void) {

  // #define DBL_MAX 1.7976931348623158e+308 /* max value */
  const int num_vertices = 7;
  double G[num_vertices][num_vertices] = {
    0,  25.0, DBL_MAX,  DBL_MAX,  DBL_MAX,  DBL_MAX,  DBL_MAX,
    DBL_MAX,  0,  10.0, 14.0, DBL_MAX,  DBL_MAX,  DBL_MAX,
    1,  DBL_MAX,  0,  DBL_MAX,  DBL_MAX,  16.0, DBL_MAX,
    DBL_MAX,  6.0,  18.0, 0,  DBL_MAX,  DBL_MAX,  DBL_MAX,
    DBL_MAX,  DBL_MAX,  DBL_MAX,  DBL_MAX,  0,  DBL_MAX,  DBL_MAX,
    DBL_MAX,  DBL_MAX,  DBL_MAX,  32.0, 42.0, 0,  14.0,
    DBL_MAX,  DBL_MAX,  DBL_MAX,  DBL_MAX,  DBL_MAX,  11.0, 0,
  };

  // adjacency list for G
  struct list* Adj_array_G = CreateArrayOfAdjList(num_vertices, G);

  // print the adjacency lists
  printf("Ajacency List of G\n");
  for(int i=0; i<num_vertices; i++)
    PrintAdjList(i, Adj_array_G);

  // get the transpose of G
  double GT[num_vertices][num_vertices];
  GetTransposeOfG(num_vertices, G, GT);

  // adjacency list for GT
  struct list* Adj_array_GT = CreateArrayOfAdjList(num_vertices, GT);

  // print the adjacency lists
  printf("Ajacency List of GT\n");
  for(int i=0; i<num_vertices; i++)
    PrintAdjList(i, Adj_array_GT);

  // do DFS on G
  int source_vertex = 0;
  struct DFS_vertex* DFS_result = DFS(num_vertices, Adj_array_G, source_vertex, NULL);
  printf("DFS result on G; Source Vertex: %d\n", source_vertex);
  printDFS(num_vertices, DFS_result);
  
  SCC(num_vertices, G);
  // if source vertex changes, does the SCC() result change?
  // write your answer below
  //
  //yes.it works well (we can change source_vertex at line 80)
  //
  
  return 0;
}

void printDFS(int n, struct DFS_vertex* DFS_vertices)
{
	for (int u = 0; u<n; u++)
	{
		if (DFS_vertices[u].pi == NULL)
			printf("%d: d=%d, f=%d, pi=%d(root)\n", DFS_vertices[u].index, DFS_vertices[u].d, DFS_vertices[u].f, -1);
		else
			printf("%d: d=%d, f=%d, pi=%d\n", DFS_vertices[u].index, DFS_vertices[u].d, DFS_vertices[u].f, DFS_vertices[u].pi->index);
	}
}

int DFS_VISIT(int n, struct list* Adj_array, struct DFS_vertex* DFS_vertices, int u, int time)
{
	// write your code here
  DFS_vertices[u].c = GRAY;
  time = time + 1;
  DFS_vertices[u].d = time;
  struct list* curr = &Adj_array[u];
  while (curr->next != NULL)
  {
    curr = curr->next;
    if(DFS_vertices[curr->vertex_index].c == WHITE){
      DFS_vertices[curr->vertex_index].pi = &DFS_vertices[u];
      time = DFS_VISIT(n, Adj_array, DFS_vertices, curr->vertex_index, time);
    }
  }
  DFS_vertices[u].c = BLACK;
  time = time + 1;
  DFS_vertices[u].f = time;
	return time;
}

struct DFS_vertex* DFS(int n, struct list* Adj_array, int source_index, int* indexOrder)
{
  struct DFS_vertex* DFS_vertices = (struct DFS_vertex*)malloc(sizeof(struct DFS_vertex)*n);
	for (int u = 0; u<n; u++)
	{
    DFS_vertices[u].index = u;
		DFS_vertices[u].c = WHITE;
		DFS_vertices[u].pi = NULL;
	}
	int time = 0;

	// do DFS_VISIT for the source vertex first
	time = DFS_VISIT(n, Adj_array, DFS_vertices, source_index, time);

	for (int i = 0; i<n; i++)
	{
		if (indexOrder == NULL)
		{
			if (DFS_vertices[i].c == WHITE)
				time = DFS_VISIT(n, Adj_array, DFS_vertices, i, time);
		}
		else
		{
			if (DFS_vertices[indexOrder[i]].c == WHITE)
				time = DFS_VISIT(n, Adj_array, DFS_vertices, indexOrder[i], time);
		}
	}
  return DFS_vertices;
}

void GetTransposeOfG(int n, double G[n][n], double GT[n][n])
{
  // write your code here
  int i, j;
  for(i = 0; i< n; i++){
    for(j=0; j< n; j++){
      GT[i][j] = G[j][i];
    }
  }
}

struct list* CreateArrayOfAdjList(int n, double G[n][n])
{
  struct list* Adj_array = (struct list*)malloc(sizeof(struct list)*n);
  for (int i=0; i<n; i++)
  {
    Adj_array[i].next = NULL;
    struct list* prev = &Adj_array[i];
    for (int j=0; j<n; j++)
    {
      if (i==j) continue;
      if (G[i][j] != DBL_MAX)
      {
        struct list* curr = newList(-1, -1);
        prev->next = curr;
        curr->vertex_index = j;
        curr->weight = G[i][j];
        curr->next = NULL;
        prev = curr;
      }
    }
  }
  return Adj_array;
}

void PrintAdjList(int vertex_index, struct list* Adj_array)
{
  printf("%d → ", vertex_index);

  struct list* curr = &Adj_array[vertex_index];
  while (curr->next != NULL)
  {
    curr = curr->next;
    printf("%d, %f → ", curr->vertex_index, curr->weight);
  }
  printf("nil\n");
  return;
}

void print_scc(int n, struct list* Adj_array, int source_index, int* indexOrder)
{
  printf("\nSCC result : \n");
  struct DFS_vertex* DFS_vertices = (struct DFS_vertex*)malloc(sizeof(struct DFS_vertex)*n);
	for (int u = 0; u<n; u++)
	{
    DFS_vertices[u].index = u;
    DFS_vertices[u].f = -1;
		DFS_vertices[u].c = WHITE;
		DFS_vertices[u].pi = NULL;
	}
	int time = 0;
	int last_time;

	// do DFS_VISIT for the source vertex first
	time = DFS_VISIT(n, Adj_array, DFS_vertices, source_index, time);
  printf("{");
  for(int i=0; i<n; i++){
    if(DFS_vertices[i].f <= time && DFS_vertices[i].f!=-1){
      printf("%d, ", DFS_vertices[i].index);
    }
  }
  printf("}");
  last_time = time;
	for (int i = 0; i<n; i++)
	{
			if (DFS_vertices[indexOrder[i]].c == WHITE){
				time = DFS_VISIT(n, Adj_array, DFS_vertices, indexOrder[i], time);
				printf(", {");
    		for(int i=0; i<n; i++){
          if(DFS_vertices[i].f>last_time && DFS_vertices[i].f!=-1){
            printf("%d, ", DFS_vertices[i].index);
          }
        }
        printf("}");
        last_time = time;
			}
	}
}
