#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#define ROW 50
#define trainRow 40
#define testRow 10
#define COLUMN 692
int main (int argc, char *argv[])
{
   double elapsed_time;//Parallel execution time
   int id;

   int p;
   int i=0,j=0,k,num,startRow,endRow;
   MPI_Init (&argc, &argv);//初始化
   MPI_Barrier(MPI_COMM_WORLD);//同步
   elapsed_time = -MPI_Wtime();
   MPI_Comm_rank (MPI_COMM_WORLD, &id);//进程编号
   MPI_Comm_size (MPI_COMM_WORLD, &p);//进程个数
   MPI_Status status;
   //int a=1;
   //while(a);
   //进程0读书据,把数据分给每个进程(行随机分配)
   if(trainRow%p==0)
   num=trainRow/p;
   else
   num=trainRow/p+1;//进程分配的最大行数
   printf("num=%d\n",num);
   int **buff,*b,globalSample[ROW][COLUMN+1],p1num[COLUMN],p0num[COLUMN],p1Denom=0,p0Denom=0,pAbnum=0,global_pAbnum=0;
   int global_p1num[COLUMN],global_p0num[COLUMN],global_p1Denom=2.0,global_p0Denom=2.0;//global probality of feature
   for(i=0;i<COLUMN;i++){
        global_p1num[i]=0;
        global_p0num[i]=0;
        //p0num[i]=0;
        //p1num[i]=0;
        }
    double final_p0[COLUMN],final_p1[COLUMN],final_pA=0.0;
   //the buffer of each processor
   b=(int*)malloc(num*(COLUMN+1)*sizeof(int));
   buff=(int**)malloc(num*sizeof(int*));
   for(i=0;i<num;i++)
   buff[i]=&b[i*(COLUMN+1)];

//variables for test
int testnum;
if(testRow%p==0)
testnum=testRow/p;
else
testnum=testRow/p+1;
int errorCount=0,label=1,global_errorcount=0;
double p1=0.0;double p0=0.0;

   //进程0读书据,把数据分给每个进程(行随机分配)
   if(!id){
    FILE *fin=fopen("samples.txt","r");
	if(fin==NULL){
		printf("fail\n");
		exit(0);
	  }//if
	for(i=0;i<ROW;i++){
	   for(j=0;j<COLUMN;j++)
	   fscanf(fin,"%d",&globalSample[i][j]);
       if(i%2==0)
         globalSample[i][j]=1;
       else
         globalSample[i][j]=0;
       //printf("%d load data\n",id);
      }//for
    fclose(fin);
/*
    //scan data
    for(i=0;i<num;i++){
	   for(j=0;j<COLUMN+1;j++)
	   printf("%d ",globalSample[i][j]);//for
    printf("\n");
      }//for
    printf("\n");
    */
   int temp_i=0;
  //destribe data to for processor
   for(k=1;k<p;k++){
      startRow=(k-1)*num;
      endRow=k*num;
      for(temp_i=0,i=startRow;i<endRow;i++,temp_i++){
	     for(j=0;j<COLUMN+1;j++){
	         buff[temp_i][j]=globalSample[i][j];
	      }
        }
        /*
        if(k==1)
        for(i=0;i<num;i++){
	     for(j=0;j<COLUMN+1;j++)
	        printf("%d ",buff[i][j]);//for
       printf("\n");
        }//for
        */
        //send data to each processor
      MPI_Send(*buff,num*(COLUMN+1),MPI_INT,k,0,MPI_COMM_WORLD);
     }

    for(k=1;k<p;k++){
    MPI_Recv(p0num,COLUMN,MPI_INT,k,0,MPI_COMM_WORLD,&status);
    MPI_Recv(&p0Denom,1,MPI_INT,k,0,MPI_COMM_WORLD,&status);
    MPI_Recv(p1num,COLUMN,MPI_INT,k,0,MPI_COMM_WORLD,&status);
    MPI_Recv(&p1Denom,1,MPI_INT,k,0,MPI_COMM_WORLD,&status);
    MPI_Recv(&pAbnum,1,MPI_INT,k,0,MPI_COMM_WORLD,&status);
    for(j=0;j<COLUMN;j++){
       global_p0num[j]+=p0num[j];
       global_p1num[j]+=p1num[j];
        }
         global_p0Denom+=p0Denom;
         global_p1Denom+=p1Denom;
         global_pAbnum+=pAbnum;
      }//for k
       //processor o process the remaining data

    startRow=(p-1)*num;
    endRow=trainRow;
    /*
    printf("\nprocessor 0 data\n");
    for(i=startRow;i<endRow;i++){
	   for(j=0;j<COLUMN;j++)
	   printf("%d ",globalSample[i][j]);
	   printf("\n");
	       }
        */
    for(i=startRow;i<endRow;i++){
	   for(j=0;j<COLUMN;j++){
	    if(globalSample[i][COLUMN]==1){
	       global_p1num[j]+=globalSample[i][j];
	       global_p1Denom+=globalSample[i][j];
	        }
        else{
           global_p0num[j]+=globalSample[i][j];
	       global_p0Denom+=globalSample[i][j];
          }
	   }//for
      if(globalSample[i][COLUMN]==1)
	    global_pAbnum+=1;
    }//for



// no problem
      //printf("\nafter processor 0 perform add :\nglobal_p1num\n");
      for(j=0;j<COLUMN;j++){
       global_p1num[j]=global_p1num[j]+1;
       //printf("%d ",global_p1num[j]);
      }
       printf("\n");

       //printf("global_p0num\n");
      for(j=0;j<COLUMN;j++){
       global_p0num[j]=global_p0num[j]+1;
       //printf("%d ",global_p0num[j]);
       }

       //printf("\nglobal_p0Denom=%d global_p1Denom=%d\n",global_p0Denom,global_p1Denom);


      // compute probality
      for(j=0;j<COLUMN;j++){
       final_p0[j]=log(global_p0num[j]/(global_p0Denom*1.0));
       final_p1[j]=log(global_p1num[j]/(global_p1Denom*1.0));
        }
        final_pA=global_pAbnum/(trainRow*1.0);
        /*
        printf("final_p1\n");
        for(j=0;j<COLUMN;j++)
            printf("%lf ",final_p1[j]);
        //printf("pAbnum/trainRow=%lf\n",global_pAbnum/(trainRow*1.0));
*/
       //processor 0 distribute the probability array class 0 final_p0 and class 1 final_p1 to each processor
       for(k=1;k<p;k++){
          MPI_Send(final_p0,(COLUMN),MPI_DOUBLE,k,0,MPI_COMM_WORLD);
          MPI_Send(final_p1,(COLUMN),MPI_DOUBLE,k,0,MPI_COMM_WORLD);
          MPI_Send(&final_pA,1,MPI_DOUBLE,k,0,MPI_COMM_WORLD);
         }

         //processor 0 test remaining sample
         for(i=(p-1)*testnum+trainRow;i<ROW;i++){
             p1=0.0,p0=0.0;
             for(j=0;j<COLUMN;j++){
                 p1+=globalSample[i][j]*final_p1[j];
                 p0+=globalSample[i][j]*final_p1[j];
                 }
            if(p1>p0)
            label=1;
            else
              label=0;
            if(label!=globalSample[i][COLUMN])
              errorCount++;
             }
   }//if(!id)



 else{
     //each processor receive data from processor
     MPI_Recv(*buff,num*(COLUMN+1),MPI_INT,0,0,MPI_COMM_WORLD,&status);
/*
     // processor 2 print data received from processor 0
    if(id==2){
       printf("processor %d print data\n",id);
       for(i=0;i<num;i++){
	     for(j=0;j<COLUMN+1;j++)
	        printf("%d ",buff[i][j]);//for
       printf("\n");
        }//for
      }

*/
//each processor compute p of feature
for(j=0;j<COLUMN;j++){
    p0num[j]=0;
    p1num[j]=0;//initialiaze,otherwiese there are many random
    }
    pAbnum=0;
    for(i=0;i<num;i++){
	   for(j=0;j<COLUMN;j++){
	    if(buff[i][COLUMN]==1){
	       p1Denom+=buff[i][j];
	       p1num[j]+=buff[i][j];
	        }
        else
        {
	       p0Denom+=buff[i][j];
	       p0num[j]+=buff[i][j];
        }
	   }//for
	   if(buff[i][COLUMN]==1)
	   pAbnum=pAbnum+1;
    }//for
/*

    if(id==2){
         printf("processor %d print p0num\n",id);
	     for(j=0;j<20;j++)
	        printf("%d ",p0num[j]);//for
	        printf("\n");
	        printf("processor %d print p1num\n",id);
	        for(j=0;j<20;j++)
	        printf("%d ",p1num[j]);//for
	        printf("\n");
	        printf("p0Denom=%d,p1Denom=%d ",p0Denom,p1Denom);//for

        }
*/
  //printf("processor %d print pAbnum %d \n",id,pAbnum);

//send p0num,p1num,p0Denom,p1Denom to processor 0
  MPI_Send(p0num,COLUMN,MPI_INT,0,0,MPI_COMM_WORLD);
  MPI_Send(&p0Denom,1,MPI_INT,0,0,MPI_COMM_WORLD);
  MPI_Send(p1num,COLUMN,MPI_INT,0,0,MPI_COMM_WORLD);
  MPI_Send(&p1Denom,1,MPI_INT,0,0,MPI_COMM_WORLD);
  MPI_Send(&pAbnum,1,MPI_INT,0,0,MPI_COMM_WORLD);

//each processor receive the probability array class 0 final_p0 and class 1 final_p1
  MPI_Recv(final_p0,COLUMN,MPI_DOUBLE,0,0,MPI_COMM_WORLD,&status);
  MPI_Recv(final_p1,COLUMN,MPI_DOUBLE,0,0,MPI_COMM_WORLD,&status);
  MPI_Recv(&final_pA,1,MPI_DOUBLE,0,0,MPI_COMM_WORLD,&status);

/*// processor 2 print data received from processor 0
if(id==2){
  printf("processor %d receive final_p1\n",id);
  for(j=0;j<COLUMN;j++)
  printf(" %lf",final_p1[j]);
  printf("\n");
}*/

//test the last 10 sample
  for (i=trainRow+(id-1)*testnum;i<trainRow+(id)*testnum;i++){
      p1=0.0,p0=0.0;
    for(j=0;j<COLUMN;j++){
        p1+=globalSample[i][j]*final_p1[j];
        p0+=globalSample[i][j]*final_p1[j];
        }
        if(p1>p0)
        label=1;
        else
        label=0;
        if(label!=globalSample[i][COLUMN])
        errorCount++;
   }
 }

MPI_Reduce (&errorCount, &global_errorcount, 1, MPI_INT, MPI_SUM,
      0, MPI_COMM_WORLD);
if(!id){
   printf("the global_errorcount=%d\n",global_errorcount);
   printf("the errorrate=%lf\n",global_errorcount/(testRow*1.0));
   elapsed_time += MPI_Wtime();
   printf ("Total elapsed time: %10.6f\n", elapsed_time);
   }

   MPI_Finalize ();
   return 0;
}

