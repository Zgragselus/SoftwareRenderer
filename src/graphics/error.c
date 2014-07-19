#include "gfx.h"
#include "../memory/memory.h"

// Last error message string
char *error_msg = NULL;
unsigned int error_msg_len = 0;

/* Setting error message */
void gfxSetErrorMessage(const char* msg)
{
	unsigned int len = strlen(msg) + 1;
	
	if(error_msg != NULL)
	{
		gfx_free(error_msg);
		error_msg = NULL;
	}
	
	error_msg_len = len;
	error_msg = gfx_alloc(sizeof(char) * error_msg_len, 1);
	
	unsigned int i = 0;
	for(i = 0; i < len - 1; i++)
	{
		error_msg[i] = msg[i];
	}
	i++;
	error_msg[i] = '\0';
}

/* Let user read it! */
char* gfxGetLastError()
{
	if(error_msg == NULL)
	{
		gfxSetErrorMessage("No Error!\n");
	}
	
	return error_msg;
}
