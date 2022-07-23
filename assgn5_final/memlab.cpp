/**
 * @file memlab.cpp
 * @author Anurat Bhattacharya (19CS10071) Srijan Das(19CS30046)
 * @brief 
 * @version 0.5
 * @date 2022-03-22
 * 
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "memlab.h"
/****************GLOBALS**************/
int varSz[4] = {32,8,32,1};
sym_entry *symbolTable;
pthread_mutex_t mutex_lock;
int* basep;
Node* memcnt;
Node* listPool;
Node* root,*root2;
int n;
Stack stk;//stack of symbpol table indices
Stack freeSym;//stack of free symbol table indices
int nvar = 0;
pthread_t gc_collector;
/********************************************/
int get_cur_mem(){
    Node* head=  root;
    int ans = 0;
    while(head!=NULL){
        ans = max(ans,head->beg);
        head = head->nxt;
    }

    return ans;

}
void createMem(size_t sz){
    srand(time(NULL));
    if(sz<0){
        printf("Invalid size\n");
        exit(0);
    }
    print("--------------Create Mem with size",sz,"called -----------");
    n = ((sz+WORD_SZ-1)/WORD_SZ);
    basep = (int*) malloc( WORD_SZ*((sz+WORD_SZ-1)/WORD_SZ));
    listPool = (Node*) malloc(POOL_SIZE*sizeof(Node));
    stk.arr = (int*)malloc(STK_SZ*sizeof(int));
    freeSym.arr = (int*)malloc(STK_SZ*sizeof(int));
    symbolTable = (sym_entry*)malloc(SZ*sizeof(sym_entry));
    memcnt = listPool;
    *(memcnt) = Node();
    memcnt->beg = 0;
    memcnt->end = n-1;
    for(int i=0;i<SZ;i++) freeSym.push_back(i);
    root = memcnt;
    memcnt++;
    root2 = NULL;//Initially no filled segmetns

    pthread_mutexattr_t mtatr;
    pthread_mutexattr_init(&mtatr);
    pthread_mutex_init(&mutex_lock,&mtatr);
    #ifdef COLLECT_GARBAGE
    gc_run();
    #endif
    print("----------Memory Created-------------------");
    
}
Mem getFirstFit(size_t sz,int sym_ind,int seq){
    print("----------------Getting the first fit---------------");
    Node* head = root;//free
    int blocks = sz/WORD_SZ;
    Node* head2 = root2;//loaded
    Node* prev = NULL;
    while(head2!=NULL && head2->end<head->beg) {
        prev = head2;
        head2 = head2->nxt;
    }
    while(head!=NULL && head->end-head->beg+1<blocks){
        head = head->nxt;
        if(root2!=NULL  ){
            while(head2!=NULL && head2->end<head->beg) {
                prev = head2;
                head2 = head2->nxt;
            }
        }
    }
    if(head2!=NULL) print(head2->beg,head2->end);
    
    Mem notFound;
    notFound.ind = ACTIVE_CLR;
    notFound.size = 0;
    if(head==NULL)  return notFound;
    int ind = head->beg;
   // print(ind,blocks);
    head->beg+=blocks;
   
    Node *temp = memcnt++;
    *temp = Node();
    temp->nxt = head2;
    temp->symind = sym_ind;
    temp->seq = seq;
    temp->beg = ind;
    temp->end = ind+blocks-1;
 
    if(head2!=NULL) {
      
        temp->prev = head2->prev;
        head2->prev->nxt = temp;
        head2->prev = temp;
    }else if(prev!=NULL){
       
        prev->nxt = temp;
        temp->prev = prev;
    }else if(root2==NULL){
      
        root2 = temp;
        print(root2->beg,root2->end);
    }
 
    if(head->beg>head->end){
        //remove head
        if(head->prev!=NULL)    head->prev->nxt = head->nxt;
        if(head->nxt!=NULL) head->nxt->prev = head->prev;
    }else{
        //do nothing
        
    }
    Mem ret;
    ret.ind =ind;
    ret.size = blocks;
   
    return ret;

}
/**
 * @brief For Variables
 * 
 * @param memfd 
 * @return int 
 */

