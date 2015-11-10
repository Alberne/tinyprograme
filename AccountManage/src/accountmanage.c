#include "../include/accountmanage.h"
#include "../include/fm.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define NEW(p) (p = malloc(sizeof*(p)))  //p must be a pointer 
#define PT(s) puts(s)

static void  clearbuf(void); //clear input buffer
static p_account_node findaccount(char *aID);  //
void ListAllAccount();   /*list all account */
void ListAccount();    /*list a specifical account */
void ListTransaction(); /*list transaction of a account */
void Mkrecord(char);   /*create a transaction (withdrwa & deposit) record*/
void FreeLogs(p_node_log plog);

p_account_node headAccount = NULL;  // the header pointer of in all account

static p_account_node findaccount(char *aID) 
{
		p_account_node pnode;
		for(pnode=headAccount; pnode; pnode=pnode->next ) {
				if(0 == strcmp(aID, pnode->data.ID))
						return pnode; 
		}/*for*/

 return NULL;
}

static void  clearbuf()
{
		int c;
		while((c = getchar()) != '\n' && c != EOF )
				; /*use up all buffer characters,so every time*
             	   *	input only need a character */
  
		return;
}

int main(int argc, char *argv[])
{
		char menuID;
		do {
				PrintMenu();
				menuID = getchar();  /*input menu ID*/  
				clearbuf();
				switch(menuID) {    
				case '1':              
						Read();   
						break;
				case '2':             
						Create();
						break;
				case '3':
						Distory(); 
						break;  
				case '4':
						Deposit(); 
						break;  
				case '5':  
						Withdraw(); 
						break;  
				case '6':
						List(0); 
						break;  
				case '7':     
						List(1);  
						break;  
				case '8':
						List(2); 
						break;   
				case '9':
						Write();
						break;    
				case '0':     	
						puts("welcome to use ,I'm exiting ,see you later\n");
						break;                 //exit system
				default:
						puts("Are you kid me? please input above Menu ID..\n");
				}/*switch*/
		}while(menuID != '0');
		return 0;
}

void List(int i)
{
		void (*func_listTab[])(void) = {
				ListAllAccount, /*list all account info*/
				ListAccount,    /*list specifical account*/	    
				ListTransaction  /*list a account of transaction record*/
		};
  
		assert(!isdigit(i) && (i < sizeof(func_listTab)));
		func_listTab[i]();  /*according to menu ID call *
		                     *  corresponding functiong */
   
}


/* list all account information */

void ListAllAccount() 
{
		p_account_node pnode;
		pnode = headAccount;
		int sum;
		sum = 0;
	
		if(NULL == pnode) {
				PT("account data is empty!\n\n");
				return;
		}
		assert(headAccount);
	
		PrintAccountTitle();
		for(; pnode; pnode = pnode->next,sum++)		   
				PrintAccount(pnode->data);
		
		printf("%d Account is listting complete!\n\n",sum);
		return;	
}
	

/*list a specifical account information*/

void ListAccount() 
{
		p_account_node pnode;
		char ID[10];
	
		PT("please input AccountID=");
		scanf("%10s",ID);
  
		if(NULL == (pnode = findaccount(ID))) {
				PT("NO Exsit\n");
				return;
		}

		assert(pnode);
		PrintAccountTitle();
		PrintAccount(pnode->data);
 
	
}

/*list a specifical account and which transaction record*/
void ListTransaction()
{
		p_account_node pnode;
		p_node_log pnodelog;
		char ID[10];
		int clogs = 0;
	
		PT("please input AccountID=");
		scanf("%10s",ID);
 
		if(NULL == (pnode = findaccount(ID))) {
				PT("NO Exsit\n");
				return;
		}

		assert(pnode);
		PrintAccountTitle();
		PrintAccount(pnode->data);	
		if(NULL == (pnodelog = pnode->nlog)) {
				PT("The current account has not transaction record \n");
				return;
		}
 
		assert(pnodelog);
		PT("*******************transaction inf*********\
					**************\n\n");
		PrintLogTitle();
 
		for(; pnodelog; pnodelog = pnodelog->next,clogs++)
				PrintLog(pnodelog->data);

		printf("\n\n******************total:%d **************\
					***************\n", clogs);
		return;

}


