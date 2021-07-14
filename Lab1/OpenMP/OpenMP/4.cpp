/*#include <stdio.h>
#include <omp.h>
static long num_steps = 100000;
double step;
#define NUM_THREADS 2
void main()
{
	int i;
	double pi = 0.0;
	double sum = 0.0;
	double x = 0.0;
	step = 1.0 / (double)num_steps;
	omp_set_num_threads(NUM_THREADS); //����2�߳�
#pragma omp parallel for reduction(+:sum) private(x) //ÿ���̱߳���һ��˽�п���sum��xΪ�߳�˽�У������߳�������sum���� + ��Լ��������sum��ȫ��ֵ
		for (i = 1; i <= num_steps; i++) {
			x = (i - 0.5)*step;
			sum += 4.0 / (1.0 + x * x);
		}
	pi = sum * step;
	printf("%lf\n", pi);
	getchar();
} *///��2���̲߳μӼ��㣬�����߳�0���е�����0~49999���߳�1���е�����50000~99999