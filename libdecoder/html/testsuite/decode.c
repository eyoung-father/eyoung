#ifdef HTML_MAIN

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "libengine.h"
#include "html.h"

int parse_html_file(html_handler_t decoder, const char *filename)
{
	char buf[10];
	size_t read = 0;
	int ret = -1;
	html_work_t work = NULL;
	FILE *fp = fopen(filename, "r");
	if(!fp)
	{
		fprintf(stderr, "failed to open file %s\n", filename);
		goto failed;
	}

	work = html_work_create(decoder, 0);
	if(!work)
	{
		fprintf(stderr, "failed to alloc html private data\n");
		goto failed;
	}

	while(!feof(fp))
	{
		read = fread(buf, 1, sizeof(buf), fp);
		if(ferror(fp))
		{
			fprintf(stderr, "read error\n");
			goto failed;
		}

		if(html_decode_data(work, buf, read, feof(fp)))
		{
			fprintf(stderr, "parse buffer failed\n");
			goto failed;
		}
	}

	ret = 0;
	/*pass through*/
	fprintf(stderr, "parser OK!\n");

failed:
	if(work)
		html_work_destroy(work);
	if(fp)
		fclose(fp);
	return ret;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	html_handler_t decoder = NULL;
	engine_t engine = NULL;
	if(argc != 3)
	{
		fprintf(stderr, "Usage: decode <signature_file> <html_file>\n");
		return -1;
	}

	debug_html_lexer = 1;
	debug_html_parser = 1;
	debug_html_mem = 1;
	debug_html_detect = 1;
	debug_engine_parser = 1;
	debug_engine_lexier = 1;
	debug_engine_init = 1;
	debug_engine_compiler = 1;
	debug_engine_runtime = 1;
	
	engine = ey_engine_create("html");
	if(!engine)
	{
		fprintf(stderr, "create html engine failed\n");
		ret = -1;
		goto failed;
	}
	
	if(ey_engine_load(engine, &argv[1], 1))
	{
		fprintf(stderr, "load html signature failed\n");
		ret = -1;
		goto failed;
	}
	
	decoder = html_decoder_init(engine);
	if(!decoder)
	{
		fprintf(stderr, "create html decoder failed\n");
		ret = -1;
		goto failed;
	}

	ret = parse_html_file(decoder, argv[2]);

failed:
	if(decoder)
		html_decoder_finit(decoder);
	if(engine)
		ey_engine_destroy(engine);
	return ret;
}
#endif
