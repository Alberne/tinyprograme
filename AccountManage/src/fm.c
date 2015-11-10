#include <stdlib.h>
#include <stdio.h>

#include "../include/accountmanage.h"
#include "../include/fm.h"

#define CLE_SCREEN "clear"
#define PF(m)  printf(m)

void PrintMenu()
 {
		PF("----------------  Menu  ------------------\n");
		PF("------------------------------------------\n\n");
		PF("1. Read file;\n");
		PF("2. Create account;\n");
		PF("3. Distory account;\n");
		PF("4. Deposit;\n");
		PF("5. Withdraw;\n");
		PF("6. List all account;\n");
		PF("7. Find account;\n");
		PF("8. Find log;\n");
		PF("9. Write file;\n");
		PF("0. Exit\n");
		PF("------------------------------------------\n\n");
		PF("please make a choice::\n");
}


void PrintAccountTitle() 
{
		PF("--------------------------------------------------------\n");
		PF("\t ID \t|\t Name \t|\t Balance \t\n");
}

void PrintAccount(account p) 
{
		printf("%10s \t|\t%s \t|\t%f\n\n\n",p.ID,p.Name,p.Balance);
}

void PrintLogTitle() 
{
		PF("\t DateTime \t|\t W&D \t|\t Amount \t\n");
}

void PrintLog(log p) {
		printf("%23s |\t%c \t|\t%f\n",p.DateTime,p.WD,p.Amount);
}








