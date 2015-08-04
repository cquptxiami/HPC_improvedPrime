//author:wanghong
#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include "MyMPI.h"
#define MIN(a,b)  ((a)<(b)?(a):(b))
//wh
int main (int argc, char *argv[])
{
    int count;              ///局部素数个数
    double elapsed_time;    ///运行时间
    int first;              ///每组中第一个是prime倍数的数
    int global_count;       ///全局素数个数
    int high_value;         ///每组的最后一个元素值
    int i;                  ///用于循环
    int id;                 ///程序ID
    int index;              ///子程序0数组标号
    int low_value;          ///每组的第一个元素值
    char * marked;          ///指向局部数组
    int n;                  ///求素数的范围
    int p;                  ///子程序数目
    int proc0_size;         ///子程序0的数组大小
    int prime;              ///下一个要删除其倍数的素数
    int size;               ///局部素数个数
    MPI_Init (&argc, &argv);    ///MPI初始化
    MPI_Barrier(MPI_COMM_WORLD);///所有程序同时开始
    elapsed_time = -MPI_Wtime();///获得当前时间的负值
    MPI_Comm_rank (MPI_COMM_WORLD, &id);///得到该通信子程序的ID号
    MPI_Comm_size (MPI_COMM_WORLD, &p);///得到通信关联组大小
    ///判断传入参数个数是否正确，不正确则终止程序
    if (argc != 2)
    {
        if (!id) printf ("Command line: %s <m>\n", argv[0]);
        MPI_Finalize();
        exit (1);
    }
    n = atoi(argv[1]);///从传入参数得到的大小
    low_value = 3 + 2*(BLOCK_LOW(id,p,(n-1)/2));///算出该子程序持有的数组的最小值
    high_value = 3 + 2*(BLOCK_HIGH(id,p,(n-1)/2));///算出该子程序持有的数组的最大值
    size = BLOCK_SIZE(id,p,(n-1)/2);///算出该子程序持有的数组元素个数
    proc0_size = ((n-1)/2)/p;///算出进程0的数组元素个数
    ///如果进程0所控制的最大数小于n的平方根，异常退出
    if ((3 + 2*proc0_size) < (int) sqrt((double) n))
    {
        if (!id) printf ("Too many processes\n");
        MPI_Finalize();
        exit (1);
    }
    marked = (char *) malloc (size);///分配数组空间
    ///如果分配空间失败，异常退出
    if (marked == NULL)
    {
        printf ("Cannot allocate enough memory\n");
        MPI_Finalize();
        exit (1);
    }
    for (i = 0; i < size; i++) marked[i] = 0;///初始化数组，0表示未标记
    if (!id) index = 0;///初始化index，只有子程序0使用该变量
    prime = 3;///第一个素数为3(偶数已经去掉)
    /*************************核心算法*****************************/
    do
    {
        if (prime * prime > low_value)
            first = (prime * prime - low_value)/2;
        else
        {
            if (!(low_value % prime)) first = 0;
            else
                first = (prime - low_value % prime+1)/2+((prime-1)/2)*((prime - low_value % prime)%2);
        }
        for (i = first; i < size; i += prime)
        {
            marked[i] = 1;
        }
        if (!id)
        {
            while (marked[++index]);
            prime = 2*index + 3;
        }
        MPI_Bcast (&prime,  1, MPI_INT, 0, MPI_COMM_WORLD);
    }
    while (prime * prime <= n);
    /*************************************************************/
    ///计算局部数组个数
    count = 0;
    for (i = 0; i < size; i++)
        if (!marked[i])
        {
            count++;
        }
    ///将所有局部素数数目求和发送给子程序0
    MPI_Reduce (&count, &global_count, 1, MPI_INT, MPI_SUM,
                0, MPI_COMM_WORLD);
    elapsed_time += MPI_Wtime();///计算运行时间
    ///子程序0负责输出信息
    if (!id)
    {
        printf ("%d primes are less than or equal to %d\n",
                global_count+1, n);
        printf ("Total elapsed time: %10.6f\n", elapsed_time);
    }
    MPI_Finalize ();///MPI终止
    return 0;
}

