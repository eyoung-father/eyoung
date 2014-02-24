%{
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "html.h"
#include "html_private.h"
#include "html_lex.h"

#ifdef YY_REDUCTION_CALLBACK
#undef YY_REDUCTION_CALLBACK
#endif
#define YY_REDUCTION_CALLBACK(data,name,id,val)					\
	do															\
	{															\
		if(html_element_detect(data,name,id,val,				\
			cluster_buffer,cluster_buffer_len)<0)				\
		{														\
			ey_html_debug(debug_html_detect, "find attack!\n");	\
			return -1;											\
		}														\
	}while(0)

#define this_priv ((html_data_t*)priv_data)
#define this_decoder ((html_decoder_t*)(this_priv->decoder))

#define NOCOPY_BREAK											\
{																\
	if(!this_priv->create_dom)									\
		break;													\
}
%}

/*TOKEN declaration*/
%token SYM_LEX_CONTINUE					/*	<lex-continue>	*/
%token SYM_TEXT							/*	text			*/
%token SYM_TAG_START_FLAG				/*	<				*/
%token SYM_TAG_START_FLAG2				/*	</				*/
%token SYM_TAG_END_FLAG					/*	>				*/
%token SYM_TAG_END_FLAG2				/*	/>				*/
%token SYM_EQUAL						/*	=				*/

/*for html tag*/
%token SYM_TAG_A						/*	a				*/
%token SYM_TAG_ABBR						/*	abbr			*/
%token SYM_TAG_ACRONYM					/*	acronym			*/
%token SYM_TAG_ADDRESS					/*	address			*/
%token SYM_TAG_APPLET					/*	applet			*/
%token SYM_TAG_AREA						/*	area			*/
%token SYM_TAG_ARTICLE					/*	article			*/
%token SYM_TAG_ASIDE					/*	aside			*/
%token SYM_TAG_AUDIO					/*	audio			*/
%token SYM_TAG_B						/*	b				*/
%token SYM_TAG_BASE						/*	base			*/
%token SYM_TAG_BASEFONT					/*	basefont		*/
%token SYM_TAG_BDI						/*	bdi				*/
%token SYM_TAG_BDO						/*	bdo				*/
%token SYM_TAG_BIG						/*	big				*/
%token SYM_TAG_BLOCKQUOTE				/*	blockquote		*/
%token SYM_TAG_BODY						/*	body			*/
%token SYM_TAG_BR						/*	br				*/
%token SYM_TAG_BUTTON					/*	button			*/
%token SYM_TAG_CANVAS					/*	canvas			*/
%token SYM_TAG_CAPTION					/*	caption			*/
%token SYM_TAG_CENTER					/*	center			*/
%token SYM_TAG_CITE						/*	cite			*/
%token SYM_TAG_CODE						/*	code			*/
%token SYM_TAG_COL						/*	col				*/
%token SYM_TAG_COLGROUP					/*	colgroup		*/
%token SYM_TAG_COMMAND					/*	command			*/
%token SYM_TAG_DATALIST					/*	datalist		*/
%token SYM_TAG_DD						/*	dd				*/
%token SYM_TAG_DEL						/*	del				*/
%token SYM_TAG_DETAILS					/*	details			*/
%token SYM_TAG_DFN						/*	dfn				*/
%token SYM_TAG_DIR						/*	dir				*/
%token SYM_TAG_DIV						/*	div				*/
%token SYM_TAG_DL						/*	dl				*/
%token SYM_TAG_DT						/*	dt				*/
%token SYM_TAG_EM						/*	em				*/
%token SYM_TAG_EMBED					/*	embed			*/
%token SYM_TAG_FIELDSET					/*	fieldset		*/
%token SYM_TAG_FIGCAPTION				/*	figcaption		*/
%token SYM_TAG_FIGURE					/*	figure			*/
%token SYM_TAG_FONT						/*	font			*/
%token SYM_TAG_FOOTER					/*	footer			*/
%token SYM_TAG_FORM						/*	form			*/
%token SYM_TAG_FRAME					/*	frame			*/
%token SYM_TAG_FRAMESET					/*	frameset		*/
%token SYM_TAG_H1						/*	h1				*/
%token SYM_TAG_H2						/*	h2				*/
%token SYM_TAG_H3						/*	h3				*/
%token SYM_TAG_H4						/*	h4				*/
%token SYM_TAG_H5						/*	h5				*/
%token SYM_TAG_H6						/*	h6				*/
%token SYM_TAG_HEAD						/*	head			*/
%token SYM_TAG_HEADER					/*	header			*/
%token SYM_TAG_HGROUP					/*	hgroup			*/
%token SYM_TAG_HR						/*	hr				*/
%token SYM_TAG_HTML						/*	html			*/
%token SYM_TAG_I						/*	i				*/
%token SYM_TAG_IFRAME					/*	iframe			*/
%token SYM_TAG_IMG						/*	img				*/
%token SYM_TAG_INPUT					/*	input			*/
%token SYM_TAG_INS						/*	ins				*/
%token SYM_TAG_KEYGEN					/*	keygen			*/
%token SYM_TAG_KBD						/*	kbd				*/
%token SYM_TAG_LABEL					/*	label			*/
%token SYM_TAG_LEGEND					/*	legend			*/
%token SYM_TAG_LI						/*	li				*/
%token SYM_TAG_LINK						/*	link			*/
%token SYM_TAG_LAYER					/*	layer			*/
%token SYM_TAG_MAP						/*	map				*/
%token SYM_TAG_MARK						/*	mark			*/
%token SYM_TAG_MENU						/*	menu			*/
%token SYM_TAG_META						/*	meta			*/
%token SYM_TAG_METER					/*	meter			*/
%token SYM_TAG_NAV						/*	nav				*/
%token SYM_TAG_NOFRAMES					/*	noframes		*/
%token SYM_TAG_NOSCRIPT					/*	noscript		*/
%token SYM_TAG_OBJECT					/*	object			*/
%token SYM_TAG_OL						/*	ol				*/
%token SYM_TAG_OPTGROUP					/*	optgroup		*/
%token SYM_TAG_OPTION					/*	option			*/
%token SYM_TAG_OUTPUT					/*	output			*/
%token SYM_TAG_P						/*	p				*/
%token SYM_TAG_PARAM					/*	param			*/
%token SYM_TAG_PRE						/*	pre				*/
%token SYM_TAG_PROGRESS					/*	progress		*/
%token SYM_TAG_Q						/*	q				*/
%token SYM_TAG_RP						/*	rp				*/
%token SYM_TAG_RT						/*	rt				*/
%token SYM_TAG_RUBY						/*	ruby			*/
%token SYM_TAG_S						/*	s				*/
%token SYM_TAG_U						/*	u				*/
%token SYM_TAG_SAMP						/*	samp			*/
%token SYM_TAG_SCRIPT					/*	script			*/
%token SYM_TAG_SECTION					/*	section			*/
%token SYM_TAG_SELECT					/*	select			*/
%token SYM_TAG_SMALL					/*	small			*/
%token SYM_TAG_SOURCE					/*	source			*/
%token SYM_TAG_SPAN						/*	span			*/
%token SYM_TAG_STRIKE					/*	strike			*/
%token SYM_TAG_STRONG					/*	strong			*/
%token SYM_TAG_STYLE					/*	style			*/
%token SYM_TAG_SUB						/*	sub				*/
%token SYM_TAG_SUMMARY					/*	summary			*/
%token SYM_TAG_SUP						/*	sup				*/
%token SYM_TAG_TABLE					/*	table			*/
%token SYM_TAG_TBODY					/*	tbody			*/
%token SYM_TAG_TD						/*	td				*/
%token SYM_TAG_TH						/*	th				*/
%token SYM_TAG_TR						/*	tr				*/
%token SYM_TAG_TT						/*	tt				*/
%token SYM_TAG_TEXTAREA					/*	textarea		*/
%token SYM_TAG_TFOOT					/*	tfoot			*/
%token SYM_TAG_THEAD					/*	thead			*/
%token SYM_TAG_TIME						/*	time			*/
%token SYM_TAG_TITLE					/*	title			*/
%token SYM_TAG_UL						/*	ul				*/
%token SYM_TAG_VAR						/*	var				*/
%token SYM_TAG_VIDEO					/*	video			*/
%token SYM_TAG_XML						/*	xml				*/
%token SYM_TAG_BGSOUND					/*	bgsound			*/
%token SYM_TAG_MARQUEE					/*	marquee			*/
%token SYM_TAG_USERDATA					/*	others			*/

/*
 * for html event token
 */
