/*
 * GTK+ interface functions for Xdialog.
 */

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <stdio.h>
#ifdef STDC_HEADERS
#	include <stdlib.h>
#	include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#	include <unistd.h>
#endif
#include <sys/stat.h>

#include "interface.h"
#include "callbacks.h"
#include "support.h"


/* Global structure and variables */
extern Xdialog_data Xdialog;
extern gboolean dialog_compat;

int glist_size=0;


/* Fixed font loading and character size (in pixels) initialization */

static gint xmult = XSIZE_MULT;
static gint ymult = YSIZE_MULT;
static gint ffxmult = XSIZE_MULT;
static gint ffymult = YSIZE_MULT;

static int count_line=0;


static int create_buttons(int next_y=0);


//-------------------------------------------------------------------------

class Fl_Line : public Fl_Box {
public:
	Fl_Line(int X, int Y, int W, int H)
		: Fl_Box(X, Y, W, H)
	{
	}

	void draw() {
		Fl_Box::draw();
		fl_color(150, 150, 150); // default color is same as window background
		fl_line(x(), y(), x() + w(), y() + h()); // diagonal line
	}
};


//-------------------------------------------------------------------------

/* font_init() is used for two purposes: load the fixed font that Xdialog may
 * use, and calculate the character size in pixels (both for the fixed font
 * and for the font currently in use: the later one may be a proportionnal
 * font and the character width is therefore an averaged value).
 */

static void font_init(void)
{
	// select our font
  fl_font(FL_BOLD, LABEL_TEXT_HEIGHT);
  int width=0, height;
  fl_measure(ALPHANUM_CHARS, width, height);
  ffxmult = width / 62;		/* 62 = strlen(ALPHANUM_CHARS) */
	ffymult = height;

  xmult = ffxmult;
	ymult = ffymult;
}

//-------------------------------------------------------------------------

/* Custom text wrapping (the GTK+ one is buggy) */

static void wrap_text(gchar *str, gint reserved_width)
{
	gint max_line_width, n = 0;
	gchar *p = str, *last_space = NULL;
	gchar tmp[MAX_LABEL_LENGTH];

	if (Xdialog.xsize != 0)
		max_line_width = (Xdialog.size_in_pixels ? Xdialog.xsize :
							   Xdialog.xsize * xmult)
				  - reserved_width - 4 * xmult;
	else
		max_line_width = Xdialog.xsize - reserved_width - 6 * xmult;

	do {
		if (*p == '\n') {
			n = 0;
			last_space = NULL;
		} else {
			tmp[n++] = *p;
			tmp[n] = 0;
			if (fl_width(tmp) < max_line_width) {
				if (*p == ' ')
					last_space = p;
			} else {
				if (last_space != NULL) {
					*last_space = '\n';
					p = last_space;
					n = 0;
					last_space = NULL;
				} else if (*p == ' ')
					last_space = p;
			}
		}
	} while (++p < str + strlen(str));
}

//-------------------------------------------------------------------------

static char *add_newline(char *text)
{
  char *p=new char[MAX_LABEL_LENGTH];
  char *s=new char[MAX_LABEL_LENGTH];
  p=text;
  for(int i=0;i<strlen(text);i++) {
    if(*p=='\\'&&*(p+1)=='n') {
      *s='\n';
      p++;
    }
    else {
      *s=*p;
    }
    p++;
    s++;
  }
  return s;
}

//-------------------------------------------------------------------------

static char * removeBackSlash(char *input)
{
  char *p=input;
  while(*p!='\0') {
    if(*p=='\n')
      *p=0x20;
    p++;
  }
  return input;
}

//-------------------------------------------------------------------------

int get_label_height(const char *label_text, int width, int &height)
{
	char text[MAX_LABEL_LENGTH];
	backslash_n_to_linefeed(label_text, text, MAX_LABEL_LENGTH);
	//strcpysafe(text, label_text, MAX_LABEL_LENGTH);
	//removeBackSlash(text);
	trim_string(NULL, text, MAX_LABEL_LENGTH);
	fl_measure(text, width, height);
}

//-------------------------------------------------------------------------

static Fl_Box *set_label(const char *label_text, bool expand ,int x,int y,int w,int h)
{
	char *text=new char[MAX_LABEL_LENGTH];
	backslash_n_to_linefeed(label_text, text, MAX_LABEL_LENGTH);
	//strcpysafe(text, label_text, MAX_LABEL_LENGTH);
	//removeBackSlash(text);
	trim_string(NULL, text, MAX_LABEL_LENGTH);
	Fl_Box *text_box = new Fl_Box(x,y,w,h,0);
	text_box->labelcolor(FL_BLACK);
	text_box->align(FL_ALIGN_WRAP);
	text_box->label(text);
	return text_box;
}

//-------------------------------------------------------------------------

static int item_status( char *status, char *tag)
{
    if (tag[0] == 0) {
			return -2;
    }

		if (!strcasecmp(status, "on"))
			return 1;

		if (!strcasecmp(status, "unavailable")) {
			return -1;
		}
		return 0;
}

//-------------------------------------------------------------------------

static void set_timeout(void)
{
	if (Xdialog.timeout > 0) {
		Xdialog.window->show();
		Fl::add_timeout(Xdialog.timeout,timeout_exit,NULL );
	}
}

//-------------------------------------------------------------------------

