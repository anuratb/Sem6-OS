#include<bits/stdc++.h>
#include<pthread.h>
#include<ctime>
#include<time.h>
#include <chrono>
#include<shared_mutex>
#include<sys/shm.h>
#include <sys/ipc.h>
#include<sys/wait.h>
#include<unistd.h>
#define fr(i,b,c)   for(int i=b;i<c;i++)
#define N 1000
#define SZ 8
#define USE_MUTEX

#ifndef USE_MUTEX
    #define pthread_mutex_lock(mutex_lock) {}
    #define pthread_mutex_unlock(mutex_lock) {}
#endif
using namespace std;
template<class T>
void print(T t)  {
    cout<<t<<"\n";    
}
template<class T,class...S>
void print(T t,S... ss)  {
    cout<<t<<" ";
    print(ss...);
}

typedef struct _proc_data{
    int prodId;
    /**
     * @brief i: 0,N/2
     *        j: 0,N/2
     */
    int i,j;
    int cnt;//number of blocks done
    int flg;//1 for already acessed 0 for not yet acessed
    int *Mat;
    _proc_data *res;//result for current mult
    int matId;
}Jobs;

/**
 * @brief Job Queue 
 * 
 */
typedef struct _queue{
    Jobs J[SZ];
    int beg,lst;
    int sz;
    _queue():beg(0),lst(-1),sz(0) {}
    void push(Jobs x){
        J[(++lst)%SZ] = x;
        sz++; 
    }
    void pop(){
        beg++;
        beg%=SZ;
        sz--;
    }
    Jobs& operator[](int x){
        return J[(beg+x)%SZ];
    }
}Queue;
typedef int (*func)() ;
/************************************Variables shared***********************************/
//func *rnd;
int *jobs_created = 0;//Number of jobs created
int *jobs_done = 0;
int *new_jobs = 0;
int *tot;//total matrices needed to create
Queue *Q;//job queue
pthread_mutex_t *mutex_lock;
void **memcnt;//shared memory counter
/*************************************************************************************/


void produceJob(void *arg);
void consumeJob(void *arg);





void mult(int *A,int *B,int *C,int i1,int j1,int i2,int j2,int i3,int j3,int n1,int n2,int n3){
    /*
    print("---------------------------------------");
    fr(i,0,N){
        fr(j,0,N)   cout<<*(C + N*i+j)<<" ";
        cout<<"\n";
    }*/
    //print(i1,j1,i2,j2,i3,j3);
    print("Enter mult pid ",getpid());
    fr(i,0,n1)  fr(j,0,n3) {
        int val = 0;
        fr(k,0,n2)  val+=((*(A + N*(i1+i)+j1+k)) * (*(B + N*(i2+k)+j2+j)));
        pthread_mutex_lock(mutex_lock);
        *(C + N*(i3+i) + (j3+j))+=val;
        pthread_mutex_unlock(mutex_lock);
    }
    /*
     fr(k,0,n2)  {
        
        pthread_mutex_lock(mutex_lock);
        *(C + N*(i3+i) + (j3+j))+=((*(A + N*(i1+i)+j1+k)) * (*(B + N*(i2+k)+j2+j)));
        pthread_mutex_unlock(mutex_lock);
       
    }*/
    print("Exit mult pid ",getpid());
    /*
    fr(i,0,N){
        fr(j,0,N)   cout<<*(C + N*i+j)<<" ";
        cout<<"\n";
    }
    print("---------------------------------------");*/
}