%token SYM_EVT_ONAFTERPRINT				/*	onafterprint	*/
%token SYM_EVT_ONBEFOREPRINT			/*	onbeforeprint	*/
%token SYM_EVT_ONBEFOREONLOAD			/*	onbeforeonload	*/
%token SYM_EVT_ONBLUR					/*	onblur			*/
%token SYM_EVT_ONFOCUS					/*	onfocus			*/
%token SYM_EVT_ONHASCHANGE				/*	onhaschange		*/
%token SYM_EVT_ONLOAD					/*	onload			*/
%token SYM_EVT_ONMESSAGE				/*	onmessage		*/
%token SYM_EVT_ONOFFLINE				/*	onoffline		*/
%token SYM_EVT_ONONLINE					/*	ononline		*/
%token SYM_EVT_ONPAGEHIDE				/*	onpagehide		*/
%token SYM_EVT_ONPAGESHOW				/*	onpageshow		*/
%token SYM_EVT_ONPOPSTATE				/*	onpopstate		*/
%token SYM_EVT_ONREDO					/*	onredo			*/
%token SYM_EVT_ONRESIZE					/*	onresize		*/
%token SYM_EVT_ONSTORAGE				/*	onstorage		*/
%token SYM_EVT_ONUNDO					/*	onundo			*/
%token SYM_EVT_ONUNLOAD					/*	onunload		*/
%token SYM_EVT_ONCHANGE					/*	onchange		*/
%token SYM_EVT_ONCONTEXTMENU			/*	oncontextmenu	*/
%token SYM_EVT_ONFORMCHANGE				/*	onformchange	*/
%token SYM_EVT_ONFORMINPUT				/*	onforminput		*/
%token SYM_EVT_ONINPUT					/*	oninput			*/
%token SYM_EVT_ONINVALID				/*	oninvalid		*/
%token SYM_EVT_ONRESET					/*	onreset			*/
%token SYM_EVT_ONSELECT					/*	onselect		*/
%token SYM_EVT_ONSUBMIT					/*	onsubmit		*/
%token SYM_EVT_ONKEYDOWN				/*	onkeydown		*/
%token SYM_EVT_ONKEYPRESS				/*	onkeypress		*/
%token SYM_EVT_ONKEYUP					/*	onkeyup			*/
%token SYM_EVT_ONCLICK					/*	onclick			*/
%token SYM_EVT_ONDBLCLICK				/*	ondblclick		*/
%token SYM_EVT_ONDRAG					/*	ondrag			*/
%token SYM_EVT_ONDRAGEND				/*	ondragend		*/
%token SYM_EVT_ONDRAGENTER				/*	ondragenter		*/
%token SYM_EVT_ONDRAGLEAVE				/*	ondragleave		*/
%token SYM_EVT_ONDRAGOVER				/*	ondragover		*/
%token SYM_EVT_ONDRAGSTART				/*	ondragstart		*/
%token SYM_EVT_ONDROP					/*	ondrop			*/
%token SYM_EVT_ONMOUSEDOWN				/*	onmousedown		*/
%token SYM_EVT_ONMOUSEMOVE				/*	onmousemove		*/
%token SYM_EVT_ONMOUSEOUT				/*	onmouseout		*/
%token SYM_EVT_ONMOUSEOVER				/*	onmouseover		*/
%token SYM_EVT_ONMOUSEUP				/*	onmouseup		*/
%token SYM_EVT_ONMOUSEWHEEL				/*	onmousewheel	*/
%token SYM_EVT_ONSCROLL					/*	onscroll		*/
%token SYM_EVT_ONABORT					/*	onabort			*/
%token SYM_EVT_ONCANPLAY				/*	oncanplay		*/
%token SYM_EVT_ONCANPLAYTHROUGH			/*	oncanplaythrough*/
%token SYM_EVT_ONDURATIONCHANGE			/*	ondurationchange*/
%token SYM_EVT_ONEMPTIED				/*	onemptied		*/
%token SYM_EVT_ONENDED					/*	onended			*/
%token SYM_EVT_ONERROR					/*	onerror			*/
%token SYM_EVT_ONLOADEDDATA				/*	onloadeddata	*/
%token SYM_EVT_ONLOADEDMETADATA			/*	onloadedmetadata*/
%token SYM_EVT_ONLOADSTART				/*	onloadstart		*/
%token SYM_EVT_ONPAUSE					/*	onpause			*/
%token SYM_EVT_ONPLAY					/*	onplay			*/
%token SYM_EVT_ONPLAYING				/*	onplaying		*/
%token SYM_EVT_ONPROGRESS				/*	onprogress		*/
%token SYM_EVT_ONRATECHANGE				/*	onratechange	*/
%token SYM_EVT_ONREADYSTATECHANGE		/*	onreadystatechange*/
%token SYM_EVT_ONSEEKED					/*	onseeked		*/
%token SYM_EVT_ONSEEKING				/*	onseeking		*/
%token SYM_EVT_ONSTALLED				/*	onstalled		*/
%token SYM_EVT_ONSUSPEND				/*	onsuspend		*/
%token SYM_EVT_ONTIMEUPDATE				/*	ontimeupdate	*/
%token SYM_EVT_ONVOLUMECHANGE			/*	onvolumechange	*/
%token SYM_EVT_ONWAITING				/*	onwaiting		*/
%token SYM_EVT_ONSTART					/*	onstart			*/

/*
 * common prot
 */
%token SYM_PROT_ACCESSKEY				/*	accesskey		*/
%token SYM_PROT_CLASS					/*	class			*/
%token SYM_PROT_CONTENTEDITABLE			/*	contenteditable	*/
%token SYM_PROT_CONTEXTMENU				/*	contextmenu		*/
%token SYM_PROT_DIR						/*	dir				*/
%token SYM_PROT_DRAGGABLE				/*	draggable		*/
%token SYM_PROT_HIDDEN					/*	hidden			*/
%token SYM_PROT_ID						/*	id				*/
%token SYM_PROT_ITEM					/*	item			*/
%token SYM_PROT_ITEMPROP				/*	itemprop		*/
%token SYM_PROT_LANG					/*	lang			*/
%token SYM_PROT_SPELLCHECK				/*	spellcheck		*/
%token SYM_PROT_STYLE					/*	style			*/
%token SYM_PROT_SUBJECT					/*	subject			*/
%token SYM_PROT_TABINDEX				/*	tabindex		*/
%token SYM_PROT_TITLE					/*	title			*/
%token SYM_PROT_USERDATA				/*	data-'userdef'	*/
%token SYM_PROT_TEMPLATE				/*	template		*/
%token SYM_PROT_REGISTRATIONMARK		/*	registrationmark*/
%token SYM_PROT_IRRELEVANT				/*	irrelevant		*/

/*
 * tag private prop
 */
