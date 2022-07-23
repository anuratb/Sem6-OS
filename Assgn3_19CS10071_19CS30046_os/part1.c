#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>

int r1, c1, r2, c2; //global variables

typedef struct _process_data
{
    double *A;
    double *B;
    double *C;
    int veclen, i, j;
} ProcessData;

void *mult(void *param)
{
    int i, j, k;
    ProcessData *arg = (ProcessData *)param;
    i = arg->i;
    j = arg->j;
    k = arg->veclen;
    double m1, m2;
    *(arg->C + i * c2 + j) = 0.0;
    for (int p = 0; p < k; p++)
    {
        m1 = *(arg->A + i * c1 + p);
        m2 = *(arg->B + p * c2 + j);
        *(arg->C + i * c2 + j) += m1 * m2;
    }
}

int main()
{
    int shmid, status, pid, i, j;
    key_t key;
    key = 45;
    printf("Enter the no. of rows and columns of matrix 1\n");
    scanf("%d %d", &r1, &c1);
    printf("Enter the no. of rows and columns of matrix 2\n");
    scanf("%d %d", &r2, &c2);
    if (c1 != r2)
    {
        printf("Not compatible for multiplication.\n");
        printf("Exiting program\n");
        exit(0);
    }
    shmid = shmget(key, (r1 * c1 + r2 * c2 + r1 * c2) * sizeof(double), IPC_CREAT | 0666); // storing matrices A<B and the product in shared memory segment
    if (shmid < 0)
    {
        printf("Error in creation of shared memory\n");
        exit(0);
    }
    double *sh_data, *a1, *b1, *res;
    sh_data = (double *)shmat(shmid, NULL, 0);
    void *old = sh_data;
    a1 = sh_data;
    printf("Enter the elements of first matrix in row major order\n");

    for (i = 0; i < r1; i++)
        for (j = 0; j < c1; j++)
        {
            scanf("%lf", sh_data);
            sh_data++;
        }
    b1 = sh_data;
    printf("Enter the elements of second matrix in row major order\n");

    for (i = 0; i < r2; i++)
        for (j = 0; j < c2; j++)
        {
            scanf("%lf", sh_data);
            sh_data++;
        }
    res = sh_data;
    ProcessData *pd;
    ;
    pd->A = a1;
    pd->B = b1;
    pd->C = res;
    pd->veclen = c1;

    for (i = 0; i < r1; i++)
        for (j = 0; j < c2; j++)
        {
            pd->i = i;
            pd->j = j;
            if ((pid = fork()) == 0)
            {
                mult(pd);
                exit(0);
            }
        }

    while ((pid = wait(&status)) != -1)
        ;

    printf("Resultant matrix A * B = C :\n");
    for (i = 0; i < r1; i++)
    {
        for (j = 0; j < c2; j++)
            printf("%lf\t", *(res + i * c2 + j));
        printf("\n");
    }
    shmdt(old);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}