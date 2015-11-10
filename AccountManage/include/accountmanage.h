#ifndef _ACCOUNTMANAGE_H_
#define _ACCOUNTMANAGE_H_
#define DATAFILE "data"
#define DEBUG 1

typedef struct log log;    //depositing and withdrawing money log
typedef struct node_log node_log; // above logs node
typedef node_log *p_node_log;  
typedef struct account account; //account log
typedef struct account_node account_node;// above account logs node
typedef account_node *p_account_node;

struct log {
		char DateTime[25]; //deposition & withdraw time
		char WD;       // deposition or withdraw
		float Amount;  // a amount of money
};

struct node_log {
		log data;   // the data of log
		p_node_log next; // the next one

};

struct account {
		char ID[10];   // account ID
		char Name[15]; //saving account of name
		float Balance; // account balance
};

 struct account_node {
		account data;   //the data of account
		p_node_log nlog; //the pointer to  D & w records 
		p_account_node next; //the next account node
};

void Read(void);// read data from local file

void Write(void); //write data to local file

/**
*@function name: List
*
*@param   int type [IN]: which type information to show
*   @value 0: all account
*          1: specifical account
*          2: one transaction of specifical account
*
*@function description
*          search a  information about account
*
*@return : void
*
**/

 void List(int type);

 void Create(void);  //create a new account and insert into nodes

 void Deposit(void);  //depositing moeny and record this operation log

 void Withdraw(void); //withdraw money and record this operation log

 void Distory(void);  //distory sepcifical account

 p_account_node FindaAccont(char*);//according to ID serach correlation  account

#endif 
