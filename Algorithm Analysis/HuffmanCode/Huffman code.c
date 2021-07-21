#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

char* sorted_code[128];

struct zNode {
  struct zNode* left;
  struct zNode* right;
  char letter;
  int freq;
};

struct QNode { 
    struct zNode* key; 
    struct QNode* next; 
}; 
struct QNode* deq_return = NULL;
struct Queue { 
    struct QNode *front, *rear; 
}; 

struct zNode* newZ(int ch, int count){
  struct zNode* temp = (struct zNode*)malloc(sizeof(struct zNode)); 
  temp->left = NULL; 
  temp->right = NULL;
  temp->letter = ch;
  temp->freq = count;
  return temp; 
}

struct QNode* newNode(struct zNode* z) 
{ 
    struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode)); 
    temp->key = z; 
    temp->next = NULL; 
    return temp; 
} 
  
// A utility function to create an empty queue 
struct Queue* createQueue() 
{ 
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue)); 
    q->front = q->rear = NULL; 
    return q; 
} 

void dfs(struct zNode* z, char str[]){
  if(z->left==NULL && z->right ==NULL){
    //printf("%c :  %s\n", z->letter, str);
    sorted_code[z->letter] = (char*)malloc(sizeof(char)*strlen(str));
    strcpy(sorted_code[z->letter], str);
  }
  if(z->left!=NULL){
    char tempstr1[100]={0x0,};
    strcpy(tempstr1, str);
    strcat(tempstr1, "0");
    dfs(z->left, tempstr1);
  }
  if(z->right!=NULL){
    char tempstr2[100]={0x0,};
    strcpy(tempstr2, str);
    strcat(tempstr2, "1");
    dfs(z->right, tempstr2);
  }
}

// The function to add a key k to q 
void enQueue(struct Queue* q, struct zNode* z) 
{ 
    // Create a new LL node 
    struct QNode* temp = newNode(z); 
  
    // If queue is empty, then new node is front and rear both 
    if (q->rear == NULL) { 
        q->front = q->rear = temp; 
        return; 
    } 
  
    // Add the new node at the end of queue and change rear
    q->rear->next = temp; 
    q->rear = temp;
}

void deQueue(struct Queue* q) 
{ 
    // If queue is empty, return NULL. 
    if (q->front == NULL) 
        return; 
  
    // Store previous front and move front one node ahead 
    struct QNode* min = q->front;
    struct QNode* temp = q->front;
    while(temp != NULL){
      if(temp->key->freq < min->key->freq){
        min = temp;
      }
      temp = temp->next;
    }
    if(q->front == min){
      deq_return = q->front;
      q->front = q->front->next;
    }
    else{
      temp = q->front;
      while(1){
        if(temp->next == min){
          deq_return = temp->next;
          temp->next = temp->next->next;
          break;
        }
        temp = temp->next;
      }
    }
    temp = q->front;
    if (q->front == NULL){
        q->rear = NULL;
    }
    else{
      while(1){
        if(temp->next==NULL){
          q->rear = temp;
          break;
        }
        temp= temp->next;
      }
    }
}

int main(void) {
  char text[] = "HGU CSEE is a higher educational institution that cultivates excellent and honest professionals who will serve God and people by playing a role as salt and light in the darkening world. A CSEE student should strive to develop expertise in IT-related fields based on the unshakable integrity of the Christian spirit exemplified by love and righteousness. This guideline aims to clarify the standard of honesty, sincerity and responsibility the students should keep in order that they may overcome temptations they encounter during their studies and develop into trustworthy and capable professionals.";
  //char text[]="aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbccccccccccccddddddddddddddddeeeeeeeeefffff";
  //printf("%s\n", text);
  
  int i;
  int letter_count = 0;
  int* ascii= (int*)malloc(sizeof(int)*strlen(text));
  int count[128] ={0,};
  struct Queue* q = createQueue();
  for(i=0; i<strlen(text); i++){
    ascii[i] = text[i];
  }
  for(i=0; i<strlen(text); i++){
    count[text[i]]++;
  }
  for(i=0; i<128; i++){
    if(count[i]>0){
      letter_count++;
      struct zNode * temp =NULL;
      temp = newZ(i, count[i]);
      enQueue(q, temp);
    }
  }
  for(i=1; i<letter_count; i++){
    int sum = 0;
    struct zNode* temp_node = newZ(0, 0);
    deQueue(q);
    temp_node->left = deq_return->key;
    sum+= deq_return->key->freq;
    deQueue(q);
    temp_node->right = deq_return->key;
    sum+= deq_return->key->freq;
    temp_node->freq = sum;
    enQueue(q, temp_node);
  }
  deQueue(q);
  char code[100] ={0x0,};
  
  dfs(deq_return->key, code);
  
  for(i=0; i<128; i++){
    if(sorted_code[i]!=NULL){
      printf("%c : %s\n", i, sorted_code[i]);
    }
  }
  
  //printf("The length of the give text is %ld\n", strlen(text));
  
  // hint: check the number of distinct letters
  // hint: calculate the frequency of each letter
  
  // find "Huffman-Code" pseudo code in the lexture note
  // hint: refer to the following website to get an example of how to implement queue
  // https://www.geeksforgeeks.org/queue-linked-list-implementation/
  // but you need to implemnt queue extraction function
  
  // get an optimal Huffman-Code
  
  // print your optimal Huffman code

  return 0;
}

////////////////////////////////////////////////////////////
//
// mention your official classes' schedule here
// ex) OOO class 4pm - 6pm on 5/7(Fri) (You may be later asked to provide proof of that schedule.)
//
////////////////////////////////////////////////////////////