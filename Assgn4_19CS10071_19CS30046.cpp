/**
 * @file prog.cpp
 * @author Anurat Bhattacharya (19CS10071) Srijan Das(19CS30046) (Group 42)
 * @brief OS LAB assignment 4
 * @version 0.1
 * @date 2022-03-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include<bits/stdc++.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/shm.h>
#include<sys/wait.h>
using namespace std;
template<class T>
void print(T t)  {
    cout<<t<<endl;    
}
template<class T,class...S>
void print(T t,S... ss)  {
    cout<<t<<" ";
    print(ss...);
}

#define MX 1000
#define MAX_DEG 500
#define DONE 0
#define RUNNING 1
#define IDLE 2
#define MAX_COMPLETION_TIME 250

/**
 * @brief Node structure for modelling each node of the dependency tree
 * 
 */
struct Node{
    int job_id;//Job id
    int tm;//time of completion in milisec
    pthread_mutex_t lock;
    Node* adj[MAX_DEG];
    Node* par;
    int deg = 0;//number of dependent nodes
    int done = 0;//number of childdren done
    int status = IDLE;
    Node(){

        job_id = rand()%100000000+1;
        tm = rand()%MAX_COMPLETION_TIME+1;
        pthread_mutexattr_t mtatr;
        pthread_mutexattr_init(&mtatr);
        pthread_mutexattr_setpshared(&mtatr,PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&lock,&mtatr);
        par = NULL;
        deg = 0;
        done = 0;
        status = IDLE;
    }

};

/***********************SHARED MEMORY POINTERS****************************/
int *jobs_created ;//Number of jobs created
int *jobs_done ;//Number of jobs completed
int *jobs_idle;//number of idle jobs
int *active_producers;//number of active producer threads
pthread_mutex_t *mutex_lock; //global mutex created when modifying the shared memory variable(not some individual node)
void **memcnt;//shared memory counter
Node* tree;//Dependency tree
/************************************************************************/

/**
 * @brief For generating jobs for initialization
 *          Note that this function is called before forking basically for initialisation
 * 
 * @param n Number of jobs to initially generate given a root
 */
void generate_jobs(int n){
    for(int i=0;i<n;i++){
        int index = rand()%(*jobs_created);       
        Node* temp = tree+index;
        Node* nxt = (Node*)(*memcnt);
        (*nxt) = Node();
        (*memcnt)+=sizeof(Node);
        (temp->adj)[temp->deg] = nxt;
        nxt->par = temp;
        temp->deg++;
        (*jobs_created)++;
        (*jobs_idle)++;
    }
}
/**
 * @brief Producer Thread
 * 
 * @param args Contains the thread id
 * @return void* 
 */
void* runnerP(void* args){
    /*
    pthread_mutex_lock(mutex_lock);
    (*active_producers)--;
    pthread_mutex_unlock(mutex_lock);
    pthread_exit(0);*/
    /****************
     * Code Required (My part)
     * 
     ****************/
    int tid = *((int*)args);
    time_t start = time(NULL);
    int tot = rand()%11 + 10;
    while(1){

        time_t now = time(NULL);        
        if(now-start>tot)    break;
        //Produce the tasks
        pthread_mutex_lock(mutex_lock);
        if((*jobs_idle)==0) {
            pthread_mutex_unlock(mutex_lock);
            continue;
        }
        pthread_mutex_unlock(mutex_lock);
        
        int index = rand()%(*jobs_created);
       
        Node* temp = tree;
        
        while(temp[index].status!=IDLE)  {
            index++;
            index%=(*jobs_created);
            time_t now = time(NULL);        
            if(now-start>tot)    break;
        }
       
        temp = temp+index;
        
        
        pthread_mutex_lock(&(temp->lock));
        int ind = temp->deg;
        Node* nxt;
        if(temp->status==IDLE){
            nxt = (Node*)(*memcnt);
            pthread_mutex_lock(mutex_lock);  
            (*nxt) = Node();  
            pthread_mutex_lock(&(nxt->lock)); 
            temp->adj[ind] = nxt;
            temp->deg++;     
            nxt->par = temp;            
            (*memcnt)+=sizeof(Node);
            (*jobs_created)++;
            (*jobs_idle)++;
            pthread_mutex_unlock(&(nxt->lock)); 
            pthread_mutex_unlock(mutex_lock);
            pthread_mutex_unlock(&(temp->lock));
        }else{
            pthread_mutex_unlock(&(temp->lock));
            continue;
        }
        
       
        print("---------","Producer Thread",tid,"Produced Job :",nxt->job_id,"with completion time:",nxt->tm,"ms ------------");
        print("---------","Jobs Done:",*jobs_done,"Jobs Created:",*jobs_created,"Active Producers:",*active_producers," ------------");
        usleep(rand()%300000+200000);//sleep 200ms to 500ms
        time_t now_ = time(NULL);        
        if(now_-start>tot)    break;


    }
    pthread_mutex_lock(mutex_lock);
    (*active_producers)--;
    pthread_mutex_unlock(mutex_lock);
    usleep(rand()%10000);
    print("Exitting Producer",tid);
    pthread_exit(0);
}
/**
 * @brief To get the first idle leaf node(whose depending jobs have been finished)
 * 
 * @param root 
 * @return Node* 
 */