static int countBackSlash(char *input)
{
  int count=0;
  while(*input!='\0')
  {
    if(*input=='\n')
      count++;
    input++;
  }
  return count;
}

//-------------------------------------------------------------------------

void get_maxsize(int *x, int *y)
{
  #define TITLEBAR_HEIGHT_GUESS 35

  font_init();

  if (Xdialog.size_in_pixels) {
  	*x = Fl::w() - (xmult*2);
    *y = Fl::h() - TITLEBAR_HEIGHT_GUESS - (ymult*2);
  }
  else {
  	*x = (Fl::w()/xmult) - 2;
    *y = ((Fl::h()-TITLEBAR_HEIGHT_GUESS)/ymult) - 2;
  }
}

//-------------------------------------------------------------------------

static int get_window_width_in_pixels(void)
{
	if (Xdialog.size_in_pixels) {
			return Xdialog.xsize;
	}
	return Xdialog.xsize * xmult;
}

//-------------------------------------------------------------------------

static int get_window_height_in_pixels(void)
{
	if (Xdialog.size_in_pixels) {
			return Xdialog.ysize;
	}
	return Xdialog.ysize * ymult;
}

//-------------------------------------------------------------------------

int calc_browser_height(Fl_Browser_ *browser, int num_items)
{
	// (incr_size is protected)
	float sf=Fl::screen_scale(0);
	return (5/sf)+((browser->textsize()+2)*num_items);
}

//-------------------------------------------------------------------------

int calc_tree_height(Fl_Tree *tree, int num_items)
{
	return (num_items*(ymult+tree->linespacing()))+tree->marginbottom()+tree->margintop();
}

//-------------------------------------------------------------------------

static int set_label(int y, const char *label_text, bool sep_flag)
{
	// abort if nothing to set
	if (label_text==NULL || label_text[0]==0) {
		return y;
	}

	// get client window area
  int clientwidth=get_window_width_in_pixels()-(CLIENT_BORDER_SIZE*2);

  // get height of any title on client area
  int height=0;
	get_label_height(label_text, clientwidth, height);
	int next_y=y+height+BORDER_SIZE;

	set_label(label_text, true, CLIENT_BORDER_SIZE, y, clientwidth, height);

	if (sep_flag) {
		Fl_Line *sep=new Fl_Line(CLIENT_BORDER_SIZE, next_y, clientwidth, 0);
		next_y+=sep->h()+BORDER_SIZE;
	}

	return next_y;
}

//-------------------------------------------------------------------------

static int create_buttons(int next_y)
{
	// setup text for labels if not already set
	if (Xdialog.ok_label[0] == 0)
	{
		strcpysafe(Xdialog.ok_label, "Ok", MAX_BUTTON_LABEL_LENGTH);
	}

	if (Xdialog.cancel_label[0] == 0)
	{
		strcpysafe(Xdialog.cancel_label, "Cancel", MAX_BUTTON_LABEL_LENGTH);
	}


	// setup for buttons
	int buttoncount=0;

	const char *labels[XDIALOG_MAX_BUTTONS];
	const Fl_Callback *callbacks[XDIALOG_MAX_BUTTONS];
	int shortcuts[XDIALOG_MAX_BUTTONS];

	memset(labels, 0, sizeof(labels));
	memset(callbacks, 0, sizeof(callbacks));
	memset(shortcuts, 0, sizeof(shortcuts));

	if (Xdialog.buttons) {

		if (Xdialog.wizard) {
			callbacks[0]=click_pre;
			callbacks[1]=click_no;
			callbacks[2]=click_next;
			buttoncount=3;
		}
		else {
			if (Xdialog.ok_button) {
				callbacks[buttoncount]=click_yes;
				if (!Xdialog.yesno_button) {
					labels[buttoncount]=Xdialog.ok_label;
					shortcuts[buttoncount]=FL_Enter;
				}
				else labels[buttoncount]="&Yes";
				buttoncount++;
			}

			if (Xdialog.cancel_button) {
				callbacks[buttoncount]=click_no;
				if (!Xdialog.yesno_button) {
					labels[buttoncount]=Xdialog.cancel_label;
					shortcuts[buttoncount]=FL_Escape;
				}
				else labels[buttoncount]="&No";
				buttoncount++;
			}

			if (Xdialog.print) {
				callbacks[buttoncount]=click_print;
				labels[buttoncount]=PRINT;
				buttoncount++;
			}
		}

		if (Xdialog.help) {
			callbacks[buttoncount]=click_help;
			labels[buttoncount]=HELP;
			shortcuts[buttoncount]=FL_F+1;
			buttoncount++;
		}
	}

	// calc button spacing
	int btsp = (get_window_width_in_pixels() - (XDIALOG_BUTTON_WIDTH*buttoncount))/(buttoncount+1);

	// create buttons
	for (int b=0, btw=btsp; b<buttoncount; btw+=btsp+XDIALOG_BUTTON_WIDTH, b++) {
		// create a button
		Fl_Button *btn=new Fl_Button(btw, get_window_height_in_pixels()- XDIALOG_BUTTON_HEIGHT - BORDER_SIZE, XDIALOG_BUTTON_WIDTH, XDIALOG_BUTTON_HEIGHT, 0);
		// now figure out what label and callback it should have
		btn->label(labels[b]);
		btn->callback(callbacks[b]);
		btn->shortcut(shortcuts[b]);
    btn->click_on_enter_key(1);
	}


	// next_y set to zero means no checkbox allowed
	if (Xdialog.check && next_y!=0)	{
		int clientwidth=Xdialog.window->w()-(CLIENT_BORDER_SIZE*2);

		Fl_Box *back_box = new Fl_Box(CLIENT_BORDER_SIZE, next_y, clientwidth, ymult+(BORDER_SIZE*2), 0);
		back_box->box(FL_THIN_DOWN_FRAME);

		Fl_Check_Button *check = new Fl_Check_Button(CLIENT_BORDER_SIZE+BORDER_SIZE, next_y+BORDER_SIZE, clientwidth-(BORDER_SIZE*2), ymult, 0);
		//check->activate();
		//check->active();
		check->callback(CheckCallback);
		check->label(Xdialog.check_label);

		next_y+=check->h()+BORDER_SIZE;

		if (Xdialog.checked) {
			check->value(1);
		}
		else {
			check->value(0);
		}
	}

	return next_y;
}

