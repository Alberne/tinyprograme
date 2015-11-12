
#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct dormitory {
        char dormID[10];     /*dormitory ID*/
        char StudentID[10];  /*student ID*/
        char Name[10];       /*student name*/
        char Date[15];       /*check in time*/
} *pdormitory;

typedef struct dormitory_node {//dormitory 链表结构体   
        pdormitory data;
		char dvalid;          //1:数据有效 0：数据无效
        struct dormitory_node *next;
} *pdormitory_node;

typedef struct harrayinfo {
		pdormitory_node headary; //数据节点的链表指针
		int count;            //链表的节点个数
		char update;          //1：链表信息更新， 0：没更新
} *pharrayinfo;

/*比较器函数指针变量*/
typedef int(*compar)(const void *, const void *);
#define PFseparator  printf("--------------------------------------\n\n")
#define PFTitle   do{\
        PFseparator;       \
        printf(" Dormitory ID | Student ID | Name | Date\n"); \
}while(0) /*no trailing ;*/

#define CLESCREEN 
#define PT(s) printf(s)
#define MENUS  10    /*该系统的菜单个数*/
#define DATELEN  15   /*日期格式："2015-12-30"*/
#define DATA  "dormitory.dat"   /*数据存储文件*/
#define DEBUG  1  //1: debug mode 0: normal mode
#define MEMBLOCK   10   //内存块个数，内存分配策略，已块为单位分配
#define N 64       //缓冲区大小，用于存储一个dormitory结构体数据


#define PFVAR_INT(varg) fprintf(stderr,#varg"....%d\n",varg);

/*内存分配器管理
* p：必须是指针，分配完后指向内存地址
* c：内存块的个数
* type:p的指针类型
* msg:分配失败时，提示信息
**/
#define PNEW(p,c,msg,type) do{\
		p = (type)malloc(c * sizeof(*(p)));\
		if(!p)\
			assert(!(msg));\
		printf("sizeof(*p)=%d,sizeof(p)=%d,length=%d\n",sizeof(*p),sizeof(p),c);\
}while(0) /*no trailing ;*/

/*达到屏幕停顿效果*/
#define PAUSERUN  do{\
	for (; getchar() != '\n'; )\
			;\
	PT("please input Enter key to continue");\
	getchar();\
}while(0)

/*ary 必须是数组类型*/
#define GET_ARR_LEN(ary,len) (len = sizeof(ary)/sizeof(ary[0]))  
#define FILED_OFFSET(t, f)  (int)(&((t*)0)->f))
void readfile(void);
void add(void);
void viewall(void);
void editdorm(void);
void removeone(void);
void savefile(void);
void exitsys(void);

pdormitory_node *sortbydorid(unsigned int *len);
pdormitory_node *sortbystdid(unsigned int *len);
pdormitory_node *sortbynameid(unsigned int *len);

/**以下是三个查找函数，基于二分法查找**/
void findbydorid(void);
void findbystdid(void);
void findbynameid(void);

char *getcurdate(); //获得当前日期
void print_usage();

/**二分法查找函数 *
* base: 查找数组(已有序)
* val: 查找值
* pstart：已匹配元素的相对位移
* cmp:数组元素之间的比较器
* 返回值：匹配元素的个数*/
int binary_search(void **base, void *val, void *pstart, int n, compar cmp);

//插入排序
void insert_sort(void **base, int len, compar cmp);
void insert(pdormitory_node *parry,int pos, compar cmp);

//冒泡排序
void bubble_sort(void **base, int len, compar cmp);
	
/*compare rule: 0: equal to; less than 0: less than;
		more than 0: more than;*/
int compar_stuid(const void*, const void*);
int compar_dormid(const void*, const void*);
int compar_nameid(const void*, const void*);

/*print struct dormitory */
void print_one_dormitory(pdormitory p);
void print_all_dormitory(pdormitory_node p);
int  create_one_array(pdormitory_node **);

void init_args(); //程序运行前 初始化参数
pdormitory_node *search_account(unsigned int *len, int *pstart);

/**
*create a pdormitory,if argument is not null 
*and new on assign to argument
**/
pdormitory create(pdormitory*);

