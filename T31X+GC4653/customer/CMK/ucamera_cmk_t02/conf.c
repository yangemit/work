#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include "conf.h"

 

typedef struct item_t {
    char *key;
    char *value;
}ITEM;

/*
 *ȥ���ַ����Ҷ˿ո�
 */
char *strtrimr(char *pstr)
{
    int i;
    i = strlen(pstr) - 1;
    while (isspace(pstr[i]) && (i >= 0))
        pstr[i--] = '\0';
    return pstr;
}
/*
 *ȥ���ַ�����˿ո�
 */
char *strtriml(char *pstr)
{
    int i = 0,j;
    j = strlen(pstr) - 1;
    while (isspace(pstr[i]) && (i <= j))
        i++;
    if (0<i)
        strcpy(pstr, &pstr[i]);
    return pstr;
}
/*
 *ȥ���ַ������˿ո�
 */
char *strtrim(char *pstr)
{
    char *p;
    p = strtrimr(pstr);
    return strtriml(p);
}


/*
 *�������ļ���һ�ж���key��value,����itemָ��
 *line--�������ļ�������һ��
 */
int  get_item_from_line(char *line,  ITEM *item)
{
    char *p = strtrim(line);
    int len = strlen(p);
		printf("string length=%d,%s\n",len,p);
    if(len <= 0){
			printf("no char in line\n");
        return 1;//����
    }
    else if(p[0]=='#'){
        return 2;
    }else{
        char *p2 = strchr(p, '=');
        *p2++ = '\0';
        item->key = (char *)malloc(strlen(p) + 1);		
			//	printf("p1=%s\n",p);

        item->value = (char *)malloc(strlen(p2) + 1);
				//	printf("p2=%s\n",p2);
        strcpy(item->key,p);
        strcpy(item->value,p2);		
		

        }
    return 0;//��ѯ�ɹ�
}

int file_to_items(const char *file,  ITEM *items,  int *num)
{
    char line[1024];
    FILE *fp;
    fp = fopen(file,"r");
    if(fp == NULL)
    	{
    	
        return 1;
    	}
    int i = 0;
    while(fgets(line, 1023, fp)){
            char *p = strtrim(line);
        int len = strlen(p);
        if(len <= 0){
            continue;
        }
        else if(p[0]=='#'){
            continue;
        }else{
            char *p2 = strchr(p, '=');
            /*������Ϊֻ��keyûʲô����*/
            if(p2 == NULL)
                continue;
            *p2++ = '\0';
            items[i].key = (char *)malloc(strlen(p) + 1);
            items[i].value = (char *)malloc(strlen(p2) + 1);
            strcpy(items[i].key,p);
            strcpy(items[i].value,p2);

            i++;
        }
    }
    (*num) = i;
    fclose(fp);
    return 0;
}

/*
 *��ȡvalue
 */
int read_conf_value(const char *key, char *value,const char *file)
{
    char line[1024];
    FILE *fp;
    fp = fopen(file,"r");
    if(fp == NULL)
    	{
    	printf("open config file%s failed\n",file);
        return 1;//�ļ��򿪴���
    	}
    while (fgets(line, 1023, fp)){
        ITEM item;			
		//		printf("line=%s\n",line);

        get_item_from_line(line,&item);
		//					printf("item->key=%s\n",item.key);
		//				printf("item->value=%s\n",item.value);
        if(strcmp(item.key,key)==0){
			printf("find the key\n");
            strcpy(value,item.value);

            fclose(fp);
            free(item.key);
            free(item.value);
            break;
        }

    }
    return 0;//�ɹ�

}
int write_conf_value(const char *key,char *value,const char *file)
{
    ITEM items[20];// �ٶ������������20��
    int num=0;//�洢���ļ���ȡ����Ч��Ŀ
    file_to_items(file, items, &num);

    int i=0;
 
    //����Ҫ�޸ĵ���
    for(i=0;i<num;i++){
        if(!strcmp(items[i].key, key)){
            items[i].value = value;
            break;
        }
    }
		//didn't find the key,create new key
    if(i>=num)
    	{	
			i=num;
			items[i].key=key;
			items[i].value = value;
			num++;  
		}
    // ���������ļ�,Ӧ���б��ݣ�����Ĳ����Ὣ�ļ��������
    FILE *fp;
    fp = fopen(file, "w");
    if(fp == NULL)
        return 1;

    i=0;
    for(i=0;i<num;i++){
        fprintf(fp,"%s=%s\n",items[i].key, items[i].value);
        //printf("%s=%s\n",items[i].key, items[i].value);
    }
    fclose(fp);
    //�������
/*    i=0;
    for(i=0;i<num;i++){
        free(items[i].key);
        free(items[i].value);
    }*/

    return 0;

}