//-------------------------------------------------------------------------

static void set_window_size_and_placement(void)
{
	if (Xdialog.xsize != 0 && Xdialog.ysize != 0) {
		if (Xdialog.size_in_pixels)
			Xdialog.window->resize(0,0,Xdialog.xsize,Xdialog.ysize);
		else
		  Xdialog.window->resize(0, 0, Xdialog.xsize * xmult, Xdialog.ysize * ymult);
	}
	else {
	  Xdialog.xsize=400;
	  Xdialog.ysize=350;
	  Xdialog.size_in_pixels=true;
	  Xdialog.window->resize(0,0,Xdialog.xsize,Xdialog.ysize);
	}

	/* Allow the window to grow, shrink and auto-shrink */
	Xdialog.window->resizable();

	/* Set the window placement policy */
	if (Xdialog.set_origin) {
		Xdialog.window->position(
            Xdialog.xorg >= 0 ? (Xdialog.size_in_pixels ? Xdialog.xorg : Xdialog.xorg*xmult) :
            Fl::w() + Xdialog.xorg - Xdialog.xsize - 2 * xmult,
            Xdialog.yorg >= 0 ? (Xdialog.size_in_pixels ? Xdialog.yorg : Xdialog.yorg*ymult) :
            Fl::h() + Xdialog.yorg - Xdialog.ysize - 3 * ymult);
	}
	else {
      Xdialog.window->position((Fl::w() - Xdialog.window->w())/2, (Fl::h() - Xdialog.window->h())/2);
	}
}

//-------------------------------------------------------------------------

void open_window(void)
{
  Fl_Window *window;
	Fl_Pack *vbox;

	font_init();

	Xdialog.window = new Fl_Window(0, 0, Xdialog.title);
	/* Open a new GTK top-level window */
	window = Xdialog.window;

	/* adjust titles based on what is provided */
	if (Xdialog.backtitle[0]==0) {
		if (strlen(Xdialog.title) < MAX_BACKTITLE_LENGTH) {
			strcpy(Xdialog.backtitle, Xdialog.title);
			Xdialog.title[0]=0;
		}
	}

	/* Set the window title */
	window->label(Xdialog.backtitle);


	/* Set the internal border so that the child widgets do not
	* expand to the whole window (prettier) */
	window->border(xmult / 2);

	/* Create the root vbox widget in which all other boxes will
	* be packed. By setting the "homogeneous" parameter to false,
	* we allow packing with either gtk_box_pack_start() or
	* gtk_box_pack_end() and disallow any automatic repartition
	* of additional space between the child boxes (each box may
	* therefore be set so to expand or not): this is VERY important
	* and should not be changed !
	*/
	window->begin();

	/* Set the window size and placement policy */
	set_window_size_and_placement();

	if (Xdialog.beep & BEEP_BEFORE && Xdialog.exit_code != 2)
		fl_beep(FL_BEEP_DEFAULT);

	Xdialog.exit_code = 255;
}


//-------------------------------------------------------------------------

/*
 * The Xdialog widgets...
 */

void create_msgbox(gchar *optarg, gboolean yesno)
{
	open_window();

	int next_y=set_label(CLIENT_BORDER_SIZE, Xdialog.title, true);
	next_y =set_label(next_y, optarg, false);

	if (!yesno) {
		Xdialog.cancel_button=false;
	}

	create_buttons(next_y);
	set_timeout();
}

//-------------------------------------------------------------------------

void create_infobox(gchar *optarg, gint timeout)
{

  // adjust height of window if going to add a button
  if (!dialog_compat && Xdialog.buttons) {
  	// presume if working in lines/chars need to add space, if pixes
  	// then already space for any buttons
		if (!Xdialog.size_in_pixels) {
				Xdialog.ysize+=1;
		}
  }


	open_window();

	int next_y=set_label(CLIENT_BORDER_SIZE, Xdialog.title, true);
	next_y =set_label(next_y, optarg, false);

	if (!dialog_compat && Xdialog.buttons) {
		if (timeout>0) {
			Xdialog.cancel_button=false;
		}
		else {
			Xdialog.ok_button=false;
		}

		create_buttons(next_y);
	}

	if (timeout>0)
	{
		Fl::add_timeout(timeout/1000, info_timeout);
	}
}

//-------------------------------------------------------------------------