int main(){

    
    int status = 0;
    srand(time(NULL));
    int nn;
    /*********************************USER INPUT*******************************/
    int p,c;
    cout<<"Enter number of producers and consumers respectively:";
    cin>>p>>c;    
    cout<<"Enter number of matrices to multiply:";
    cin>>nn;
    /************************************************************************/




    /***********************ALLOCATE SHARED MEMORY****************/
    int shmid ;
    key_t ky;
    
    shmid = shmget(ky,4*sizeof(int)+sizeof(Queue)+sizeof(void*)+(2*nn-1)*N*N*sizeof(int)+sizeof(pthread_mutex_t),IPC_CREAT|0666);
    if(shmid<0){
        perror("Error in Allocating shared memory\n");
        exit(0);
    }
    void *old_addr = shmat(shmid,NULL,0);
    (memcnt) = (void**)old_addr;
    (*memcnt) = (void*)memcnt;
    (*memcnt) +=sizeof(void*);
    //rnd = (func*)(*memcnt);
    //*rnd = rand;
    //(*memcnt) +=sizeof(rand);
    jobs_created = (int*)(*memcnt);
    *jobs_created = 0;
    (*memcnt) +=sizeof(int);
    jobs_done = (int*)(*memcnt);
    (*jobs_done) = 0;
    (*memcnt) +=sizeof(int);
    new_jobs = (int*)(*memcnt);
    (*new_jobs) = 0;
    (*memcnt) +=sizeof(int);
    tot = (int*)(*memcnt);
    *tot = nn;
    (*memcnt) +=sizeof(int);
    Q = (Queue*)(*memcnt);
    (*Q) = Queue();
    //Shared Mutex
    (*memcnt) +=sizeof(Queue);
    mutex_lock = (pthread_mutex_t*)(*memcnt);
    pthread_mutexattr_t mtatr;
    pthread_mutexattr_init(&mtatr);
    pthread_mutexattr_setpshared(&mtatr,PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mutex_lock, &mtatr);
    (*memcnt) +=sizeof(pthread_mutex_t);
    /***********************************************************/


    int *prodId = new int[p];
    fr(i,1,p+1) prodId[i-1] = i;
    int *workerId = new int[c];
    fr(i,1,c+1) workerId[i-1] = i;

    /*************EXPERIMENT*********************/
    //pthread_mutex_lock(mutex_lock);

   // print("Lock1");
   // pthread_mutex_unlock(mutex_lock);

   // pthread_mutex_lock(mutex_lock);
   // print("Lock2");

   // pthread_mutex_unlock(mutex_lock);

    /*******************************************/

    /**********************Spawn new processes for computation**************/
    //cout<<(*jobs_created)<<"\n";
    using namespace std::chrono;
    auto beg = high_resolution_clock::now();
    fr(i,0,p)   if(fork()==0)   {
        produceJob(prodId+i);
        exit(0);
    }     
    
    fr(i,0,c)   if(fork()==0)   {
        consumeJob(workerId+i);
        exit(0);
    }
    
    while(wait(&status)>=0);
    auto endd = high_resolution_clock::now();
    auto time_taken = duration_cast<microseconds>(endd - beg);
    /*********************************************************************/


    /************************PRINT ANSWER*******************************/
    /*
    cout<<"Printing ans:\n";
    fr(i,0,N){
        fr(j,0,N)   cout<<*((*Q)[0].Mat + N*i+j)<<" ";
        cout<<"\n";
    }*/
    int sm = 0;
    fr(i,0,N)   sm+=*((*Q)[0].Mat + N*i+i);
    cout<<"Main Diagonal Sum: "<<sm<<"Time Taken: "<<time_taken.count()<<"\n";
    /*****************************************************************/

    /**************************Deallocation and Destruction of mutex********************/
    pthread_mutexattr_destroy(&mtatr);
    pthread_mutex_destroy(mutex_lock);
    shmdt(old_addr);
    shmctl(shmid, IPC_RMID, NULL);
    /*****************************************************************/

    return 0;
}




