#include<bits/stdc++.h>
#include "memlab.h"
using namespace std;
void f(int a,int type){
    //int type = rand()%4;
    int fd = createArr(type,50000);
    for(int i=0;i<50000;i++){
       
        assignVar(fd,i,rand()%(1<<varSz[type]));
    }
    pop_last();
}
int main(){
    srand(time(NULL));
    createMem(250*(1<<10)*(1<<10));
    
    for(int i=0;i<10;i++)   f(2,i%4);
    pthread_exit(0);
}