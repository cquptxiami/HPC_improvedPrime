#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#define SIZE 245057//训练样本个数及特征
#define Fnum 4
#define K 3//knn 参数
//选择排序，同时同步样本的标签（对每个训练样本和测试样本的距离自然序排序，同步对应的标签，排序K趟，不需要全排）
void fun(double *a,double *value,int *b,int *label,int k,int length){
    int i,j,temp_label,min;
    double tem_value;
    for(i=0;i<k;i++)
   {     min=i;
        for(j=i+1;j<length;j++)
          if(a[min]>a[j])
             min=j;
        tem_value=a[i];
        a[i]=a[min];
        a[min]=tem_value;

        temp_label=b[i];
        b[i]=b[min];
        b[min]=temp_label;

   }
//排序后的数组复制到新的数组通信
    for(i=0;i<length;i++){
       value[i]=a[i];
       label[i]=b[i];}
}
//选择排序0进程对有所进程本地排序结果规约，然后在knn趟排序，得出标签（与上个函数的参数不同）
int get_label(double *a,int *b,int knn,int length){
int count1,count2;
    int i,j,temp_label,min;
    double tem_value;
    for(i=0;i<knn;i++)
   {     min=i;
        for(j=i+1;j<length;j++)
          if(a[min]>a[j])
             min=j;
        tem_value=a[i];
        a[i]=a[min];
        a[min]=tem_value;

        temp_label=b[i];
        b[i]=b[min];
        b[min]=temp_label;

   }

    for(i=0;i<length;i++){
    if(b[i]==1)
      count1++;
    else
      count2++;
    }
   if(count1>count2)
       return 1;
   return 2;
}