//全局变量声明
pdormitory_node pheadarry;  // the array head pointer of all record   
static char membufs[512];  //approximately 10 dormitory struct size
static struct harrayinfo arrayinfo;

int main(int argc, char* argv[])
{
		int menuID = 0;
		static void (*fmenuarry[]) (void) = { 
			 /* 0 */   exitsys, /* exit system */
             /* 1 */   readfile,  /* read from local file */
             /* 2 */   add,  /* add record */
             /* 3 */   viewall, /* view all record */
             /* 4 */   editdorm, /* edit specifical record */
             /* 5 */   removeone, /* delete specific record */
             /* 6 */   findbydorid,
             /* 7 */   findbystdid,
             /* 8 */   findbynameid,
             /* 9 */   savefile   /* save data to file */
		}; /*array index must match menu ID index*/

		printf("<%s> complie to %s\n\n",__FILE__, __DATE__);
		init_args(); //全局变量初始化

		do{
				CLESCREEN;
				PT("\n\n----------------- Menu  -------------------\n\n");
				PFseparator;
				PT("1. load data from file;\n");
				PT("2. add record;\n");
				PT("3. view all record\n");
				PT("4. Edit record;\n");
				PT("5. Delete record;\n");
				PT("6. Find record by dormitory ID;\n");
				PT("7. Find record by student ID\n");
				PT("8. Find record by name;\n");
				PT("9. save data to file;\n");
				PT("0. Exit system\n");
				PFseparator;
				PT("please you make choice menu ID:");
				scanf("%d",&menuID);
				assert(!isdigit(MENUS));
				if(menuID > (MENUS-1) || menuID < 0) {  
						PT("are you kid me? please input above Menu ID\n");
						continue;    // return main menu
				}
				fmenuarry[menuID]();  //call corresponds to function 
		 }while(menuID != 0);

		return 0;
}

void init_args()
{
		arrayinfo.count = 0;
		arrayinfo.headary = NULL;
		arrayinfo.update = 0;

		pheadarry = NULL;
		memset(membufs, 0, sizeof(membufs));

}
int compar_stuid(const void *s1, const void *s2) 
{
		pdormitory_node tmp1;
		pdormitory tmp2;
		tmp1 = (pdormitory_node)s1;
		tmp2 = (pdormitory)s2;
		return strcmp(tmp1->data->StudentID, tmp2->StudentID); 
}

int compar_dormid(const void *s1, const void *s2)
{
		pdormitory_node tmp1;
		pdormitory tmp2;
		tmp1 = (pdormitory_node)s1;
		tmp2 = (pdormitory)s2;
		return strcmp(tmp1->data->dormID, tmp2->dormID);
}

int compar_nameid(const void *s1, const void *s2)
{
		pdormitory_node tmp1;
		pdormitory tmp2;
		tmp1 = (pdormitory_node)s1;
		tmp2 = (pdormitory)s2;
		return strcmp(tmp1->data->Name, tmp2->Name);
}

int create_one_array(pdormitory_node **pdor)
{
		int s_arrys = 0;
		pdormitory_node tmp;
		pdormitory_node *p;
	
		tmp = pheadarry;
		if(!tmp) {
				*pdor = NULL;
				return 0;
		}
		
		#if DEBUG
		PFVAR_INT(s_arrys);
		#endif

		for(; tmp; tmp=tmp->next) 
				if(tmp->dvalid)	 //有效数据
						s_arrys++; /*how many nodes*/

		if(s_arrys != arrayinfo.count)//账户总数全局变量，与当前链表中的实际值不符合
				PT("<<fatal warrning>> arrayinfo.count isn't match nodes\n\n");

		PNEW(p, s_arrys, "create array failed\n", pdormitory_node*);
		#if DEBUG
		PT("malloc array by sort\n");
		PFVAR_INT(s_arrys);
		#endif

		*pdor = p;
		return s_arrys;
}