void create_gauge(gchar *optarg, gint percent)
{

  int value;
  if (percent < 0)
		value = 0;
	else if (percent > 100)
		value = 100;
	else
		value = percent;

  #define PROGRESS_LABEL_BUFFER_SIZE 5

	char *label_value=new char[PROGRESS_LABEL_BUFFER_SIZE];
	snprintf(label_value, PROGRESS_LABEL_BUFFER_SIZE-1, "%d%%",percent);
	label_value[PROGRESS_LABEL_BUFFER_SIZE-1]=0;

	open_window();

  // compute client width area
  int clientwidth=Xdialog.window->w()-(CLIENT_BORDER_SIZE*2);

  // add title
	int next_y=set_label(CLIENT_BORDER_SIZE, Xdialog.title, true);

	// setup label that can change - we'll set up to 3 lines
	int labelheight=0;
	get_label_height(optarg, clientwidth, labelheight);
	// make it up to three lines (this can be improved to resize everything in the callback instead)
	labelheight*=3;

	Fl_Widget *label=(Fl_Widget *)set_label(optarg, true, CLIENT_BORDER_SIZE, next_y, clientwidth, labelheight);
	label->align(FL_ALIGN_INSIDE|FL_ALIGN_TOP|FL_ALIGN_WRAP);
	next_y+=label->h()+BORDER_SIZE;

	int btw=clientwidth/10;
	Fl_Progress *progress=new Fl_Progress(CLIENT_BORDER_SIZE+btw, next_y, btw*8, 25);

	progress->minimum(0);
	progress->maximum(100);
	progress->color(0x88888800);
	progress->selection_color(0x4444ff00);
	progress->labelcolor(FL_WHITE);
	progress->value(percent);
	progress->label(label_value);

	Xdialog.widget1=(Fl_Widget *)progress;
	Xdialog.widget4=label;
	Xdialog.label_text[0] = 0;
	Xdialog.new_label = Xdialog.check = false;

	Fl::add_timeout(1,gauge_timeout);
}

//-------------------------------------------------------------------------

void create_progress(gchar *optarg, gint leading, gint maxdots)
{
}

//-------------------------------------------------------------------------

void create_tailbox(gchar *optarg)
{
	// open file
	FILE *infile;
	infile=fopen(optarg,"r");
	if (!infile) {
    Xdialog.exit_code=255;
	  printf("Xdialog:can't open %s\n",optarg);
	  exit(0);
	}

	// get file size
	fseek(infile, 0L, SEEK_END);
	int sz = ftell(infile);
	fseek(infile, 0L, SEEK_SET);


	// create buffer to read file (assume success)
	char *file_content=new char[sz+1];
	file_content[sz]=0;

	int nchars = fread(file_content, 1, sz, infile);

	/* Calculate the maximum line length and lines count */
	int i, n = 0, llen = 0, lcnt = 1;

	for (i = 0; i < nchars; i++) {
		if (file_content[i] != '\n') {
			if (file_content[i] == '\t')
				n += 8;
			else
				n++;
		}
		else {
			if (n > llen)
				llen = n;
			n = 0;
			lcnt++;
		}
	}
	fclose(infile);


	open_window();

  // compute client width area
  int clientwidth=Xdialog.window->w()-(CLIENT_BORDER_SIZE*2);
  int clientheight=Xdialog.window->h()-(CLIENT_BORDER_SIZE*2);

	int next_y=set_label(CLIENT_BORDER_SIZE, Xdialog.title, false);

	Fl_Text_Display *textview = new Fl_Text_Display(CLIENT_BORDER_SIZE, CLIENT_BORDER_SIZE, clientwidth, clientheight - (XDIALOG_BUTTON_HEIGHT * 2));
	Fl_Text_Buffer *buffer = new Fl_Text_Buffer();
	buffer->append(file_content);
	textview->buffer(buffer);

	int linesinview = textview->h() / (textview->textsize()+2);
	// adjust out scrollbar size in case it's shown
	linesinview-=Fl::scrollbar_size() / (textview->textsize()+2);

	textview->cursor_style(Fl_Text_Display::NORMAL_CURSOR);
	textview->scroll((lcnt>linesinview) ? (lcnt-linesinview) : 0, 0);

	create_buttons();
}

//-------------------------------------------------------------------------

void create_logbox(gchar *optarg)
{
}

//-------------------------------------------------------------------------

void create_textbox(gchar *optarg, gboolean editable)
{
	gint i, n = 0, llen = 0, lcnt = 0;

	open_window();

  // compute client width area
  int clientwidth=Xdialog.window->w()-(CLIENT_BORDER_SIZE*2);
  int clientheight=Xdialog.window->h()-(CLIENT_BORDER_SIZE*2);

	// open file
	FILE *infile = fopen(optarg, "r");

	// get file size
	fseek(infile, 0L, SEEK_END);
	int sz = ftell(infile);
	fseek(infile, 0L, SEEK_SET);

	// create buffer for contents
	char *file_content=new char[sz+1];
	file_content[0]=0;
	file_content[sz]=0;

	// read file
	if (infile) {
		size_t nchars=fread(file_content, 1, sz, infile);
		file_content[nchars]=0;
		/* Calculate the maximum line length and lines count */
		for (i = 0; i < nchars; i++) {
			if (file_content[i] != '\n') {
				if (file_content[i] == '\t')
					n += 8;
				else
					n++;
			}
			else {
				if (n > llen)
					llen = n;
				n = 0;
				lcnt++;
			}
		}
		fclose(infile);
	}

	if (dialog_compat && !editable) {
		Xdialog.cancel_button = false;
	}

	if (!editable) {
		Fl_Multiline_Output *output = new Fl_Multiline_Output(CLIENT_BORDER_SIZE, CLIENT_BORDER_SIZE, clientwidth, clientheight - (XDIALOG_BUTTON_HEIGHT * 2));
		output->value(file_content);
		Xdialog.type=XDIALOG_TYPE_EDIT_READONLY;
		Xdialog.widget1=(Fl_Widget*)output;
	}
	else {
		Fl_Text_Editor *output = new Fl_Text_Editor(CLIENT_BORDER_SIZE, CLIENT_BORDER_SIZE, clientwidth, clientheight - (XDIALOG_BUTTON_HEIGHT * 2));
		Fl_Text_Buffer *buffer = new Fl_Text_Buffer();
		buffer->append(file_content);
		Xdialog.type=XDIALOG_TYPE_EDIT;
		output->buffer(buffer);
		Xdialog.widget1=(Fl_Widget*)output;
	}

	create_buttons();
	set_timeout();
}

