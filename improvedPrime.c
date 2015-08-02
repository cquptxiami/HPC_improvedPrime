#include <mpi.h>
#include <math.h>
#include <stdio.h>
#define BLOCK_LOW(id,p,n)  ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n) ( BLOCK_LOW((id)+1,p,n)-1 )
#define BLOCK_SIZE(id,p,n)  (BLOCK_LOW( (id)+1, p, n)-BLOCK_LOW((id),p,n))
#define BLOCK_OWNER(index,p,n) ((((p)*(index)+1)-1)/(n))
#define MIN(a,b)  ((a)<(b)?(a):(b))
#define BLOCK_SIZE(id,p,n)  (BLOCK_LOW( (id)+1, p, n)-BLOCK_LOW((id),p,n))
int main (int argc, char *argv[])
{
   int count;//Local prime count
   double elapsed_time;//Parallel execution time
   int first;//index of first multiple
   int global_count;//global prime count
   int high_value;//highest value on this proc
   int i;
   int id;
   int index;//index of currrent prime
   int low_value;
   char*marked;
   int n;
   int p;
   int proc0_size;
   int prime;
   int size;
   int num;
   MPI_Init (&argc, &argv);//初始化
   MPI_Barrier(MPI_COMM_WORLD);//同步
   elapsed_time = -MPI_Wtime();
   MPI_Comm_rank (MPI_COMM_WORLD, &id);//进程编号
   MPI_Comm_size (MPI_COMM_WORLD, &p);//进程个数
if (argc != 2) {
      if (!id) printf ("Command line: %s <m>\n", argv[0]);
      MPI_Finalize(); exit (1);
}
 n = atoi(argv[1]);
 num=(n%2==0)?(n/2-1):((n+1)/2-1);//自然数的个数
 //printf("num=%d\n",num);//每个进程都得执行
   low_value = 3+ 2*BLOCK_LOW(id,p,num);
   high_value = 3 + 2*BLOCK_HIGH(id,p,num);
    //printf("low_value=%d,high_value=%d\n",low_value,high_value);
   size = BLOCK_SIZE(id,p,num);
   printf("process=%d,num=%d，size=%d,low_value=%d,high_value=%d\n",id,num,size,low_value,high_value);
   proc0_size = (num)/p;
   if ((2 + proc0_size) < (int) sqrt((double) n)) {
      if (!id) printf ("Too many processes\n");
      MPI_Finalize();
      exit (1);
   }

   marked = (char *) malloc (size);
   if (marked == NULL) {
      printf ("Cannot allocate enough memory\n");
      MPI_Finalize();
      exit (1);
   }
    for (i = 0; i < size; i++) marked[i] = 0;
   if (!id)
   index = 0;
   prime = 3;
   do {
       printf( "from process %d of %d\n", id, p );
      if (prime * prime > low_value)
         first = (prime * prime - low_value)/2;
      else {
         if (!(low_value % prime)) first = 0;
         else
         // first = prime - (low_value % prime);
         first = ((low_value/prime)+1)%2==0?(((low_value/prime)+2)*prime-low_value)/2:(((low_value/prime)+1)*prime-low_value)/2;
      }
        printf( "process=%d,first=%d,prime=%d\n",id,first,prime);
      for (i = first; i < size; i+=prime) {marked[i] = 1;printf( "i=%d from process %d\n", i,id);
      }
      if (!id) {
         while (marked[++index]);
         printf("index=%d\n",index);
         prime = index*2 + 3;
      }
      MPI_Bcast (&prime,  1, MPI_INT, 0, MPI_COMM_WORLD);
   } while (prime * prime <= n);
   count = 0;
   for (i = 0; i < size; i++)
      if (!marked[i]) count++;
   MPI_Reduce (&count, &global_count, 1, MPI_INT, MPI_SUM,
      0, MPI_COMM_WORLD);
   elapsed_time += MPI_Wtime();
   if (!id) {
      printf ("%d primes are less than or equal to %d\n",
         global_count+1, n);
      printf ("Total elapsed time: %10.6f\n", elapsed_time);
   }
   MPI_Finalize ();
   return 0;
}
//我是王鸿(＾－＾)V
