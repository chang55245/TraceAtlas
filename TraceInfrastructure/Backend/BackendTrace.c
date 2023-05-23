#include "Backend/BackendTrace.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>


FILE *myfile;

//trace functions
z_stream strm_DashTracer;

int TraceCompressionLevel;
char *TraceFilename;
/// <summary>
/// The maximum ammount of bytes to store in a buffer before flushing it.
/// </summary>
// #define BUFSIZE 128 * 1024
#define BUFSIZE 512 * 1024
unsigned int bufferIndex = 0;
uint8_t temp_buffer[BUFSIZE];
uint8_t storeBuffer[BUFSIZE];

static struct Node* loadRoot= NULL;
static struct Node* storeRoot= NULL;


// Binary search tree node structure
typedef struct Node {
    uint64_t key;
    bool isRed;
    uint64_t size;
    struct Node* left;
    struct Node* right;
}Node;

// Function to create a new node
static struct Node* newNode(uint64_t key,uint64_t size) {
    // Allocate memory for a new node
    Node* node = malloc(sizeof(struct Node));
    if (node == NULL) {
        printf("Error: Out of memory\n");
        exit(1);
    }

    // if (key == 0) {
    //     printf("Error new: Key is 0\n");
    // }
    // Initialize node properties
    node->key = key;
    node->size = size;
    node->isRed = true;
    node->left = NULL;
    node->right = NULL;
    return node;
}


// Function to rotate a node to the left
static struct Node* rotateLeft( Node* node) {
    // Perform a left rotation
    struct Node* newRoot = node->right;
    node->right = newRoot->left;
    newRoot->left = node;
    newRoot->isRed = node->isRed;
    node->isRed = true;
    return newRoot;
}

// Function to rotate a node to the right
static struct Node* rotateRight( struct Node* node) {
    // Perform a right rotation
    struct Node* newRoot = node->left;
    node->left = newRoot->right;
    newRoot->right = node;
    newRoot->isRed = node->isRed;
    node->isRed = true;
    return newRoot;
}

// Function to flip the colors of a node and its children
void flipColors( struct Node* node) {
    // Flip the colors of a node and its children
    node->isRed = !node->isRed;
    node->left->isRed = !node->left->isRed;
    node->right->isRed = !node->right->isRed;
}



static struct Node* insert( struct Node* root, uint64_t key,uint64_t size) {
    if (root == NULL) {
        // If the root is NULL, create a new node and return it
        
        return newNode(key,size);
    }
    
    if (root->key==0) {
        root->key= key;
        root->size = size;
        return root;
    }


    if (root->key==key) {
        root->size = size;
        return root;
    }
    
    if (key < root->key) {
        // If the key is smaller, insert it in the left subtree
        root->left = insert(root->left, key,size);
    } else if (key > root->key) {
        // If the key is larger, insert it in the right subtree
        root->right = insert(root->right, key,size);
    }

    

    // Fix violations of the Red-Black tree properties
    if (root->right != NULL && root->right->isRed && (root->left == NULL || !root->left->isRed)) {
        root = rotateLeft(root);
    }
    if (root->left != NULL && root->left->isRed && root->left->left != NULL && root->left->left->isRed) {
        root = rotateRight(root);
    }
    if (root->left != NULL && root->left->isRed && root->right != NULL && root->right->isRed) {
        flipColors(root);
    }

    return root;
}

// Function to search for a key in the binary search tree
bool search( struct Node* root, uint64_t key, uint64_t size) {
    if (root == NULL) {
        return false;
    }
    // if (root->key == 0 ) 
    // {
    //     printf("Error search: Key is 0\n");
    // }
    //get the key of *root
    


    if (key == root->key) {
        if (size>root->size) {

            return false;
        }
        return true;
    } 
    
    if (key < root->key) {
        return search(root->left, key,size);
    }          
    
    return search(root->right, key,size);
   
}


// Function to clear the binary search tree but keep the root



static struct Node* clear(  struct Node* root) {
    if (root == NULL) {
        return NULL;
    }
    root->left = clear(root->left);
    root->right = clear(root->right);
    if (root != loadRoot && root != storeRoot) {
        // free(root);
        root = NULL;
    }
    else {
        root->key = 0;
        root->size = 0;
    }
   
    return root;
}




void WriteLoadNode( struct Node* root) {
    if (root == NULL) {
        return;
    }
    WriteLoadNode(root->left);
    
    WriteLoadNode(root->right);

    char fin[128];
    sprintf(fin, "LoadAddress:%#lX\n", root->key);
    WriteStream(fin);
    memset(fin, 0, sizeof(fin));
    if (root->size == 0) {
        printf("Error: size is 0\n");
        exit(4);
    }

    sprintf (fin, "LoadSize:%#lX\n", root->size);
    WriteStream(fin);
}