int get(int memfd){
    if(symbolTable[memfd].isEmpty  ){
        print("------------!!!Invalid Access!!!!------------");
        return -1;
    }
    int loc = symbolTable[memfd].offset;

    print("------------Getting Data block (as WORD_SZ = 4)----------");
    int val = basep[loc];

    return val;
}
int get(int memfd,int i){
    if(symbolTable[memfd].isEmpty  ){
        print("------------!!!Invalid Access!!!!------------");
        return -1;
    }
    if(symbolTable[memfd].isArr==0){
        print("------------!!!Not an Array!!!!------------");
        return -1;
    }
    if(i*varSz[symbolTable[memfd].v.type]  >= symbolTable[memfd].v.sz){
        print("------------!!!Out of Bounds Error!!!!------------");
        return -1;
    }
    int beg = symbolTable[memfd].offset;
    int sz = varSz[symbolTable[memfd].v.type];
    int sz1 =  sz*i;
    int sz2 = sz1 + sz-1;
    int b1 = (sz1/8)/WORD_SZ;
    int b2 = (sz2/8)/WORD_SZ;
    print("--------------","Start Bit pos:",sz1,"End bit pos:",sz2);
    print(beg+b1,beg+b2,basep[beg+b1],basep[beg+b2]);
    if(b1==b2){
        int val  =basep[beg+b1];
        sz1-=(b1*8*WORD_SZ);
        sz2-=(b1*8*WORD_SZ);
        int ind1 = 32-sz1;
        int ind2 = 32-sz2;
        ind2--;
        int mask = (ind1==32?-1:((1<<ind1)-1));
        mask^=(ind2==32?-1:((1<<ind2)-1));
        val = val&mask;
        val>>=ind2;
        return val;
    }   else{
        int val1  =basep[beg+b1];
        int val2 = basep[beg+b2];
        sz1-=(b1*8*WORD_SZ);
        sz2-=(b2*8*WORD_SZ);
        int ind1 = 32-sz1;
        int ind2 = 32-sz2;
        ind2--;
        int mask = (ind1==32?-1:((1<<ind1)-1));
        val1&=mask;
        mask = ((-1)^(ind2==32?-1:((1<<ind2)-1)));
        val2&=mask;
        val2>>=ind2;
        val1<<=(32-ind2);
        val1|=val2;
        return val1;
    }  
    return 0;
}
/**
 * @brief Assign Variable from another variable
 * 
 * @param dest 
 * @param src 
 * @return int 
 */
