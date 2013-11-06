#ifdef POP3_MAIN

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "libengine.h"
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
	engine_t engine = NULL;
	if(argc != 3)
	{
		fprintf(stderr, "Usage: pop3_parser <signature_file> <message_file>\n");
		return -1;
	}
	debug_pop3_server_lexer = 0;
	debug_pop3_server_parser = 0;
	debug_pop3_client_lexer = 0;
	debug_pop3_client_parser = 0;
	debug_pop3_mem = 0;
	debug_pop3_detect = 1;
	debug_engine_parser = 0;
	debug_engine_lexier = 0;
	debug_engine_init = 0;
	debug_engine_compiler = 0;
	debug_engine_runtime = 0;
	
	engine = ey_engine_create("pop3");
	if(!engine)
	{
		fprintf(stderr, "create pop3 engine failed\n");
		ret = -1;
		goto failed;
	}
	
	if(ey_engine_load(engine, &argv[1], 1))
	{
		fprintf(stderr, "load pop3 signature failed\n");
		ret = -1;
		goto failed;
	}
	
	decoder = pop3_decoder_init(engine);
	if(!decoder)
	{
		fprintf(stderr, "create pop3 decoder failed\n");
		ret = -1;
		goto failed;
	}

	ret = parse_pop3_file(decoder, argv[2]);

failed:
	if(decoder)
		pop3_decoder_finit(decoder);
	if(engine)
		ey_engine_destroy(engine);
	return ret;
}
#endif