%token SYM_PROT_OPEN					/*	open			*/
%token SYM_PROT_DATA					/*	data			*/
%token SYM_PROT_NOWRAP					/*	nowrap			*/
%token SYM_PROT_DATETIME				/*	datetime		*/
%token SYM_PROT_ROWS					/*	rows			*/
%token SYM_PROT_LIST					/*	list			*/
%token SYM_PROT_FORMTARGETNEW			/*	formtargetNew	*/
%token SYM_PROT_AUTOFOCUSNEW			/*	autofocusNew	*/
%token SYM_PROT_ICON					/*	icon			*/
%token SYM_PROT_MAXLENGTH				/*	maxlength		*/
%token SYM_PROT_WIDTH					/*	width			*/
%token SYM_PROT_ARCHIVE					/*	archive			*/
%token SYM_PROT_HREF					/*	href			*/
%token SYM_PROT_PRELOAD					/*	preload			*/
%token SYM_PROT_MULTIPLE				/*	multiple		*/
%token SYM_PROT_HREFLANG				/*	hreflang		*/
%token SYM_PROT_CELLSPACING				/*	cellspacing		*/
%token SYM_PROT_COLSPAN					/*	colspan			*/
%token SYM_PROT_ACTION					/*	action			*/
%token SYM_PROT_CLASSID					/*	classid			*/
%token SYM_PROT_PATTERN					/*	pattern			*/
%token SYM_PROT_COLOR					/*	color			*/
%token SYM_PROT_HIGH					/*	high			*/
%token SYM_PROT_PING					/*	ping			*/
%token SYM_PROT_ISMAP					/*	ismap			*/
%token SYM_PROT_HTTPEQUIV				/*	html-equiv		*/
%token SYM_PROT_HSPACE					/*	hspace			*/
%token SYM_PROT_COMPACT					/*	compact			*/
%token SYM_PROT_LANGUAGE				/*	language		*/
%token SYM_PROT_REQUIRED				/*	required		*/
%token SYM_PROT_SPAN					/*	span			*/
%token SYM_PROT_FORMACTIONNEW			/*	formactionNew	*/
%token SYM_PROT_RULES					/*	rules			*/
%token SYM_PROT_AXIS					/*	axis			*/
%token SYM_PROT_METHOD					/*	method			*/
%token SYM_PROT_BGCOLOR					/*	bgcolor			*/
%token SYM_PROT_SHAPE					/*	shape			*/
%token SYM_PROT_USEMAP					/*	usemap			*/
%token SYM_PROT_FOR						/*	for				*/
%token SYM_PROT_SCOPED					/*	scoped			*/
%token SYM_PROT_FORMACTION				/*	formaction		*/
%token SYM_PROT_FORMENCTYPE				/*	formenctype		*/
%token SYM_PROT_FORMMETHOD				/*	formmethod		*/
%token SYM_PROT_FORMTARGET				/*	formtarget		*/
%token SYM_PROT_FORMNOVALIDATE			/*	fornovalidate	*/
%token SYM_PROT_FORMNOVALIDATENEW		/*	fornovalidateNew*/
%token SYM_PROT_CONTENT					/*	content			*/
%token SYM_PROT_INPUTMODE				/*	inputmode		*/
%token SYM_PROT_CITE					/*	cite			*/
%token SYM_PROT_VSPACE					/*	vspace			*/
%token SYM_PROT_XMLNS					/*	xmlns			*/
%token SYM_PROT_CODETYPE				/*	codetype		*/
%token SYM_PROT_TARGET					/*	target			*/
%token SYM_PROT_VALUE					/*	value			*/
%token SYM_PROT_AUTOFOCUS				/*	autofocus		*/
%token SYM_PROT_MEDIA					/*	media			*/
%token SYM_PROT_COORDS					/*	coords			*/
%token SYM_PROT_PROFILE					/*	profile			*/
%token SYM_PROT_HEADERS					/*	headers			*/
%token SYM_PROT_VALUETYPE				/*	valuetype		*/
%token SYM_PROT_REPLACE					/*	replace			*/
%token SYM_PROT_MARGINHEIGHT			/*	marginheight	*/
%token SYM_PROT_BORDER					/*	border			*/
%token SYM_PROT_FRAMEBORDER				/*	frameborder		*/
%token SYM_PROT_ASYNC					/*	async			*/
%token SYM_PROT_FACE					/*	face			*/
%token SYM_PROT_CELLPADDING				/*	cellpadding		*/
%token SYM_PROT_STANDBY					/*	standby			*/
%token SYM_PROT_ALT						/*	alt				*/
%token SYM_PROT_ACCEPTCHARSET			/*	accept-charset	*/
%token SYM_PROT_FORMMETHODNEW			/*	formmethodNew	*/
%token SYM_PROT_AUTOPLAY				/*	autoplay		*/
%token SYM_PROT_REV						/*	rev				*/
%token SYM_PROT_LOOP					/*	loop			*/
%token SYM_PROT_CODE					/*	code			*/
%token SYM_PROT_SRC						/*	src				*/
%token SYM_PROT_CHECKED					/*	checked			*/
%token SYM_PROT_SCROLLING				/*	scrolling		*/
%token SYM_PROT_SCOPE					/*	scope			*/
%token SYM_PROT_DEFER					/*	defer			*/
%token SYM_PROT_XMLSPACE				/*	xml:space		*/
%token SYM_PROT_CHALLENGE				/*	challenge		*/
%token SYM_PROT_SCHEME					/*	scheme			*/
%token SYM_PROT_DECLARE					/*	declare			*/
%token SYM_PROT_CHAR					/*	char			*/
%token SYM_PROT_READONLY				/*	readonly		*/
%token SYM_PROT_XMLLANG					/*	xml:lang		*/
%token SYM_PROT_MAX						/*	max				*/
%token SYM_PROT_ROWSPAN					/*	rowspan			*/
%token SYM_PROT_KEYTYPE					/*	keytype			*/
%token SYM_PROT_AUTOCOMPLETE			/*	autocomplete	*/
%token SYM_PROT_SELECTED				/*	selected		*/
%token SYM_PROT_CODEBASE				/*	codebase		*/
%token SYM_PROT_STEP					/*	step			*/
%token SYM_PROT_NOHREF					/*	nohref			*/
%token SYM_PROT_CHARSET					/*	charset			*/
%token SYM_PROT_FORMNEW					/*	formNew			*/
%token SYM_PROT_FORMENCTYPENEW			/*	formenctypeNew	*/
%token SYM_PROT_REL						/*	rel				*/
%token SYM_PROT_MIN						/*	min				*/
%token SYM_PROT_NAME					/*	name			*/
%token SYM_PROT_TYPE					/*	type			*/
%token SYM_PROT_NOSHADE					/*	noshade			*/
%token SYM_PROT_MANIFEST				/*	manifest		*/
%token SYM_PROT_ALIGN					/*	align			*/
%token SYM_PROT_HEIGHT					/*	height			*/
%token SYM_PROT_ACCEPT					/*	accept			*/
%token SYM_PROT_ENCTYPE					/*	enctype			*/
%token SYM_PROT_DISABLED				/*	disabled		*/
%token SYM_PROT_CONTROLS				/*	controls		*/
%token SYM_PROT_LONGDESC				/*	longdesc		*/
%token SYM_PROT_MARGINWIDTH				/*	marginwidth		*/
%token SYM_PROT_NORESIZE				/*	noresize		*/
%token SYM_PROT_COLS					/*	cols			*/
%token SYM_PROT_SIZE					/*	size			*/
%token SYM_PROT_RADIOGROUP				/*	radiogroup		*/
%token SYM_PROT_VALIGN					/*	valign			*/
%token SYM_PROT_OBJECT					/*	object			*/
%token SYM_PROT_CHAROFF					/*	charoff			*/
%token SYM_PROT_LOW						/*	low				*/
%token SYM_PROT_START					/*	start			*/
%token SYM_PROT_SUMMARY					/*	summary			*/
%token SYM_PROT_OPTIMUM					/*	optimum			*/
%token SYM_PROT_ABBR					/*	abbr			*/
%token SYM_PROT_FORM					/*	form			*/
%token SYM_PROT_LABEL					/*	label			*/
%token SYM_PROT_FRAME					/*	frame			*/
%token SYM_PROT_ALLOWSCRIPTACCESS		/*	AllowScriptAccess*/
%token SYM_PROT_PLACEHOLDER				/*	placeholder		*/
%token SYM_PROT_AUTOSUBMIT				/*	autosubmit		*/
%token SYM_PROT_LOWSRC					/*	lowsrc			*/
%token SYM_PROT_BACKGROUND				/*	background		*/

%union
{
	html_string_t string;
	enum html_tokentype id;
	struct html_node *node;
	struct html_node_prot *prot;
	html_node_prot_list_t prot_list;
	html_node_list_t node_list;
}

%type <string>			html_tag_event_value_ 
						html_tag_prot_value_
						html_doc_text
						SYM_TEXT
%type <id>				html_tag_name_ 
						html_tag_event_name_ 
						html_tag_prot_name_
%type <node>			html_tag
%type <node>			html_tag_begin_
%type <prot>			html_tag_prot_event_ 
%type <prot>			html_tag_prot_ 
%type <prot>			html_tag_event_
%type <prot_list>		html_tag_prot_event_list_ 
%type <node_list>		html_doc 

%destructor
{
	html_free_string(this_decoder, &$$);
}<string>

%destructor
{
	html_free_node(this_decoder, $$);
}<node>

%destructor
{
	html_free_prot(this_decoder, $$);
}<prot>

%destructor
{
	html_free_prot_list(this_decoder, &$$);
}<prot_list>

%destructor
{
	html_free_node_list(this_decoder, &$$);
}<node_list>

%debug
%verbose
%defines "html_parser.h"
%output "html_parser.c"
%define api.prefix html_
%define api.pure full
%define api.push-pull push
%parse-param {void *priv_data}

%start html_init
%%
html_init: 
	html_doc
	{
		NOCOPY_BREAK;
		TAILQ_CONCAT(&this_priv->html_root, &$1, next);
	}
	;

html_doc:
	empty
	{
		TAILQ_INIT(&$$);
	}
	| html_doc html_tag
	{
		NOCOPY_BREAK;
		
		$$ = $1;
		TAILQ_INSERT_TAIL(&$$, $2, next);
	}
	| html_doc html_doc_text
	{
		NOCOPY_BREAK;

		html_node_t *node = html_alloc_node(this_decoder);
		if(!node)
		{
			ey_html_debug(debug_html_parser, "alloc doc node failed\n");
			YYABORT;
		}

		node->type = SYM_TEXT;
		node->text = $2;

		$$ = $1;
		TAILQ_INSERT_TAIL(&$$, node, next);
	}
	;