int assignVarV(int dest,int src){
    if(symbolTable[dest].isEmpty  ){
        print("------------!!!Invalid Access!!!!------------");
        return -1;
    }
    if(symbolTable[src].isEmpty  ){
        print("------------!!!Invalid Access!!!!------------");
        return -1;
    }
    auto &lf = symbolTable[dest];
    auto &rt = symbolTable[src];
    if((lf.v.type!=rt.v.type)||(lf.isArr!=rt.isArr)){
        print("------------!!!Type Error!!!!------------");
        return 0;
    }
    if(rt.isArr){
        int n = rt.v.sz / varSz[rt.v.type];
        for(int i=0;i<n;i++){
            assignVar(dest,i,get(src,i));
        }
    }else   assignVar(dest,get(src));
    return 1;
}
int createVar(int type)
{
    if(!(type>=0&&type<=3)){
        print("------------!!!Invalid Type!!!!------------");
        return -1;
    }
    string str= "";
    if(type ==INT)  str = "INT";
    else if(type ==CHAR)    str = "CHAR";
    else if(type==BOOL) str = "BOOL";
    else if(type==mdINT)    str = "MEDIUM INT";
    print("--------------Create Var of type",str,"called -----------");
	
    pthread_mutex_lock(&mutex_lock);
    
    print(freeSym.size());
    if(freeSym.size()==0){
        return -1;//Symbol Table full
    }
    int cur_symtable_index = freeSym.pop_back();
    symbolTable[cur_symtable_index].seq = nvar++;
    auto it = getFirstFit(WORD_SZ,cur_symtable_index,symbolTable[cur_symtable_index].seq);//returns star mem block index
    print("---------Adding to symbol table entry",cur_symtable_index,"and at loc:",it.ind*WORD_SZ,"------------");
    if(it.ind==-1)    return -1;//Failure in allocating memory
  //  if(val.first<=val.second)   Intv.insert(val);
    symbolTable[cur_symtable_index].flag = 0;
    symbolTable[cur_symtable_index].isEmpty = 0;
    symbolTable[cur_symtable_index].offset = it.ind;//careful here
    symbolTable[cur_symtable_index].v.sz = varSz[type];//number of bits needed
    symbolTable[cur_symtable_index].v.type = type;
    symbolTable[cur_symtable_index].v.val = 0;
    symbolTable[cur_symtable_index].isArr = 0;
    //Intv.insert(val);
    //update stack
    stk.push_back(cur_symtable_index);
    it.ind++;
    it.size-=1;
    //insert(it);
    pthread_mutex_unlock(&mutex_lock);


    //Update free space
   // int ret = it.ind;
    
 //   symbol_ind[ret] = cur_symtable_index;
    int mem_footprint = get_cur_mem();
    print("---------Memory Footprint:",mem_footprint,"------------");
    return cur_symtable_index;

}
// memfd1 is logical address space pointer
void assignVar(int memfd1, int val)
{
    if(symbolTable[memfd1].isEmpty){
        print("------------!!!Invalid Access!!!!------------");
        return;
    }
    print("-----------Assignvar called for ",memfd1,"ans value:",val,"-----------");
	int *pos;
	pos=basep+(symbolTable[memfd1].offset);
	*pos=val;
}
void assignVar(int memfd, int offset,int data)
{
    print("-----------Assignvar called for ",memfd,"ans value:",data,"-----------");
    if(symbolTable[memfd].isEmpty  ){
        print("------------!!!Invalid Access!!!!------------");
        return ;
    }
    if(symbolTable[memfd].isArr==0){
        print("------------!!!Not an Array!!!!------------");
        return ;
    }
    if(offset*varSz[symbolTable[memfd].v.type]  >= symbolTable[memfd].v.sz){
        print("------------!!!Out of Bounds Error!!!!------------");
        return ;
    }
	int beg = symbolTable[memfd].offset;
    int sz = varSz[symbolTable[memfd].v.type];
    int sz1 = sz*offset;
    int sz2 = sz1 + sz-1;
    int b1 = (sz1/8)/WORD_SZ;
    int b2 = (sz2/8)/WORD_SZ;
    if(b1==b2){
        int &val  =basep[beg+b1];
        sz1-=(b1*8*WORD_SZ);
        sz2-=(b1*8*WORD_SZ);
        int ind1 = 32-sz1;
        int ind2 = 32-sz2;
        ind2--;
        int mask = (ind1==32?-1:((1<<ind1)-1));
        mask^=(ind2==32?-1:((1<<ind2)-1));
        val = val&(~mask);
        print("Offsets to assign:",ind1,ind2,data,(data<<ind2),val);
        val|=(data<<ind2);
        print("-----------Value to be assigned:",val);
        //val>>=ind2;
       // return val;
    }else{
       // ----data      ------
       // 000011111|1111000000
       //     <----     <-----
       //      ind1      ind2  (size of segemnt not index)     
        int &val1  =basep[beg+b1];
        int &val2 = basep[beg+b2];
        sz1-=(b1*8*WORD_SZ);
        sz2-=(b2*8*WORD_SZ);
        int ind1 = 32-sz1;
        int ind2 = 32-sz2;
        ind2--;
        int mask = (ind1==32?-1:((1<<ind1)-1));
        val1&=(~mask);
        mask = ((-1)^(ind2==32?-1:((1<<ind2)-1)));
        val2&=(~mask);
        val2|=(data<<ind2);
        val1|=(data>>(32-ind2));
     //   return val1;
    }  
}
//Remove from Symbol table rest managed by the garbage collector
int freeElem(int memfd){
    if(symbolTable[memfd].isEmpty){
        print("------------!!!Already Empty!!!!------------");
        return 0;
    }
   // print("--------------------------");
   // int symbol_t_ind = symbol_ind[memfd/WORD_SZ];

    int symbol_t_ind = memfd;
    auto &symbol  = symbolTable[symbol_t_ind];
    if(symbol.isEmpty)  return 0;//already empty
    
    symbol.flag = 0;
    symbol.isEmpty = 1 ;
    freeSym.push_back(symbol_t_ind);
    print("---------Freeing symbol table entry",symbol_t_ind,"------------");
    return 1;



}

