#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "getword.h"
int getword(FILE *fp, char *buf, int size) 
{
	int i, c;
	i = c = 0;
	assert(fp && buf && size >1);
	c = getc(fp);
	
	/*jump over the spaceing of word before*/
	for(; c != EOF; c = getc(fp))
		if(isalpha(c)) {
			if(i < size -1 ) {
				buf[i++] = c;
			}/*if*/
			break;
		}/*if*/

	/*read one word*/
	for(c = getc(fp); c != EOF && (isalpha(c) || c == '_'); c = getc(fp)) {
		if(i < size -1)
			buf[i++] = c;

	}	

	if(i < size)
		buf[i] = '\0';
	else
		buf[size-1] = '\0';

	if(c != EOF)
		ungetc(c, fp);
       
       return i > 0;
	
	
}
