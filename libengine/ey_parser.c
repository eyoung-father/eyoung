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
	if(!ey_parser_fslab(eng))
	{
		snprintf(name, sizeof(name), "%s parser fslab\n", eng->name);
		name[63] = '\0';
		ey_parser_fslab(eng) = engine_fzinit(name, 64, NULL);
		if(!ey_parser_fslab(eng))
		{
			engine_init_error("create parser fslab failed\n");
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
	{
		ey_hash_destroy(ey_filename_hash(eng));
		ey_filename_hash(eng) = NULL;
	}

	if(ey_parser_fslab(eng))
	{
		engine_fzfinit(ey_parser_fslab(eng));
		ey_parser_fslab(eng) = NULL;
	}
	assert(eng->parser==NULL);
}

static int ey_output_code_cfile(ey_engine_t *eng, FILE *fp, ey_code_t *code)
{
	fprintf(fp, "#line %d \"%s\"\n", code->location.first_line, code->location.filename);
	fprintf(fp, "%s\n", code->raw_code);
	return 0;
}

static int ey_output_import_cfile(ey_engine_t *eng, FILE *fp, ey_code_t *import)
{
	fprintf(fp, "#line %d \"%s\"\n", import->location.first_line, import->location.filename);
	return ey_extract_library(eng, import->filename, fp);
}

static int ey_output_event_cfile(ey_engine_t *eng, FILE *fp, ey_code_t *event)
{
	fprintf(fp, "#line %d \"%s\"\n", event->location.first_line, event->location.filename);
	fprintf(fp, "typedef %s %s;\n", event->event->define, event->event->name);
	return 0;
}

static int ey_output_prologue_cfile(ey_engine_t *eng, FILE *fp, ey_code_t *prologue)
{
	assert(eng!=NULL);
	assert(fp!=NULL);
	assert(prologue!=NULL);

	switch(prologue->type)
	{
		case EY_CODE_NORMAL:
		{
			return ey_output_code_cfile(eng, fp, prologue);
		}
		case EY_CODE_IMPORT:
		{
			return ey_output_import_cfile(eng, fp, prologue);
		}
		case EY_CODE_EVENT:
		{
			return ey_output_event_cfile(eng, fp, prologue);
		}
		default:
		{
			break;
		}
	}
	return -1;
}

static int ey_output_rules_cfile(ey_engine_t *eng, FILE *fp, ey_signature_list_t *signature_list)
{
	assert(eng!=NULL);
	assert(fp!=NULL);
	assert(signature_list!=NULL);

	ey_signature_t *signature=NULL;
	TAILQ_FOREACH(signature, signature_list, link)
	{
		int line=0;
		ey_rhs_signature_t *rhs_signature = NULL;
		TAILQ_FOREACH(rhs_signature, &signature->rhs_signature_list, link)
		{
			int column = 0;
			ey_rhs_item_t *rhs_item = NULL;
			TAILQ_FOREACH(rhs_item, &rhs_signature->rhs_item_list, link)
			{
				ey_event_t *event = ey_find_event(eng, rhs_item->event_name);
				if(!event)
				{
					engine_init_error("cannot find event %s in line %d(%u, %d, %d)\n", 
						rhs_item->event_name, 
						rhs_item->location.first_line,
						signature->signature_id, line, column);
					return -1;
				}

				ey_rhs_item_condition_t *condition = rhs_item->condition;
				if(condition && condition->raw_code)
				{
					condition->func_name = (char*)engine_fzalloc(MAX_CONDITION_FUNC_NAME_LEN, ey_parser_fslab(eng));
					if(!condition->func_name)
					{
						engine_init_error("failed to alloc func name in line %d(%u, %d, %d)\n",
							rhs_item->location.first_line,
							signature->signature_id, line, column);
						return -1;
					}
					snprintf(condition->func_name, MAX_CONDITION_FUNC_NAME_LEN, "__condition_%u_%d_%d",
						signature->signature_id, line, column);

					fprintf(fp, "int %s(void* _LINK_, %s *_THIS_)\n", condition->func_name, event->name);
					fprintf(fp, "{\n");
					fprintf(fp, "#line %d \"%s\"\n", condition->location.first_line, condition->location.filename);
					fprintf(fp, "\treturn %s;\n", condition->raw_code);
					fprintf(fp, "}\n");
				}

				ey_rhs_item_action_t *action = rhs_item->action;
				if(action && action->raw_code)
				{
					action->func_name = (char*)engine_fzalloc(MAX_ACTION_FUNC_NAME_LEN, ey_parser_fslab(eng));
					if(!action->func_name)
					{
						engine_init_error("failed to alloc func name in line %d(%u, %d, %d)\n",
							rhs_item->location.first_line,
							signature->signature_id, line, column);
						return -1;
					}
					snprintf(action->func_name, MAX_ACTION_FUNC_NAME_LEN, "__action_%u_%d_%d",
						signature->signature_id, line, column);

					fprintf(fp, "int %s(void* _LINK_, %s *_THIS_)\n", action->func_name, event->name);
					fprintf(fp, "#line %d \"%s\"\n", action->location.first_line, action->location.filename);
					fprintf(fp, "%s\n", action->raw_code);
				}
				column++;
			}
			line++;
		}
	}
	return 0;
}

static int ey_output_cfile(ey_engine_t *eng, char *src_file, ey_signature_file_t *signature_file)
{
	if(!eng || !src_file || !src_file[0] || !signature_file)
	{
		engine_init_error("%s bad parameter\n", __FUNCTION__);
		return -1;
	}
	
	FILE *cfile = NULL;
	if(!signature_file->output_file)
	{
		int len = strlen(src_file);
		signature_file->output_file = (char*)engine_fzalloc(len+3, ey_parser_fslab(eng));
		if(!signature_file->output_file)
		{
			engine_init_error("alloc output filename failed\n");
			goto failed;
		}
		snprintf(signature_file->output_file, len+3, "%s.c", src_file);
	}

	cfile = fopen(signature_file->output_file, "w");
	if(!cfile)
	{
		engine_init_error("open c file %s failed\n", signature_file->output_file);
		goto failed;
	}

	/*output prologue*/
	ey_code_t *prologue = NULL;
	TAILQ_FOREACH(prologue, &signature_file->prologue_list, link)
	{
		if(ey_output_prologue_cfile(eng, cfile, prologue))
		{
			engine_init_error("output prologue failed, line %d\n", prologue->location.first_line);
			goto failed;
		}
	}
	
	/*output rule condition/action function*/
	if(ey_output_rules_cfile(eng, cfile, &signature_file->signature_list))
	{
		engine_init_error("output rules failed\n");
		goto failed;
	}

	if(ey_output_code_cfile(eng, cfile, signature_file->epilogue))
	{
		engine_init_error("output epilogue failed\n");
		goto failed;
	}
	
	fclose(cfile);
	return 0;

failed:
	if(cfile)
		fclose(cfile);
	return -1;
}

int ey_parse_file(ey_engine_t *eng, const char *filename, int need_link)
{
	FILE *fp = NULL;
	yyscan_t lexer = NULL;
	gram_pstate *pstate = NULL;
	ey_parser_t *parser = NULL;
	GRAM_STYPE sval;
	GRAM_LTYPE lval = {1,1,1,1,filename};
	int token = 0, pstate_ret = 0;
	int ret = -1;

	if(!eng || !filename)
	{
		engine_init_error("null engine or filename\n");
		goto failed;
	}
	
	if(ey_hash_find(ey_filename_hash(eng), (void*)filename))
	{
		engine_init_debug("file %s has been parsed\n", filename);
		goto failed;
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
	
	parser->filename = engine_fzalloc(strlen(filename)+1, ey_parser_fslab(eng));
	if(!parser->filename)
	{
		engine_init_error("copy filename failed\n");
		goto failed;
	}
	strcpy(parser->filename, filename);

	parser->buffer = (char*)engine_fzalloc(DEFAULT_BUFFER_SIZE, ey_parser_fslab(eng));
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
	{
		engine_init_error("find error while parsing %s\n", parser->filename);
		goto failed;
	}
	engine_init_debug("parse %s successfully\n", parser->filename);
	
	if(ey_output_cfile(eng, parser->filename, parser->signature_file))
	{
		engine_init_error("load file %s failed\n", parser->filename);
		goto failed;
	}
	engine_init_debug("load file %s successfully, output %s\n", parser->filename, parser->signature_file->output_file);

	if(ey_compile_signature_file(eng, parser->signature_file, need_link))
	{
		engine_init_error("compile file %s failed\n", parser->signature_file->output_file);
		goto failed;
	}

	if(ey_compile_post_action(eng, parser->signature_file))
	{
		engine_init_error("do post compile %s action s failed\n", parser->filename);
		goto failed;
	}

	engine_init_debug("compile %s successfully\n", parser->signature_file->output_file);
	
	ret = 0;

failed:
	if(parser)
	{
		if(parser->filename)
			engine_fzfree(ey_parser_fslab(eng), parser->filename);
		if(parser->buffer)
			engine_fzfree(ey_parser_fslab(eng), parser->buffer);
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
	return ret;
}