//-------------------------------------------------------------------------

void create_inputbox(gchar *optarg, gchar *options[], gint entries)
{
	open_window();
  int clientwidth=Xdialog.window->w()-(CLIENT_BORDER_SIZE*2);

	Fl_Input *input=NULL;

	if(entries==1) {
		int next_y=set_label(CLIENT_BORDER_SIZE, Xdialog.title, true);
		next_y=set_label(next_y, optarg, false);

		input=new Fl_Input(CLIENT_BORDER_SIZE+BORDER_SIZE, next_y, clientwidth-(BORDER_SIZE*2), 25, 0);
		input->value(options[0]);

		next_y+=input->h()+BORDER_SIZE;
		next_y=create_buttons(next_y);

		if (Xdialog.passwd)	{
			Fl_Check_Button *typing_hide = new Fl_Check_Button(CLIENT_BORDER_SIZE, next_y, clientwidth, XDIALOG_BUTTON_HEIGHT, 0);
			typing_hide->label("Hide typing");
			typing_hide->callback(HidetypingCallback);
		}
	}

	Xdialog.widget1=input;
	Xdialog.type=XDIALOG_TYPE_INPUT;
	Xdialog.widget1=(Fl_Widget *)input;
}

//-------------------------------------------------------------------------

void create_combobox(gchar *optarg, gchar *options[], gint list_size)
{
}

//-------------------------------------------------------------------------

void create_rangebox(gchar *optarg, gchar *options[], gint ranges)
{

	open_window();

  // compute client width area
  int clientwidth=Xdialog.window->w()-(CLIENT_BORDER_SIZE*2);

	int next_y=set_label(CLIENT_BORDER_SIZE, Xdialog.title, false);
	next_y=set_label(next_y, optarg, false);

  int min,max,deflt;

  Fl_Hor_Slider *slider=new Fl_Hor_Slider(CLIENT_BORDER_SIZE+BORDER_SIZE, next_y, clientwidth-(BORDER_SIZE*2), SLIDER_HEIGHT);

  if(ranges==1) {

		slider->minimum(atoi(options[0]));
		slider->maximum(atoi(options[1]));
		slider->value(atoi(options[2]));
		slider->label(options[2]);
		slider->callback(sliderCallback);
  }

	next_y+=slider->h()+BORDER_SIZE;
	create_buttons(next_y);

	Xdialog.type=XDIALOG_TYPE_RANGEBOX;
	Xdialog.widget1=slider;

	if (Xdialog.interval>0) {
	  Fl::add_timeout(Xdialog.interval/1000,OutputCallback_rangebox);
	}
}

//-------------------------------------------------------------------------

void create_spinbox(gchar *optarg, gchar *options[], gint spins)
{
}

//-------------------------------------------------------------------------

void create_itemlist(char *optarg, int type, char *options[], int list_size)
{
  char temp[MAX_ITEM_LENGTH];
  char *_optarg=new char[MAX_LABEL_LENGTH];
  trim_string(optarg,_optarg,MAX_LABEL_LENGTH);

  // calc number of params per listbox item
  int params=3+Xdialog.tips;

  open_window();

  // compute client width area
  int clientwidth=Xdialog.window->w()-(CLIENT_BORDER_SIZE*2);


	int next_y=set_label(CLIENT_BORDER_SIZE, Xdialog.title, false);
	next_y=set_label(next_y, optarg, false);

	int browser_y=next_y;

	// guess on height
	int browser_h=Xdialog.list_height * ymult;

	Fl_Check_Browser *browser = new Fl_Check_Browser(CLIENT_BORDER_SIZE, browser_y, clientwidth, browser_h);

  browser->full_kb_select(1);

	// get the actual height of items
	browser_h=calc_browser_height(browser, Xdialog.list_height);
	browser->size(browser->w(), browser_h);
	browser->type(FL_MULTI_BROWSER);

	Xdialog_array(list_size);

	browser->has_scrollbar(Fl_Check_Browser::VERTICAL);

	for (int i = 0; i < list_size;i++)
	{
		strcpysafe(Xdialog.array[i].tag, options[params*i], MAX_ITEM_LENGTH);
		Xdialog.array[i].state=item_status(options[params*i+2], Xdialog.array[i].tag);
		temp[0] = 0;
		if (Xdialog.tags && strlen(options[params*i]) != 0) {
			strcpysafe(temp, options[params*i], MAX_ITEM_LENGTH);
			strcatsafe(temp, ": ", MAX_ITEM_LENGTH);
		}
		strcatsafe(temp, options[params*i + 1], MAX_ITEM_LENGTH);
		browser->add(temp);
		if(Xdialog.array[i].state==1)
		{
		  browser->set_checked(i+1);
		}
	}
	Xdialog.window->resizable(browser);
	create_buttons();
	if (Xdialog.interval>0)
	{
		Fl::add_timeout(Xdialog.interval / 1000, OutputCallback_CheckList);
	}
	Xdialog.widget1=browser;
	Xdialog.type=XDIALOG_TYPE_CHECKLIST;
	set_timeout();
}