void WriteStoreNode( struct Node* root) {
    if (root == NULL) {
        return;
    }
    WriteStoreNode(root->left);
    
    WriteStoreNode(root->right);

    char fin[128];
    sprintf(fin, "StoreAddress:%#lX\n", root->key);
    WriteStream(fin);
    memset(fin, 0, sizeof(fin));
    if (root->size == 0) {
        printf("Error: size is 0\n");
        exit(4);
    }
    sprintf (fin, "StoreSize:%#lX\n", root->size);
    WriteStream(fin);
}


void SwitchKernel()
{
    // if see kernel enter or exit, call this to clear the tree for load and store

    // if (loadRoot != NULL && loadRoot->key!=0) 
    // {
    //     WriteLoadNode(loadRoot);
    
    // }
    // if (storeRoot!= NULL && storeRoot->key!=0) 
    // {
    //     WriteStoreNode(storeRoot);  
    // }
    
    
    loadRoot = clear(loadRoot);

    storeRoot = clear(storeRoot);
}

void WriteStream(char *input)
{
    size_t size = strlen(input);
    if (bufferIndex + size >= BUFSIZE)
    {
        BufferData();
    }
    memcpy(storeBuffer + bufferIndex, input, size);
    bufferIndex += size;
}

///Modified from https://stackoverflow.com/questions/4538586/how-to-compress-a-buffer-with-zlib
void BufferData()
{
    strm_DashTracer.next_in = storeBuffer;
    strm_DashTracer.avail_in = bufferIndex;
    strm_DashTracer.next_out = temp_buffer;
    strm_DashTracer.avail_out = BUFSIZE;

    while (strm_DashTracer.avail_in != 0)
    {
        int defResult = deflate(&strm_DashTracer, Z_NO_FLUSH);

        if (defResult != Z_OK)
        {
            fprintf(stderr, "Zlib compression error %d",defResult);
            if (defResult ==Z_BUF_ERROR) {
                fprintf(stderr, "input buffer is too small");
            }
            else if (defResult ==Z_MEM_ERROR ) {
                fprintf(stderr, "ouput buffer is too small");
            }
            exit(-1);
        }

        if (strm_DashTracer.avail_out == 0)
        {
            for (int i = 0; i < BUFSIZE; i++)
            {
                fputc(temp_buffer[i], myfile);
            }
            strm_DashTracer.next_out = temp_buffer;
            strm_DashTracer.avail_out = BUFSIZE;
        }
    }
    for (uint32_t i = 0; i < BUFSIZE - strm_DashTracer.avail_out; i++)
    {
        fputc(temp_buffer[i], myfile);
    }
    strm_DashTracer.next_out = temp_buffer;
    strm_DashTracer.avail_out = BUFSIZE;
    bufferIndex = 0;
}

void Write(char *inst, int line, int block, uint64_t func)
{
    char suffix[128];
#if defined _WIN32
    sprintf(suffix, ";line:%d;block:%d;function:%llu\n", line, block, func);
#else
    sprintf(suffix, ";line:%d;block:%d;function:%lu\n", line, block, func);
#endif
    size_t size = strlen(inst) + strlen(suffix);
    char fin[size];
    strcpy(fin, inst);
    strncat(fin, suffix, 128);
    WriteStream(fin);
}

void WriteAddress(char *inst, int line, int block, uint64_t func, char *address)
{
    char suffix[128];
#if defined _WIN32
    sprintf(suffix, ";line:%d;block:%d;function:%llu;address:%llu\n", line, block, func, (uint64_t)address);
#else
    sprintf(suffix, ";line:%d;block:%d;function:%lu;address:%lu\n", line, block, func, (uint64_t)address);
#endif
    size_t size = strlen(inst) + strlen(suffix);
    char fin[size];

    strcpy(fin, inst);
    strncat(fin, suffix, 128);
    WriteStream(fin);
}

void OpenFile()
{
    char *tcl = getenv("TRACE_COMPRESSION");
    if (tcl != NULL)
    {
        int l = atoi(tcl);
        TraceCompressionLevel = l;
    }
    else
    {
        TraceCompressionLevel = 5;
    }

    TraceCompressionLevel = 1;
    strm_DashTracer.zalloc = Z_NULL;
    strm_DashTracer.zfree = Z_NULL;
    strm_DashTracer.opaque = Z_NULL;
    deflateInit(&strm_DashTracer, TraceCompressionLevel);
    char *tfn = getenv("TRACE_NAME");
    if (tfn != NULL)
    {
        TraceFilename = tfn;
    }
    else
    {
        TraceFilename = "raw.trc";
    }

    myfile = fopen(TraceFilename, "w");
    WriteStream("TraceVersion:3\n");
}

void CloseFile()
{
    strm_DashTracer.next_in = storeBuffer;
    strm_DashTracer.avail_in = bufferIndex;
    strm_DashTracer.next_out = temp_buffer;
    strm_DashTracer.avail_out = BUFSIZE;
    deflate(&strm_DashTracer, Z_FINISH);
    for (uint32_t i = 0; i < BUFSIZE - strm_DashTracer.avail_out; i++)
    {
        fputc(temp_buffer[i], myfile);
    }

    deflateEnd(&strm_DashTracer);
    //fclose(myfile); //breaks occasionally for some reason. Likely a glibc error.
}





