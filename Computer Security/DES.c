#include <stdio.h>
#include <string.h>
#include <stdlib.h>


const char* filepath = "des.txt";

//unsigned long long plain = 0x123456abcd132536;
//unsigned long long key = 0xAABB09182736CCDD;
unsigned long long plain = 0;
unsigned long long key = 0;
unsigned long long cypher = 0;

int parityDropTable[56] = {0, };
int compressionTable[48] = {0, };
int initPTable[64] = {0, };
int finalPTable[64] = {0,};
int expansionPbox[48] = {0, };
int straightPbox[32] = {0, };
int sboxTable[8][64] = {0, };
char buffer[1024] = {0x0, };
unsigned long long roundkeys[16] = {0, };
char input[256] = {0x0, };

void loadArrFromLine(char str[], int store[])
{
    char *ptr = strtok(str, "{");
    ptr = strtok(NULL, ",");
    if(*ptr == '{'){//sbox case
        ptr = ptr+1;
    }
    int i=0;
    while(ptr != NULL)
    {
        store[i++] = atoi(ptr);
        ptr = strtok(NULL, ",");
    }
}

void loadAllArr(){
    int i;
    FILE *fp = fopen(filepath, "r");
    if(fp == 0x0){
        printf("wrong file path\n");
        return;
    }
    fgets(buffer, sizeof(buffer), fp);
    loadArrFromLine(buffer, parityDropTable); fgets(buffer, sizeof(buffer), fp);
	fgets(buffer, sizeof(buffer), fp);
    loadArrFromLine(buffer, compressionTable); fgets(buffer, sizeof(buffer), fp);
    fgets(buffer, sizeof(buffer), fp);
    loadArrFromLine(buffer, initPTable); fgets(buffer, sizeof(buffer), fp);
    fgets(buffer, sizeof(buffer), fp);
    loadArrFromLine(buffer, finalPTable); fgets(buffer, sizeof(buffer), fp);
    fgets(buffer, sizeof(buffer), fp);
    loadArrFromLine(buffer, expansionPbox); fgets(buffer, sizeof(buffer), fp);
    fgets(buffer, sizeof(buffer), fp);
    loadArrFromLine(buffer, straightPbox); fgets(buffer, sizeof(buffer), fp);
    for(i=0; i<8; i++){
        fgets(buffer, sizeof(buffer), fp);
        loadArrFromLine(buffer, sboxTable[i]);
    }
}

unsigned long long pbox(unsigned long long target, int arr[], int beforeSize, int afterSize)
{
    int i;
    unsigned long long result = 0;
    for(i=0; i<afterSize; i++)
    {
        unsigned long long bit = (target >> (beforeSize - arr[i])) & 1;
        if(bit == 1){
            result |= (bit << (afterSize-1-i));
        }
    }
    return result;
}
void splithalf(unsigned long long cKey, unsigned long long* leftKey, unsigned long long* rightKey, int size, unsigned int bit)
{
    *leftKey = ((cKey>>size) & bit);
    *rightKey = (cKey & bit);
}
unsigned long long parityDrop(){
    return pbox(key, parityDropTable, 64, 56);
}

unsigned long long merge(unsigned long long left, unsigned long long right, int size){
    unsigned long long result = 0;
    result |= left<<size;
    result |= right;
    return result;
}
unsigned long long keyComp(unsigned long long tempKey)
{
    return pbox(tempKey, compressionTable, 56, 48);
}
void roundKeyGeneration()
{
    int i;
    unsigned long long cypherKey = parityDrop();
    unsigned long long leftKey = 0;
    unsigned long long rightKey = 0;
    splithalf(cypherKey, &leftKey, &rightKey, 28, 0xfffffff);
    for(i=1; i<=16; i++)
    {
        if(i==1||i==2||i==9||i==16)
        {
            leftKey = leftKey<<1;
            leftKey |= ((leftKey>>28) & 1);
            rightKey = rightKey<<1;
            rightKey |= ((rightKey>>28) & 1);
        }
        else{
            leftKey = leftKey<<2;
            leftKey |= (leftKey>>28 & (0b11));
            rightKey = rightKey<<2;
            rightKey |= (rightKey>>28 & (0b11));
        }
        roundkeys[i-1] = keyComp(merge((leftKey & 0xfffffff), (rightKey & 0xfffffff), 28));
    }
}
unsigned long long sbox(unsigned long long right){
    int i;
    unsigned long long result = 0;
    for(i=0; i<8; i++){
        int temp =0;
        int row = 0;
        int col = 0;
        temp = ((right >> (6* (8-1-i))) & 0b111111);
        row = (((temp>>4)&0b10) | (temp & 1));
        col = ((temp>>1) & 0b1111);
        result = result << 4;
        result |= sboxTable[i][4*col + row];
    }
    return result;
}
unsigned long long roundFunction()
{
    unsigned long long left = 0;
    unsigned long long right = 0;
    unsigned long long temp = 0;
    int i = 0;
    splithalf(cypher, &left, &right, 32, 0xffffffff);
    for(i=0; i<16; i++){
        temp = right;
        right = pbox(right, expansionPbox, 32, 48);
        right = right ^ roundkeys[i];
        right = sbox(right);
        right = pbox(right, straightPbox, 32, 32);
        right = left ^ right;
        left = temp;
    }
    temp = right;
    right = left;
    left = temp;
    return merge(left, right, 32);
}

int main()
{
    int i;
    
    printf("enter plain text (64bit hexadecimal) : ");
    scanf("%s", input);
    sscanf(input, "%llx", &plain);
    printf("enter key (64bit hexadecimal) : ");
    scanf("%s", input);
    sscanf(input, "%llx", &key);
    loadAllArr();
    roundKeyGeneration();
    cypher = pbox(plain, initPTable, 64, 64);
    cypher = roundFunction();
    cypher = pbox(cypher, finalPTable, 64, 64);
    printf("\n==============================");
    printf("\ncypher text : %llx\n", cypher);
    return 0;
}
