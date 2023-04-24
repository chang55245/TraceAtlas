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

// Binary search tree node structure
struct Node {
    int64_t key;
    bool isRed;
    struct Node* left;
    struct Node* right;
};

// Function to create a new node
struct Node* newNode(int64_t key) {
    // Allocate memory for a new node
    struct Node* node = (struct Node*)malloc(sizeof(struct Node));
    if (node == NULL) {
        printf("Error: Out of memory\n");
        exit(1);
    }
    // Initialize node properties
    node->key = key;
    node->isRed = true;
    node->left = NULL;
    node->right = NULL;
    return node;
}


// Function to rotate a node to the left
struct Node* rotateLeft(struct Node* node) {
    // Perform a left rotation
    struct Node* newRoot = node->right;
    node->right = newRoot->left;
    newRoot->left = node;
    newRoot->isRed = node->isRed;
    node->isRed = true;
    return newRoot;
}

// Function to rotate a node to the right
struct Node* rotateRight(struct Node* node) {
    // Perform a right rotation
    struct Node* newRoot = node->left;
    node->left = newRoot->right;
    newRoot->right = node;
    newRoot->isRed = node->isRed;
    node->isRed = true;
    return newRoot;
}

// Function to flip the colors of a node and its children
void flipColors(struct Node* node) {
    // Flip the colors of a node and its children
    node->isRed = !node->isRed;
    node->left->isRed = !node->left->isRed;
    node->right->isRed = !node->right->isRed;
}



struct Node* insert(struct Node** root, int64_t key) {
    if (*root == NULL) {
        // If the root is NULL, create a new node and return it
        
        return newNode(key);
    }

    if (key < (*root)->key) {
        // If the key is smaller, insert it in the left subtree
        insert(&(*root)->left, key);
    } else if (key > (*root)->key) {
        // If the key is larger, insert it in the right subtree
        insert(&(*root)->right, key);
    }

    // Fix violations of the Red-Black tree properties
    if ((*root)->right != NULL && (*root)->right->isRed && ((*root)->left == NULL || !(*root)->left->isRed)) {
        (*root) = rotateLeft((*root));
    }
    if ((*root)->left != NULL && (*root)->left->isRed && (*root)->left->left != NULL && (*root)->left->left->isRed) {
        (*root) = rotateRight((*root));
    }
    if ((*root)->left != NULL && (*root)->left->isRed && (*root)->right != NULL && (*root)->right->isRed) {
        flipColors((*root));
    }

    return (*root);
}

// Function to search for a key in the binary search tree
bool search(struct Node* root, int64_t key) {
    if (root == NULL) {
        return false;
    }

    if (key == root->key) {
        return true;
    } 
    
    if (key < root->key) {
        return search(root->left, key);
    }          
    
    return search(root->right, key);
   
}


void clear(struct Node* root) {
    if (root == NULL) {
        return;
    }

    clear(root->left);
    root->left = NULL;
    clear(root->right);
    root->right = NULL;
    free(root);
    root = NULL;
}

void clear_tree(struct Node** root) {
    if (*root == NULL) {
        return;
    }

    clear(*root);
    *root = NULL;
}


struct Node* loadRoot= NULL;
struct Node* storeRoot= NULL;

void SwitchKernel()
{
    // if see kernel enter or exit, call this to clear the tree for load and store
    clear_tree(&loadRoot);
    clear_tree(&storeRoot);
    loadRoot= NULL;
    storeRoot= NULL;
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

void LoadDump(void *address)
{
    if (!search(loadRoot, (int64_t)address))
    {
        loadRoot = insert(&loadRoot, (int64_t)address);
        char fin[128];
        // fprintf(stderr, "load \n");
        sprintf(fin, "LoadAddress:%#lX\n", (uint64_t)address);
        WriteStream(fin);
    }
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
void StoreDump(void *address)
{

    if (!search(storeRoot, (int64_t)address))
    {
        storeRoot = insert(&storeRoot, (int64_t)address);
        char fin[128];
        // fprintf(stderr, "load \n");
        sprintf(fin, "StoreAddress:%#lX\n", (uint64_t)address);
        WriteStream(fin);
    }
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