#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include "html_detect.h"
#include "html_util.h"
#include "libutil.h"

#define PARAM_SIZE 512
#define TEST_TEMPLATE "<p class=\"$1\">$0</p>"

//#define TRAINING_DATA "part1-part2"
#define TRAINING_DATA "cGFydDEtcGFydDI="
//#define TEST_DATA "content-dummy\" onload=\"javascript:alert('abc')"
#define TEST_DATA "Y29udGVudC1kdW1teSIgb25sb2FkPSJqYXZhc2NyaXB0OmFsZXJ0KCdhYmMnKQo="

static int my_preprocessor(int cnt, int size, html_string_t *params)
{
	char *ptr = NULL;
	
	//base64 decode
	int left = 0;
	int decoded_len = base64_stream_decode(params[0].buf, params[0].buf, params[0].len, &left);
	assert(left == 0);
	params[0].len = decoded_len;

	ptr = strchr(params[0].buf, '-');
	assert(ptr != NULL);

	*ptr = '\0';
	params[0].len = strlen(params[0].buf);

	ptr++;
	assert(*ptr != '\0');
	strcpy(params[1].buf, ptr);
	params[1].len = strlen(ptr);

	return 2;
}

int main(int argc, char *argv[])
{
	xss_work_t work = NULL;
	char p1[PARAM_SIZE], p2[PARAM_SIZE];
	html_string_t params[2] = {{p1,0}, {p2,0}};

	debug_html_lexer = 0;
	debug_html_parser = 0;
	debug_html_mem = 0;
	debug_html_detect = 1;

	if(xss_module_init(NULL))
	{
		fprintf(stderr, "xss init failed\n");
		goto failed;
	}
	
	strncpy(params[0].buf, TRAINING_DATA, PARAM_SIZE);
	params[0].len = sizeof(TRAINING_DATA) - 1;
	work = xss_training(TEST_TEMPLATE, my_preprocessor, 1, 2, params);
	if(!work)
	{
		fprintf(stderr, "xss training failed\n");
		goto failed;
	}

	strncpy(params[0].buf, TEST_DATA, PARAM_SIZE);
	params[0].len = sizeof(TEST_DATA) - 1;
	xss_check(work, 1, 2, params);

	xss_untraining(work);
	xss_module_finit(NULL);

	return 0;

failed:
	if(work)
		xss_untraining(work);
	xss_module_finit(NULL);
	return -1;
}