html_doc_text: 
	SYM_TEXT
	{
		NOCOPY_BREAK;

		if(!html_alloc_string(this_decoder, $1.buf, $1.len, &$$))
		{
			ey_html_debug(debug_html_parser, "failed to duplicate doc text value\n");
			$1.buf = NULL;
			$1.len = 0;
			YYABORT;
		}
	}
	;

empty:
	{
	}
	;

html_tag:
	html_tag_begin_ html_doc html_tag_finish_
	{
		NOCOPY_BREAK;

		TAILQ_CONCAT(&$1->child, &$2, next);
		$$ = $1;
	}
	;

html_tag_begin_:
	SYM_TAG_START_FLAG html_tag_name_ html_tag_prot_event_list_ html_tag_begin_end_
	{
		NOCOPY_BREAK;

		html_node_t *node = html_alloc_node(this_decoder);
		if(!node)
		{
			ey_html_debug(debug_html_parser, "alloc tag node failed\n");
			YYABORT;
		}

		node->type = $2;
		TAILQ_CONCAT(&node->prot, &$3, next);
		$$ = node;
	}
	;

html_tag_begin_end_:
	SYM_TAG_END_FLAG
	{
	}
	| SYM_TAG_END_FLAG2
	{
	}
	;

html_tag_finish_:
	empty
	{
	}
	| SYM_TAG_START_FLAG2 html_tag_name_ html_tag_finish_end_
	{
	}
	;

html_tag_finish_end_:
	SYM_TAG_END_FLAG
	{
	}
	| SYM_TAG_END_FLAG2
	{
	}
	;

html_tag_prot_event_list_:
	empty
	{
		TAILQ_INIT(&$$);
	}
	| html_tag_prot_event_list_ html_tag_prot_event_
	{
		NOCOPY_BREAK;

		$$ = $1;
		TAILQ_INSERT_TAIL(&$$, $2, next);
	}
	;

html_tag_prot_event_:
	html_tag_prot_
	{
		$$ = $1;
	}
	| html_tag_event_
	{
		$$ = $1;
	}
	;

html_tag_prot_:
	html_tag_prot_name_ SYM_EQUAL html_tag_prot_value_
	{
		NOCOPY_BREAK;

		html_node_prot_t *prot = html_alloc_prot(this_decoder);
		if(!prot)
		{
			ey_html_debug(debug_html_parser, "alloc node prot failed\n");
			YYABORT;
		}
		prot->type = $1;
		prot->value = $3;
		$$ = prot;
	}
	| html_tag_prot_name_
	{
		NOCOPY_BREAK;

		html_node_prot_t *prot = html_alloc_prot(this_decoder);
		if(!prot)
		{
			ey_html_debug(debug_html_parser, "alloc node prot failed\n");
			YYABORT;
		}
		prot->type = $1;
		$$ = prot;
	}
	;

html_tag_prot_value_:
	SYM_TEXT
	{
		NOCOPY_BREAK;

		if(!html_alloc_string(this_decoder, $1.buf, $1.len, &$$))
		{
			ey_html_debug(debug_html_parser, "failed to duplicate prot value\n");
			$1.buf = NULL;
			$1.len = 0;
			YYABORT;
		}
	}
	;

html_tag_event_:
	html_tag_event_name_ SYM_EQUAL html_tag_event_value_
	{
		NOCOPY_BREAK;

		html_node_prot_t *prot = html_alloc_prot(this_decoder);
		if(!prot)
		{
			ey_html_debug(debug_html_parser, "alloc node prot failed\n");
			YYABORT;
		}
		prot->type = $1;
		prot->value = $3;
		$$ = prot;
	}
	| html_tag_event_name_
	{
		NOCOPY_BREAK;

		html_node_prot_t *prot = html_alloc_prot(this_decoder);
		if(!prot)
		{
			ey_html_debug(debug_html_parser, "alloc node prot failed\n");
			YYABORT;
		}
		prot->type = $1;
		$$ = prot;
	}
	;

html_tag_event_value_:
	SYM_TEXT
	{
		NOCOPY_BREAK;
		
		if(!html_alloc_string(this_decoder, $1.buf, $1.len, &$$))
		{
			ey_html_debug(debug_html_parser, "failed to duplicate event value\n");
			$1.buf = NULL;
			$1.len = 0;
			YYABORT;
		}
	}
	;

html_tag_event_name_:
	SYM_EVT_ONAFTERPRINT
	{
		$$ = SYM_EVT_ONAFTERPRINT;
	}
	| SYM_EVT_ONBEFOREPRINT
	{
		$$ = SYM_EVT_ONBEFOREPRINT;
	}
	| SYM_EVT_ONBEFOREONLOAD
	{
		$$ = SYM_EVT_ONBEFOREONLOAD;
	}
	| SYM_EVT_ONBLUR
	{
		$$ = SYM_EVT_ONBLUR;
	}
	| SYM_EVT_ONFOCUS
	{
		$$ = SYM_EVT_ONFOCUS;
	}
	| SYM_EVT_ONHASCHANGE
	{
		$$ = SYM_EVT_ONHASCHANGE;
	}
	| SYM_EVT_ONLOAD
	{
		$$ = SYM_EVT_ONLOAD;
	}
	| SYM_EVT_ONMESSAGE
	{
		$$ = SYM_EVT_ONMESSAGE;
	}
	| SYM_EVT_ONOFFLINE
	{
		$$ = SYM_EVT_ONOFFLINE;
	}
	| SYM_EVT_ONONLINE
	{
		$$ = SYM_EVT_ONONLINE;
	}
	| SYM_EVT_ONPAGEHIDE
	{
		$$ = SYM_EVT_ONPAGEHIDE;
	}
	| SYM_EVT_ONPAGESHOW
	{
		$$ = SYM_EVT_ONPAGESHOW;
	}
	| SYM_EVT_ONPOPSTATE
	{
		$$ = SYM_EVT_ONPOPSTATE;
	}
	| SYM_EVT_ONREDO
	{
		$$ = SYM_EVT_ONREDO;
	}
	| SYM_EVT_ONRESIZE
	{
		$$ = SYM_EVT_ONRESIZE;
	}
	| SYM_EVT_ONSTORAGE
	{
		$$ = SYM_EVT_ONSTORAGE;
	}
	| SYM_EVT_ONUNDO
	{
		$$ = SYM_EVT_ONUNDO;
	}
	| SYM_EVT_ONUNLOAD
	{
		$$ = SYM_EVT_ONUNLOAD;
	}
	| SYM_EVT_ONCHANGE
	{
		$$ = SYM_EVT_ONCHANGE;
	}
	| SYM_EVT_ONCONTEXTMENU
	{
		$$ = SYM_EVT_ONCONTEXTMENU;
	}
	| SYM_EVT_ONFORMCHANGE
	{
		$$ = SYM_EVT_ONFORMCHANGE;
	}
	| SYM_EVT_ONFORMINPUT
	{
		$$ = SYM_EVT_ONFORMINPUT;
	}
	| SYM_EVT_ONINPUT
	{
		$$ = SYM_EVT_ONINPUT;
	}
	| SYM_EVT_ONINVALID
	{
		$$ = SYM_EVT_ONINVALID;
	}
	| SYM_EVT_ONRESET
	{
		$$ = SYM_EVT_ONRESET;
	}
	| SYM_EVT_ONSELECT
	{
		$$ = SYM_EVT_ONSELECT;
	}
	| SYM_EVT_ONSUBMIT
	{
		$$ = SYM_EVT_ONSUBMIT;
	}
	| SYM_EVT_ONKEYDOWN
	{
		$$ = SYM_EVT_ONKEYDOWN;
	}
	| SYM_EVT_ONKEYPRESS
	{
		$$ = SYM_EVT_ONKEYPRESS;
	}
	| SYM_EVT_ONKEYUP
	{
		$$ = SYM_EVT_ONKEYUP;
	}
	| SYM_EVT_ONCLICK
	{
		$$ = SYM_EVT_ONCLICK;
	}
	| SYM_EVT_ONDBLCLICK
	{
		$$ = SYM_EVT_ONDBLCLICK;
	}
	| SYM_EVT_ONDRAG
	{
		$$ = SYM_EVT_ONDRAG;
	}
	| SYM_EVT_ONDRAGEND
	{
		$$ = SYM_EVT_ONDRAGEND;
	}
	| SYM_EVT_ONDRAGENTER
	{
		$$ = SYM_EVT_ONDRAGENTER;
	}
	| SYM_EVT_ONDRAGLEAVE
	{
		$$ = SYM_EVT_ONDRAGLEAVE;
	}
	| SYM_EVT_ONDRAGOVER
	{
		$$ = SYM_EVT_ONDRAGOVER;
	}
	| SYM_EVT_ONDRAGSTART
	{
		$$ = SYM_EVT_ONDRAGSTART;
	}
	| SYM_EVT_ONDROP
	{
		$$ = SYM_EVT_ONDROP;
	}
	| SYM_EVT_ONMOUSEDOWN
	{
		$$ = SYM_EVT_ONMOUSEDOWN;
	}
	| SYM_EVT_ONMOUSEMOVE
	{
		$$ = SYM_EVT_ONMOUSEMOVE;
	}
	| SYM_EVT_ONMOUSEOUT
	{
		$$ = SYM_EVT_ONMOUSEOUT;
	}
	| SYM_EVT_ONMOUSEOVER
	{
		$$ = SYM_EVT_ONMOUSEOVER;
	}
	| SYM_EVT_ONMOUSEUP
	{
		$$ = SYM_EVT_ONMOUSEUP;
	}
	| SYM_EVT_ONMOUSEWHEEL
	{
		$$ = SYM_EVT_ONMOUSEWHEEL;
	}
	| SYM_EVT_ONSCROLL
	{
		$$ = SYM_EVT_ONSCROLL;
	}
	| SYM_EVT_ONABORT
	{
		$$ = SYM_EVT_ONABORT;
	}
	| SYM_EVT_ONCANPLAY
	{
		$$ = SYM_EVT_ONCANPLAY;
	}
	| SYM_EVT_ONCANPLAYTHROUGH
	{
		$$ = SYM_EVT_ONCANPLAYTHROUGH;
	}
	| SYM_EVT_ONDURATIONCHANGE
	{
		$$ = SYM_EVT_ONDURATIONCHANGE;
	}
	| SYM_EVT_ONEMPTIED
	{
		$$ = SYM_EVT_ONEMPTIED;
	}
	| SYM_EVT_ONENDED
	{
		$$ = SYM_EVT_ONENDED;
	}
	| SYM_EVT_ONERROR
	{
		$$ = SYM_EVT_ONERROR;
	}
	| SYM_EVT_ONLOADEDDATA
	{
		$$ = SYM_EVT_ONLOADEDDATA;
	}
	| SYM_EVT_ONLOADEDMETADATA
	{
		$$ = SYM_EVT_ONLOADEDMETADATA;
	}
	| SYM_EVT_ONLOADSTART
	{
		$$ = SYM_EVT_ONLOADSTART;
	}
	| SYM_EVT_ONPAUSE
	{
		$$ = SYM_EVT_ONPAUSE;
	}
	| SYM_EVT_ONPLAY
	{
		$$ = SYM_EVT_ONPLAY;
	}
	| SYM_EVT_ONPLAYING
	{
		$$ = SYM_EVT_ONPLAYING;
	}
	| SYM_EVT_ONPROGRESS
	{
		$$ = SYM_EVT_ONPROGRESS;
	}
	| SYM_EVT_ONRATECHANGE
	{
		$$ = SYM_EVT_ONRATECHANGE;
	}
	| SYM_EVT_ONREADYSTATECHANGE
	{
		$$ = SYM_EVT_ONREADYSTATECHANGE;
	}
	| SYM_EVT_ONSEEKED
	{
		$$ = SYM_EVT_ONSEEKED;
	}
	| SYM_EVT_ONSEEKING
	{
		$$ = SYM_EVT_ONSEEKING;
	}
	| SYM_EVT_ONSTALLED
	{
		$$ = SYM_EVT_ONSTALLED;
	}
	| SYM_EVT_ONSUSPEND
	{
		$$ = SYM_EVT_ONSUSPEND;
	}
	| SYM_EVT_ONTIMEUPDATE
	{
		$$ = SYM_EVT_ONTIMEUPDATE;
	}
	| SYM_EVT_ONVOLUMECHANGE
	{
		$$ = SYM_EVT_ONVOLUMECHANGE;
	}
	| SYM_EVT_ONWAITING
	{
		$$ = SYM_EVT_ONWAITING;
	}
	| SYM_EVT_ONSTART
	{
		$$ = SYM_EVT_ONSTART;
	}
	;

