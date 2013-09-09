#include <assert.h>
#include "ey_engine.h"
#include "ey_signature.h"
#include "gram_parser.h"
#include "gram_lexer.h"

static unsigned int hash_filename(void *filename)
{
	return ((unsigned int)filename)>>3;
}

static int compare_filename(void *k, void *v)
{
	if(!k || !v)
		return 1;
	
	return strcmp((char*)k, (char*)v);
}

int ey_parser_init(ey_engine_t *eng)
{
	char name[64];
	if(!ey_filename_fslab(eng))
	{
		snprintf(name, sizeof(name), "%s filename fslab\n", eng->name);
		name[63] = '\0';
		ey_filename_fslab(eng) = engine_fzinit(name, 128, NULL);
		if(!ey_filename_fslab(eng))
		{
			engine_init_error("create filename fslab failed\n");
			return -1;
		}
	}

	if(!ey_filename_hash(eng))
	{
		snprintf(name, sizeof(name), "%s filename hash\n", eng->name);
		name[63] = '\0';
		ey_filename_hash(eng) = ey_hash_create(name, 10, 8192, hash_filename, compare_filename, NULL, NULL);
		if(!ey_filename_hash(eng))
		{
			engine_init_error("create filename hash failed\n");
			return -1;
		}
	}

	return 0;
}

void ey_parser_finit(ey_engine_t *eng)
{
	if(!eng)
		return;

	if(ey_filename_hash(eng))
		ey_hash_init(ey_filename_hash(eng));
	if(ey_filename_fslab(eng))
		engine_fzclear(ey_filename_fslab(eng));
	assert(eng->parser==NULL);
}

int ey_parse_file(ey_engine_t *eng, const char *filename)
{
	FILE *fp = NULL;
	yyscan_t lexer = NULL;
	gram_pstate *pstate = NULL;
	ey_parser_t *parser = NULL;
	GRAM_STYPE sval;
	GRAM_LTYPE lval = {1,1,1,1,filename};
	int token = 0, pstate_ret = 0;

	if(!eng || !filename)
	{
		engine_init_error("null engine or filename\n");
		return -1;
	}
	
	if(ey_hash_find(ey_filename_hash(eng), (void*)filename))
	{
		engine_init_debug("file %s has been parsed\n", filename);
		return 0;
	}

	if((fp=fopen(filename, "r")) == NULL)
	{
		engine_init_error("open file %s failed\n", filename);
		goto failed;
	}

	parser = (ey_parser_t*)engine_malloc(sizeof(ey_parser_t));
	if(!parser)
	{
		engine_init_error("alloc parser failed\n");
		goto failed;
	}
	memset(parser, 0, sizeof(*parser));

	if(gram_lex_init_extra(eng, &lexer))
	{
		engine_init_error("alloc lexer failed\n");
		goto failed;
	}
	gram_set_in(fp, lexer);
	parser->lexer = lexer;
	
	pstate = gram_pstate_new();
	if(!pstate)
	{
		engine_init_error("alloc pstate failed\n");
		goto failed;
	}
	parser->parser = pstate;
	
	parser->filename = engine_fzalloc(strlen(filename)+1, ey_filename_fslab(eng));
	if(!parser->filename)
	{
		engine_init_error("copy filename failed\n");
		goto failed;
	}
	strcpy(parser->filename, filename);

	parser->buffer = (char*)engine_malloc(DEFAULT_BUFFER_SIZE);
	if(!parser->buffer)
	{
		engine_init_error("init parser buffer failed\n");
		goto failed;
	}
	parser->buffer_size = DEFAULT_BUFFER_SIZE;
	parser->buffer_len = 0;
	eng->parser = parser;
	
	if(ey_hash_insert(ey_filename_hash(eng), parser->filename, parser->filename))
	{
		engine_init_error("insert filename %s to filename hash failed\n", parser->filename);
		goto failed;
	}

	while(1)
	{
		token = gram_lex(&sval, &lval, lexer);
		pstate_ret = gram_push_parse(pstate, token, &sval, &lval, (void*)eng);
		if(pstate_ret != YYPUSH_MORE)
			break;
	}

	if(pstate_ret != YYPUSH_MORE && pstate_ret != 0)
		engine_init_error("find error while parsing %s\n", parser->filename);
	else
		engine_init_debug("parse %s successfully\n", parser->filename);
	
failed:
	if(parser)
	{
		if(parser->filename)
			engine_fzfree(ey_filename_fslab(eng), parser->filename);
		if(parser->buffer)
			engine_free(parser->buffer);
	}
	if(pstate)
		gram_pstate_delete(pstate);
	if(lexer)
		gram_lex_destroy(lexer);
	if(parser)
		engine_free(parser);
	if(fp)
		fclose(fp);
	eng->parser = NULL;
	return 0;
}
