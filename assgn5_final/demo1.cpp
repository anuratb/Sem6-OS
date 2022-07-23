#include<bits/stdc++.h>
#include "memlab.h"
using namespace std;
void f(int a,int type){
    int fd = createArr(type,50000);
    for(int i=0;i<50000;i++){
        assignVar(fd,i,rand()%1000000007);
    }
    pop_last();
}
int main(){
    srand(time(NULL));
    createMem(250*(1<<10)*(1<<10));
    for(int i=0;i<10;i++)   f(2,INT);
    pthread_exit(0);
}