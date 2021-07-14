#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <mpi.h>
#include<limits.h>
using namespace std;
#define NUM_PROCS 3 

int INF = INT_MAX;//无穷大
void PSRS(int *a, int len,int len_t);

int main(int argc, char* argv[])
{
	int localPID;
	int Num;//进程数
	int n, i, j, k;//数组大小
	int n_t;//处理不是进程倍数
	MPI_Init(&argc, &argv);//初始化
	MPI_Comm_rank(MPI_COMM_WORLD, &localPID);//本地进程号
	MPI_Comm_size(MPI_COMM_WORLD, &Num);//总进程数
	if (localPID == 0)
	{
		cout << "请输入数组大小：";
		cin >> n;
		if (n%NUM_PROCS != 0)
			n_t = NUM_PROCS - n % NUM_PROCS;
		else
			n_t = 0;

	}
	
	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&n_t, 1, MPI_INT, 0, MPI_COMM_WORLD);
	srand((int)time(0));//随机数种子
	int *a = new int[n+n_t];
	
	for (i = 0; i < n; i++)
	{
		a[i] = rand() % 100;
	}
	for (i = n; i < n + n_t; i++)
	{
		a[i] = INF;
	}
	if (localPID == 0)
	{
		cout << "随机数为：";
		for (i = 0; i < n; i++)
		{
			cout << a[i] << " ";
		}
		cout << "\n";
	}
	MPI_Barrier(MPI_COMM_WORLD);
	PSRS(a, n, n_t);
	sort(a, a + n);
	if (localPID == 0)
	{
		cout << "答案验证：";
		for (i = 0; i < n; i++)
			cout << a[i] << " ";
		cout << "\n";
	}
	MPI_Finalize();
	return 0;
}

