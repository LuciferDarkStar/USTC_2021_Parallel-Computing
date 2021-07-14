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

int INF = INT_MAX;//�����
void PSRS(int *a, int len,int len_t);

int main(int argc, char* argv[])
{
	int localPID;
	int Num;//������
	int n, i, j, k;//�����С
	int n_t;//�����ǽ��̱���
	MPI_Init(&argc, &argv);//��ʼ��
	MPI_Comm_rank(MPI_COMM_WORLD, &localPID);//���ؽ��̺�
	MPI_Comm_size(MPI_COMM_WORLD, &Num);//�ܽ�����
	if (localPID == 0)
	{
		cout << "�����������С��";
		cin >> n;
		if (n%NUM_PROCS != 0)
			n_t = NUM_PROCS - n % NUM_PROCS;
		else
			n_t = 0;

	}
	
	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&n_t, 1, MPI_INT, 0, MPI_COMM_WORLD);
	srand((int)time(0));//���������
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
		cout << "�����Ϊ��";
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
		cout << "����֤��";
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
	int seg;//�ֶ�
	int *sample = new int[NUM_PROCS*NUM_PROCS];//����
	int *pivot = new int[NUM_PROCS - 1];//��Ԫ
	int b[NUM_PROCS][NUM_PROCS][100] = { 0 };//��Ԫ����
	int c[NUM_PROCS][NUM_PROCS] = { 0 };//ͳ����Ŀ
	int Len[NUM_PROCS] = { 0 };//���ֺ���γ���
	int *Final = new int[len];//������
	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Status status[ProcNum];
	MPI_Request request[ProcNum];
	seg = (len + len_t) / ProcNum;

	MPI_Barrier(MPI_COMM_WORLD);
	sort(a + id * seg, a + id * seg + seg);//�ֲ�����
	for (i = 0; i < ProcNum; i++)//�������
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
	//�ռ�����
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
		//��������
		sort(sample, sample + NUM_PROCS * NUM_PROCS);
		for (i = 0; i < NUM_PROCS - 1; i++)//ѡ����Ԫ
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
		cout << "��Ԫ=";
		for (i = 0; i < NUM_PROCS - 1; i++)//ѡ����Ԫ
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
	//�㲥��Ԫ
	MPI_Bcast(pivot, NUM_PROCS - 1, MPI_INT, 0, MPI_COMM_WORLD);//�㲥��Ԫ
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
	//��Ԫ����
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
	//�ռ�����
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
			MPI_Send(c, NUM_PROCS*NUM_PROCS, MPI_INT, i, 2, MPI_COMM_WORLD);//�㲥����
		}
	}
	else
	{
		MPI_Recv(c, NUM_PROCS*NUM_PROCS, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);//�����̽��ܼ���
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
	//�����鷢�͵�������
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
			cout << "����" << i << "\n";
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
	//������㲥
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
			cout << "����" << i << "\n";
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
	//������γ���
	for (i = 0; i < NUM_PROCS; i++)
		for (j = 0; j < NUM_PROCS; j++)
		{
			Len[i] = c[j][i] + Len[i];
		}
	//�»��ּ��
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
	sort(Final + L, Final + L + Len[id]);//�ֲ�����
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
		cout << "��������";
		for (i = 0; i < len; i++)
			cout << Final[i] << " ";
			cout << "\n";
	}
	MPI_Barrier(MPI_COMM_WORLD);
}