//-------------------------------------------------------------------------

void create_buildlist(gchar *optarg, gchar *options[], gint list_size)
{
  glist_size=list_size;
  if(Xdialog.xsize==0 && Xdialog.ysize==0)
  {
    Xdialog.xsize=50;
    Xdialog.ysize=12;
  }

  // calc number of params per listbox item
  int params=3+Xdialog.tips;

  open_window();

  // compute client width area
  int clientwidth=Xdialog.window->w()-(CLIENT_BORDER_SIZE*2);

	int next_y=set_label(CLIENT_BORDER_SIZE, Xdialog.title, false);
	next_y=set_label(next_y, optarg, false);

	// guess on height
	int browser_h=Xdialog.list_height * ymult;

	Fl_Browser *first_Browser = new Fl_Browser(CLIENT_BORDER_SIZE, next_y, clientwidth*0.4, browser_h);
  first_Browser->full_kb_select(1);
	first_Browser->item_shortcuts(TRUE);
	first_Browser->callback(BrowserCallback);

	// get the actual height of items
	browser_h=calc_browser_height(first_Browser, Xdialog.list_height);
	first_Browser->size(first_Browser->w(), browser_h);

	Fl_Button *add = new Fl_Button(CLIENT_BORDER_SIZE + (clientwidth*0.4) + BORDER_SIZE, next_y+(browser_h*0.3), (clientwidth*0.2)-(BORDER_SIZE*2), XDIALOG_BUTTON_HEIGHT);
	add->label("&ADD");
	Fl_Button *remove = new Fl_Button(CLIENT_BORDER_SIZE + (clientwidth*0.4) + BORDER_SIZE, next_y+(browser_h*0.6), (clientwidth*0.2)-(BORDER_SIZE*2), XDIALOG_BUTTON_HEIGHT);
	remove->label("&REMOVE");


	Fl_Browser *second_Browser = new Fl_Browser(CLIENT_BORDER_SIZE + (clientwidth*0.6), next_y, clientwidth*0.4, browser_h);
	second_Browser->full_kb_select(1);
	second_Browser->item_shortcuts(TRUE);
	second_Browser->callback(BrowserCallback);



	first_Browser->has_scrollbar(Fl_Check_Browser::VERTICAL);
	first_Browser->type(FL_MULTI_BROWSER);
	first_Browser->when(FL_WHEN_ENTER_KEY);
	second_Browser->has_scrollbar(Fl_Check_Browser::VERTICAL);
	second_Browser->type(FL_MULTI_BROWSER);
	second_Browser->when(FL_WHEN_ENTER_KEY);

	//////
  char temp[MAX_LABEL_LENGTH];
	Xdialog_array(list_size);
	for (int i = 0; i < list_size;i++)
	{
		strcpysafe(Xdialog.array[i].tag, options[params*i], MAX_ITEM_LENGTH);
		Xdialog.array[i].state=item_status(options[(params*i)+2], Xdialog.array[i].tag);
		temp[0] = 0;
		strcpysafe(temp, options[(params*i) + 1], MAX_ITEM_LENGTH);
		if(Xdialog.array[i].state==1)
		    second_Browser->add(temp, (void*) i);
		else
		    first_Browser->add(temp, (void*) i);
		strcpysafe(Xdialog.array[i].name,temp,MAX_LABEL_LENGTH);
	}

	/////
	first_Browser->select(1);
	second_Browser->select(1);


	add->callback(AddCallback);
	remove->callback(RemoveCallback);

	if(first_Browser->size()==0)
	  add->deactivate();

	if(first_Browser->size()>0)
	  add->activate();

	if(second_Browser->size()==0)
	  remove->deactivate();

	if(second_Browser->size()>0)
	  remove->activate();

	next_y+=first_Browser->h()+BORDER_SIZE;
	create_buttons(next_y);

	glist_size=list_size;
	Xdialog.type=XDIALOG_TYPE_BROWSERLIST;
	Xdialog.widget1=first_Browser;
	Xdialog.widget2=second_Browser;
	Xdialog.widget3=add;
	Xdialog.widget4=remove;

	if (Xdialog.interval>0)
	{
		Fl::add_timeout(Xdialog.interval/1000,OutputCallback_BuildList);
	}

	set_timeout();

}

//-------------------------------------------------------------------------