Node * dfs_search(Node * root)
{
   // print("Node: ",root->job_id,"status:",root->status);
    if(root->status!=IDLE)  return NULL;
    else if(root->deg==root->done)  return root;
	for(int i=0;i<root->deg;i++)
	{
        Node* cur = dfs_search(root->adj[i]);
        if(cur!=NULL)   return cur;
		
	}
	return NULL;
}
/**
 * @brief Consumer Thread
 * 
 * @param args Contains the thread id 
 * @return void* 
 */
void* runnerC(void* args){
    //pthread_exit(0);
    int tid = *((int*)args);
    /****************
     * Code Required(Your Part)
     * 
     ****************/
    while (1)
    {
        pthread_mutex_lock(mutex_lock);
        if ((*jobs_created) == (*jobs_done)  && (*active_producers)==0) // check if all the jobs are executed and no more active producers
        {
            pthread_mutex_unlock(mutex_lock);
            break;
        }

        if ((*jobs_created) == 0){
            pthread_mutex_unlock(mutex_lock);
            continue;
        }
        pthread_mutex_unlock(mutex_lock);


        
        Node *temp = tree;
        //print("ROOT STATUS: ",tree->status);
        temp = dfs_search(temp);  
        //print("Jobs Done",*jobs_done,"Jobs Created: ",*jobs_created,"Active producers:",*active_producers);
        if(temp==NULL)   continue;
        
        pthread_mutex_lock(&(temp->lock));
        if (temp->status == IDLE&&temp->done==temp->deg)
        {
            //first change to running state
            pthread_mutex_lock(mutex_lock);
            temp->status = RUNNING;
            (*jobs_idle)--;
            pthread_mutex_unlock(mutex_lock);
            pthread_mutex_unlock(&(temp->lock));

            //complete the process
            usleep(temp->tm * 1000); //sleep for the specified amount of time (as defined in the job)

            //change to sone state
            pthread_mutex_lock(&(temp->lock)); 
            if(temp->par!=NULL) pthread_mutex_lock(&(temp->par->lock));
            pthread_mutex_lock(mutex_lock);

            temp->status = DONE;
            if(temp->par!=NULL) temp->par->done++;            
            (*jobs_done)++;

            if(temp->par!=NULL) pthread_mutex_unlock(&(temp->par->lock));
            pthread_mutex_unlock(&(temp->lock));
            pthread_mutex_unlock(mutex_lock);
            

            print("---------", "Consumer Thread", tid, "Completed Job :", temp->job_id, "with completion time:", temp->tm, "ms ------------");
            print("---------","Jobs Done:",*jobs_done,"Jobs Created:",*jobs_created,"Active Producers:",*active_producers," ------------");
            //fflush(stdout);
        }else   pthread_mutex_unlock(&(temp->lock));
        pthread_mutex_lock(mutex_lock);
        if ((*jobs_created) == (*jobs_done)  && (*active_producers)==0) // check if all the jobs are executed and no more active producers
        {
            pthread_mutex_unlock(mutex_lock);
            break;
        }
        pthread_mutex_unlock(mutex_lock);
    }
    usleep(rand()%10000);
    print("Exitting Consumer:",tid);
    pthread_exit(0);
}


