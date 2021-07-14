#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
using namespace std;
#define NUM_THREADS 3 


void PSRS(int *a,int len)
{
	int seg_len = len / NUM_THREADS;//每段长度
	int seg_len_last = len - (NUM_THREADS - 1)*seg_len;//特别处理最后一段
	int id;//线程号
	int i,j,k;
	int *sample=new int[NUM_THREADS*NUM_THREADS];//采样
	int *pivot = new int[NUM_THREADS - 1];//主元
	int b[NUM_THREADS][NUM_THREADS][100] = { 0 };//主元划分
	int c[NUM_THREADS][NUM_THREADS] = { 0 };//统计数目
	int *Final = new int[len];//排序结果
	int Len[NUM_THREADS] = { 0 };
	int L = 0;
	omp_set_num_threads(NUM_THREADS);
#pragma omp parallel shared(a,sample,pivot,b,c,Final,Len,seg_len,seg_len_last,len) private(id,i,j,k,L)//各段局部排序
	{
		id = omp_get_thread_num();
		if (id == NUM_THREADS - 1)
		{
			sort(a + id * seg_len, a + id * seg_len + seg_len_last);
		}
		else
		{
			sort(a + id * seg_len, a + id * seg_len + seg_len);
		}
		//正则采样
		for (i = 0; i < NUM_THREADS; i++)
			sample[id*NUM_THREADS + i] = a[id*seg_len + i * seg_len / NUM_THREADS];

#pragma omp barrier//同步 
		//debug
		/*
		#pragma omp critical
		{
			cout << id << ":";
			i = 0;
			while (1)
			{
				cout << a[id*seg_len + i] << " ";
				i++;
				if ((id != NUM_THREADS - 1 && i == seg_len) || (id == NUM_THREADS - 1 && i == seg_len_last))
					break;
			}
			cout << "\n";
		}
		#pragma omp master
		{
			for (i = 0; i < NUM_THREADS*NUM_THREADS; i++)
				cout << sample[i] << " ";
			cout << "\n";
		}
		*/
#pragma omp master//采样排序
		{
			sort(sample, sample + NUM_THREADS * NUM_THREADS);
			//debug
			/*
			for (i = 0; i < NUM_THREADS*NUM_THREADS; i++)
				cout << sample[i] << " ";
			cout << "\n";
			*/
			for (i = 0; i < NUM_THREADS - 1; i++)//选择主元
			{
				pivot[i] = sample[i*NUM_THREADS + NUM_THREADS];
				//cout << pivot[i] << " ";
			}
			//cout << "\n";
		}
#pragma omp barrier//同步 
		//主元划分
		if (id != NUM_THREADS - 1)//一般处理
		{
			j = 0;
			k = 0;
			for (i = 0; i < seg_len; i++)
			{
				if (a[id*seg_len + i] <= pivot[j] && j<NUM_THREADS-1)
				{
					b[id][j][k] = a[id*seg_len + i];
					c[id][j]++;
					k++;
				}
				else
				{
					if (j < NUM_THREADS - 1)
					{
						k = 0;
						j++;
						b[id][j][k] = a[id*seg_len + i];
						c[id][j]++;
						k++;
					}
					else
					{
						b[id][j][k] = a[id*seg_len + i];
						c[id][j]++;
						k++;
					}
				}
			}
		}
		else//最后一段特殊处理 
		{
			j = 0;
			k = 0;
			for (i = 0; i < seg_len_last; i++)
			{
				if (a[id*seg_len + i] <= pivot[j] && j < NUM_THREADS - 1)
				{
					b[id][j][k] = a[id*seg_len + i];
					c[id][j]++;
					k++;
				}
				else
				{
					if (j < NUM_THREADS - 1)
					{
						k = 0;
						j++;
						b[id][j][k] = a[id*seg_len + i];
						c[id][j]++;
						k++;
					}
					else
					{
						b[id][j][k] = a[id*seg_len + i];
						c[id][j]++;
						k++;
					}
				}
			}
		}
		
#pragma omp barrier//同步 
//debug
/*
#pragma omp master
		{	
			for (i = 0; i < NUM_THREADS; i++)
			{
				for (j = 0; j < NUM_THREADS; j++)
				{
					for (k = 0; k < c[i][j]; k++)
					{
						cout << b[i][j][k] << " ";
					}
					cout << "\n";
				}

			}
		}
*/		
		//统计各划分总长度
		for (i = 0; i < NUM_THREADS; i++)
			Len[id] = c[i][id] + Len[id];
		//cout << id << ":" << Len[id] << "\n";
#pragma omp barrier//同步
		//全局交换
		L = 0;
		for (i = 0; i < id; i++)
			L = L + Len[i];
		//cout <<id<<" t："<< L << "\n";
		i = 0;
		for (j = 0; j < NUM_THREADS; j++)
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
#pragma omp barrier//同步
#pragma omp master
		{
			cout << "排序结果：";
			for (i = 0; i < len; i++)
				cout << Final[i] << " ";
			cout << "\n";
		}
	}
}

int main()
{
	int n,i;//数组大小
	cout << "请输入数组大小：";
	cin >> n;
	srand((int)time(0));//随机数种子
	int *a = new int[n];
	printf("随机数组为：");
	for (i = 0; i < n; i++)
	{
		a[i] = rand() % 100;
		cout << a[i] << " ";
	}
	cout << "\n";
	PSRS(a,n);
	sort(a, a + n);
	cout << "答案验证：";
	for (i = 0; i < n; i++)
		cout << a[i] << " ";
	cout << "\n";
	system("pause");
}