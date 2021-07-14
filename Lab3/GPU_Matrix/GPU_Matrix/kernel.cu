#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include<cuda.h>
#include<string.h>
#include<ctime>

#define BLOCK_NUM 4   // 块数量
#define THREAD_NUM 2 // 每个块中的线程数
#define R_SIZE (BLOCK_NUM * THREAD_NUM) // 矩阵行列数
#define M_SIZE (R_SIZE * R_SIZE) //矩阵规模

__global__ void mat_mul(int* mat1, int* mat2, int* result)
{
	const int bid = blockIdx.x; //块 id
	const int tid = threadIdx.x; //进程 id
	// 每个线程计算一行
	const int row = bid * THREAD_NUM + tid; //计算当前进程所需计算的行数
	for (int c = 0; c < R_SIZE; c++)
	{
		for (int n = 0; n < R_SIZE; n++)
		{
			result[row * R_SIZE + c] += mat1[row * R_SIZE + n] * mat2[n * R_SIZE + c];
		}
	}
}

int main(int argc, char* argv[])
{
	int* mat1, *mat2, *result;
	int* g_mat1, *g_mat2, *g_mat_result;
	double time_pc, time_normal;

	clock_t startTime, endTime;

	// 用一位数组表示二维矩阵
	mat1 = (int*)malloc(M_SIZE * sizeof(int));
	mat2 = (int*)malloc(M_SIZE * sizeof(int));
	result = (int*)malloc(M_SIZE * sizeof(int));

	// initialize
	for (int i = 0; i < M_SIZE; i++)
	{
		mat1[i] = rand() % 10;
		mat2[i] = rand() % 10;
		result[i] = 0;
	}

	printf("矩阵 1 为：\n");
	for (int i = 0; i < M_SIZE; i++)
		if((i + 1) % R_SIZE == 0)
			printf("%d\n", mat1[i]);
		else
			printf("%d ", mat1[i]);

	printf("\n矩阵 2 为：\n");
	for (int i = 0; i < M_SIZE; i++)
		if ((i + 1) % R_SIZE == 0)
			printf("%d\n", mat2[i]);
		else
			printf("%d ", mat2[i]);
	

	cudaMalloc((void**)&g_mat1, sizeof(int) * M_SIZE);
	cudaMalloc((void**)&g_mat2, sizeof(int) * M_SIZE);
	cudaMalloc((void**)&g_mat_result, sizeof(int) * M_SIZE);

	cudaMemcpy(g_mat1, mat1, sizeof(int) * M_SIZE, cudaMemcpyHostToDevice);
	cudaMemcpy(g_mat2, mat2, sizeof(int) * M_SIZE, cudaMemcpyHostToDevice);

	/*并行方法*/
	startTime = clock();//计时开始

	mat_mul <<<BLOCK_NUM, THREAD_NUM >>> (g_mat1, g_mat2, g_mat_result);

	cudaMemcpy(result, g_mat_result, sizeof(int) * M_SIZE, cudaMemcpyDeviceToHost);

	endTime = clock();//计时结束
	time_pc = (double)(endTime - startTime) / CLOCKS_PER_SEC;
	printf("并行所用时间: %lf s\n", time_pc);


	/*串行方法*/
	startTime = clock();//计时开始

	for (int r = 0; r < R_SIZE; r++) {
		for (int c = 0; c < R_SIZE; c++) {
			for (int n = 0; n < R_SIZE; n++) {
				result[r * R_SIZE + c] += mat1[r * R_SIZE + n] * mat2[n * R_SIZE + c];
			}
		}
	}

	endTime = clock();//计时结束
	time_normal = (double)(endTime - startTime) / CLOCKS_PER_SEC;
	printf("串行所用时间: %lf s\n", time_normal);

	printf("加速比为：%lf\n", time_normal / time_pc);

	printf("\n二矩阵乘积为：\n");
	for (int i = 0; i < M_SIZE; i++)
		if ((i + 1) % R_SIZE == 0)
			printf("%d\n\n", result[i]);
		else
			printf("%d ", result[i]);
}