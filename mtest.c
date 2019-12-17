#include <stdio.h>
#include <malloc.h> 
#include <unistd.h>
#include <alloca.h>

void afunc(void)
{
	static int level = 0; // ջ�Ĳ��� 
	auto int stack_var; // ջ�еı��� 

	if (++level == 3) // �����������������ջֻ������ 
		return;

	printf("\tStack level %d: address of stack_var: %p\n",level, & stack_var); // ��ӡջ�Ĳ�������Ӧջ�б����ĵ�ַ 
	afunc(); // �������� 
}

int bss_var; // ����BSS�����ݱ��� 
int data_var = 42; // �������ݱ�����ֵ 

int main(int argc, char **argv) 
{
char *p, *b, *nb; // �����ַ�ָ�� 

printf("Text Locations:\n"); // �ı�λ�� ������Σ� 
printf("\tAddress of main: %p\n", main); // main������ַ
printf("\tAddress of arguments : \n\targc: %p, argv: %p\n",&argc, argv); // main����������ַ
printf("\tAddress of afunc: %p\n", afunc); // afunc������ַ

printf("Stack Locations:\n"); // ջλ�� 
afunc(); //ʹ�õݹ麯������ջ������ 

p = (char *) alloca(32); //ʹ��alloca������ջ������ռ� 
if (p != NULL) 
{
	printf("\tStart of alloca()'ed array: %p\n", p); // ��ӡalloca�������ʼ��ַ 
	printf("\tEnd of alloca()'ed array: %p\n", p + 31); // ��ӡalloca����Ľ�����ַ
}
printf("Data Locations:\n"); // ����λ�� 
printf("\tAddress of data_var: %p\n", & data_var); // ��ӡ���ݱ����ĵ�ַ 

printf("BSS Locations:\n"); // BSSλ�� 
printf("\tAddress of bss_var: %p\n", & bss_var); // ��ӡBSS���ݱ����ĵ�ַ 

b = sbrk((ptrdiff_t) 32); // ����˳���ĵ�ַ�ռ� 
nb = sbrk((ptrdiff_t) 0); // �õ������˵ĵ�ַ�ռ�Ŀǰ�����ĵط� 
printf("Heap Locations:\n"); //�ѵ�λ�� 
printf("\tInitial end of heap: %p\n", b); // ��ӡ��ʼ�ĵ�ַ�ռ�����ĵط��ĵ�ַ 
printf("\tNew end of heap: %p\n", nb); // ��ӡ��һ������ĵ�ַ�ռ�����ĵط��ĵ�ַ 

b = sbrk((ptrdiff_t) -16); // ��С�˳���ĵ�ַ�ռ� 
nb = sbrk((ptrdiff_t) 0); // �õ���С�˵ĵ�ַ�ռ�Ŀǰ�����ĵط�
printf("\tAfter shrinking, the end of heap: %p\n", nb); // ��ӡ��С��ĵ�ַ�ռ�����ĵط��ĵ�ַ 
b = sbrk((ptrdiff_t) +16); // �ٴ�����˳���ĵ�ַ�ռ� 
nb = sbrk((ptrdiff_t) 0); // �õ���ʱ�ĵ�ַ�ռ�Ŀǰ�����ĵط� 
printf("\tFinal end of heap: %p\n", nb); // ��ӡ�ٴ������ĵ�ַ�ռ�����ĵط��ĵ�ַ
sleep(10000); // ��֤���򲻻����̽��� 
}
