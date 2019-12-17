#include <stdio.h>
#include <stdlib.h>

//����ͷ��βָ�� 
int* first = NULL;
int* last = NULL;

//�Ժ���������ǰ���壬���ں����໥���� 
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

//��ʼ��һ��ģ��Ķ�
//�൱���ڴ�Ĵ�С�����ɷ�������� 
void initmemory (int size) 
{
  int a = multi8( size + 12 );
  first = (int*) malloc (a * sizeof(int));
  last = first + a;
  *last = 0;
  first = first + 1;
  *first = a - 8;
}

//���䳤�ȣ�����ࣩ�ֽڵĿ�
void* myalloc (int length) 
{
  int* p = firstBlock ();
  int memor = multi8(length + 4);
  while ( p < lastBlock () ) 
  {
    if (isAllocated(p) == 1) //�ѱ����� 
	{ 
      p = nextBlock(p);
    } 
	else //Ѱ��һ��δ������ĵط� 
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
        *p = memor + 1; //�ѷ��� 
        int* q = nextBlock(p);
        *q = temp - memor; //δ���� 
        p = p + 1;
        break;
      }
      else {
        p = nextBlock(p);
        // û�пռ�
        if (*p == 0)  
		{ p = NULL; 
		break;
		} 
      }
    }
  }
  return p; 
}

//�ͷ��ѷ���Ŀ� 
void myfree (void* ptr) 
{
  ptr = (int *) ptr - 1;
  *((int *)ptr) = *((int *)ptr) & (-2);
}

//�����ѣ��ϲ�δ��������ڴ�
//�����ڴ���Ƭ�����µĵ�Ч 
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

//����ָ����ϵ�һ�����ָ�� 
int* firstBlock () 
{ 
	return first; 
}

//����ָ���ǿ��ָ��
int* lastBlock () 
{ 
	return last; 
}

//ȷ��λ��p�Ŀ��Ƿ񱻷��� 
int isAllocated(int* p) 
{ 
	return *p % 2; 
} 

//����ָ��p����Ŀ��ָ��
int* nextBlock(int* p) 
{ 
	return p + (*p / 4); 
}

//���ؾ���a�����8�ı��� 
int multi8 (int a) 
{
  if (a % 8 == 0) //���a�Ѿ���8�ı����򷵻�a 
  { 
  return a; 
  }
  else 
  { 
  return ((a/8)+1)*8; //���ؾ���a�����8�ı��� 
  }
}

//��ӡ��ʾ��ʱ�ڴ��Ƿ����ɹ� 
void printresult(int *p) 
{
	int* temp = firstBlock();
	int i = 0;
	//����ʧ����ָ��Ϊ�� 
	if (p == NULL)
		printf("allocation failed\n");
	else
		printallocation();
	printf("\n");
}

int main() 
{
	//��������int��ָ��
    int *p1, *p2;
    
    //��ʼ���Ѵ�С����ӡ��ʱ����� 
    printf("initial allocation\n");
    initmemory(56);
    //��ӡ��ʱ�ڴ������
    printallocation(); 
    
    //��myalloc���������СΪ20���ڴ� 
    printf("malloc 20\n");
    p1 = (int *)myalloc(20);
    //��ӡ��ʱ�ڴ������
	printresult(p1);
    
    //��myalloc���������СΪ10���ڴ�
    printf("malloc 10\n");
    p2 = (int *)myalloc(10);
    //��ӡ��ʱ�ڴ������
	printresult(p2);
    
    //��myfree�����ͷ�p1ָ�룬�ͷŴ�СΪ20 
    printf("free 20\n");
    myfree(p1); 
	p1 = NULL;
	//��ӡ��ʱ�ڴ������
	printallocation();
    
	//��myalloc���������СΪ4���ڴ�    
    printf("malloc 4\n");
    p1 = (int *)myalloc(4);
    //��ӡ��ʱ�ڴ������
	printresult(p1);

	//��myfree�����ͷ�p2ָ�룬�ͷŴ�СΪ10 
    printf("free 10\n");
    myfree(p2); 
	p2 = NULL;
	//��ӡ��ʱ�ڴ������
	printallocation();
    
    //��myalloc���������СΪ30���ڴ�
    printf("malloc 30\n");
    p2 = (int *)myalloc(30);
    //��ӡ��ʱ�ڴ������
	printresult(p2);
    
    //��δ��������ڴ���кϲ� 
    printf("coalesce\n");
    coalesce();
    //��ӡ��ʱ�ڴ������
    printallocation();
    
    //��myalloc���������СΪ30���ڴ�
    printf("malloc 30 \n");  
    p2 = (int *) myalloc(30);
    //��ӡ��ʱ�ڴ������
	printresult(p2);
    
    //�ͷ�����ָ�� 
    printf("free everything\n");
    myfree(p1); 
	p1 = NULL;
    myfree(p2); 
	p2 = NULL;
	//��ӡ��ʱ�ڴ������
    printallocation();
    
    //��myalloc���������СΪ56���ڴ�
    printf("malloc 56\n");
    p1 = (int *)myalloc(56);
    //��ӡ��ʱ�ڴ������
	printresult(p1);
    
    //��δ��������ڴ���кϲ�  
    printf("coalesce\n");
    coalesce();
    //��ӡ��ʱ�ڴ������
    printallocation();
    
    //��myalloc���������СΪ56���ڴ�
    printf("malloc 56\n");
    p1 = (int *)myalloc(56);
    //��ӡ��ʱ�ڴ������
	printresult(p1);
}
