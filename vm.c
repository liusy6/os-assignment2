#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include<getopt.h>
//定义页表条目数、页面大小、TLB条目数、帧大小
#define PAGE_TABLE_ENTRIES 256
#define PAGE_SIZE  256
#define TLB_SIZE   16
#define FRAME_SIZE 256

//定义获取页码和偏移的掩码
#define PAGE_NUMBER_MASK 0x0000FFFF
#define OFFSET_MASK 0x000000FF

int main(int argc, char *argv[])
{
	int framenumber;//定义物理内存的帧数
	//定义两个字符数组，用于存储命令行获取的参数 
	char str[3];//存帧数 
	char str1[4];//存策略名 
	
	//通过getopt函数获取命令行传入的参数 
	int opt;
    char *string = "n:p:";
    while ((opt = getopt(argc, argv, string))!= -1)
    {  
        if (opt == 'n')
        	strcpy( str , optarg );
        else if (opt == 'p')
        	strcpy( str1 , optarg );
    }
	//通过传入的参数确定物理内存帧数framenumber  
	if (str[0] == '1')
		framenumber = 128;
	else
		framenumber = 256;
		
	int pm[framenumber*FRAME_SIZE];//定义物理内存数组
	int pt[PAGE_TABLE_ENTRIES];//定义页表数组 
	int TLBpn[TLB_SIZE];//定义TLB页号数组，存储页码值 
	int TLBfn[TLB_SIZE];//定义TLB帧数数组，存储页号数组中对应页号在物理内存中的帧数 
	
	unsigned int la;//定义逻辑地址 
	unsigned int pa;//定义物理地址
	unsigned int value;//定义储存在物理内存中获得的物理地址上的带符号字节值
	
	//定义由逻辑地址通过位掩码、位移动得到的页码和偏移
	unsigned int number;
	unsigned int offset;
	
	//定义缓冲部分，用于存储在后备文件中读出页的数据以更新物理内存 
	char buf[PAGE_SIZE];
	//定义时间数组，记录可用页码对应的最后一次使用时间（作为计时器LRU） 
	int time[PAGE_TABLE_ENTRIES];
	
	int j = 0;//不页面置换时作为存储的帧数，页面置换时作为标志 
	int i = 0;//作为循环使用的下标 
	int l = 0;//作为循环使用的下标
	int total = 0;//记录读取的地址数量、作为计时器存储的时间 
	int TLB_index = 0;//TLB两个数组的下标 
	int TLB_hits = 0;//记录TLB的命中数 
	int page_faults = 0;//记录缺页错误数量 
	
	//定义文件指针 
	FILE* addressFile;//储存逻辑地址的文件addresses.txt                          
    FILE* backingStore; //后备存储BACKING_STORE.bin
    //打开BACKING_STORE.bin，缺页时读取 
    backingStore = fopen("BACKING_STORE.bin", "rb");
    //打开addresses.txt，读取逻辑地址 
    addressFile = fopen("addresses.txt", "rb");
    //打开address_copy.txt，将读取到的逻辑地址写入该文件 
    FILE* address_copy = fopen("address_copy.txt", "w");
    //打开physical_address.txt，将通过地址转换后获取到的物理地址写入该文件
    FILE* physical_address = fopen("physical_address.txt","w");
    //打开value.txt，将储存在获得的物理地址上的带符号字节值写入该文件
    FILE* values = fopen("values.txt", "w");
    
    // 指针为空则文件打开失败 
    if(backingStore == NULL)
	{
        printf("Can't open BACKING_STORE.bin");
        return -1;
    }   
    if(addressFile == NULL)
	{
        printf("Can't open addresses.txt");
        return -1;
    }
    
    //对定义的数组进行初始化 
    for (i = 0;i < 256;i++)
    {
    	pt[i] = -1;
		time[i] = -1; 
	} 
	for (i = 0;i < 16;i++)
    {
    	TLBpn[i] = -1; 
    	TLBfn[i] = -1;
	} 
	
	//当从addresses.txt中还能读取到逻辑地址，继续操作
	while((fscanf(addressFile, "%d", &la)) == 1)
	{
		//读到的逻辑地址写入对应文件
		fprintf (address_copy, "%d\n",la); 
		//通过位掩码和位移动获取页码和偏移
		number = (la & PAGE_NUMBER_MASK) >> 8;
        offset = la & OFFSET_MASK;
        total++;//更新读取到的逻辑地址数量 
        int TLB_hit = -1;//设置判断TLB是否命中的标志
        int num = -1;//设置一个标志数组确定TLB命中时对应数组中的下标
		//遍历TLB表若命中则记录下标并计算物理地址 
        for (i = 0;i < 16;i++)
        {
        	if (TLBpn[i] == number)
        	{
        		TLB_hit = TLBfn[i];
        		pa = TLB_hit * 256 + offset;
        		num = i;
        		time[number] = total;
			}
		}
		//如果命中，TLB_hits加1
		if (TLB_hit != -1)
		{
			TLB_hits++;
			//当采用LRU策略时，命中时要更改两个TLB数组的顺序，保持对应 
			//使最近使用的页表始终在数组的最后 
			if (str1[0] == 'l')
			{
				for (i = num;i < TLB_index;i++)
            	{
            		TLBpn[i] = TLBpn[i+1];
            		TLBfn[i] = TLBfn[i+1];
				}
				TLBpn[TLB_index] = number;
				TLBfn[TLB_index] = TLB_hit;
			}
		}
		//如果物理内存中没有对应页码的数据，从后备存储文件中获取可用的页帧，存进物理内存
		else if (pt[number] == -1)
		{
			//在BACKING_STORE.bin中查找 
			fseek(backingStore, number*256, SEEK_SET);
			//查找完成后将数据存进buf 
            fread(buf, sizeof(char), 256, backingStore);
            //在物理页帧数为128时采用FIFO策略下，需要进行页面置换时的操作 
			if (str1[0] == 'f' && j > 127 && framenumber == 128)
			{
				//将所有物理内存有的页码对应的物理帧数减小1
				//最先使用的页对应的物理帧数原来为0，现在减为-1，将其换出 
            	for (i = 0; i < 256;i++)
            	{
            		if (pt[i] != -1)
            			pt[i] --;
				}
				//此时换进来的页码在物理内存中的帧数为最后一帧 
				pt[number] = 127;
				//将物理内存中未换出的页码对应的数据往前移一页的位置，以保持对应 
				for(l = 0; l < 127;l++)
					for(i = 0; i < 256; i++)
					{
                 		pm[l*256 + i] = pm[(l+1)*256 + i];
            		}
            	//将换进来的页对应的数据存入物理内存中最后一帧的位置 
            	for(i = 0; i < 256; i++)
				{
                 	pm[127*256 + i] = buf[i];
            	}
            	//更改TLB帧数数组中的值以保证与物理内存更新同步 
            	for (i = TLB_index;i >= 0;i--)
            	{
            		TLBfn[i]--;
				}
				//计算此时的物理地址 
            	pa = pt[number] * 256 + offset;
			}
			
			//在物理页帧数为128时采用LRU策略下，需要进行页面置换时的操作
			if (str1[0] == 'l' && j > 127 && framenumber == 128)
			{
            	int min = 1000;//设置一个最早使用时间标志 
            	int index = -1;//记录最早使用的页的页码
				//遍历获取最早使用的页的页码 
            	for (i = 0;i < 256;i++)
            	{
            		if (time[i] < min && time[i] != -1)
            		{
            			index = i;
            			min = time[i];
					}
				}
				//将此时换入的页的数据替换最早使用的页在物理内存中的数据 
            	for (i = 0;i < 256;i++)
            	{
            		pm[pt[index]*256 + i] = buf[i];
				}
				//存储此时换入页在物理内存中的帧数 
				pt[number] = pt[index];
				//将换出页在物理内存中的帧数设置为-1，标志为物理内存中无此页数据 
				pt[index] = -1;
				//计算物理地址 
            	pa = pt[number] * 256 + offset;
            	//更新计时数组 
            	time[number] = total;
            	//将换出的页标志为不可换出页 
            	time[index] = -1;
			}
			
			//在物理页帧数为256，物理内存足以存储所有页的数据时的操作
			//以及在物理页帧数为128，但当前的物理内存还可进行存储不需置换时的操作 
			if (framenumber == 256 || (framenumber == 128 && j <=127 ))
			{
				//用j表示页码存储在物理内存中的帧数 
				pt[number] = j;
				//将buf中缓存的数据填入物理内存中 
				for(i = 0; i < 256; i++)
				{
                 	pm[j*256 + i] = buf[i];
            	}
            	//计算物理地址 
            	pa = j * 256 + offset;
            	//更新计时数组 
            	time[number] = total;
			}
			
            //当TLB的两个数组未存满时，将页码及对应物理内存中的帧数按顺序存入两个数组 
            if (TLB_index < 15)
			{
				TLBpn[TLB_index] = number;
				TLBfn[TLB_index] = pt[number];
				TLB_index++;//更新TLB_index
			}
			// 当TLB的两个数组存满时，将数组内的值均前移一位，将新加进的值放在数组最后 
			else
			{
				for (i = 0; i < TLB_index;i++)
				{
					TLBpn[i] = TLBpn[i+1];
					TLBfn[i] = TLBfn[i+1];
				}
				TLBpn[TLB_index] = number;
				TLBfn[TLB_index] = pt[number];
			}
            //更新j、page_faults值 
            j++;
            page_faults++;
            //计算物理地址 
            pa = pt[number] *256 + offset;
		}
		//当TLB未命中，但此时物理内存中有对应的页码的数据时的操作 
		else
		{
			//更新TLB两个数组 
			if (TLB_index < 15)
			{
				TLBpn[TLB_index] = number;
				TLBfn[TLB_index] = pt[number];
				TLB_index++;
			}
			else
			{
				for (i = 0; i < TLB_index;i++)
				{
					TLBpn[i] = TLBpn[i+1];
					TLBfn[i] = TLBfn[i+1];
				}
				TLBpn[TLB_index] = number;
				TLBfn[TLB_index] = pt[number];
			}
			//计算物理地址 
			pa = pt[number] *256 + offset;
			//更新计时数组 
			time[number] = total;
		}
		//将计算到的物理地址存入physical_address文件 
		fprintf (physical_address, "%d\n",pa);
		//从物理内存中取出值 
		value = pm[pa];
		//打印值 
		printf("%d\n", value);
		//将值写入values文件中 
		fprintf(values, "%d\n", value);
	}
	//打印Page Faults、TLB Hits 
	printf("Page Faults = %d\n", page_faults);
	printf("TLB Hits = %d\n", TLB_hits);
	//计算缺页率、TLB命中率
	float page_fault_rate = ((float)page_faults / (float)total) * 100;
    //printf("Page-fault rate: %.2f%%\n", page_fault_rate);
    float TLB_hit_rate = ((float)TLB_hits / (float)total) * 100;
    //printf("TLB hit rate: %.2f%%\n", TLB_hit_rate);
}