html_tag_name_:
	SYM_TAG_A
	{
		$$ = SYM_TAG_A;
	}
	| SYM_TAG_ABBR
	{
		$$ = SYM_TAG_ABBR;
	}
	| SYM_TAG_ACRONYM
	{
		$$ = SYM_TAG_ACRONYM;
	}
	| SYM_TAG_ADDRESS
	{
		$$ = SYM_TAG_ADDRESS;
	}
	| SYM_TAG_APPLET
	{
		$$ = SYM_TAG_APPLET;
	}
	| SYM_TAG_AREA
	{
		$$ = SYM_TAG_AREA;
	}
	| SYM_TAG_ARTICLE
	{
		$$ = SYM_TAG_ARTICLE;
	}
	| SYM_TAG_ASIDE
	{
		$$ = SYM_TAG_ASIDE;
	}
	| SYM_TAG_AUDIO
	{
		$$ = SYM_TAG_AUDIO;
	}
	| SYM_TAG_B
	{
		$$ = SYM_TAG_B;
	}
	| SYM_TAG_BASE
	{
		$$ = SYM_TAG_BASE;
	}
	| SYM_TAG_BASEFONT
	{
		$$ = SYM_TAG_BASEFONT;
	}
	| SYM_TAG_BDI
	{
		$$ = SYM_TAG_BDI;
	}
	| SYM_TAG_BDO
	{
		$$ = SYM_TAG_BDO;
	}
	| SYM_TAG_BIG
	{
		$$ = SYM_TAG_BIG;
	}
	| SYM_TAG_BLOCKQUOTE
	{
		$$ = SYM_TAG_BLOCKQUOTE;
	}
	| SYM_TAG_BODY
	{
		$$ = SYM_TAG_BODY;
	}
	| SYM_TAG_BR
	{
		$$ = SYM_TAG_BR;
	}
	| SYM_TAG_BUTTON
	{
		$$ = SYM_TAG_BUTTON;
	}
	| SYM_TAG_CANVAS
	{
		$$ = SYM_TAG_CANVAS;
	}
	| SYM_TAG_CAPTION
	{
		$$ = SYM_TAG_CAPTION;
	}
	| SYM_TAG_CENTER
	{
		$$ = SYM_TAG_CENTER;
	}
	| SYM_TAG_CITE
	{
		$$ = SYM_TAG_CITE;
	}
	| SYM_TAG_CODE
	{
		$$ = SYM_TAG_CODE;
	}
	| SYM_TAG_COL
	{
		$$ = SYM_TAG_COL;
	}
	| SYM_TAG_COLGROUP
	{
		$$ = SYM_TAG_COLGROUP;
	}
	| SYM_TAG_COMMAND
	{
		$$ = SYM_TAG_COMMAND;
	}
	| SYM_TAG_DATALIST
	{
		$$ = SYM_TAG_DATALIST;
	}
	| SYM_TAG_DD
	{
		$$ = SYM_TAG_DD;
	}
	| SYM_TAG_DEL
	{
		$$ = SYM_TAG_DEL;
	}
	| SYM_TAG_DETAILS
	{
		$$ = SYM_TAG_DETAILS;
	}
	| SYM_TAG_DFN
	{
		$$ = SYM_TAG_DFN;
	}
	| SYM_TAG_DIR
	{
		$$ = SYM_TAG_DIR;
	}
	| SYM_TAG_DIV
	{
		$$ = SYM_TAG_DIV;
	}
	| SYM_TAG_DL
	{
		$$ = SYM_TAG_DL;
	}
	| SYM_TAG_DT
	{
		$$ = SYM_TAG_DT;
	}
	| SYM_TAG_EM
	{
		$$ = SYM_TAG_EM;
	}
	| SYM_TAG_EMBED
	{
		$$ = SYM_TAG_EMBED;
	}
	| SYM_TAG_FIELDSET
	{
		$$ = SYM_TAG_FIELDSET;
	}
	| SYM_TAG_FIGCAPTION
	{
		$$ = SYM_TAG_FIGCAPTION;
	}
	| SYM_TAG_FIGURE
	{
		$$ = SYM_TAG_FIGURE;
	}
	| SYM_TAG_FONT
	{
		$$ = SYM_TAG_FONT;
	}
	| SYM_TAG_FOOTER
	{
		$$ = SYM_TAG_FOOTER;
	}
	| SYM_TAG_FORM
	{
		$$ = SYM_TAG_FORM;
	}
	| SYM_TAG_FRAME
	{
		$$ = SYM_TAG_FRAME;
	}
	| SYM_TAG_FRAMESET
	{
		$$ = SYM_TAG_FRAMESET;
	}
	| SYM_TAG_H1
	{
		$$ = SYM_TAG_H1;
	}
	| SYM_TAG_H2
	{
		$$ = SYM_TAG_H2;
	}
	| SYM_TAG_H3
	{
		$$ = SYM_TAG_H3;
	}
	| SYM_TAG_H4
	{
		$$ = SYM_TAG_H4;
	}
	| SYM_TAG_H5
	{
		$$ = SYM_TAG_H5;
	}
	| SYM_TAG_H6
	{
		$$ = SYM_TAG_H6;
	}
	| SYM_TAG_HEAD
	{
		$$ = SYM_TAG_HEAD;
	}
	| SYM_TAG_HEADER
	{
		$$ = SYM_TAG_HEADER;
	}
	| SYM_TAG_HGROUP
	{
		$$ = SYM_TAG_HGROUP;
	}
	| SYM_TAG_HR
	{
		$$ = SYM_TAG_HR;
	}
	| SYM_TAG_HTML
	{
		$$ = SYM_TAG_HTML;
	}
	| SYM_TAG_I
	{
		$$ = SYM_TAG_I;
	}
	| SYM_TAG_IFRAME
	{
		$$ = SYM_TAG_IFRAME;
	}
	| SYM_TAG_IMG
	{
		$$ = SYM_TAG_IMG;
	}
	| SYM_TAG_INPUT
	{
		$$ = SYM_TAG_INPUT;
	}
	| SYM_TAG_INS
	{
		$$ = SYM_TAG_INS;
	}
	| SYM_TAG_KEYGEN
	{
		$$ = SYM_TAG_KEYGEN;
	}
	| SYM_TAG_KBD
	{
		$$ = SYM_TAG_KBD;
	}
	| SYM_TAG_LABEL
	{
		$$ = SYM_TAG_LABEL;
	}
	| SYM_TAG_LEGEND
	{
		$$ = SYM_TAG_LEGEND;
	}
	| SYM_TAG_LI
	{
		$$ = SYM_TAG_LI;
	}
	| SYM_TAG_LINK
	{
		$$ = SYM_TAG_LINK;
	}
	| SYM_TAG_LAYER
	{
		$$ = SYM_TAG_LAYER;
	}
	| SYM_TAG_MAP
	{
		$$ = SYM_TAG_MAP;
	}
	| SYM_TAG_MARK
	{
		$$ = SYM_TAG_MARK;
	}
	| SYM_TAG_MENU
	{
		$$ = SYM_TAG_MENU;
	}
	| SYM_TAG_META
	{
		$$ = SYM_TAG_META;
	}
	| SYM_TAG_METER
	{
		$$ = SYM_TAG_METER;
	}
	| SYM_TAG_NAV
	{
		$$ = SYM_TAG_NAV;
	}
	| SYM_TAG_NOFRAMES
	{
		$$ = SYM_TAG_NOFRAMES;
	}
	| SYM_TAG_NOSCRIPT
	{
		$$ = SYM_TAG_NOSCRIPT;
	}
	| SYM_TAG_OBJECT
	{
		$$ = SYM_TAG_OBJECT;
	}
	| SYM_TAG_OL
	{
		$$ = SYM_TAG_OL;
	}
	| SYM_TAG_OPTGROUP
	{
		$$ = SYM_TAG_OPTGROUP;
	}
	| SYM_TAG_OPTION
	{
		$$ = SYM_TAG_OPTION;
	}
	| SYM_TAG_OUTPUT
	{
		$$ = SYM_TAG_OUTPUT;
	}
	| SYM_TAG_P
	{
		$$ = SYM_TAG_P;
	}
	| SYM_TAG_PARAM
	{
		$$ = SYM_TAG_PARAM;
	}
	| SYM_TAG_PRE
	{
		$$ = SYM_TAG_PRE;
	}
	| SYM_TAG_PROGRESS
	{
		$$ = SYM_TAG_PROGRESS;
	}
	| SYM_TAG_Q
	{
		$$ = SYM_TAG_Q;
	}
	| SYM_TAG_RP
	{
		$$ = SYM_TAG_RP;
	}
	| SYM_TAG_RT
	{
		$$ = SYM_TAG_RT;
	}
	| SYM_TAG_RUBY
	{
		$$ = SYM_TAG_RUBY;
	}
	| SYM_TAG_S
	{
		$$ = SYM_TAG_S;
	}
	| SYM_TAG_U
	{
		$$ = SYM_TAG_U;
	}
	| SYM_TAG_SAMP
	{
		$$ = SYM_TAG_SAMP;
	}
	| SYM_TAG_SCRIPT
	{
		$$ = SYM_TAG_SCRIPT;
	}
	| SYM_TAG_SECTION
	{
		$$ = SYM_TAG_SECTION;
	}
	| SYM_TAG_SELECT
	{
		$$ = SYM_TAG_SELECT;
	}
	| SYM_TAG_SMALL
	{
		$$ = SYM_TAG_SMALL;
	}
	| SYM_TAG_SOURCE
	{
		$$ = SYM_TAG_SOURCE;
	}
	| SYM_TAG_SPAN
	{
		$$ = SYM_TAG_SPAN;
	}
	| SYM_TAG_STRIKE
	{
		$$ = SYM_TAG_STRIKE;
	}
	| SYM_TAG_STRONG
	{
		$$ = SYM_TAG_STRONG;
	}
	| SYM_TAG_STYLE
	{
		$$ = SYM_TAG_STYLE;
	}
	| SYM_TAG_SUB
	{
		$$ = SYM_TAG_SUB;
	}
	| SYM_TAG_SUMMARY
	{
		$$ = SYM_TAG_SUMMARY;
	}
	| SYM_TAG_SUP
	{
		$$ = SYM_TAG_SUP;
	}
	| SYM_TAG_TABLE
	{
		$$ = SYM_TAG_TABLE;
	}
	| SYM_TAG_TBODY
	{
		$$ = SYM_TAG_TBODY;
	}
	| SYM_TAG_TD
	{
		$$ = SYM_TAG_TD;
	}
	| SYM_TAG_TH
	{
		$$ = SYM_TAG_TH;
	}
	| SYM_TAG_TR
	{
		$$ = SYM_TAG_TR;
	}
	| SYM_TAG_TT
	{
		$$ = SYM_TAG_TT;
	}
	| SYM_TAG_TEXTAREA
	{
		$$ = SYM_TAG_TEXTAREA;
	}
	| SYM_TAG_TFOOT
	{
		$$ = SYM_TAG_TFOOT;
	}
	| SYM_TAG_THEAD
	{
		$$ = SYM_TAG_THEAD;
	}
	| SYM_TAG_TIME
	{
		$$ = SYM_TAG_TIME;
	}
	| SYM_TAG_TITLE
	{
		$$ = SYM_TAG_TITLE;
	}
	| SYM_TAG_UL
	{
		$$ = SYM_TAG_UL;
	}
	| SYM_TAG_VAR
	{
		$$ = SYM_TAG_VAR;
	}
	| SYM_TAG_VIDEO
	{
		$$ = SYM_TAG_VIDEO;
	}
	| SYM_TAG_XML
	{
		$$ = SYM_TAG_XML;
	}
	| SYM_TAG_BGSOUND
	{
		$$ = SYM_TAG_BGSOUND;
	}
	| SYM_TAG_MARQUEE
	{
		$$ = SYM_TAG_MARQUEE;
	}
	| SYM_TAG_USERDATA
	{
		$$ = SYM_TAG_USERDATA;
	}
	;