void produceJob(void *arg){
   // mt19937 mt = mt19937(time(nullptr));

    srand(time(NULL)+getpid()%97);
   // cout<<(*jobs_created)<<"\n";
    int id = *((int*)arg);
    while(1){
        //cout<<(*jobs_created)<<endl;
         
        pthread_mutex_lock(mutex_lock);
        if((*jobs_created)==(*tot))   {
            pthread_mutex_unlock(mutex_lock);
            break;
        }
        pthread_mutex_unlock(mutex_lock);
        while(Q->sz==SZ);   
         
        Jobs J;
        J.prodId = id;
        J.flg = 0;
        J.cnt = J.i = J.j = 0;
        
        J.matId = rand()%100000 +1;
        J.Mat = (int*)(*memcnt);

       // cout<<jobs_created<<" "<<(*jobs_created)<<endl;
        
        pthread_mutex_lock(mutex_lock);
        if((*jobs_created)==(*tot))   {
            pthread_mutex_unlock(mutex_lock);
            break;
        }
        *memcnt +=(N*N*sizeof(int));//update memory pointer        
        (*jobs_created)++; 
        pthread_mutex_unlock(mutex_lock);
        fr(i,0,N) {   
            //todo
            fr(j,0,N)  *( J.Mat + N*i+j)  = ((int)rand()%19) -9;
        }
        while(Q->sz==SZ);  
        sleep(rand()%4);
        pthread_mutex_lock(mutex_lock);
        Q->push(J);
        pthread_mutex_unlock(mutex_lock);
        printf("-----Job Generated--ProdId:%d--PID:%d--MatrixID:%d-----\n",J.prodId,getpid(),J.matId);

    }
    //cout<<"Exitting Produce "<<(*jobs_created)<<(*tot)<<"\n";
    
    
}
void consumeJob(void *arg){
    srand(time(NULL)+getpid()%97);
   // mt19937 mt = mt19937(time(nullptr));
    int id = *((int*)arg);
    while(1){
       

        
        sleep(rand()%4);
        
        pthread_mutex_lock(mutex_lock);
        if((*jobs_done)+1==(*jobs_created)+(*new_jobs)&&(*jobs_created)==(*tot)) {
            pthread_mutex_unlock(mutex_lock);
            break;
        }
        pthread_mutex_unlock(mutex_lock);
        //print(id,*(jobs_done));
        while((Q->sz<=1)||((*Q)[0].j ==N)){
           // print("Stuck 0");
            if((*jobs_done)+1==(*jobs_created)+(*new_jobs)&&(*jobs_created)==(*tot)) {            
                break;
            }
        }
        
        pthread_mutex_lock(mutex_lock);
        if((*jobs_done)+1==(*jobs_created)+(*new_jobs)&&(*jobs_created)==(*tot)) {
            pthread_mutex_unlock(mutex_lock);
            break;
        }
        int i1,i2,j1,j2;
        Jobs &J1 = (*Q)[0];
        Jobs &J2 = (*Q)[1];
        i1 = J1.i;i2 = J2.i;
        j1=  J1.j;j2 = J2.j;
        
        
       // printf("Worker %d working...\n",id);
        
        if((*Q)[0].j ==N) {
            pthread_mutex_unlock(mutex_lock);
            continue;
        }
        print("Worker",id,"working on Matrix ",J1.matId,J2.matId,"...",i1,i2,j1,j2);
        
        //update the jobs
       // print("Here");
        
        
        // 0000 -> 0001 -> 1000 -> 1001 -> 0110 -> 0111 -> 1110 -> 1111 -> 2110
        if(J1.i==N/2&&J2.j==N/2){
            J1.j+=(N/2);
            J2.i = J1.j;
            if(J1.j==N){
               
            }else{
                J1.i = J2.j = 0;
            }
        }else{
            (J2.j+=(N/2))%=N;
            if(J2.j==0) J1.i += (N/2);
        }
        print("After Updattion,Matrix",J1.matId,"-->",J1.i,J1.j,"Matrix",J2.matId,"-->",J2.i,J2.j);
        //if(J1.cnt==3&&J2.cnt==3)   continue;
        int *C;
        if(J1.flg==0&&J2.flg==0){
            J1.flg = J2.flg = 1;
            C = (int*)(*memcnt);
            (*memcnt)+=(N*N*sizeof(int));
            fr(i,0,N){
                
                fr(j,0,N)   *(C + N*i+j)  =0;
            }
            //add to queue
            Jobs J;
            J.prodId = id;
            J.flg = 0;
            J.cnt = J.i = J.j = 0;
            J.matId = rand()%100000 +1;
            J.Mat = C;    
            while(Q->sz==SZ);   //Deadlock     
            Q->push(J);
            J1.res = Q->J + Q->lst;
            J2.res = Q->J + Q->lst;
            
            (*new_jobs)++;
            printf("-----Job Generated--WorkerId:%d--PID:%d--MatrixID:%d-----\n",J.prodId,getpid(),J.matId);

            
        }else   C = J1.res->Mat; 
        print("Worker",id,"mutex over 1");
        pthread_mutex_unlock(mutex_lock);
        //do the operation
        mult(J1.Mat,J2.Mat,C,i1,j1,i2,j2,i1,j2,N/2,N/2,N/2);
       
        sleep(rand()%(4));
        print("Process ",getpid(),"waiting for checkout ,main multiplication task completed\n");
        pthread_mutex_lock(mutex_lock); 
        print("Process ",getpid(),"entered for checkout ,main multiplication task completed\n");
        J1.cnt ++;
        J2.cnt = J1.cnt ;
        //pthread_mutex_unlock(mutex_lock); 
        print("Worker ",id," work over coutnt: ",J1.cnt,"over matrices",J1.matId,"and",J2.matId,"added to matrix",J1.res->matId);
        //fflush(stdout);
        //pthread_mutex_lock(mutex_lock); 
        if(J1.cnt==8){
            
            Q->pop();                          
            Q->pop();
            (*jobs_done)+=2;
            printf("Matrix %d,%d popped by worker %d",J1.matId,J2.matId,id);print("Job Done:",(*jobs_done),"\n");
            if((*jobs_done)+1==(*jobs_created)+(*new_jobs)&&(*jobs_created)==(*tot)) {
                pthread_mutex_unlock(mutex_lock);
                break;
            }
        }
        
        pthread_mutex_unlock(mutex_lock);

        /*
        J2.cnt++;
        
        (J2.j+=(N/2))%=N;
        if(J2.j==0) (J2.i+=(N/2));
        if(J2.cnt==4){
            if(J1.cnt==3){
                //completed multiplication of current pair of matrices                
                
            }
            else{
                J1.cnt++;        
                
                (J1.j+=(N/2))%=N;
                if(J1.j==0) (J1.i+=(N/2));
                J2.cnt = 0;
                J2.i = J2.j = 0;
            }            
        }*/
    }    
    
   // print("Exitting Worker\n");
}