void Distory()
{
		p_account_node pnode ,pre_pnode;
		char ID[10];
		pnode = pre_pnode = NULL;
	
		PT("please input will delete AccountID=");
		scanf("%10s",ID);
  
		if(!headAccount){
				PT("\n\n account is empty\n\n");
				return;
		}
  
		for(pnode = headAccount; pnode; pre_pnode = pnode,\
				pnode = pnode->next ) {
					
				if(0 == strcmp(ID, pnode->data.ID))
						break;   /*find, break loop*/
		}
 
		if(!pnode) { 
				PT("\n\nNo account is matched\n\n");
				return;
		}
 	
		if(pnode->data.Balance > 0) {
				PT("this account not delete becase of balance isn't zero\n\n");
				return;
		}
  	
		if(pre_pnode != NULL) // not head of array ,rebinding arrays
				pre_pnode->next = pnode->next; /*original pre_pnode->next* 
								*  pointer to pnode*/
        	else
				headAccount = NULL;  //head of array     
    	 	
		FreeLogs(pnode->nlog); //free 
		free(pnode);
		pnode = NULL;
		PT("account delete complete!\n\n");	
}

void FreeLogs(p_node_log plog)
{
		p_node_log pnext, p;
		pnext = plog;
	
		for(; pnext; ) {
				p = pnext;
				pnext = pnext->next;
				free(p);	
				p = NULL;
		}/*for*/
	
	}
	
void Mkrecord(char flg) 
{
		char ID[10];
		float amount;
		char dt[25];
		struct tm *tmp;
		amount = 0;
		time_t t;
		p_account_node pNodeAccount; 
		p_node_log  pNodeLog;

		memset(dt, 0, sizeof(dt));
		if(headAccount == NULL) {
				PT("No data exsit, please Read file to load data\n");  //load data from local file
				return;
		}
		PT("please input AccountID=");
		scanf("%10s",ID);
		if(NULL == (pNodeAccount = findaccount(ID))) {
				PT("NO Exsit\n");
				return;
		}

		PrintAccountTitle();
		PrintAccount(pNodeAccount->data);
		PT("please input amount of money=");
		scanf("%f",&amount);
 
		if(flg == 'W') { // withdraw money record 
				PT("you withdraw meony is now...\n\n"); 	
				if(amount > (pNodeAccount->data.Balance)) {
						PT("\n\n**No sufficient funds**\n\n");
						return;
				}else {
					  /* subtract amount updating the latest balance*/
						pNodeAccount->data.Balance -= amount; 
				} 
 	  		
		} else if(flg == 'D') {
			  /*deposit money record and  add to amount*/
				pNodeAccount->data.Balance += amount;  
		}	
 		
 	
		t = time(NULL);   //create timestamp
		tmp = localtime(&t);
		assert(tmp);
		
		// time format: 2015-09-25:14:11:59
		assert(0 != strftime(dt, sizeof(dt), "%Y-%m-%d:%H:%M:%S", tmp));  
 
		PT("update information below .....\n");
		PrintAccountTitle();
		PrintAccount(pNodeAccount->data);
		NEW(pNodeLog);
		assert(pNodeLog);

		/**
		* create a deposit record information about 
		* W&D flag,  amount money, operation date
		**/
		pNodeLog->data.WD = flg;  // flg is deposit or withdraw flag
		pNodeLog->data.Amount = amount;
		memset(pNodeLog->data.DateTime, 0, sizeof(pNodeLog->data.DateTime));
		strcpy(pNodeLog->data.DateTime, dt);
 
		pNodeLog->next = pNodeAccount->nlog;  
		pNodeAccount->nlog = pNodeLog;
 
		PT("-----------transaction info---------------\n\n");   
		PrintLogTitle();
		PrintLog(pNodeLog->data);
 
		PT("update information complete....");
		clearbuf();
		return;

}


void Deposit ()
{
		Mkrecord('D');
}

void Withdraw()
{
		Mkrecord('W');
}