html_tag_prot_name_:
	SYM_PROT_ACCESSKEY
	{
		$$ = SYM_PROT_ACCESSKEY;
	}
	| SYM_PROT_CLASS
	{
		$$ = SYM_PROT_CLASS;
	}
	| SYM_PROT_CONTENTEDITABLE
	{
		$$ = SYM_PROT_CONTENTEDITABLE;
	}
	| SYM_PROT_CONTEXTMENU
	{
		$$ = SYM_PROT_CONTEXTMENU;
	}
	| SYM_PROT_DIR
	{
		$$ = SYM_PROT_DIR;
	}
	| SYM_PROT_DRAGGABLE
	{
		$$ = SYM_PROT_DRAGGABLE;
	}
	| SYM_PROT_HIDDEN
	{
		$$ = SYM_PROT_HIDDEN;
	}
	| SYM_PROT_ID
	{
		$$ = SYM_PROT_ID;
	}
	| SYM_PROT_ITEM
	{
		$$ = SYM_PROT_ITEM;
	}
	| SYM_PROT_ITEMPROP
	{
		$$ = SYM_PROT_ITEMPROP;
	}
	| SYM_PROT_LANG
	{
		$$ = SYM_PROT_LANG;
	}
	| SYM_PROT_SPELLCHECK
	{
		$$ = SYM_PROT_SPELLCHECK;
	}
	| SYM_PROT_STYLE
	{
		$$ = SYM_PROT_STYLE;
	}
	| SYM_PROT_SUBJECT
	{
		$$ = SYM_PROT_SUBJECT;
	}
	| SYM_PROT_TABINDEX
	{
		$$ = SYM_PROT_TABINDEX;
	}
	| SYM_PROT_TITLE
	{
		$$ = SYM_PROT_TITLE;
	}
	| SYM_PROT_USERDATA
	{
		$$ = SYM_PROT_USERDATA;
	}
	| SYM_PROT_TEMPLATE
	{
		$$ = SYM_PROT_TEMPLATE;
	}
	| SYM_PROT_REGISTRATIONMARK
	{
		$$ = SYM_PROT_REGISTRATIONMARK;
	}
	| SYM_PROT_IRRELEVANT
	{
		$$ = SYM_PROT_IRRELEVANT;
	}
	| SYM_PROT_OPEN
	{
		$$ = SYM_PROT_OPEN;
	}
	| SYM_PROT_DATA
	{
		$$ = SYM_PROT_DATA;
	}
	| SYM_PROT_NOWRAP
	{
		$$ = SYM_PROT_NOWRAP;
	}
	| SYM_PROT_DATETIME
	{
		$$ = SYM_PROT_DATETIME;
	}
	| SYM_PROT_ROWS
	{
		$$ = SYM_PROT_ROWS;
	}
	| SYM_PROT_LIST
	{
		$$ = SYM_PROT_LIST;
	}
	| SYM_PROT_FORMTARGETNEW
	{
		$$ = SYM_PROT_FORMTARGETNEW;
	}
	| SYM_PROT_AUTOFOCUSNEW
	{
		$$ = SYM_PROT_AUTOFOCUSNEW;
	}
	| SYM_PROT_ICON
	{
		$$ = SYM_PROT_ICON;
	}
	| SYM_PROT_MAXLENGTH
	{
		$$ = SYM_PROT_MAXLENGTH;
	}
	| SYM_PROT_WIDTH
	{
		$$ = SYM_PROT_WIDTH;
	}
	| SYM_PROT_ARCHIVE
	{
		$$ = SYM_PROT_ARCHIVE;
	}
	| SYM_PROT_HREF
	{
		$$ = SYM_PROT_HREF;
	}
	| SYM_PROT_PRELOAD
	{
		$$ = SYM_PROT_PRELOAD;
	}
	| SYM_PROT_MULTIPLE
	{
		$$ = SYM_PROT_MULTIPLE;
	}
	| SYM_PROT_HREFLANG
	{
		$$ = SYM_PROT_HREFLANG;
	}
	| SYM_PROT_CELLSPACING
	{
		$$ = SYM_PROT_CELLSPACING;
	}
	| SYM_PROT_COLSPAN
	{
		$$ = SYM_PROT_COLSPAN;
	}
	| SYM_PROT_ACTION
	{
		$$ = SYM_PROT_ACTION;
	}
	| SYM_PROT_CLASSID
	{
		$$ = SYM_PROT_CLASSID;
	}
	| SYM_PROT_PATTERN
	{
		$$ = SYM_PROT_PATTERN;
	}
	| SYM_PROT_COLOR
	{
		$$ = SYM_PROT_COLOR;
	}
	| SYM_PROT_HIGH
	{
		$$ = SYM_PROT_HIGH;
	}
	| SYM_PROT_PING
	{
		$$ = SYM_PROT_PING;
	}
	| SYM_PROT_ISMAP
	{
		$$ = SYM_PROT_ISMAP;
	}
	| SYM_PROT_HTTPEQUIV
	{
		$$ = SYM_PROT_HTTPEQUIV;
	}
	| SYM_PROT_HSPACE
	{
		$$ = SYM_PROT_HSPACE;
	}
	| SYM_PROT_COMPACT
	{
		$$ = SYM_PROT_COMPACT;
	}
	| SYM_PROT_LANGUAGE
	{
		$$ = SYM_PROT_LANGUAGE;
	}
	| SYM_PROT_REQUIRED
	{
		$$ = SYM_PROT_REQUIRED;
	}
	| SYM_PROT_SPAN
	{
		$$ = SYM_PROT_SPAN;
	}
	| SYM_PROT_FORMACTIONNEW
	{
		$$ = SYM_PROT_FORMACTIONNEW;
	}
	| SYM_PROT_RULES
	{
		$$ = SYM_PROT_RULES;
	}
	| SYM_PROT_AXIS
	{
		$$ = SYM_PROT_AXIS;
	}
	| SYM_PROT_METHOD
	{
		$$ = SYM_PROT_METHOD;
	}
	| SYM_PROT_BGCOLOR
	{
		$$ = SYM_PROT_BGCOLOR;
	}
	| SYM_PROT_SHAPE
	{
		$$ = SYM_PROT_SHAPE;
	}
	| SYM_PROT_USEMAP
	{
		$$ = SYM_PROT_USEMAP;
	}
	| SYM_PROT_FOR
	{
		$$ = SYM_PROT_FOR;
	}
	| SYM_PROT_SCOPED
	{
		$$ = SYM_PROT_SCOPED;
	}
	| SYM_PROT_FORMENCTYPE
	{
		$$ = SYM_PROT_FORMENCTYPE;
	}
	| SYM_PROT_FORMTARGET
	{
		$$ = SYM_PROT_FORMTARGET;
	}
	| SYM_PROT_FORMMETHOD
	{
		$$ = SYM_PROT_FORMMETHOD;
	}
	| SYM_PROT_FORMACTION
	{
		$$ = SYM_PROT_FORMACTION;
	}
	| SYM_PROT_FORMNOVALIDATE
	{
		$$ = SYM_PROT_FORMNOVALIDATE;
	}
	| SYM_PROT_FORMNOVALIDATENEW
	{
		$$ = SYM_PROT_FORMNOVALIDATENEW;
	}
	| SYM_PROT_CONTENT
	{
		$$ = SYM_PROT_CONTENT;
	}
	| SYM_PROT_INPUTMODE
	{
		$$ = SYM_PROT_INPUTMODE;
	}
	| SYM_PROT_CITE
	{
		$$ = SYM_PROT_CITE;
	}
	| SYM_PROT_VSPACE
	{
		$$ = SYM_PROT_VSPACE;
	}
	| SYM_PROT_XMLNS
	{
		$$ = SYM_PROT_XMLNS;
	}
	| SYM_PROT_CODETYPE
	{
		$$ = SYM_PROT_CODETYPE;
	}
	| SYM_PROT_TARGET
	{
		$$ = SYM_PROT_TARGET;
	}
	| SYM_PROT_VALUE
	{
		$$ = SYM_PROT_VALUE;
	}
	| SYM_PROT_AUTOFOCUS
	{
		$$ = SYM_PROT_AUTOFOCUS;
	}
	| SYM_PROT_MEDIA
	{
		$$ = SYM_PROT_MEDIA;
	}
	| SYM_PROT_COORDS
	{
		$$ = SYM_PROT_COORDS;
	}
	| SYM_PROT_PROFILE
	{
		$$ = SYM_PROT_PROFILE;
	}
	| SYM_PROT_HEADERS
	{
		$$ = SYM_PROT_HEADERS;
	}
	| SYM_PROT_VALUETYPE
	{
		$$ = SYM_PROT_VALUETYPE;
	}
	| SYM_PROT_REPLACE
	{
		$$ = SYM_PROT_REPLACE;
	}
	| SYM_PROT_MARGINHEIGHT
	{
		$$ = SYM_PROT_MARGINHEIGHT;
	}
	| SYM_PROT_BORDER
	{
		$$ = SYM_PROT_BORDER;
	}
	| SYM_PROT_FRAMEBORDER
	{
		$$ = SYM_PROT_FRAMEBORDER;
	}
	| SYM_PROT_ASYNC
	{
		$$ = SYM_PROT_ASYNC;
	}
	| SYM_PROT_FACE
	{
		$$ = SYM_PROT_FACE;
	}
	| SYM_PROT_CELLPADDING
	{
		$$ = SYM_PROT_CELLPADDING;
	}
	| SYM_PROT_STANDBY
	{
		$$ = SYM_PROT_STANDBY;
	}
	| SYM_PROT_ALT
	{
		$$ = SYM_PROT_ALT;
	}
	| SYM_PROT_ACCEPTCHARSET
	{
		$$ = SYM_PROT_ACCEPTCHARSET;
	}
	| SYM_PROT_FORMMETHODNEW
	{
		$$ = SYM_PROT_FORMMETHODNEW;
	}
	| SYM_PROT_AUTOPLAY
	{
		$$ = SYM_PROT_AUTOPLAY;
	}
	| SYM_PROT_REV
	{
		$$ = SYM_PROT_REV;
	}
	| SYM_PROT_LOOP
	{
		$$ = SYM_PROT_LOOP;
	}
	| SYM_PROT_CODE
	{
		$$ = SYM_PROT_CODE;
	}
	| SYM_PROT_SRC
	{
		$$ = SYM_PROT_SRC;
	}
	| SYM_PROT_CHECKED
	{
		$$ = SYM_PROT_CHECKED;
	}
	| SYM_PROT_SCROLLING
	{
		$$ = SYM_PROT_SCROLLING;
	}
	| SYM_PROT_SCOPE
	{
		$$ = SYM_PROT_SCOPE;
	}
	| SYM_PROT_DEFER
	{
		$$ = SYM_PROT_DEFER;
	}
	| SYM_PROT_XMLSPACE
	{
		$$ = SYM_PROT_XMLSPACE;
	}
	| SYM_PROT_CHALLENGE
	{
		$$ = SYM_PROT_CHALLENGE;
	}
	| SYM_PROT_SCHEME
	{
		$$ = SYM_PROT_SCHEME;
	}
	| SYM_PROT_DECLARE
	{
		$$ = SYM_PROT_DECLARE;
	}
	| SYM_PROT_CHAR
	{
		$$ = SYM_PROT_CHAR;
	}
	| SYM_PROT_READONLY
	{
		$$ = SYM_PROT_READONLY;
	}
	| SYM_PROT_XMLLANG
	{
		$$ = SYM_PROT_XMLLANG;
	}
	| SYM_PROT_MAX
	{
		$$ = SYM_PROT_MAX;
	}
	| SYM_PROT_ROWSPAN
	{
		$$ = SYM_PROT_ROWSPAN;
	}
	| SYM_PROT_KEYTYPE
	{
		$$ = SYM_PROT_KEYTYPE;
	}
	| SYM_PROT_AUTOCOMPLETE
	{
		$$ = SYM_PROT_AUTOCOMPLETE;
	}
	| SYM_PROT_SELECTED
	{
		$$ = SYM_PROT_SELECTED;
	}
	| SYM_PROT_CODEBASE
	{
		$$ = SYM_PROT_CODEBASE;
	}
	| SYM_PROT_STEP
	{
		$$ = SYM_PROT_STEP;
	}
	| SYM_PROT_NOHREF
	{
		$$ = SYM_PROT_NOHREF;
	}
	| SYM_PROT_CHARSET
	{
		$$ = SYM_PROT_CHARSET;
	}
	| SYM_PROT_FORMNEW
	{
		$$ = SYM_PROT_FORMNEW;
	}
	| SYM_PROT_FORMENCTYPENEW
	{
		$$ = SYM_PROT_FORMENCTYPENEW;
	}
	| SYM_PROT_REL
	{
		$$ = SYM_PROT_REL;
	}
	| SYM_PROT_MIN
	{
		$$ = SYM_PROT_MIN;
	}
	| SYM_PROT_NAME
	{
		$$ = SYM_PROT_NAME;
	}
	| SYM_PROT_TYPE
	{
		$$ = SYM_PROT_TYPE;
	}
	| SYM_PROT_NOSHADE
	{
		$$ = SYM_PROT_NOSHADE;
	}
	| SYM_PROT_MANIFEST
	{
		$$ = SYM_PROT_MANIFEST;
	}
	| SYM_PROT_ALIGN
	{
		$$ = SYM_PROT_ALIGN;
	}
	| SYM_PROT_HEIGHT
	{
		$$ = SYM_PROT_HEIGHT;
	}
	| SYM_PROT_ACCEPT
	{
		$$ = SYM_PROT_ACCEPT;
	}
	| SYM_PROT_ENCTYPE
	{
		$$ = SYM_PROT_ENCTYPE;
	}
	| SYM_PROT_DISABLED
	{
		$$ = SYM_PROT_DISABLED;
	}
	| SYM_PROT_CONTROLS
	{
		$$ = SYM_PROT_CONTROLS;
	}
	| SYM_PROT_LONGDESC
	{
		$$ = SYM_PROT_LONGDESC;
	}
	| SYM_PROT_MARGINWIDTH
	{
		$$ = SYM_PROT_MARGINWIDTH;
	}
	| SYM_PROT_NORESIZE
	{
		$$ = SYM_PROT_NORESIZE;
	}
	| SYM_PROT_COLS
	{
		$$ = SYM_PROT_COLS;
	}
	| SYM_PROT_SIZE
	{
		$$ = SYM_PROT_SIZE;
	}
	| SYM_PROT_RADIOGROUP
	{
		$$ = SYM_PROT_RADIOGROUP;
	}
	| SYM_PROT_VALIGN
	{
		$$ = SYM_PROT_VALIGN;
	}
	| SYM_PROT_OBJECT
	{
		$$ = SYM_PROT_OBJECT;
	}
	| SYM_PROT_CHAROFF
	{
		$$ = SYM_PROT_CHAROFF;
	}
	| SYM_PROT_LOW
	{
		$$ = SYM_PROT_LOW;
	}
	| SYM_PROT_START
	{
		$$ = SYM_PROT_START;
	}
	| SYM_PROT_SUMMARY
	{
		$$ = SYM_PROT_SUMMARY;
	}
	| SYM_PROT_OPTIMUM
	{
		$$ = SYM_PROT_OPTIMUM;
	}
	| SYM_PROT_ABBR
	{
		$$ = SYM_PROT_ABBR;
	}
	| SYM_PROT_FORM
	{
		$$ = SYM_PROT_FORM;
	}
	| SYM_PROT_LABEL
	{
		$$ = SYM_PROT_LABEL;
	}
	| SYM_PROT_FRAME
	{
		$$ = SYM_PROT_FRAME;
	}
	| SYM_PROT_ALLOWSCRIPTACCESS
	{
		$$ = SYM_PROT_ALLOWSCRIPTACCESS;
	}
	| SYM_PROT_PLACEHOLDER
	{
		$$ = SYM_PROT_PLACEHOLDER;
	}
	| SYM_PROT_AUTOSUBMIT
	{
		$$ = SYM_PROT_AUTOSUBMIT;
	}
	| SYM_PROT_LOWSRC
	{
		$$ = SYM_PROT_LOWSRC;
	}
	| SYM_PROT_BACKGROUND
	{
		$$ = SYM_PROT_BACKGROUND;
	}
	;