/**
* insert a new record into array
**/
void add()
{
		pdormitory pnewdorm;
		pdormitory_node pnode;
		create(&pnewdorm);  //create dormitory struct
	
		PNEW(pnode,1, "malloc failed",pdormitory_node); //create dormitory node 
		pnode->data = pnewdorm;
		pnode->dvalid = 1;
		arrayinfo.count++;  //更新链表节点计数

		pnode->next = pheadarry;  // inset into array on head
		pheadarry = pnode;
		PAUSERUN;
		PT("add completely ,press Enter to continue\n");  
}

 pdormitory create(pdormitory *p)
 {
		pdormitory pdorm = NULL;
        char sc[4];
		PNEW(pdorm, 1, "malloc a new struct failed",pdormitory);
		memset(pdorm, 0, sizeof(*pdorm));
        PT("please input a studen ID=");
		scanf("%s",pdorm->StudentID);
		PT("please input name=");
		scanf("%s",pdorm->Name);
		PT("please input dormitory ID=");
		scanf("%s",pdorm->dormID);
		PT("today as check-in time (YES/N0)?");
        memset(sc, 0, sizeof sc);
        scanf("%s",sc);
		if(!strncmp(sc, "YES", 3) || !strncmp(sc, "yes", 3)){
				strcpy(pdorm->Date, getcurdate());
		}else {
				PT("please input check-in time(example:2015-12-30)=");
				scanf("%s",pdorm->Date);
        }
 
		if(p != NULL) 
  		*p = pdorm;
 
		return pdorm;
}

/**
* get current calendar time and convert to character sequences
**/
char *getcurdate() 
{
		time_t t = NULL;
		struct tm *tmp= NULL;
		char *pdate = NULL;
		PNEW(pdate,DATELEN, "malloc date memory failed",char*);
		memset(pdate,'0',DATELEN);
		t = time(NULL); //get calendar time
		tmp = localtime(&t);
		assert(tmp);
		if(0 == strftime(pdate, DATELEN, "%Y-%m-%d", tmp))
				assert(!"calendar convert to character sequences faild\n");		
		return pdate; //only sucess can return pdate
}


	
void readfile()
{
		FILE *fp;
		int i, cunt;
		int offset;
		char *pmembufs;
		pmembufs = membufs;
		offset = sizeof(struct dormitory); 
		cunt = 0;
		pdormitory_node pdor_node = NULL;
		pdormitory pdor = NULL;
		arrayinfo.count = 0;
		arrayinfo.headary = NULL;
		if(pheadarry) {
				PT("data always exsit\n");
				return;
		}
				
		fp = fopen(DATA, "r");
		if(fp == NULL) {
				PT("file no exsit\n\n");
				return;
		}	
			
		do {
				memset(membufs, '0', sizeof(membufs));
				/** trick: 为了提升数据读取性能，一次读取多个dormitory节点数据
				*暂时存入到全局缓冲区membufs中*/
				cunt = fread(membufs, sizeof(struct dormitory), MEMBLOCK, fp); 	   
				
				#if DEBUG
				printf("buffer size=%d malloc size=%d\n\n", sizeof(membufs),\
								sizeof(struct dormitory)*MEMBLOCK);
				puts("buffer data >>:\n\n");
				fwrite(membufs,sizeof(*pdor),cunt,stdout);
				puts("\n\n");
				#endif	

     			if(ferror(fp)) {
						perror("read file occur failed");
  	 					return;
  				}
  	
   				/** trick: 为了提升内存分配效率，一次分配cunt个连续的节点内存*/
     			if(cunt != 0) { 
						i = 0 ;
						//pdor_node 指向cunt个连续的dormitory_node节点的内存
     					PNEW( pdor_node, cunt, "malloc failed",dormitory_node*);
						//pdor pointer 指向cunt个dormitory节点的内存
						PNEW( pdor, cunt, "malloc data mem failed",dormitory*); 
						
						//为分配的链表节点建立链表关系
						for(; i < cunt ; i++) {  	 

								#if DEBUG
								PT("----------link relation-----------\n\n");
								printf("pdor_node[%d]-->",i);
								#endif  	 
								pdor_node[i].dvalid = 1;
								pdor_node[i].next = &pdor_node[i+1];	 	  

								#if DEBUG
           						printf("pdor_node[%d]\n\n",i);
								#endif 	
  	
						}/*for*/
  	  
     					/** build up link relation with data elements node 
						*eg: pdor_node[i].data ---> &pdor[i]	**/ 
						for(i = 0; i<cunt; i++) {
        						memcpy(&pdor[i], pmembufs, offset);
    	 						pmembufs += offset;  // pointer to next one 
         						pdor_node[i].data = &pdor[i];  
    					}/*for*/
  	 
						/*the series node insert into array head   
						* pheadarry --> pdor_node(a serial of nodes)-- 
						*   ---->pheadarry (original ) **/ 
						
						pdor_node[cunt-1].next = pheadarry;  
  						pheadarry = pdor_node;
						arrayinfo.count += cunt; //记录节点个数
						cunt = 0;  //计数器清空，以便下次使用
				}/*if*/
		} while(!feof(fp));
	
		printf("read data completely! total data is:%d\n",arrayinfo.count);
		arrayinfo.headary = pheadarry;

		#if DEBUG
		PT("debug-mode: all data information below:\n");
		print_all_dormitory(pheadarry);	
		#endif
		PAUSERUN;
}
	

