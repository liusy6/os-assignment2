#include <stdio.h>
#include <malloc.h> 
#include <unistd.h>
#include <alloca.h>

void afunc(void)
{
	static int level = 0; // 栈的层数 
	auto int stack_var; // 栈中的变量 

	if (++level == 3) // 避免无穷遍历，控制栈只有两层 
		return;

	printf("\tStack level %d: address of stack_var: %p\n",level, & stack_var); // 打印栈的层数及对应栈中变量的地址 
	afunc(); // 遍历操作 
}

int bss_var; // 定义BSS的数据变量 
int data_var = 42; // 定义数据变量的值 

int main(int argc, char **argv) 
{
char *p, *b, *nb; // 定义字符指针 

printf("Text Locations:\n"); // 文本位置 （代码段） 
printf("\tAddress of main: %p\n", main); // main函数地址
printf("\tAddress of arguments : \n\targc: %p, argv: %p\n",&argc, argv); // main函数参数地址
printf("\tAddress of afunc: %p\n", afunc); // afunc函数地址

printf("Stack Locations:\n"); // 栈位置 
afunc(); //使用递归函数表现栈的增长 

p = (char *) alloca(32); //使用alloca函数在栈上申请空间 
if (p != NULL) 
{
	printf("\tStart of alloca()'ed array: %p\n", p); // 打印alloca数组的起始地址 
	printf("\tEnd of alloca()'ed array: %p\n", p + 31); // 打印alloca数组的结束地址
}
printf("Data Locations:\n"); // 数据位置 
printf("\tAddress of data_var: %p\n", & data_var); // 打印数据变量的地址 

printf("BSS Locations:\n"); // BSS位置 
printf("\tAddress of bss_var: %p\n", & bss_var); // 打印BSS数据变量的地址 

b = sbrk((ptrdiff_t) 32); // 扩大此程序的地址空间 
nb = sbrk((ptrdiff_t) 0); // 得到扩大了的地址空间目前结束的地方 
printf("Heap Locations:\n"); //堆的位置 
printf("\tInitial end of heap: %p\n", b); // 打印初始的地址空间结束的地方的地址 
printf("\tNew end of heap: %p\n", nb); // 打印第一次扩大的地址空间结束的地方的地址 

b = sbrk((ptrdiff_t) -16); // 缩小此程序的地址空间 
nb = sbrk((ptrdiff_t) 0); // 得到缩小了的地址空间目前结束的地方
printf("\tAfter shrinking, the end of heap: %p\n", nb); // 打印缩小后的地址空间结束的地方的地址 
b = sbrk((ptrdiff_t) +16); // 再次扩大此程序的地址空间 
nb = sbrk((ptrdiff_t) 0); // 得到此时的地址空间目前结束的地方 
printf("\tFinal end of heap: %p\n", nb); // 打印再次扩大后的地址空间结束的地方的地址
sleep(10000); // 保证程序不会立刻结束 
}