void clear(){
    root = NULL;
    //lazy update
    /*
    print("Groups",nGrps);
    for(int i=0;i<nGrps;i++){
        //-2 means the rest part of the chain is empty
        cnt[i] = 0;
        get(i,0,freeSeg).ind = LAZY_CLR;
        get(i,0,freeSegRev).ind = LAZY_CLR;        
        auto it = get(i,0,freeSeg);
        print(it.ind,it.size);
    }*/
}
int createArr(int type,int size){
    if(!(type>=0&&type<=3)){
        print("-------!!!!Invalid type!!!!-----");
        return -1;
    }
    if(size<1){
        print("------!!!!!Invalid size!!!!!-----");
        return -1;
    }
    //minimum number of blocks needed
    int nblks = ((size*varSz[type]+7)/8+WORD_SZ-1)/WORD_SZ;
    string str= "";
    if(type ==INT)  str = "INT";
    else if(type ==CHAR)    str = "CHAR";
    else if(type==BOOL) str = "BOOL";
    else if(type==mdINT)    str = "MEDIUM INT";
    print("--------------Create Arr of type",str,"ans size",size,"called -----------");
    //get first fit index from the set(from memory segment)
  //  print("Waiting to create Variable");
    pthread_mutex_lock(&mutex_lock);
    
    
    if(freeSym.size()==0){
        return -1;//Symbol Table full
    }
    int cur_symtable_index = freeSym.pop_back();
    symbolTable[cur_symtable_index].seq = nvar++;
    auto it = getFirstFit(nblks*WORD_SZ,cur_symtable_index,symbolTable[cur_symtable_index].seq);//returns star mem block index
    
    if(it.ind==-1)    return -1;//Failure in allocating memory
    print("---------Adding to symbol table entry",cur_symtable_index,"and at loc:",it.ind*WORD_SZ,"------------");
  //  if(val.first<=val.second)   Intv.insert(val);
    symbolTable[cur_symtable_index].flag = 0;
    symbolTable[cur_symtable_index].isEmpty = 0;
    symbolTable[cur_symtable_index].offset = it.ind;//careful here
    symbolTable[cur_symtable_index].v.sz = size*varSz[type];
    symbolTable[cur_symtable_index].v.type = type;
    symbolTable[cur_symtable_index].v.val = 0;
    symbolTable[cur_symtable_index].isArr = 1;
    //Intv.insert(val);
    //update stack
    //stk.push_back(cur_symtable_index);
    it.ind++;
    it.size-=1;
    //insert(it);
    stk.push_back(cur_symtable_index);
    pthread_mutex_unlock(&mutex_lock);


    //Update free space
   // int ret = it.ind;
    
 //   symbol_ind[ret] = cur_symtable_index;
    int mem_footprint = get_cur_mem();
    FILE *fp = fopen("mem_footprint.txt","a");
    fprintf(fp,"%d\n",mem_footprint);
    fclose(fp);
   // print("---------Memory Footprint:",mem_footprint,"------------");
    return cur_symtable_index;

}
void gc_initialize(){
   // pthread_mutex_lock(&mutex_lock);
    for(int i=0;i<SZ;i++)   symbolTable[i].isEmpty = 1;
   // pthread_mutex_unlock(&mutex_lock);
}
void merge(Node* a,Node* b){
    if((a->end+1)!=(b->beg))    return;
    a->nxt = b->nxt;
    if(b->nxt!=NULL)    b->nxt->prev = a;
    a->end = b->end;
}
void mark_and_sweep(){
    //mark
    for(int i=0;i<stk.size();i++)   {
        symbolTable[stk[i]].flag = 1;
    }
    for(int i=0;i<SZ;i++){
        if(symbolTable[i].isEmpty)    continue;
        if(symbolTable[i].flag==1){
            symbolTable[i].flag = 0;
        }else   freeElem(i);//lazy update
    }

}
/**
 * @brief Naive garbage collector(No compaction)
 * 
 * @param args 
 * @return void* 
 */