void exitsys()
{
		if(arrayinfo.update) { //退出系统前检查信息是否更新
				PT("data has update, system save information automatically\n");
				savefile();
		}

}

void viewall() 
{
		if(!pheadarry) {
				PT("No Data display\n");
				return ;
		}
		print_all_dormitory(pheadarry);
		PAUSERUN;
}

void editdorm()
{
		unsigned int findcunt; //匹配元素的个数
		int pindx;  //第一个匹配元素的位置
		char askflg[4];   
		pdormitory_node *pfindary;
		pdormitory_node *pary;
		pdormitory temp;
		pary = search_account(&findcunt, &pindx);

		if(findcunt == 0) {
				PT("No match data\n");
				PAUSERUN;
				goto out_edit;

		}

		/*在匹配的元素*/
		pfindary = &pary[pindx];
		printf("--has %d matched,information as below---\n\n",findcunt);
		while (findcunt > 0) {
				PFTitle;
				print_one_dormitory((*pfindary)->data);
				PT("do you want edit this account(yes/no)?");
				scanf("%s",askflg);
				if (strcmp(askflg, "yes") == 0) {//更新记录信息
						temp = (*pfindary)->data;	//链表节点指针

						PT("new student id=");
						scanf("%s",temp->StudentID);

						PT("new dormitory id=");
						scanf("%s",temp->dormID);

						PT("new name=");
						scanf("%s",temp->Name);

						PT("new check-in time=");
						scanf("%s",temp->Date);

						PT("-----updated information as below------\n");
						PFTitle;
						print_one_dormitory(temp);

						arrayinfo.update = 1;  //标记信息已被修改
				}/*if*/

				pfindary++;
				findcunt--;
		}/*while*/
		

out_edit:

		if(pary)
				free(pary);// free memeory

}

void removeone()
{
		unsigned int findcunt; //匹配元素的个数
		int pindx;  //第一个匹配元素的位置
		char askflg[4];   
		pdormitory_node *pfindary;
		pdormitory_node *pary;

		pary = search_account(&findcunt, &pindx);

		if(findcunt == 0) {
				PT("No match data\n");
				PAUSERUN;
				goto out_edit;

		}

		/*在匹配的元素*/
		pfindary = &pary[pindx]; //第一个匹配元素在数组中的位置
		printf("--has %d matched,information as below---\n\n",findcunt);
		while (findcunt > 0) {
				PFTitle;
				print_one_dormitory((*pfindary)->data);
				PT("do you want delete this account(yes/no)?");
				scanf("%s",askflg);
				if (strcmp(askflg, "yes") == 0) {//删除记录
						/*由于以块为单位分配内存，不是释放内存，暂用标记为标记*/
						(*pfindary)->dvalid = 0;
						arrayinfo.update = 1;  //标记信息已被修改
						arrayinfo.count--;  //更新有效数据总数
						PT("delete completely!\n");
				}/*if*/
				pfindary++;
				findcunt--;
		}/*while*/
		

out_edit:

		if(pary)
				free(pary);// free memeory

}
pdormitory_node *search_account(unsigned int *len, int *pstart)
{
		char askflg[4];
		char keyword[20];
		pdormitory_node *pary; //账户节点的指针数组，用于排序
		int findindx; //第一个匹配元素的位置
		dormitory keynode;
		int  idx; //关键字匹配类型
		int  findcunt; //根据关键字查找到匹配账户的个数
				
		PT("whether display all accounts(yes/no):");
		memset(askflg, 0, sizeof(askflg));
		memset(keyword, 0, sizeof(keyword));
	    idx = findcunt = 0;

		if(arrayinfo.count == 0)
				return NULL; //没有有效数据
		scanf("%s",askflg);
		if (strcmp(askflg,"yes") == 0)
				viewall(); //预览所有的账号信息

		PT("what keywords to find, as below? \n");
		PT("\t [1] find by student id\n");
		PT("\t [2] find by dormitory id\n");
		PT("\t [3] find by check-in date\n");
		PT("\t [4] find by student name\n");

		PT("please input index ,as above:");
		scanf("%d",&idx);

		PT("please input keywords:");
		scanf("%s",keyword);
		findindx = 0;

		switch (idx) {
		case 1:   //by student id
				pary = sortbystdid(NULL); //pary数组按studentID有序
				PT("choiced mode is [1] ,search by student id...\n");
				strcpy(keynode.StudentID, keyword);
				findcunt = binary_search((void**)pary, &keynode,\
					         (void *)&findindx, arrayinfo.count, compar_stuid);
				break;
		case 2:

				break;
		case 3:

				break;
		case 4:

				break;
		default:
				PT("input error,please input again\n");
				break;
		}/*switch*/

		if(len && pstart) {
				*pstart = findindx;
				*len = findcunt;
		}
		return pary;
}


