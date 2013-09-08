#ifdef SNORT_DEBUG

#include "snort_info.h"
#include "snort_signature.h"
#include "snort_parser.h"
#include "snort_lexer.h"

static int convert_file(const char *filename)
{
	FILE *fp = NULL;
	yyscan_t lexer = NULL;
	snort_pstate *pstate = NULL;
	SNORT_STYPE sval;
	SNORT_LTYPE lval = {1,1,1,1};
	int token = 0, pstate_ret = 0;

	if(!filename)
	{
		snort_init_error("null filename\n");
		return -1;
	}
	
	if((fp=fopen(filename, "r")) == NULL)
	{
		snort_init_error("open file %s failed\n", filename);
		goto failed;
	}

	if(snort_lex_init(&lexer))
	{
		snort_init_error("alloc lexer failed\n");
		goto failed;
	}
	snort_set_in(fp, lexer);
	
	pstate = snort_pstate_new();
	if(!pstate)
	{
		snort_init_error("alloc pstate failed\n");
		goto failed;
	}
	
	while(1)
	{
		token = snort_lex(&sval, &lval, lexer);
		pstate_ret = snort_push_parse(pstate, token, &sval, &lval);
		if(pstate_ret != YYPUSH_MORE)
			break;
	}

	if(pstate_ret != YYPUSH_MORE && pstate_ret != 0)
		snort_init_error("find error while parsing %s\n", filename);
	else
		snort_init_debug("parse %s successfully\n", filename);
	
failed:
	if(pstate)
		snort_pstate_delete(pstate);
	if(lexer)
		snort_lex_destroy(lexer);
	if(fp)
		fclose(fp);
	return 0;
}

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		snort_init_error("need snort filename list as the parameter\n");
		return -1;
	}
	
	debug_snort_parser = 1;
	debug_snort_lexer = 1;
	debug_snort_init = 1;
	int i;
	for(i=1; i<argc; i++)
		convert_file(argv[i]);
	return 0;
}
#endif