void* gc_runner(void *args){

   
    
   // int tempfd = open(".temp",O_CREAT|O_RDONLY);
    while(1){
      

        sleep(2);
        auto beg = clock();
        print("------One Iterration of GC started-------------");
        pthread_mutex_lock(&mutex_lock);
        //mark;
        mark_and_sweep();
        Node *head2  =root2;//loaded
        Node *p1 ;//free
        p1 =  root;
        //iterate over loaded
        while(head2!=NULL){
            
         //   print("Huuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu");
            printList(head2);
            printList(p1);
            if(!(symbolTable[head2->symind].isEmpty || (symbolTable[head2->symind].seq!=head2->seq))) {
               // print("Skipping");
                head2 = head2->nxt;
                continue;
            }
            Node* prev = head2;
            while((head2->nxt!=NULL)&&(symbolTable[head2->symind].isEmpty || (symbolTable[head2->symind].seq!=head2->seq)) ) {
                if((!(symbolTable[head2->nxt->symind].isEmpty || (symbolTable[head2->nxt->symind].seq!=head2->nxt->seq)))||(head2->nxt->beg!=(head2->end+1))){
                    break;
                }

                head2 = head2->nxt;
                continue;
            }
            //delete batch starting from prev to head
            //delete from loaded
            if(prev->prev!=NULL)    prev->prev->nxt = head2->nxt;
            if(head2->nxt!=NULL)    head2->nxt->prev = prev->prev;
            if(prev->prev==NULL && head2->nxt==NULL)   root2 = NULL;
            if(root2==prev) root2 = root2->nxt;
            //print(prev->beg,head2->end);
            while(p1!=NULL  &&  p1->nxt!=NULL  && p1->nxt->end <prev->beg)    p1 = p1->nxt;
            
            
            
            
            Node* cur = memcnt++;
            (*cur) = Node();
            cur->beg = prev->beg;
            cur->end = head2->end;
            cur->prev = p1;
            cur->nxt = p1->nxt;
            if(p1->beg>prev->beg){
                root = cur;
                p1->prev = cur;
                cur->prev = NULL;
                cur->nxt = p1;
                p1 =root;
                head2 = head2->nxt;
              //  print("Tick Tok..........................");
              //  printList(root);
                if( cur->end+1==cur->nxt->beg){
                    cur->end = cur->nxt->end;
                    cur->nxt = NULL;

                }
               //  printList(root);
              //   printList(root2);
                continue;
            }
            Node *p2 = p1->nxt;
            if(root==NULL){
                root = cur;
                p1 =  root;
            }else if(p1->end+1==cur->beg){
                p1->end = cur->end;
            }else{
              //  print("----------------Here-----------------");
                cur->nxt = p1->nxt;
                if(p1->nxt!=NULL)   p1->nxt->prev = cur;
                p1->nxt = cur;
                cur->prev = p1;
                p1 = cur;
            }
          //  printList(p1);
            if(p1->nxt!=NULL)   merge(p1,p1->nxt);
            
            head2 = head2->nxt;

           
        }
        
        
        
        print("------------Garbage Collection over starting compaction--------------");
        compact();
        pthread_mutex_unlock(&mutex_lock);
        int end = clock();
        int intv = (end-beg);
        FILE* f = fopen("time.txt","a");
        fprintf(f,"Time Taken for GC: %d\n",intv);
        fclose(f);

    }
    pthread_exit(0);
    
}
void send_gc_signal(){
    int fd = open(".temp",O_WRONLY);
    char buf[] = "1";
    write(fd,buf,1);
}
void end_session(){
    
    pthread_join(gc_collector,NULL);
}
void gc_run(){
    gc_initialize();
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&gc_collector,&attr,gc_runner,NULL);
    //pthread_join(gc_collector,NULL);
}
void compact(){
   // return;
    print("-------------------Compaction Started-----------------------");
    int j=0;
    Node* i = root2;

    while(i!=NULL){
        int b = j;
        for(int k=i->beg;k<=i->end;k++,j++){
            basep[j] = basep[k]; 
        }
        i->beg = b;
        i->end = j-1;
        symbolTable[i->symind].offset = b;
        i = i->nxt;
    }
    clear();
    root = memcnt++;
    (*root) = Node();
    root->beg = j;
    root->end  =n-1;

   
    print("------------Memory Compacted successfully-------------");
}
void pop_last(){
    pthread_mutex_lock(&mutex_lock);
    int a = stk.pop_back();
    pthread_mutex_unlock(&mutex_lock);
   // freeSym.push_back(a); //Delyaed to the garbage collector
}
void printList(Node* head){
    //if(head==NULL)  return;
    cout<<"List:[ ";
    while(head!=NULL){
        cout<<"("<<head->beg<<","<<head->end<<" )-->";
        head = head->nxt;
    }
    cout<<"]"<<endl;
}