void findbydorid() 
{

}
void findbynameid(void) {
	
}

void findbystdid() 
{
		pdormitory_node *porder, *pidx; // order array  by studentID 
		unsigned int count, finds;
		int indx;           //first matched index
		char stdid[10];
		dormitory starget;

		finds = indx = 0;
		memset(stdid, 0, sizeof(stdid));
		PT("please input you want to search student ID:");
		scanf("%s",stdid);
		strcpy(starget.StudentID, stdid);
		pidx = NULL;
		porder = sortbystdid(&count); 
		if(!porder) {//no data
				PT("No data\n");
				goto out_work;
		}
		finds = binary_search((void**)porder, &starget, (void *)&indx, count, compar_stuid);
		if(finds == 0) {
				PT("No matched\n");
				goto out_work;
		}else {//有元素找到，输出全部
				pidx = &porder[indx];
				printf("has %d elements ,as below\n", finds);
				PFTitle;
				while(finds > 0) {
						print_one_dormitory((*pidx)->data);
						pidx++;
						finds--;
				}/*while*/
		}

out_work:
		if(!porder)
				free(porder);

		PAUSERUN;
}

int binary_search(void **base, void *val, void *pstart, int n, compar cmp)
{
		int low, mid, high;
		int fmid, nmid;
		int matchs;
		pdormitory pval;
		pdormitory_node *pbase;
	
		pval = (pdormitory)val;
		pbase =(pdormitory_node*) base;
		low = 0;
		high = n-1; // index is 0	
		matchs = 0; //how many match
		if(strlen((*pval).StudentID) == 0) {
				PT("you want to find what?\n");
				return 0;
		}
		if(!(*pbase))  
				return 0; //数组为空
		assert(pbase && *pbase);//保证要查找的数组不为空
		while(low<=high) {
				mid = (low+high)/2;
				if(cmp(pbase[mid],pval) == 0) {//已找到，前后搜索，检查是否有重复值
						for(fmid=mid-1; (fmid>=low) && cmp(pbase[fmid],pval)==0;)
								fmid--; //before search
						for(nmid=mid+1;(nmid<=high) && cmp(pbase[nmid],pval)==0;)
								nmid++; //behind search
				fmid++; // the first match
				nmid--; //the last match
				matchs = (nmid-fmid)+1;//number of match
				*(int*)pstart = fmid;	
				return matchs;

				}else if (cmp(pbase[mid],pval) < 0) {
						low = mid + 1;		

				}else {
						high = mid - 1;
				}	
		}/* while */
		return 0; //没有找到匹配的返回0
} 

