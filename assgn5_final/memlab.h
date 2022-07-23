/**
 * @file memlab.h
 * @author Anurat Bhattacharya (19CS10071) Srijan Das (19CS30046)
 * @brief Asignment 5
 * @version 0.5
 * @date 2022-03-21
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef MEM_LAB
#define MEM_LAB

#include<pthread.h>
#include<bits/stdc++.h>
#include<unistd.h>
#include<stdio.h>
#include<fcntl.h>
#include<time.h>
#include<sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
using namespace std;
#define INT 0
#define CHAR 1
#define mdINT 2
#define BOOL 3
#define WORD_SZ 4
#define SZ 500000 //Number of words in memory
#define ADDR_SZ 16  //16 bit address space
#define STK_SZ 1000
#define SET_SZ 1000
#define LAZY_CLR -2
#define POOL_SIZE 1000
#define ACTIVE_CLR -1
#define COLLECT_GARBAGE

//extern int GRP_SZ ;//set Adaptively from createMem
//extern int CHAIN_SZ ;
extern int varSz[4] ; //Variable type sizes
template<class T>
void print(T t)  {
    cout<<t<<endl;    
}
template<class T,class...S>
void print(T t,S... ss)  {
    cout<<t<<" ";
    print(ss...);
}

/**
 * @brief Stack But with the feasture of also allowing random acces(more like a vector)
 * 
 */
class Mem{
    public:
    //starting logical address
    int ind;
    //number of 4 byte blocks
    int size;
    
    bool operator<(const Mem& other){
        if(size==other.size)  return ind<other.ind;
        return size<other.size;
    }
    bool operator>(const Mem& other){
        if(size==other.size)  return ind>other.ind;
        return size>other.size;
    }
    bool operator==(const Mem& other){
        return (size==other.size); 
    }
    bool operator!=(const Mem& other){
        return !(size==other.size); 
    }
    bool operator<=(const Mem& other){
        return (*this)<other || (*this)==other;
    }
    bool operator>=(const Mem& other){
        return (*this)>other || (*this)==other;
    }

};

class Node{
    public:
    int beg;//Logical address
    int seq;//sequence number of the variable
    int symind;//symbol table index
    int end;//end logoical address
    Node* nxt,*prev;//Pointer to the next free block
    Node() : nxt(NULL),prev(NULL){

    }

};
class Stack{
    ///int arr[STK_SZ];
    public:
    int *arr;
    int top;//top index
    int sz;
    
    
    Stack() :top(-1),sz(0){}
    int size() const {
        return sz;
    }
    void push_back(int x){
        if(sz==STK_SZ)    return;
        arr[++top]   = x;
        sz++;
    }
    int pop_back(){
        if(sz==0) return -1;
        return arr[top--];
        sz--;
    }
    int& back(){
        
        return arr[top];
    }
    int& operator[](int ind){
        return arr[ind];
    }
};
class Variable{
    public:
    int val;
    int type;
    int sz;//size in bits
};
struct sym_entry{
    int offset;//index in memory segment(that is the block index)
    int flag;//Initilized to 0 Used in state rotation while doing mark and sweep
    int isEmpty;//1 for empty
    Variable v;
    int isArr;//Indicate if current entry is for an array
    int seq;//indicattes the sequence number of the variable generated

};
/**
 * @brief Create a Mem object
 * 
 * @param totalSpace Total space in bytes
 */
void createMem(size_t totalSpace);//total or like malloc
/**
 * @brief Create a Var object
 * 
 * @param type 
 * @return int The memory value location
 */
int createVar(int type);
/**
 * @brief Assign variable ar position memfd1 with val
 * 
 * @param memfd1 The input pointer to logical address space
 * @param val The value to be assigned
 */
void assignVar(int memfd1, int val);
/**
 * @brief Assign to array index
 * 
 * @param memfd1 start address of array
 * @param offset index of array
 * @param val value
 */
void assignVar(int memfd1, int offset,int val);
/**
 * @brief Create a Arr object
 * 
 * @param type Type of array elem
 * @param size Number of elements
 * @return int 
 */
int createArr(int type,int size);
/**
 * @brief 
 * 
 * @param memfd 
 * @return int 1 if success 0 if failure
 */
int freeElem(int memfd);



/**
 * @brief Pops last element created from scope stack
 * 
 */
void pop_last();

/**
 * @brief To insert free segment into the bookeeping data structures
 * 
 * @param it 
 */
//void insert(Mem it);
int get(int memfd,int i);
int get(int memfd);
/**
 * @brief To clear all Free Seg book keeping Data structures
 * 
 */
void printList(Node* head);
void clear();
void end_session();
void send_gc_signal();
/************************************Global Variables ***********************************/
//Set Available;//set of free available blocks set used to implement best fit algorithm
//Bst<pair<int,int>> Intv;//For storing symbol table empty elements
//Bst<pair<int,int>> Intv2;//For storing Mem free blocks ordered by index

//Stores the free segments as a 2D array 
//freeSeg[i][j] gets the free seg from the ith group


extern sym_entry *symbolTable;
extern int* basep;
extern Node* listPool;
extern Node* memcnt;
extern int nvar;//number of variables ddeclared so far
extern Node* root;//for storing free segments
extern Node* root2;//for storing filled segments
extern int n;
extern pthread_mutex_t mutex_lock;
extern Stack stk;//stack of symbpol table indices
extern Stack freeSym;//stack of free symbol table indices
/*************************************************************************************/


/***********GARBAGE COLLECTION**********************/
void gc_initialize();
extern pthread_t gc_collector;
void gc_run();
void compact();
void* gc_runner(void* arg); 
/**************************************************/
#endif
