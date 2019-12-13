#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include<getopt.h>
//����ҳ����Ŀ����ҳ���С��TLB��Ŀ����֡��С
#define PAGE_TABLE_ENTRIES 256
#define PAGE_SIZE  256
#define TLB_SIZE   16
#define FRAME_SIZE 256

//�����ȡҳ���ƫ�Ƶ�����
#define PAGE_NUMBER_MASK 0x0000FFFF
#define OFFSET_MASK 0x000000FF

int main(int argc, char *argv[])
{
	int framenumber;//���������ڴ��֡��
	//���������ַ����飬���ڴ洢�����л�ȡ�Ĳ��� 
	char str[3];//��֡�� 
	char str1[4];//������� 
	
	//ͨ��getopt������ȡ�����д���Ĳ��� 
	int opt;
    char *string = "n:p:";
    while ((opt = getopt(argc, argv, string))!= -1)
    {  
        if (opt == 'n')
        	strcpy( str , optarg );
        else if (opt == 'p')
        	strcpy( str1 , optarg );
    }
	//ͨ������Ĳ���ȷ�������ڴ�֡��framenumber  
	if (str[0] == '1')
		framenumber = 128;
	else
		framenumber = 256;
		
	int pm[framenumber*FRAME_SIZE];//���������ڴ�����
	int pt[PAGE_TABLE_ENTRIES];//����ҳ������ 
	int TLBpn[TLB_SIZE];//����TLBҳ�����飬�洢ҳ��ֵ 
	int TLBfn[TLB_SIZE];//����TLB֡�����飬�洢ҳ�������ж�Ӧҳ���������ڴ��е�֡�� 
	
	unsigned int la;//�����߼���ַ 
	unsigned int pa;//���������ַ
	unsigned int value;//���崢���������ڴ��л�õ������ַ�ϵĴ������ֽ�ֵ
	
	//�������߼���ַͨ��λ���롢λ�ƶ��õ���ҳ���ƫ��
	unsigned int number;
	unsigned int offset;
	
	//���建�岿�֣����ڴ洢�ں��ļ��ж���ҳ�������Ը��������ڴ� 
	char buf[PAGE_SIZE];
	//����ʱ�����飬��¼����ҳ���Ӧ�����һ��ʹ��ʱ�䣨��Ϊ��ʱ��LRU�� 
	int time[PAGE_TABLE_ENTRIES];
	
	int j = 0;//��ҳ���û�ʱ��Ϊ�洢��֡����ҳ���û�ʱ��Ϊ��־ 
	int i = 0;//��Ϊѭ��ʹ�õ��±� 
	int l = 0;//��Ϊѭ��ʹ�õ��±�
	int total = 0;//��¼��ȡ�ĵ�ַ��������Ϊ��ʱ���洢��ʱ�� 
	int TLB_index = 0;//TLB����������±� 
	int TLB_hits = 0;//��¼TLB�������� 
	int page_faults = 0;//��¼ȱҳ�������� 
	
	//�����ļ�ָ�� 
	FILE* addressFile;//�����߼���ַ���ļ�addresses.txt                          
    FILE* backingStore; //�󱸴洢BACKING_STORE.bin
    //��BACKING_STORE.bin��ȱҳʱ��ȡ 
    backingStore = fopen("BACKING_STORE.bin", "rb");
    //��addresses.txt����ȡ�߼���ַ 
    addressFile = fopen("addresses.txt", "rb");
    //��address_copy.txt������ȡ�����߼���ַд����ļ� 
    FILE* address_copy = fopen("address_copy.txt", "w");
    //��physical_address.txt����ͨ����ַת�����ȡ���������ַд����ļ�
    FILE* physical_address = fopen("physical_address.txt","w");
    //��value.txt���������ڻ�õ������ַ�ϵĴ������ֽ�ֵд����ļ�
    FILE* values = fopen("values.txt", "w");
    
    // ָ��Ϊ�����ļ���ʧ�� 
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
    
    //�Զ����������г�ʼ�� 
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
	
	//����addresses.txt�л��ܶ�ȡ���߼���ַ����������
	while((fscanf(addressFile, "%d", &la)) == 1)
	{
		//�������߼���ַд���Ӧ�ļ�
		fprintf (address_copy, "%d\n",la); 
		//ͨ��λ�����λ�ƶ���ȡҳ���ƫ��
		number = (la & PAGE_NUMBER_MASK) >> 8;
        offset = la & OFFSET_MASK;
        total++;//���¶�ȡ�����߼���ַ���� 
        int TLB_hit = -1;//�����ж�TLB�Ƿ����еı�־
        int num = -1;//����һ����־����ȷ��TLB����ʱ��Ӧ�����е��±�
		//����TLB�����������¼�±겢���������ַ 
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
		//������У�TLB_hits��1
		if (TLB_hit != -1)
		{
			TLB_hits++;
			//������LRU����ʱ������ʱҪ��������TLB�����˳�򣬱��ֶ�Ӧ 
			//ʹ���ʹ�õ�ҳ��ʼ������������ 
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
		//��������ڴ���û�ж�Ӧҳ������ݣ��Ӻ󱸴洢�ļ��л�ȡ���õ�ҳ֡����������ڴ�
		else if (pt[number] == -1)
		{
			//��BACKING_STORE.bin�в��� 
			fseek(backingStore, number*256, SEEK_SET);
			//������ɺ����ݴ��buf 
            fread(buf, sizeof(char), 256, backingStore);
            //������ҳ֡��Ϊ128ʱ����FIFO�����£���Ҫ����ҳ���û�ʱ�Ĳ��� 
			if (str1[0] == 'f' && j > 127 && framenumber == 128)
			{
				//�����������ڴ��е�ҳ���Ӧ������֡����С1
				//����ʹ�õ�ҳ��Ӧ������֡��ԭ��Ϊ0�����ڼ�Ϊ-1�����任�� 
            	for (i = 0; i < 256;i++)
            	{
            		if (pt[i] != -1)
            			pt[i] --;
				}
				//��ʱ��������ҳ���������ڴ��е�֡��Ϊ���һ֡ 
				pt[number] = 127;
				//�������ڴ���δ������ҳ���Ӧ��������ǰ��һҳ��λ�ã��Ա��ֶ�Ӧ 
				for(l = 0; l < 127;l++)
					for(i = 0; i < 256; i++)
					{
                 		pm[l*256 + i] = pm[(l+1)*256 + i];
            		}
            	//����������ҳ��Ӧ�����ݴ��������ڴ������һ֡��λ�� 
            	for(i = 0; i < 256; i++)
				{
                 	pm[127*256 + i] = buf[i];
            	}
            	//����TLB֡�������е�ֵ�Ա�֤�������ڴ����ͬ�� 
            	for (i = TLB_index;i >= 0;i--)
            	{
            		TLBfn[i]--;
				}
				//�����ʱ�������ַ 
            	pa = pt[number] * 256 + offset;
			}
			
			//������ҳ֡��Ϊ128ʱ����LRU�����£���Ҫ����ҳ���û�ʱ�Ĳ���
			if (str1[0] == 'l' && j > 127 && framenumber == 128)
			{
            	int min = 1000;//����һ������ʹ��ʱ���־ 
            	int index = -1;//��¼����ʹ�õ�ҳ��ҳ��
				//������ȡ����ʹ�õ�ҳ��ҳ�� 
            	for (i = 0;i < 256;i++)
            	{
            		if (time[i] < min && time[i] != -1)
            		{
            			index = i;
            			min = time[i];
					}
				}
				//����ʱ�����ҳ�������滻����ʹ�õ�ҳ�������ڴ��е����� 
            	for (i = 0;i < 256;i++)
            	{
            		pm[pt[index]*256 + i] = buf[i];
				}
				//�洢��ʱ����ҳ�������ڴ��е�֡�� 
				pt[number] = pt[index];
				//������ҳ�������ڴ��е�֡������Ϊ-1����־Ϊ�����ڴ����޴�ҳ���� 
				pt[index] = -1;
				//���������ַ 
            	pa = pt[number] * 256 + offset;
            	//���¼�ʱ���� 
            	time[number] = total;
            	//��������ҳ��־Ϊ���ɻ���ҳ 
            	time[index] = -1;
			}
			
			//������ҳ֡��Ϊ256�������ڴ����Դ洢����ҳ������ʱ�Ĳ���
			//�Լ�������ҳ֡��Ϊ128������ǰ�������ڴ滹�ɽ��д洢�����û�ʱ�Ĳ��� 
			if (framenumber == 256 || (framenumber == 128 && j <=127 ))
			{
				//��j��ʾҳ��洢�������ڴ��е�֡�� 
				pt[number] = j;
				//��buf�л�����������������ڴ��� 
				for(i = 0; i < 256; i++)
				{
                 	pm[j*256 + i] = buf[i];
            	}
            	//���������ַ 
            	pa = j * 256 + offset;
            	//���¼�ʱ���� 
            	time[number] = total;
			}
			
            //��TLB����������δ����ʱ����ҳ�뼰��Ӧ�����ڴ��е�֡����˳������������� 
            if (TLB_index < 15)
			{
				TLBpn[TLB_index] = number;
				TLBfn[TLB_index] = pt[number];
				TLB_index++;//����TLB_index
			}
			// ��TLB�������������ʱ���������ڵ�ֵ��ǰ��һλ�����¼ӽ���ֵ����������� 
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
            //����j��page_faultsֵ 
            j++;
            page_faults++;
            //���������ַ 
            pa = pt[number] *256 + offset;
		}
		//��TLBδ���У�����ʱ�����ڴ����ж�Ӧ��ҳ�������ʱ�Ĳ��� 
		else
		{
			//����TLB�������� 
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
			//���������ַ 
			pa = pt[number] *256 + offset;
			//���¼�ʱ���� 
			time[number] = total;
		}
		//�����㵽�������ַ����physical_address�ļ� 
		fprintf (physical_address, "%d\n",pa);
		//�������ڴ���ȡ��ֵ 
		value = pm[pa];
		//��ӡֵ 
		printf("%d\n", value);
		//��ֵд��values�ļ��� 
		fprintf(values, "%d\n", value);
	}
	//��ӡPage Faults��TLB Hits 
	printf("Page Faults = %d\n", page_faults);
	printf("TLB Hits = %d\n", TLB_hits);
	//����ȱҳ�ʡ�TLB������
	float page_fault_rate = ((float)page_faults / (float)total) * 100;
    //printf("Page-fault rate: %.2f%%\n", page_fault_rate);
    float TLB_hit_rate = ((float)TLB_hits / (float)total) * 100;
    //printf("TLB hit rate: %.2f%%\n", TLB_hit_rate);
}