pdormitory_node *sortbystdid(unsigned int *len)
{
		pdormitory_node *array_stdid, *p;
		pdormitory_node ptmp;
		unsigned int n;
		int i;

		//取得 pdormitory 类型的数组
		n = create_one_array(&array_stdid);
		if(n == 0)
				return NULL; //没有数据
		p = array_stdid;
		if (len)   //len不为时，记录账号总数
				*len = n;

		ptmp = pheadarry;
		if(!ptmp) {
				PT("No data to sort\n");
				return NULL;
		}

		for(i = 0; ptmp; ptmp = ptmp->next,i++)
				if(ptmp->dvalid)	
						p[i] = ptmp; //数组元素赋值
	
		#if DEBUG
		PT("sort before by student ID\n");
		for(p = array_stdid, i = 0; i < n; i++)
				print_one_dormitory(p[i]->data);
		#endif

		//采用插入排序法，对数组排序
		insert_sort((void**)array_stdid, n, compar_stuid);

		#if DEBUG
		PT("sort after by student ID\n");
		for(p = array_stdid, i = 0; i < n; i++)
				print_one_dormitory(p[i]->data);
		#endif
		return array_stdid;
}

void insert_sort(void **base, int len, compar cmp)
{
		pdormitory_node *pbase;
		int i , n;

		pbase = (pdormitory_node*) base;
		n = len;
		assert(n>0);
		//插入排序，第一个元素已经有序
		//所以从第二个开始
		for(i=1; i<n; i++)
				insert(pbase, i, cmp);//insert into order array
}

void insert(pdormitory_node *parry,int pos, compar cmp)
{
		int j;
		pdormitory_node *tmp;
		pdormitory_node val;
	
		tmp = parry;
		val = tmp[pos];
		for(j=pos; j>0 && cmp(tmp[j-1],val->data)>0; j--)
				tmp[j] = tmp[j-1];  //元素往后移动
	
		tmp[j] = val; //元素放入到正确的位置

}

void savefile()
{
		int icnt, idata, datas;
		char *ptr;
		FILE *fp =NULL;
		pdormitory_node pnode;

		pnode = pheadarry;
		idata = sizeof(struct dormitory);
		icnt = 0;
		datas = 0;
		if(pnode == NULL){
				PT("data is empty!\n");
				return ;
		}

		fp = fopen(DATA,"w");
		if(!fp) {
				PT("open file is failed!\n");
				return;
		}

		memset(membufs, 0, sizeof membufs);
		ptr = membufs;
		while(pnode != NULL) {
				//以10个结构体数据为一块，存入到缓存中，再把缓存写入到文件
				for(icnt = 0, ptr = membufs; icnt <= 10 && pnode;) {
						if(pnode->dvalid) { //只写有效数据
								memcpy(ptr, pnode->data, idata);
								#if DEBUG
								PFTitle;
								print_one_dormitory(pnode->data);
								#endif
								ptr += idata;
								datas++;
								icnt++;
						}/*if*/

						pnode = pnode->next;  		
				}/*for : copy data to bufer, at most 10 struct data*/

				if((icnt-1) != fwrite(membufs, idata, icnt-1, fp)) {
						PT("--warning--:  data isn't write completely \n");
				}
				memset(membufs, 0, sizeof membufs); //clear current data to sotrage next one

		}/*while*/

		printf("save completely of %d data!\n",datas);
		fclose(fp);
		PAUSERUN;
		PT("Press Enter to continue\n");
}

void print_usage()
{
		PT("<dormitorymanage verbose> is debug mode that can display detail \n");
		PT("<dromitorymanage > is normal mode\n");
}


void print_all_dormitory(pdormitory_node p)
{
		pdormitory_node tmp = p;
        PFTitle;
		while(tmp) {
				if(tmp->dvalid) 
					print_one_dormitory(tmp->data);
				

				#if DEBUG 
				if(!tmp->dvalid) {
					PT("\t\t \n<----- invaild data ----->\n");
					printf("\t invaild flag is=%d\n",tmp->dvalid);
					print_one_dormitory(tmp->data);
					PT("\t\t <-----------end------------->\n");
				}
				#endif
				tmp = tmp->next;	
		}
}

void print_one_dormitory(pdormitory p)
{
		printf("%10s",p->dormID);
		printf("%10s",p->StudentID);
		printf("%10s",p->Name);
		printf("%15s",p->Date);
		puts("\n");

}