int main(){
    int status = 0;
    srand(time(NULL));
    int p,c;
    /*********************************USER INPUT*******************************/
    cout<<"Enter number of producer threads: ";cin>>p;
    cout<<"Enter number of consumer threads: ";cin>>c;
    /**************************************************************************/

    //Intialize mutex attribute
    pthread_mutexattr_t mtatr;
    pthread_mutexattr_init(&mtatr);
    pthread_mutexattr_setpshared(&mtatr,PTHREAD_PROCESS_SHARED);

    /************ALLOCATE SHARED MEMORY**********************************************/
    int shmid ;
    size_t size  = 4*sizeof(int) + sizeof(void*) + sizeof(Node)+ sizeof(pthread_mutex_t)+MX*sizeof(Node);
    //print("SIZE: ",size);
    shmid = shmget(IPC_PRIVATE,size,IPC_CREAT|0666);
    if(shmid<0){
        perror("Error in Allocating shared memory\n");
        exit(0);
    }
    void *old_addr = shmat(shmid,NULL,0);
    (memcnt) = (void**)old_addr;
    (*memcnt) = (void*)memcnt;
    (*memcnt) +=sizeof(void*);
    active_producers = (int*)(*memcnt);
    *active_producers = p;
    (*memcnt) +=sizeof(int);
    jobs_created = (int*)(*memcnt);
    *jobs_created = 0;
    (*memcnt) +=sizeof(int);
    jobs_done = (int*)(*memcnt);
    (*jobs_done) = 0;
    (*memcnt) +=sizeof(int);
    jobs_idle = (int*)(*memcnt);
    (*jobs_idle) = 0;
    (*memcnt) +=sizeof(int);
    mutex_lock = (pthread_mutex_t*)(*memcnt);
    pthread_mutex_init(mutex_lock, &mtatr);
    (*memcnt) +=sizeof(pthread_mutex_t);
    tree = (Node*)(*memcnt);
    (*tree) = Node();
    (*jobs_created)++;
    (*jobs_idle)++;
    
    (*memcnt)+=sizeof(Node);
    
    /********************************************************************************/

    //Initialize with random nodes
    generate_jobs(rand()%200+300);
   //generate_jobs(30);
   // print("Root",tree->job_id,":status:",tree->status);

    int pid = fork() ;
    if(pid==0){
        //consumer
        pthread_t *consumers = new pthread_t[c];
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        int *workerId = new int[c];
        for(int i=0;i<c;i++) workerId[i] = i+1;
        for(int i=0;i<c;i++)    pthread_create(&consumers[i],&attr,runnerC,workerId+i);
        for(int i=0;i<c;i++)    pthread_join(consumers[i],NULL);
    }else{
        //producer
       
        pthread_t *producers = new pthread_t[p];
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        int *prodId = new int[p];
        for(int i=0;i<p;i++) prodId[i] = i+1;
        for(int i=0;i<p;i++)    pthread_create(&producers[i],&attr,runnerP,prodId+i);      
        for(int i=0;i<p;i++)    pthread_join(producers[i],NULL);
        waitpid(pid,NULL,0);
        print("Jobs Done",*jobs_done,"Jobs Created: ",*jobs_created,"Active producers:",*active_producers);
        //print("Root",tree->job_id,":status:",tree->status);
        /**************************Deallocation and Destruction of mutex and shared memory********************/
        pthread_mutexattr_destroy(&mtatr);
        pthread_mutex_destroy(mutex_lock);
        shmdt(old_addr);
        shmctl(shmid, IPC_RMID, NULL);
        /**********************************************************************************/
    }


    

    return 0;
}
/**
 * @brief TODO
 * Intitializations of the struct(possible to do when creating as well)
 */