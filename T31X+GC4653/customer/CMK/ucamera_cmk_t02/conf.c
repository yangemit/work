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
 *去除字符串右端空格
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
 *去除字符串左端空格
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
 *去除字符串两端空格
 */
char *strtrim(char *pstr)
{
    char *p;
    p = strtrimr(pstr);
    return strtriml(p);
}


/*
 *从配置文件的一行读出key或value,返回item指针
 *line--从配置文件读出的一行
 */
int  get_item_from_line(char *line,  ITEM *item)
{
    char *p = strtrim(line);
    int len = strlen(p);
		printf("string length=%d,%s\n",len,p);
    if(len <= 0){
			printf("no char in line\n");
        return 1;//空行
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
    return 0;//查询成功
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
            /*这里认为只有key没什么意义*/
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
 *读取value
 */
int read_conf_value(const char *key, char *value,const char *file)
{
    char line[1024];
    FILE *fp;
    fp = fopen(file,"r");
    if(fp == NULL)
    	{
    	printf("open config file%s failed\n",file);
        return 1;//文件打开错误
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
    return 0;//成功

}
int write_conf_value(const char *key,char *value,const char *file)
{
    ITEM items[20];// 假定配置项最多有20个
    int num=0;//存储从文件读取的有效数目
    file_to_items(file, items, &num);

    int i=0;
 
    //查找要修改的项
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
    // 更新配置文件,应该有备份，下面的操作会将文件内容清除
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
    //清除工作
/*    i=0;
    for(i=0;i<num;i++){
        free(items[i].key);
        free(items[i].value);
    }*/

    return 0;

}
