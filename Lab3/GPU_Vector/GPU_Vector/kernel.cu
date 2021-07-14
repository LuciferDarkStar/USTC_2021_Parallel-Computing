#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdio.h>

// Kernal:
__global__ void MatrixMultiply(double *a, double * b, double *c, int N) {
	int tx = threadIdx.x + blockIdx.x * blockDim.x;
	if (tx < N) {
		double sum = 0;
		for (int k = 0; k < N; ++k) {
			sum += a[tx * N + k] * b[k];
		}
		c[tx] = sum;
	}
}

cudaError_t matrixMultiplyWithCuda(double *a, double *b, double *c, size_t size);

int main()
{
	std::ifstream in("data.txt");
	int N;
	in >> N;//矩阵阶数
	if (in.fail()) {
		printf("错误①！\n");
	}
	else {
		printf("成功！\n");
	}
	// host initial
	double *a = new double[N * N];
	double *b = new double[N];
	double *c = new double[N];

	// read 
	for (int i = 0; i < N; ++i)
		for (int j = 0; j < N; ++j) in >> a[i * N + j];

	for (int i = 0; i < N; ++i) in >> b[i];

	cudaError_t cudaStatus = matrixMultiplyWithCuda(a, b, c, N);

	std::ofstream out("output.txt");
	for (int i = 0; i < N; ++i) {
		out << std::setiosflags(std::ios::fixed) << c[i] << " ";
		out << std::endl;
	}
	cudaStatus = cudaThreadExit();

	// host free 
	delete[] a;
	delete[] b;
	delete[] c;
	return 0;
}
cudaError_t matrixMultiplyWithCuda(double *a, double *b, double *c, size_t N) {
	double *dev_a = 0;
	double *dev_b = 0;
	double *dev_c = 0;
	cudaError_t cudaStatus;
	cudaStatus = cudaMalloc((void**)&dev_a, N * N * sizeof(double));
	cudaStatus = cudaMalloc((void**)&dev_b, N * sizeof(double));
	cudaStatus = cudaMalloc((void**)&dev_c, N * sizeof(double));
	cudaStatus = cudaMemcpy(dev_a, a, N * N * sizeof(double), cudaMemcpyHostToDevice);
	cudaStatus = cudaMemcpy(dev_b, b, N * sizeof(double), cudaMemcpyHostToDevice);
	if (cudaStatus != cudaSuccess) {
		printf("错误②！\n");
		goto Error;
	}
	// kernal invocation 
	dim3 threadPerBlock(500, 1, 1);
	dim3 numBlocks(N / threadPerBlock.x + 1, 1, 1);
	MatrixMultiply <<<numBlocks, threadPerBlock >>> (dev_a, dev_b, dev_c, N);
	if (cudaStatus != cudaSuccess) {
		printf("计算错误\n");
		goto Error;
	}
	cudaStatus = cudaMemcpy(c, dev_c, N * sizeof(double), cudaMemcpyDeviceToHost);
Error:
	cudaFree(dev_a);
	cudaFree(dev_b);
	cudaFree(dev_c);
	return cudaStatus;
}