int main (int argc, char *argv[])
{
   int globalSample[SIZE][Fnum];
   double elapsed_time;//Parallel execution time
   int id;
   int p;
   int i=0,j=0;
   MPI_Init (&argc, &argv);//初始化
   MPI_Barrier(MPI_COMM_WORLD);//同步
   elapsed_time = -MPI_Wtime();
   MPI_Comm_rank (MPI_COMM_WORLD, &id);//进程编号
   MPI_Comm_size (MPI_COMM_WORLD, &p);//进程个数
   MPI_Status status;
   //进程0读书据
   int test[3]={128,120,128};//测试样本
   //变量
   int **buff,*b,*label,*local_label,*ans_label;
   int k=0,num=0,startRow,endRow,buff_i,pr0_Rows,temp=0;
   double sum=0.0,*dis,*local_value,*ans_value;
   //把数据分给每个进程(行随机分配)
   if(SIZE%p==0)
   num=SIZE/p;
   else
   num=SIZE/p+1;//进程分配的最大行数
   printf("num=%d\n",num);
   pr0_Rows=SIZE-(p-1)*num;//行分配0进程处理的行数

   //全局排序过后的规约结果数组
   ans_label=(int*)malloc(p*K*sizeof(int));
   ans_value=(double*)malloc(p*K*sizeof(double));

   //每个进程的排序结果数组(最后发送给0进程)
   local_label=(int*)malloc(num*sizeof(int));
   local_value=(double*)malloc(num*sizeof(double));


   b=(int*)malloc(num*3*sizeof(int));
   buff=(int**)malloc(num*sizeof(int*));
   for(i=0;i<num;i++)
   buff[i]=&b[i*3];
   dis=(double*)malloc(num*sizeof(double));//距离数组
   label=(int*)malloc(num*sizeof(int));//标签数组

   if(!id){
    FILE *fin=fopen("Skin.txt","r");
	if(fin==NULL){
		printf("fail\n");
		exit(0);
	  }//if
	int i=0,j=0;
	for(i=0;i<SIZE;i++){
	   for(j=0;j<Fnum;j++)
	   fscanf(fin,"%d",&globalSample[i][j]);
       //printf("%d load data\n",id);
      }//for
    fclose(fin);

   for(k=1;k<p;k++){
   startRow=(k-1)*num;
   endRow=k*num;
   //printf("startRow=%d,endRow=%d\n",startRow,endRow);

   for(buff_i=0,i=startRow;i<endRow;buff_i++,i++){
      for(j=0;j<3;j++){
        buff[buff_i][j]=globalSample[i][j];
           }
   label[buff_i]=globalSample[i][3];
       }//for buff
    /*
   //验证缓存的数据
    for(i=0;i<num;i++){
	   for(j=0;j<4;j++){
	   printf(" %d",buff[i][j]);
	   }
	   printf("\n");
     }*/
  //把数据分给每个进程(行分配)
   MPI_Send(*buff,num*3,MPI_INT,k,0,MPI_COMM_WORLD);//send testSample to others
   MPI_Send(label,num,MPI_INT,k,0,MPI_COMM_WORLD);
//每一个进程计算和测试样本的距离
     }//for k


     //接受其他从从进程排序之后的结果,并把结果复制给ans数组
     for(k=1;k<p;k++){
     MPI_Recv(local_value,K,MPI_DOUBLE,k,0,MPI_COMM_WORLD,&status);
     MPI_Recv(local_label,K,MPI_INT,k,0,MPI_COMM_WORLD,&status);

     for(i=0;i<K;i++){
     ans_value[(k-1)*K+i]=local_value[i];
     ans_label[(k-1)*K+i]=local_label[i];
     }


  }


   //把剩下的数据留给0进程处理
   b=(int*)malloc(pr0_Rows*4*sizeof(int));
   buff=(int**)malloc(pr0_Rows*sizeof(int*));
   for(i=0;i<num;i++)
   buff[i]=&b[i*4];
   label=(int*)malloc(pr0_Rows*sizeof(int));

   for(buff_i=0,i=(p-1)*num;i<SIZE;buff_i++,i++){
      for(j=0;j<4;j++){
        buff[buff_i][j]=globalSample[i][j];
           }
    label[buff_i]=globalSample[i][3];
       }//for

      printf("进程%d计算距离\n",id);
      for(i=0;i<pr0_Rows;i++){
       temp=0;
	    for(j=0;j<3;j++){
         temp+=pow((buff[i][j]-test[j]),2);
	     }
	   dis[i]=sqrt(temp);
       }//for
       //选择排序
       printf("进程%d选择排序,每个进程选出k个最近邻\n",id);
       fun(dis,local_value,label,local_label,K,pr0_Rows);

       //0进程排序的结果
       for(i=K-1,j=p*K-1;i>=0;i--,j--){
        ans_value[j]=dis[i];
        ans_label[j]=label[i];
        }
/*
     printf("进程0打印规约结果ans_value:\n");
     for(i=0;i<p*K;i++){
     printf("%lf\n",ans_value[i]);
     }*/
    //在对规约的结果进行排序
    printf("测试样本的标签:%d\n",get_label(ans_value,ans_label,K,p*K));
     elapsed_time += MPI_Wtime();
     printf ("Total elapsed time: %10.6f\n", elapsed_time);
/*
    //可以打印排序后规约的结果
    printf("进程0打印对所有进程规约结果排序之后ans_value:\n");
    for(i=0;i<p*K;i++){
     printf("%lf\n",ans_value[i]);
     }
*/
  }//(!id)
   else{
     MPI_Recv(*buff,num*4,MPI_INT,0,0,MPI_COMM_WORLD,&status);
     MPI_Recv(label,num,MPI_INT,0,0,MPI_COMM_WORLD,&status);
     printf("进程%d计算距离\n",id);
     for(i=0;i<num;i++){
       temp=0;
	   for(j=0;j<3;j++){
         temp+=pow((buff[i][j]-test[j]),2);
	     }
	   dis[i]=sqrt(temp);
       }//for
       //选择排序
       printf("进程%d选择排序,每个进程选出k个最近邻,然后发送给0进程\n",id);
       fun(dis,local_value,label,local_label,K,num);

       //望0进程k个值
       MPI_Send(local_value,K,MPI_DOUBLE,0,0,MPI_COMM_WORLD);//send testSample to others
       MPI_Send(local_label,K,MPI_INT,0,0,MPI_COMM_WORLD);
   }
/*
  //验证每个进程接受的数据（没问题)
   if(id==0){
   printf("process id=%dnum=%d\n",id,num);
   for(i=0;i<pr0_Rows;i++){
    for(j=0;j<4;j++){
    printf(" %d",buff[i][j]);
    }
    printf("\n");
   }
 }
 //验证每个进程和测试样本的偶是距离（没问题)
 if(id==0){
   for(i=0;i<pr0_Rows;i++)
    printf(" %lf\n",dis[i]);
    printf("\n");
   }
   if(id==0){
   for(i=0;i<pr0_Rows;i++)
    printf(" %d\n",label[i]);
    printf("\n");
   }*/

/*
   //排序之后输出距离数组和欧式距离

   if(id==1){
   for(i=0;i<num;i++)
    printf(" %lf\n",local_value[i]);
    printf("\n");
   }
   if(id==1){
   for(i=0;i<num;i++)
    printf(" %d\n",local_label[i]);
    printf("\n");
   }
*/

   MPI_Finalize ();
   return 0;
}