void PSRS(int *a, int len,int len_t)
{
	int id, ProcNum, i, j, k, L;
	int seg;//分段
	int *sample = new int[NUM_PROCS*NUM_PROCS];//采样
	int *pivot = new int[NUM_PROCS - 1];//主元
	int b[NUM_PROCS][NUM_PROCS][100] = { 0 };//主元划分
	int c[NUM_PROCS][NUM_PROCS] = { 0 };//统计数目
	int Len[NUM_PROCS] = { 0 };//划分后各段长度
	int *Final = new int[len];//排序结果
	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Status status[ProcNum];
	MPI_Request request[ProcNum];
	seg = (len + len_t) / ProcNum;

	MPI_Barrier(MPI_COMM_WORLD);
	sort(a + id * seg, a + id * seg + seg);//局部排序
	for (i = 0; i < ProcNum; i++)//正则采样
	{
		sample[id*NUM_PROCS + i] = a[id*seg + i * seg / NUM_PROCS];
	}
	MPI_Barrier(MPI_COMM_WORLD);
	//debug
	/*
	if (id == 0)
	{
		cout << "id=" << id << "\n";
		for (i = 0; i < len; i++)
		{
			cout << a[i] << " ";
		}
		cout << "\n";
		for (i = 0; i < ProcNum; i++)
		{
			cout << sample[id*NUM_PROCS + i] << " ";
		}
		cout << "\n";
	}
	MPI_Barrier(MPI_COMM_WORLD);
	if (id == 1)
	{
		cout << "id=" << id << "\n";
		for (i = 0; i < len; i++)
		{
			cout << a[i] << " ";
		}
		cout << "\n";
		for (i = 0; i < ProcNum; i++)
		{
			cout << sample[id*NUM_PROCS + i] << " ";
		}
		cout << "\n";
	}
	MPI_Barrier(MPI_COMM_WORLD);
	if (id == 2)
	{
		cout << "id=" << id << "\n";
		for (i = 0; i < len; i++)
		{
			cout << a[i] << " ";
		}
		cout << "\n";
		for (i = 0; i < ProcNum; i++)
		{
			cout << sample[id*NUM_PROCS + i] << " ";
		}
		cout << "\n";
	}
	MPI_Barrier(MPI_COMM_WORLD);
	

	if (id == 0)
	{
		cout << "id=" << id << "\n";
		cout << "sample=" ;
		for (i = 0; i < ProcNum*ProcNum ; i++)
		{
			cout << sample[i] << " ";
		}
		cout << "\n";
	}
	MPI_Barrier(MPI_COMM_WORLD);
	*/
	//收集采样
	if(id!=0)
		MPI_Send(&sample[id*NUM_PROCS], NUM_PROCS, MPI_INT, 0,0,MPI_COMM_WORLD);
	if (id == 0)
	{
		for (i = 1; i < ProcNum; i++)
			MPI_Recv(&sample[i*NUM_PROCS], NUM_PROCS, MPI_INT, i,0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		//cout << "sample2=";
		//debug
		/*
		for (i = 0; i < ProcNum*ProcNum; i++)
		{
			cout << sample[i] << " ";
		}
		cout << "\n";
		*/
		//采样排序
		sort(sample, sample + NUM_PROCS * NUM_PROCS);
		for (i = 0; i < NUM_PROCS - 1; i++)//选择主元
		{
			pivot[i] = sample[i*NUM_PROCS + NUM_PROCS];
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
	//debug
	/*
	if (id == 0)
	{
		cout << "new sample=";
		for (i = 0; i < ProcNum*ProcNum; i++)
		{
			cout << sample[i] << " ";
		}
		cout << "\n";
		cout << "主元=";
		for (i = 0; i < NUM_PROCS - 1; i++)//选择主元
		{
			cout << pivot[i] << " ";
		}
		cout << "\n";
	}
	*/
	/*
	MPI_Barrier(MPI_COMM_WORLD);
	if (id == 1)
	{
		cout << "id_1" << "\n";
		for (i = 0; i < NUM_PROCS - 1; i++)
		{
			cout << pivot[i] << " ";
		}
		cout << "\n";
	}
	MPI_Barrier(MPI_COMM_WORLD);
	*/
	//广播主元
	MPI_Bcast(pivot, NUM_PROCS - 1, MPI_INT, 0, MPI_COMM_WORLD);//广播主元
	MPI_Barrier(MPI_COMM_WORLD);
	
	//debug
	/*
	if (id == 1)
	{
		cout << "id_1_new" << "\n";
		for (i = 0; i < NUM_PROCS - 1; i++)
		{
			cout << pivot[i] << " ";
		}
		cout << "\n";
	}
	MPI_Barrier(MPI_COMM_WORLD);
	*/
	//主元划分
	j = 0;
	k = 0;
	for (i = 0; i < seg; i++)
	{
		if (a[id*seg + i] <= pivot[j] && j < NUM_PROCS - 1)
		{
			b[id][j][k] = a[id*seg + i];
			c[id][j]++;
			k++;
		}
		else
		{
			if (j < NUM_PROCS - 1)
			{
				k = 0;
				j++;
				b[id][j][k] = a[id*seg + i];
				c[id][j]++;
				k++;
			}
			else
			{
				b[id][j][k] = a[id*seg + i];
				c[id][j]++;
				k++;
			}
		}
	}
	//收集计数
	if (id != 0)
	{
		MPI_Send(&c[id], NUM_PROCS, MPI_INT, 0, 1, MPI_COMM_WORLD);
	}
	else
	{
		for (i = 1; i < ProcNum; i++)
			MPI_Recv(&c[i], NUM_PROCS, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	if (id == 0)
	{
		/*
		cout << "count:\n";
		for (i = 0; i < NUM_PROCS; i++)
		{
			for (j = 0; j < NUM_PROCS; j++)
				cout << c[i][j] << " ";
			cout << "\n";
		}
		*/
		for (i = 1; i < NUM_PROCS; i++)
		{
			MPI_Send(c, NUM_PROCS*NUM_PROCS, MPI_INT, i, 2, MPI_COMM_WORLD);//广播计数
		}
	}
	else
	{
		MPI_Recv(c, NUM_PROCS*NUM_PROCS, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);//各进程接受计数
	}
	MPI_Barrier(MPI_COMM_WORLD);
	//debug
	/*
	if (id == 1)
	{
		cout << "id_1_count:\n";
		for (i = 0; i < NUM_PROCS; i++)
		{
			for (j = 0; j < NUM_PROCS; j++)
				cout << c[i][j] << " ";
			cout << "\n";
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
	*/
	//将分组发送到根进程
	if (id != 0)
	{
		MPI_Send(&b[id], NUM_PROCS * 100, MPI_INT, 0, 3, MPI_COMM_WORLD);
	}
	else
	{
		for (i = 1; i < NUM_PROCS; i++)
		{
			MPI_Recv(&b[i], NUM_PROCS * 100, MPI_INT, i, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
	//debug
	/*
	if(id==0)
		for (i = 0; i < NUM_PROCS; i++)
		{
			cout << "进程" << i << "\n";
			for (j = 0; j < NUM_PROCS; j++)
			{
				for (k = 0; k < c[i][j]; k++)
					cout << b[i][j][k] << " ";
				cout << "\n";
			}
			cout << "\n";
		}
	MPI_Barrier(MPI_COMM_WORLD);
	*/
	//将分组广播
	if (id == 0)
	{
		for (i = 1; i < NUM_PROCS; i++)
		{
			MPI_Send(b, NUM_PROCS*NUM_PROCS * 100, MPI_INT, i, 4, MPI_COMM_WORLD);
		}
	}
	else
	{
		MPI_Recv(b, NUM_PROCS*NUM_PROCS * 100, MPI_INT, 0, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	//debug
	/*
	if (id == 1)
		for (i = 0; i < NUM_PROCS; i++)
		{
			cout << "进程" << i << "\n";
			for (j = 0; j < NUM_PROCS; j++)
			{
				for (k = 0; k < c[i][j]; k++)
					cout << b[i][j][k] << " ";
				cout << "\n";
			}
			cout << "\n";
		}
	MPI_Barrier(MPI_COMM_WORLD);
	*/
	//计算各段长度
	for (i = 0; i < NUM_PROCS; i++)
		for (j = 0; j < NUM_PROCS; j++)
		{
			Len[i] = c[j][i] + Len[i];
		}
	//新划分间隔
	L = 0;
	for (i = 0; i < id; i++)
		L = L + Len[i];
	i = 0;
	for (j = 0; j < NUM_PROCS; j++)
	{
		for (k = 0;; k++)
		{
			if (k < c[j][id])
			{
				Final[L + i] = b[j][id][k];
				i++;
			}
			else
				break;
		}
	}
	sort(Final + L, Final + L + Len[id]);//局部排序
	MPI_Barrier(MPI_COMM_WORLD);
	if (id != 0)
	{
		MPI_Send(&Final[L], Len[id], MPI_INT, 0, 5, MPI_COMM_WORLD);
	}
	else
	{
		for (i = 1; i < NUM_PROCS; i++)
		{
			L = L + Len[i - 1];
			MPI_Recv(&Final[L], Len[i], MPI_INT, i, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
		cout << "排序结果：";
		for (i = 0; i < len; i++)
			cout << Final[i] << " ";
			cout << "\n";
	}
	MPI_Barrier(MPI_COMM_WORLD);
}