void Write()
{
 
		FILE *fp;
		p_node_log pNodeLog;
		p_account_node pNodeAccount;
		int logs;

		fp = fopen(DATAFILE,"wb+");
		assert(fp);

		pNodeAccount = headAccount;
		//walk all accounts
		for(; pNodeAccount; pNodeAccount = pNodeAccount->next) { 
				if(1 != fwrite(&pNodeAccount->data, sizeof(account), 1, fp))
						PT("account data write failed\n");
#if DEBUG
				PT("\n\n***** DEBUG *******\n\n");
				PrintAccountTitle();
				PrintAccount(pNodeAccount->data);
#endif

				logs = 0;
				pNodeLog = pNodeAccount->nlog;
				for(; pNodeLog; pNodeLog = pNodeLog->next)
						logs ++;          //  current account transaction records

				if( 1 != fwrite(&logs, sizeof(int), 1, fp))
						PT("transcation logs sum  write is faild\n");
#if DEBUG
				PT("transaction record=");
				printf("%d\n\n",logs);
#endif

				pNodeLog = pNodeAccount->nlog;
				for(; pNodeLog; pNodeLog = pNodeLog->next) {
						if(1 != fwrite(&pNodeLog->data, sizeof(log), 1, fp))
								PT("write a transcation log\n ");
#if DEBUG
				PT("-----------transaction info---------------\n\n");
				PrintLogTitle();
				PrintLog(pNodeLog->data);
#endif
				}/*for*/

 
		} /*for*/

		fclose(fp);
		PT("write file complete and press Enter return main Menu!\n");
		getchar();
		clearbuf();
}


void Read()
{
		long int fsize; 
		int iLog, len=0;
		FILE *fp;
		p_account_node pNodeAccount;
		p_node_log     pNodeLogs;
		printf("prepare to read all  account information from\
					local file <%s>.....\n\n",DATAFILE);

		if(headAccount) {
				PT("memory exsit data ,no read\n");
				return;
		}
		if(NULL == (fp = fopen(DATAFILE, "rb"))) {
				PT("no exsit file\n");
				return;
		}
 
		assert(fp);
		fseek(fp, 0L, SEEK_END); // end of file
		fsize = ftell(fp);
		rewind(fp);    //head of file
 
		if(fsize <= 0)
				PT("**the account information is empty\n");
 
		while(fsize > 0) {        //account is not empty
				len++;
				NEW(pNodeAccount);
				assert(pNodeAccount);
  
				pNodeAccount->next = headAccount;
				headAccount = pNodeAccount;
				pNodeAccount->nlog = NULL;
	
				if(1 != fread(&pNodeAccount->data, sizeof(account), 1, fp)) {
						PT(" account read is failed\n");
						exit(-1);
				}
  
				fsize -= sizeof(account);
 
				if(1 != fread(&iLog, sizeof(int), 1, fp)) {
						PT("transaction log amount read faild\n");
						exit(-1);
				}
				fsize -= sizeof(int);
   
       //read n transaction record about current account
				for(; iLog; iLog-- ) {  
						NEW(pNodeLogs);
						assert(pNodeLogs);
						if(1 != fread(&pNodeLogs->data, sizeof(log), 1, fp)) {
								PT("read iLog transaction faild\n");
								exit(-1);
						}
   
						pNodeLogs->next = pNodeAccount->nlog;
						pNodeAccount->nlog = pNodeLogs;
						fsize -= sizeof(log);
				}/*for*/
		} /*while*/

		fclose(fp);
		printf("<%s> is read complete, total [%d] Press Enter to\
            		Countinue!\n",DATAFILE,len);
		getchar();
		return;

}


void Create()
 {
		puts("create\n");

		p_account_node pNodeAccount;
		NEW(pNodeAccount);
		assert(pNodeAccount);

		PT("\n Account ID = ");
		scanf("%s",pNodeAccount->data.ID);

		PT("\n Name = ");
		scanf("%s",pNodeAccount->data.Name);

		pNodeAccount->data.Balance = 0;
		pNodeAccount->nlog = NULL;
		pNodeAccount->next = NULL;

		pNodeAccount->next = headAccount; // headAccount pointer to all accounts
		headAccount = pNodeAccount;

		PrintAccountTitle();
		PrintAccount(pNodeAccount->data);
		PT("create a new Account complete! press Enter continue\n");
		getchar();
		clearbuf();
		return ;

}