void create_menubox(gchar *optarg, gchar *options[], gint list_size)
{
	open_window();

  // compute client width area
  int clientwidth=Xdialog.window->w()-(CLIENT_BORDER_SIZE*2);

	int next_y=set_label(CLIENT_BORDER_SIZE, Xdialog.title, false);
	next_y=set_label(next_y, optarg, false);

	int browser_y=next_y;

	// guess on height
	int browser_h=Xdialog.list_height * ymult;

	Fl_Browser *browser = new Fl_Browser(CLIENT_BORDER_SIZE, next_y, clientwidth, browser_h, 0);
  browser->full_kb_select(1);
	browser->item_shortcuts(TRUE);
	browser->callback(BrowserCallback);

	// get the actual height of items
	browser_h=calc_browser_height(browser, Xdialog.list_height);
	browser->size(browser->w(), browser_h);

  Xdialog_array(list_size);
	for (int i = 0; i < list_size; i++)
	{
		strcpysafe(Xdialog.array[i].tag, options[2*i], MAX_ITEM_LENGTH);
	}

	int maxtag_x = 0;
	int maxtag_y = 0;
	int maxmenu_x = 0;
	int maxmenu_y = 0;

	for (int i = 0; i < 2*list_size;i+=2)
	{
	  int w_x=0;
	  fl_measure(options[i],w_x,maxtag_y);

	  if(w_x>maxtag_x)
	    maxtag_x=w_x;

	  w_x=0;
	  fl_measure(options[i+1],w_x,maxmenu_y);

	  if(w_x>maxmenu_x)
	    maxmenu_x=w_x;
	}

	static int widths[] = { maxtag_x+BORDER_SIZE, maxmenu_x+BORDER_SIZE, 0 };
	browser->column_widths(widths);
	browser->column_char('\t');
	browser->type(FL_HOLD_BROWSER);
	//browser->has_scrollbar(Fl_Browser::VERTICAL);

	int default_index = 1;
	for (int i = 0; i < 2 * list_size;i+=2)
	{
		char *cell = new char[strlen(options[i]) + strlen(options[i + 1])+10];
		cell[0] = 0;
		if (Xdialog.tags)
		{
			strcpy(cell, options[i]);
			cell = strcat(cell, "\t");
		}
		if (strcmp(options[i],Xdialog.default_item)==0)
		{
			default_index = i+1;
		}
		cell = strcat(cell,options[i+1]);
		browser->add(cell);
	}

	create_buttons();

	Xdialog.window->resizable(browser);

	browser->select(default_index);

	Xdialog.type=XDIALOG_TYPE_MENU;
	Xdialog.widget1=(Fl_Widget *)browser;

	if (Xdialog.interval>0)
	{
		Fl::add_timeout(Xdialog.interval/1000,OutputCallback_menubox);
	}

	set_timeout();
}

//-------------------------------------------------------------------------

void create_treeview(char *optarg, char *options[], int list_size)
{

	open_window();

  // compute client width area
  int clientwidth=Xdialog.window->w()-(CLIENT_BORDER_SIZE*2);

	int next_y=set_label(CLIENT_BORDER_SIZE, Xdialog.title, false);
	next_y=set_label(next_y, optarg, false);

	// guess on height
	int browser_h=Xdialog.list_height * ymult;

	Fl_Tree *tree = new Fl_Tree(CLIENT_BORDER_SIZE, next_y, clientwidth, browser_h, 0);

	// get the actual height of items
	browser_h=calc_tree_height(tree, Xdialog.list_height);
	tree->size(tree->w(), browser_h);
	tree->showroot(0);

	next_y+=tree->h()+BORDER_SIZE;

	int depth = 0;
	int params = 4 + Xdialog.tips;

	Xdialog_array(list_size);
	int i;

	for (i = 0; i < list_size; i++) {
		// 1=on, 0=off, -1=read-only, -2=unavailable
		int state=item_status(options[(params*i)+2], options[(params*i)]);

		if (state==-2) {
				continue;
		}

		strcpysafe(Xdialog.array[i].tag, options[params*i], MAX_ITEM_LENGTH);
		strcpysafe(Xdialog.array[i].name, options[params*i + 1], MAX_ITEM_LENGTH);

		int level = atoi(options[params*i + 3]);
		Xdialog.array[i].state=level;

		// check for at back at prior level
		if (i > 0) {
			if (atoi(options[params*(i - 1) + 3]) > level) {
				depth = level;
			}
		}

		if (i + 1 < list_size) {
			// check if next item is new level
			if (level < atoi(options[params*(i + 1) + 3])) {
				if (atoi(options[params*(i + 1) + 3]) != level + 1) {
					fprintf(stderr,
						"Xdialog: You cannot increment the --treeview depth "\
						"by more than one level each time !  Aborting...\n");
					exit(255);
				}

				depth++;
				if (depth > MAX_TREE_DEPTH) {
					fprintf(stderr,
						"Xdialog: Max allowed depth for "\
						"--treeview is %d !  Aborting...\n",
						MAX_TREE_DEPTH);
					exit(255);
				}
			}
		}

		char temp[MAX_ITEM_LENGTH];

		for(int k=i;k>=0;k--) {
			// go back to prior level item
		  if(Xdialog.array[k].state==Xdialog.array[i].state-1)
		  {
		    char *temp=new char[MAX_LABEL_LENGTH];
		    strcpysafe(temp,Xdialog.array[k].tips,MAX_LABEL_LENGTH);
		    strcatsafe(temp,"/",MAX_LABEL_LENGTH);
		    strcatsafe(temp,Xdialog.array[i].name,MAX_LABEL_LENGTH);
		    strcpysafe(Xdialog.array[i].tips,temp,MAX_LABEL_LENGTH);
		    break;
		  }
			// stop at level 0
		  if(Xdialog.array[k].state==0)
			{
		    strcpysafe(Xdialog.array[i].tips,Xdialog.array[i].name,MAX_LABEL_LENGTH);
		    break;
		  }
		}

		Fl_Tree_Item *treeitem=tree->add(Xdialog.array[i].tips);

		if (state==1) {
			treeitem->select();
			tree->set_item_focus(treeitem);
		}
		else if (state==-1) {
			treeitem->deactivate();
		}
	}

	create_buttons(next_y);

	glist_size=list_size;
	Xdialog.widget1=(Fl_Widget *)tree;
	Xdialog.type=XDIALOG_TYPE_TREE;

	if (Xdialog.interval>0)
	{
		Fl::add_timeout(Xdialog.interval/1000,OutputCallback_treeview);
	}

	set_timeout();
}

