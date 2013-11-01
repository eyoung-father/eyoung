#ifdef POP3_MAIN

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "pop3.h"

static int parse_pop3_file(pop3_handler_t decoder, const char *filename)
{
	char *line = NULL;
	size_t len = 0;
	ssize_t read = 0;
	int ret = -1;
	pop3_work_t *work = NULL;
	int lines = 0;
	FILE *fp = fopen(filename, "r");
	if(!fp)
	{
		fprintf(stderr, "failed to open file %s\n", filename);
		goto failed;
	}

	work = pop3_work_create(decoder, 0);
	if(!work)
	{
		fprintf(stderr, "failed to alloc pop3 private data\n");
		goto failed;
	}

	while ((read = getline(&line, &len, fp)) != -1)
	{
		lines++;
		if(read <=1)
		{
			fprintf(stderr, "invalid line(%d): %s\n", lines, line);
			goto failed;
		}
		if(toupper(line[0])=='C' && line[1]==':')
		{
			if(pop3_decode_data(work, line+2, read-2, 1, 0))
			{
				fprintf(stderr, "parse client failed, line(%d): %s\n", lines, line);
				goto failed;
			}
		}
		else if (toupper(line[0])=='S' && line[1]==':')
		{
			if(pop3_decode_data(work, line+2, read-2, 0, 0))
			{
				fprintf(stderr, "parse server failed, line(%d): %s\n", lines, line);
				goto failed;
			}
		}
		else
		{
			fprintf(stderr, "invalid line(%d): %s\n", lines, line);
			goto failed;
		}
	}

	/*give end flag to parser*/
	if(pop3_decode_data(work, "", 0, 1, 1))
	{
		fprintf(stderr, "parse client end flag failed");
		goto failed;
	}
	if(pop3_decode_data(work, "", 0, 0, 1))
	{
		fprintf(stderr, "parse server end flag failed");
		goto failed;
	}

	ret = 0;
	/*pass through*/
	fprintf(stderr, "parser OK!\n");

failed:
	if(work)
		pop3_work_destroy(work);
	if(line)
		free(line);
	if(fp)
		fclose(fp);
	return ret;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	pop3_handler_t decoder = NULL;
	if(argc != 2)
	{
		fprintf(stderr, "Usage: pop3_parser <file_name>\n");
		return -1;
	}
	debug_pop3_server_lexer = 1;
	debug_pop3_server_parser = 1;
	debug_pop3_client_lexer = 1;
	debug_pop3_client_parser = 1;
	debug_pop3_mem = 1;
	debug_pop3_detect = 1;
	
	decoder = pop3_decoder_init();
	if(decoder)
	{
		ret = parse_pop3_file(decoder, argv[1]);
		pop3_decoder_finit(decoder);
	}

	return ret;
}
#endif
