#include <stdio.h>
#include <stdlib.h>

//定义头、尾指针 
int* first = NULL;
int* last = NULL;

//对函数进行提前定义，用于函数相互引用 
void  initmemory(int size);
void *myalloc(int length); 
void  myfree(void *ptr);     
void  coalesce();            
void  printallocation();  
int  isAllocated(int *p);  
int *nextBlock(int *p);    
int *firstBlock();       
int *lastBlock();          
int multi8 (int a);        

//初始化一个模拟的堆
//相当于内存的大小，最多可分配的数量 
void initmemory (int size) 
{
  int a = multi8( size + 12 );
  first = (int*) malloc (a * sizeof(int));
  last = first + a;
  *last = 0;
  first = first + 1;
  *first = a - 8;
}

//分配长度（或更多）字节的块
void* myalloc (int length) 
{
  int* p = firstBlock ();
  int memor = multi8(length + 4);
  while ( p < lastBlock () ) 
  {
    if (isAllocated(p) == 1) //已被分配 
	{ 
      p = nextBlock(p);
    } 
	else //寻找一块未被分配的地方 
	{ 
      if (*p == memor) 
	  {
        *p = memor + 1;
        p = p + 1;
        break;
      }
      if (*p > memor) 
	  {
        int temp = *p;
        *p = memor + 1; //已分配 
        int* q = nextBlock(p);
        *q = temp - memor; //未分配 
        p = p + 1;
        break;
      }
      else {
        p = nextBlock(p);
        // 没有空间
        if (*p == 0)  
		{ p = NULL; 
		break;
		} 
      }
    }
  }
  return p; 
}

//释放已分配的块 
void myfree (void* ptr) 
{
  ptr = (int *) ptr - 1;
  *((int *)ptr) = *((int *)ptr) & (-2);
}

//遍历堆，合并未被分配的内存
//减轻内存碎片化导致的低效 
void coalesce() {

  int* temp = firstBlock();

  while ( *nextBlock(temp) != 0 ) 
  {
    if (isAllocated(temp) == 0 && isAllocated(nextBlock(temp)) == 0) 
	{
      *temp = *temp + *nextBlock(temp);
    }
    else 
	{
	 temp = nextBlock(temp); 
	}
 }

}

void printallocation () {
  int* temp = firstBlock();
  int i = 0;
  while ( *temp != 0 ) {
    if (isAllocated(temp) == 1) {
      printf("Block %d: size %d   allocated\n", i, *temp -1 );
    } else {
      printf("Block %d: size %d   unallocated\n", i, *temp );
    }
    i ++;
    temp = nextBlock(temp);
  }
}

//返回指向堆上第一个块的指针 
int* firstBlock () 
{ 
	return first; 
}

//返回指向标记块的指针
int* lastBlock () 
{ 
	return last; 
}

//确定位置p的块是否被分配 
int isAllocated(int* p) 
{ 
	return *p % 2; 
} 

//返回指向p后面的块的指针
int* nextBlock(int* p) 
{ 
	return p + (*p / 4); 
}

//返回距离a最近的8的倍数 
int multi8 (int a) 
{
  if (a % 8 == 0) //如果a已经是8的倍数则返回a 
  { 
  return a; 
  }
  else 
  { 
  return ((a/8)+1)*8; //返回距离a最近的8的倍数 
  }
}

//打印显示此时内存是否分配成功 
void printresult(int *p) 
{
	int* temp = firstBlock();
	int i = 0;
	//分配失败则指针为空 
	if (p == NULL)
		printf("allocation failed\n");
	else
		printallocation();
	printf("\n");
}

int main() 
{
	//定义两个int型指针
    int *p1, *p2;
    
    //初始化堆大小，打印此时的情况 
    printf("initial allocation\n");
    initmemory(56);
    //打印此时内存分配结果
    printallocation(); 
    
    //用myalloc函数申请大小为20的内存 
    printf("malloc 20\n");
    p1 = (int *)myalloc(20);
    //打印此时内存分配结果
	printresult(p1);
    
    //用myalloc函数申请大小为10的内存
    printf("malloc 10\n");
    p2 = (int *)myalloc(10);
    //打印此时内存分配结果
	printresult(p2);
    
    //用myfree函数释放p1指针，释放大小为20 
    printf("free 20\n");
    myfree(p1); 
	p1 = NULL;
	//打印此时内存分配结果
	printallocation();
    
	//用myalloc函数申请大小为4的内存    
    printf("malloc 4\n");
    p1 = (int *)myalloc(4);
    //打印此时内存分配结果
	printresult(p1);

	//用myfree函数释放p2指针，释放大小为10 
    printf("free 10\n");
    myfree(p2); 
	p2 = NULL;
	//打印此时内存分配结果
	printallocation();
    
    //用myalloc函数申请大小为30的内存
    printf("malloc 30\n");
    p2 = (int *)myalloc(30);
    //打印此时内存分配结果
	printresult(p2);
    
    //将未被分配的内存进行合并 
    printf("coalesce\n");
    coalesce();
    //打印此时内存分配结果
    printallocation();
    
    //用myalloc函数申请大小为30的内存
    printf("malloc 30 \n");  
    p2 = (int *) myalloc(30);
    //打印此时内存分配结果
	printresult(p2);
    
    //释放两个指针 
    printf("free everything\n");
    myfree(p1); 
	p1 = NULL;
    myfree(p2); 
	p2 = NULL;
	//打印此时内存分配结果
    printallocation();
    
    //用myalloc函数申请大小为56的内存
    printf("malloc 56\n");
    p1 = (int *)myalloc(56);
    //打印此时内存分配结果
	printresult(p1);
    
    //将未被分配的内存进行合并  
    printf("coalesce\n");
    coalesce();
    //打印此时内存分配结果
    printallocation();
    
    //用myalloc函数申请大小为56的内存
    printf("malloc 56\n");
    p1 = (int *)myalloc(56);
    //打印此时内存分配结果
	printresult(p1);
}
