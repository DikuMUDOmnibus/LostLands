
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void error(char *message)
{
    fprintf(stderr, "Error: %s", message);
}


void warning(char *message, int linenum, char *fn)
{
    fprintf(stderr, "%s:Line %d - %s", fn, linenum, message);
}


char *str_dup(char *string)
{
    char *ptr;

    ptr = (char *) malloc(sizeof(char) * (strlen(string) + 1));
    strcpy(ptr, string);
    return(ptr);
}


long decipher_flag_text(char *flags)
{
    char *ptr;
    long vector = 0;

    if (isdigit(*flags))
	return(atol(flags));
    ptr = flags;
    while(isalpha(*ptr)) {
	if (*ptr > 'Z')
	    vector |= (1 << (*ptr - 'a'));
	else
	    vector |= (1 << (*ptr - 'A' + 26));
	ptr++;
    }
    if (*ptr)
	return(-1);
    return(vector);
}