void checkTree( struct Node* root)
{
    uint64_t temp = 0;
    if (root == NULL) {
        return;
    }
    checkTree(root->left);
    checkTree(root->right);
    if (root->key == 0 || root->size == 0) {
        printf("Error check: Key or size is 0\n");
    }
    
    
    temp = root->key;

}

void LoadDump(void *address,int size)
{
    // printf("size:%d\n",size);
    // checkTree(loadRoot);

    if (size == 0) {
        exit(4);
    }

    if (!search(loadRoot, (uint64_t)address,(uint64_t)size) )
    {
        loadRoot = insert(loadRoot, (uint64_t)address, (uint64_t)size);
        // char fin[128];
    
        // sprintf(fin, "LoadAddress:%#lX\n", (uint64_t)address);
        // WriteStream(fin);



        char fin[128];
        sprintf(fin, "LoadAddress:%#lX\n", (uint64_t)address);
        WriteStream(fin);
        memset(fin, 0, sizeof(fin));
        if (size == 0) {
            printf("Error: size is 0\n");
            exit(4);
        }
        sprintf (fin, "LoadSize:%#lX\n", (uint64_t)size);
        WriteStream(fin);
    }
    // else {
    //     printf("skip load\n");
    // }
    // checkTree(loadRoot);
}
void DumpLoadValue(void *MemValue, int size)
{
    char fin[128];
    uint8_t *bitwisePrint = (uint8_t *)MemValue;
    sprintf(fin, "LoadValue:");
    WriteStream(fin);
    for (int i = 0; i < size; i++)
    {
        if (i == 0)
        {
            sprintf(fin, "0X%02X", bitwisePrint[i]);
        }
        else
        {
            sprintf(fin, "%02X", bitwisePrint[i]);
        }
        WriteStream(fin);
    }
    sprintf(fin, "\n");
    WriteStream(fin);
}
void StoreDump(void *address,int size)
{

    // printf("size:%d\n",size);
    // checkTree(storeRoot);

     if (size == 0) {
        exit(4);
    }

    if (!search(storeRoot, (uint64_t)address,(uint64_t)size))
    {
        storeRoot = insert(storeRoot, (uint64_t)address,(uint64_t)size);

        // char fin[128];
        
        // sprintf(fin, "StoreAddress:%#lX\n", (uint64_t)address);
        // WriteStream(fin);




        char fin[128];
        sprintf(fin, "StoreAddress:%#lX\n", (uint64_t)address);
        WriteStream(fin);
        memset(fin, 0, sizeof(fin));
        if (size == 0) {
            printf("Error: size is 0\n");
            exit(4);
        }
        sprintf (fin, "StoreSize:%#lX\n", (uint64_t)size);
        WriteStream(fin);
    }
    // else {
    //     printf("skip store\n");
    // }


    // checkTree(storeRoot);
    
}

void MemCpyDump(void *dest,void *src,void *len)
{
    char fin[128];
    sprintf(fin, "MemCpy:%#lX,%#lX,%lu\n", (uint64_t)src,(uint64_t)dest,(uint64_t)len);
    WriteStream(fin);
    // printf("MemCpy:%#lX,%#lX,%lu\n", (uint64_t)src,(uint64_t)dest,(uint64_t)len);
}

void DumpStoreValue(void *MemValue, int size)
{
    char fin[128];
    uint8_t *bitwisePrint = (uint8_t *)MemValue;
    sprintf(fin, "StoreValue:");
    WriteStream(fin);
    for (int i = 0; i < size; i++)
    {
        if (i == 0)
        {
            sprintf(fin, "0X%02X", bitwisePrint[i]);
        }
        else
        {
            sprintf(fin, "%02X", bitwisePrint[i]);
        }
        WriteStream(fin);
    }
    sprintf(fin, "\n");
    WriteStream(fin);
}

void BB_ID_Dump(uint64_t block, bool enter)
{
    char fin[128];
    if (enter)
    {
        sprintf(fin, "BBEnter:%#lX\n", block);
    }
    else
    {
        sprintf(fin, "BBExit:%#lX\n", block);
    }
    WriteStream(fin);
}

void KernelEnter(char *label)
{
    SwitchKernel();
    char fin[128];
    strcpy(fin, "KernelEnter:");
    strcat(fin, label);
    strcat(fin, "\n");
    WriteStream(fin);
}
void KernelExit(char *label)
{
    SwitchKernel();
    char fin[128];
    strcpy(fin, "KernelExit:");
    strcat(fin, label);
    strcat(fin, "\n");
    WriteStream(fin);
}

void NonKernelSplit()
{
    SwitchKernel();
    char fin[128];
    strcpy(fin, "NonKernelSplit:get\n");
    WriteStream(fin);
}