%%
int parse_html_stream(html_data_t *priv, const char *buf, size_t buf_len, int last_frag)
{
	html_pstate *parser = (html_pstate*)priv->parser.parser;
	yyscan_t lexier = (yyscan_t)priv->parser.lexier;
	YY_BUFFER_STATE input = NULL;
	int token = 0, parser_ret = 0;
	HTML_STYPE value;

	yydebug = debug_html_parser;
	priv->parser.last_frag = last_frag;
	input = html_scan_stream(buf, buf_len, priv);
	if(!input)
	{
		ey_html_debug(debug_html_parser, "create html stream buffer failed\n");
		return 1;
	}

	while(1)
	{
		memset(&value, 0, sizeof(value));
		token = html_lex(&value, lexier);
		if(token == SYM_LEX_CONTINUE)
			break;
		parser_ret = html_push_parse(parser, token, &value, (void*)priv);
		if(parser_ret != YYPUSH_MORE)
			break;
	}
	html_delete_buffer(input, lexier);

	if(parser_ret != YYPUSH_MORE && parser_ret != 0)
	{
		ey_html_debug(debug_html_parser, "find error while parsing html stream\n");
		return 2;
	}
	return 0;
}

void html_register(html_decoder_t *decoder)
{
	assert(decoder!=NULL);
	assert(decoder->engine!=NULL);

	engine_t engine = decoder->engine;
	int index = 0;
	for(index=0; index<YYNNTS; index++)
	{
		const char *name = yytname[YYNTOKENS + index];
		if(!name || name[0]=='$' || name[0]=='@')
			continue;
		yytid[YYNTOKENS + index] = ey_engine_find_event(engine, name);
		if(yytid[YYNTOKENS + index] >= 0)
			ey_html_debug(debug_html_parser, "event id of %s is %d\n", name, yytid[YYNTOKENS + index]);
		else
			ey_html_debug(debug_html_parser, "failed to register event %s\n", name);
	}
}
#undef this_priv
#undef this_decoder
