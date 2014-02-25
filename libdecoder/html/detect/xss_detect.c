#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "html.h"
#include "ey_export.h"
#include "ey_memory.h"
#include "html_detect.h"

typedef struct xss_data
{
	char *template;
	int score;
	xss_preprocess_fn preprocessor;
}xss_data_t;

static html_handler_t xss_handler;
static ey_slab_t xss_data_slab;

static int xss_parse(const char *template, xss_preprocess_fn fn, int argv_cnt, int argv_size, html_string_t *argv)
{
	/*TODO:*/
	return 1;
}

xss_work_t xss_traning(const char *template, xss_preprocess_fn fn, int argv_cnt, int argv_size, html_string_t *argv)
{
	if(!xss_handler || !xss_data_slab)
	{
		ey_html_debug(1, "xss module is not init correctly\n");
		return NULL;
	}
	
	if(!template || !template[0] || argv_cnt<=0 || argv_size<=0 || argv_cnt>argv_size || !argv)
	{
		ey_html_debug(1, "xss module is not called correctly, bad parameter\n");
		return NULL;
	}
	xss_data_t *ret = NULL;
	char *copy_temp = NULL;
	int score = 0;

	ret = (xss_data_t*)html_zalloc(xss_data_slab);
	if(!ret)
	{
		ey_html_debug(1, "alloc xss data failed\n");
		goto failed;
	}
	memset(ret, 0, sizeof(*ret));
	
	copy_temp = (char*)ey_html_malloc(strlen(template) + 1);
	if(!copy_temp)
	{
		ey_html_debug(1, "alloc xss template failed\n");
		goto failed;
	}
	strcpy(copy_temp, template);

	score = xss_parse(template, fn, argv_cnt, argv_size, argv);
	if(score <= 0)
	{
		ey_html_debug(1, "traning template failed\n");
		goto failed;
	}

	ret->template = copy_temp;
	ret->score = score;
	ret->preprocessor = fn;
	return ret;

failed:
	if(copy_temp)
		ey_html_free(copy_temp);
	if(ret)
		html_zfree(xss_data_slab, ret);
	return NULL;
}

int xss_check(xss_work_t work, int argv_cnt, int argv_size, html_string_t *argv)
{
	if(!work || argv_cnt<=0 || argv_size<=0 || argv_cnt>argv_size || !argv)
	{
		ey_html_debug(debug_html_detect, "bad paramter in xss_check\n");
		return 0;
	}

	xss_data_t *xss_data = (xss_data_t*)work;
	if(!xss_data->template)
	{
		ey_html_debug(debug_html_detect, "template is null\n");
		return 0;
	}

	int score = xss_parse(xss_data->template, xss_data->preprocessor, argv_cnt, argv_size, argv);
	if(score != xss_data->score)
	{
		ey_html_debug(debug_html_detect, "find xss!\n");
		return 1;
	}

	ey_html_debug(debug_html_detect, "input is clean!\n");
	return 0;
}

void xss_untraining(xss_work_t work)
{
	if(!work)
		return;
	
	xss_data_t *data = (xss_data_t*)work;
	if(data->template)
		ey_html_free(data->template);
	html_zfree(xss_data_slab, data);
}

int xss_module_finit(void *arg)
{
	if(xss_handler)
	{
		html_decoder_finit(xss_handler);
		xss_handler = NULL;
	}

	if(xss_data_slab)
	{
		html_zfinit(xss_data_slab);
		xss_data_slab = NULL;
	}

	ey_html_debug(1, "xss module finit successfully\n");
	return 0;
}

int xss_module_init(void *arg)
{
	xss_handler = html_decoder_init(NULL);
	if(!xss_handler)
	{
		ey_html_debug(1, "xss module init failed\n");
		goto failed;
	}
	
	xss_data_slab = html_zinit("xss check data slab", sizeof(xss_data_t));
	if(!xss_data_slab)
	{
		ey_html_debug(1, "xss data slab init failed\n");
		goto failed;
	}
	ey_html_debug(1, "xss module init successfully\n");
	return 0;

failed:
	xss_module_finit(arg);
	return -1;
}

EY_EXPORT_INIT(xss_module_init);
EY_EXPORT_FINIT(xss_module_finit);
