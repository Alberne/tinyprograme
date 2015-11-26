#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "../utility/getword.h"
#include "table.h"

void ws(char *name, FILE *fp);
void cmp(const void *x, const void *y);

int main(int argc, char *argv[])
{
	int i;
	FILE *fp;

	for(i = 1; i <= argc; i++) {
		fp = fopen(argv[i], "r");
		if(!fp) {
			fprintf(stderr, "%s: can't open. err:%s \n",
				 argv[i], strerror(errno));
			return -1;

		}else {

			ws(argv[i], fp);
			fclose(fp);
			fp = NULL;

		}

	}/*for*/


	return 0;
}

void ws(char *name, FILE *fp) 
{	
	table_t table ;
	char buf[128];
	char *word;
	int *count;
	void **ar;
	int i;

	table = NULL;
	word = NULL;
	ar = NULL;
	memset(buf, 0, sizeof(buf));
	table = table_new(0, NULL, NULL);
	
	while(getword(fp, buf, sizeof(buf))) {
		word = (char *) calloc(1, strlen(buf) + 1);
		assert(word);
		strcpy(word, buf);		
		
		count = table_get(table, word);
		if(count) {
			(*count)++;
		}else {
			count = (int *) calloc(1, sizeof(*count));
			assert(count);
			*count = 1;
			table_put(table, word, count);	
		}
	}/*while*/
	
	if(name)
		printf("%s:\n",name);
	
	ar = table_to_array(table, NULL);
	qsort(ar, table_length(table), 2 * sizeof(*ar), cmp);
	for(i = 0; ar[i]; i += 2) {
		printf("%d\t%s\n", *(int *)ar[i+1], (char *)ar[i]);
		
	}


}

void cmp(const void *x, const void *y)
{
	return strcmp(*(char **)x, *(char **)y);

}

