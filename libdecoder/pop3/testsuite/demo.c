#ifdef POP3_MAIN

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "pop3_type.h"
#include "pop3_util.h"
#include "pop3_mem.h"
#include "pop3_private.h"
#include "pop3_client_parser.h"
#include "pop3_server_parser.h"
#include "pop3_client_lex.h"
#include "pop3_server_lex.h"

static int parse_pop3_file(const char *filename)
{
	char *line = NULL;
	size_t len = 0;
	ssize_t read = 0;
	int ret = -1;
	pop3_data_t *priv_data = NULL;
	int lines = 0;
	FILE *fp = fopen(filename, "r");
	if(!fp)
	{
		fprintf(stderr, "failed to open file %s\n", filename);
		goto failed;
	}

	priv_data = pop3_alloc_priv_data(0);
	if(!priv_data)
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
			if(parse_pop3_client_stream(priv_data, line+2, read-2, 0))
			{
				fprintf(stderr, "parse client failed, line(%d): %s\n", lines, line);
				goto failed;
			}
		}
		else if (toupper(line[0])=='S' && line[1]==':')
		{
			if(parse_pop3_server_stream(priv_data, line+2, read-2, 0))
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
	if(parse_pop3_client_stream(priv_data, "", 0, 1))
	{
		fprintf(stderr, "parse client end flag failed");
		goto failed;
	}
	if(parse_pop3_server_stream(priv_data, "", 0, 1))
	{
		fprintf(stderr, "parse server end flag failed");
		goto failed;
	}

	ret = 0;
	/*pass through*/

failed:
	if(priv_data)
		pop3_free_priv_data(priv_data);
	if(line)
		free(line);
	if(fp)
		fclose(fp);
	return ret;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	if(argc != 2)
	{
		fprintf(stderr, "Usage: pop3_parser <file_name>\n");
		return -1;
	}
	pop3_mem_init();
	debug_pop3_server = 1;
	debug_pop3_client = 1;
	debug_pop3_mem = 1;
	ret = parse_pop3_file(argv[1]);
	pop3_mem_finit();

	return ret;
}
#endif