//-------------------------------------------------------------------------

void create_filesel(gchar *optarg, gboolean dsel_flag)
{
	char *output=new char[MAX_LABEL_LENGTH];
	output[0]=0;

	if(!dsel_flag) {
	  Fl_File_Chooser *chooser=new Fl_File_Chooser(optarg,"*", Fl_File_Chooser::SINGLE, Xdialog.title);
	  chooser->preview(0);
	  chooser->visible();
	  chooser->show();
	  Fl::run();
	  const char *filename=chooser->value();

// 	 output=fl_file_chooser(Xdialog.title,"",optarg,1);
	  if(filename==0)
	    Xdialog.exit_code=1;
	  else {
	    Xdialog.exit_code=0;
	    printf("%s\n",filename);
	  }
	}
	else {
	  Fl_File_Chooser *chooser=new Fl_File_Chooser(optarg,"*", Fl_File_Chooser::DIRECTORY, Xdialog.title);
	  chooser->preview(0);
	  chooser->visible();
 	  chooser->show();
	  Fl::run();
	  const char *filename=chooser->value();
// 	 output=fl_file_chooser(Xdialog.title,"",optarg,1);
	  if(filename==0)
	    Xdialog.exit_code=1;
	  else {
	    Xdialog.exit_code=0;
	    printf("%s\n",filename);
	  }
	}
}

//-------------------------------------------------------------------------

void create_colorsel(gchar *optarg, gdouble *colors)
{
  font_init();

}


//-------------------------------------------------------------------------

void create_fontsel(gchar *optarg)
{
  font_init();

}

//-------------------------------------------------------------------------

void create_calendar(gchar *optarg, gint day, gint month, gint year)
{
}

//-------------------------------------------------------------------------

void create_timebox(gchar *optarg, gint hours, gint minutes, gint seconds)
{
  if(Xdialog.xsize==0 && Xdialog.ysize==0) {
    Xdialog.xsize=40;
    Xdialog.ysize=10;
  }

	open_window();

  // compute client width area
  int clientwidth=Xdialog.window->w()-(CLIENT_BORDER_SIZE*2);

	int next_y=set_label(CLIENT_BORDER_SIZE, Xdialog.title, false);
	next_y=set_label(next_y, optarg, false);

	// make room for label on box
	//next_y+=ymult;

  int boxheight=SPINNER_HEIGHT*3;

	Fl_Box *box = new Fl_Box(CLIENT_BORDER_SIZE, next_y, clientwidth, boxheight);
	box->box(FL_BORDER_BOX);
	box->align(FL_ALIGN_INSIDE|FL_ALIGN_TOP);
	box->label("Hour:Minutes:Seconds");

	Fl_Spinner *spinner1 = new Fl_Spinner((get_window_width_in_pixels()*0.25)-(SPINNER_WIDTH/2), next_y+(boxheight*0.5), SPINNER_WIDTH, SPINNER_HEIGHT);
	spinner1->range(0,23);
	spinner1->type(FL_FLOAT_INPUT);
	spinner1->value(hours);

	Fl_Spinner *spinner2 = new Fl_Spinner((get_window_width_in_pixels()*0.50)-(SPINNER_WIDTH/2), next_y+(boxheight*0.5), SPINNER_WIDTH, SPINNER_HEIGHT);
	spinner2->range(0, 59);
	spinner2->type(FL_FLOAT_INPUT);
	spinner2->value(minutes);

	Fl_Spinner *spinner3 = new Fl_Spinner((get_window_width_in_pixels()*0.75)-(SPINNER_WIDTH/2), next_y+(boxheight*0.5), SPINNER_WIDTH, SPINNER_HEIGHT);
	spinner3->range(0, 59);
	spinner3->type(FL_FLOAT_INPUT);
	spinner3->value(seconds);

	set_label(":", true, (get_window_width_in_pixels()*0.375)-(SPINNER_WIDTH/4), next_y+(boxheight*0.5), 20, SPINNER_HEIGHT);
	set_label(":", true, (get_window_width_in_pixels()*0.625)-(SPINNER_WIDTH/4), next_y+(boxheight*0.5), 20, SPINNER_HEIGHT);

	create_buttons();

	Xdialog.type=XDIALOG_TYPE_TIME;
	Xdialog.widget1=(Fl_Widget *)spinner1;
	Xdialog.widget2=(Fl_Widget *)spinner2;
	Xdialog.widget3=(Fl_Widget *)spinner3;

	if (Xdialog.interval>0)
	{
		Fl::add_timeout(Xdialog.interval/1000,OutputCallback_timebox);
	}

	set_timeout();

}



