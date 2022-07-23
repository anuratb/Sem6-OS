#include "memlab.h"
int fibonacciProduct(int n){
    int fd = createArr(INT,n);
    assignVar(fd,0,1);
    assignVar(fd,1,1);
    for(int i=2;i<n;i++){
        int a = get(fd,i-1);
        a+=get(fd,i-2);
        assignVar(fd,i,a);
    }
    int ans = 1;
    for(int i=0;i<n;i++)    ans*=get(fd,i);
    
    pop_last();
    return ans;
}
int main(){

    int n;
    cout<<"Enter n: ";
    cin>>n;
    createMem(250*(1<<10)*(1<<10));
    int fd = createVar(INT);
    assignVar(fd,fibonacciProduct(n));
    print("Ans: ",get(fd));
    end_session();
    pthread_exit(0);
}