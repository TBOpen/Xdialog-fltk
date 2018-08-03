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


/* Fixed font loading and character size (in pixels) initialisation */

static gint xmult = XSIZE_MULT;
static gint ymult = YSIZE_MULT;
static gint ffxmult = XSIZE_MULT;
static gint ffymult = YSIZE_MULT;


static int count_line=0;

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
  fl_font(FL_BOLD,14);
  int width, height;
  fl_measure(ALPHANUM_CHARS, width, height);
  ffxmult = width / 62;		/* 62 = strlen(ALPHANUM_CHARS) */
	ffymult = height + 2;		/*  2 = spacing pixel lines */

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

static Fl_Box *set_label(const char *label_text, bool expand ,int x,int y,int w,int h)
{

	int _x=0,_y=0;
	char *text=new char[MAX_LABEL_LENGTH];
	int icon_width = 0;
	strcpysafe(text, label_text, MAX_LABEL_LENGTH);
	removeBackSlash(text);
	trim_string(NULL, text, MAX_LABEL_LENGTH);
	fl_measure(text,_x,_y);
	Fl_Box *text_box = new Fl_Box(x+10,y,w-20,h,0);
	text_box->labelsize(14);
	text_box->labelcolor(FL_BLACK);
	text_box->align(FL_ALIGN_WRAP);
	text_box->label(text);
	return text_box;
}

//-------------------------------------------------------------------------

static int item_status( char *status, char *tag)
{
#ifdef HAVE_STRCASECMP
		if (!strcasecmp(status, "on") && strlen(tag) != 0)
			return 1;

		if (!strcasecmp(status, "unavailable") || strlen(tag) == 0) {
			return -1;
		}
#else
		if ((!strcmp(status, "on") ||
		     !strcmp(status, "On") ||
		     !strcmp(status, "ON")) && strlen(tag) != 0)
			return 1;

		if (!strcmp(status, "unavailable") ||
		    !strcmp(status, "Unavailable") ||
		    !strcmp(status, "UNAVAILABLE") || strlen(tag) == 0) {
			return -1;
		}
#endif
		return 0;
}

//-------------------------------------------------------------------------

static void set_timeout(void)
{
	if (Xdialog.timeout > 0)
		Fl::add_timeout(Xdialog.timeout,timeout_exit,NULL );
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

static char *filter(char *txt)
{
  count_line=Xdialog.xsize*1.5/fl_width("a",1);
  int fill_number=0;
  char *p=new char[MAX_LABEL_LENGTH];
  char *start_p;
  start_p=p;
  char *temp=new char[MAX_LABEL_LENGTH];
  int k=0;
  if(fl_width(txt)<Xdialog.xsize-20) {
    return txt;
  }
  else {
    for(int i=0;i<strlen(txt);i++) {
      *p=txt[i];
      p++;
      if(i%count_line>count_line-2) {
        *p=0x20;
        p++;
      }
    }
  }
  *p=0;
  return start_p;
}

//-------------------------------------------------------------------------

static void set_separator(bool from_start)
{
	Fl_Line *line = new Fl_Line(0,30,Xdialog.xsize,0);
}

//-------------------------------------------------------------------------

static void set_backtitle(bool sep_flag)
{
	int back_title_height = 30;
	Fl_Box *label=new Fl_Box(0,0,Xdialog.xsize,30,0);
	char backtitle[MAX_BACKTITLE_LENGTH];
	if (strlen(Xdialog.backtitle) == 0)
		return;
	if (dialog_compat)
		backslash_n_to_linefeed(Xdialog.backtitle, backtitle, MAX_BACKTITLE_LENGTH);
	else
		trim_string(Xdialog.backtitle, backtitle, MAX_BACKTITLE_LENGTH);

	if (Xdialog.wrap || dialog_compat)
		wrap_text(backtitle, 2 * ymult / 3);
	Fl_Box *back_box = new Fl_Box(0, 0, Xdialog.xsize, 30, 0);
	back_box->label(Xdialog.backtitle);
	back_box->labelfont(0);
	back_box->labelcolor(FL_BLACK);
	back_box->align(FL_ALIGN_CENTER);
	back_box->labelsize(14);
	if (sep_flag) {
		set_separator(true);
	}
}

//-------------------------------------------------------------------------

static void create_buttons()
{
	if (Xdialog.ok_label[0] == 0)
	{
		strcpysafe(Xdialog.ok_label, "Ok", MAX_TITLE_LENGTH);
	}
	if (Xdialog.cancel_label[0] == 0)
	{
		strcpysafe(Xdialog.cancel_label, "Cancel", MAX_TITLE_LENGTH);
	}
	if(Xdialog.print)
	{
	  int btw = (Xdialog.xsize - 240) / 4;
				Fl_Button *btn_yes = new Fl_Button(btw, Xdialog.ysize - 25 - 5, 80, 25, 0);
				Fl_Button *btn_no = new Fl_Button(btw * 2 + 80, Xdialog.ysize - 25 - 5, 80, 25, 0);
				Fl_Button *btn_print = new Fl_Button(btw * 3 + 80 * 2, Xdialog.ysize - 25 - 5, 80, 25, 0);
				btn_yes->label(Xdialog.ok_label);
				btn_no->label(Xdialog.cancel_label);
				btn_print->label("Print");
				btn_yes->callback(click_yes);
				btn_no->callback(click_no);
				btn_print->callback(click_print);
	}
	if (Xdialog.ok_button&&Xdialog.cancel_button&&Xdialog.buttons&&!Xdialog.print)
	{
		if (Xdialog.wizard){
			if (!Xdialog.help){
				int btw = (Xdialog.xsize - 240) / 4;
				Fl_Button *btn_pre = new Fl_Button(btw, Xdialog.ysize - 25 - 5, 80, 25, 0);
				Fl_Button *btn_no = new Fl_Button(btw * 2 + 80, Xdialog.ysize - 25 - 5, 80, 25, 0);
				Fl_Button *btn_next = new Fl_Button(btw * 3 + 80 * 2, Xdialog.ysize - 25 - 5, 80, 25, 0);
				btn_pre->label(PREVIOUS);
				btn_no->label(CANCEL);
				btn_next->label(NEXT);
				btn_pre->callback(click_pre);
				btn_no->callback(click_no);
				btn_next->callback(click_next);
			}
			else{
				int btw = (Xdialog.xsize - 320) / 5;
				Fl_Button *btn_pre = new Fl_Button(btw, Xdialog.ysize - 25 - 5, 80, 25, 0);
				Fl_Button *btn_no = new Fl_Button(btw * 2 + 80, Xdialog.ysize - 25 - 5, 80, 25, 0);
				Fl_Button *btn_next = new Fl_Button(btw * 3 + 80 * 2, Xdialog.ysize - 25 - 5, 80, 25, 0);
				Fl_Button *btn_help = new Fl_Button(btw * 4 + 80 * 3, Xdialog.ysize - 25 - 5, 80, 25, 0);
				btn_pre->label(PREVIOUS);
				btn_no->label(CANCEL);
				btn_next->label(NEXT);
				btn_help->label(HELP);
				btn_pre->callback(click_pre);
				btn_no->callback(click_no);
				btn_next->callback(click_next);
				btn_help->callback(click_help);
			}
		}
		else
		{
			if (!Xdialog.help){
				int btw = (Xdialog.xsize - 160) / 3;
				Fl_Button *btn_yes = new Fl_Button(btw, Xdialog.ysize - 25 - 5, 80, 25, 0);
				Fl_Button *btn_no = new Fl_Button(btw * 2 + 80, Xdialog.ysize - 25 - 5, 80, 25, 0);
				btn_yes->label(Xdialog.ok_label);
				btn_no->label(Xdialog.cancel_label);
				btn_yes->callback(click_yes);
				btn_no->callback(click_no);
			}
			else{
				int btw = (Xdialog.xsize - 240) / 4;
				Fl_Button *btn_yes = new Fl_Button(btw, Xdialog.ysize - 25 - 5, 80, 25, 0);
				Fl_Button *btn_no = new Fl_Button(btw * 2 + 80, Xdialog.ysize - 25 - 5, 80, 25, 0);
				Fl_Button *btn_help = new Fl_Button(btw * 3 + 80 * 2, Xdialog.ysize - 25 - 5, 80, 25, 0);
				btn_yes->label(Xdialog.ok_label);
				btn_no->label(Xdialog.cancel_label);
				btn_help->label(HELP);
				btn_yes->callback(click_yes);
				btn_no->callback(click_no);
				btn_help->callback(click_help);
			}
		}
	}

	if (!Xdialog.ok_button&&Xdialog.buttons)
	{
		Fl_Button *btn_no = new Fl_Button(Xdialog.xsize*0.5 - 12, Xdialog.ysize - 25 - 5, 80, 25, 0);
		btn_no->label(Xdialog.cancel_label);
		btn_no->callback(click_no);
	}
	if (!Xdialog.cancel_button&&Xdialog.buttons)
	{
		Fl_Button *btn_yes = new Fl_Button(Xdialog.xsize*0.5 - 12, Xdialog.ysize - 25 - 5, 80, 25, 0);
		btn_yes->label(Xdialog.ok_label);
		btn_yes->callback(click_yes);
	}
	if (Xdialog.check)
	{
		Fl_Check_Button *check = new Fl_Check_Button(6, Xdialog.ysize - 55, 10, 6, 0);
		check->activate();
		check->active();
		check->callback(CheckCallback);
		Fl_Box *back_box = new Fl_Box(20, Xdialog.ysize - 56, 10, 5, 0);
		back_box->label(Xdialog.check_label);
		back_box->labelfont(0);
		back_box->labelcolor(FL_BLACK);
		back_box->align(FL_ALIGN_RIGHT);
		back_box->labelsize(15);
		if (Xdialog.checked)
		{
			check->value(1);
		}
		else
		{
			check->value(0);
		}
	}
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
	//gtk_window_set_policy(GTK_WINDOW(Xdialog.window), TRUE, TRUE, TRUE);
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
      //Xdialog.window->align(FL_ALIGN_CENTER);
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

	/* Set the window title */
	window->label(Xdialog.title);


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
    if(Xdialog.xsize==0&&Xdialog.ysize==0)
	  {
	    Xdialog.xsize=48;
	    Xdialog.ysize=13;
	  }
	open_window();
	//Fl_Widget *hbuttonbox;
	int check_height = CHECK_BUTTON_HEIGHT;
	int button_height = BUTTON_HEIGHT;
	int backtitle_height = BACKTITLE_HEIGHT;
	if (!Xdialog.check)
		check_height = 0;
	if (!Xdialog.buttons)

		button_height = 0;
	if (strlen(Xdialog.backtitle) == 0)
		backtitle_height = 0;

	set_backtitle(true);
	set_label(optarg,true,0,backtitle_height,Xdialog.xsize,Xdialog.ysize-check_height-button_height-backtitle_height);
	if (yesno)
	{

	  strcpysafe(Xdialog.ok_label,"Yes",MAX_LABEL_LENGTH);
	strcpysafe(Xdialog.cancel_label,"No",MAX_LABEL_LENGTH);
		create_buttons();
	}
	else
	{
		if (Xdialog.help)
		{
			int btw = (Xdialog.xsize - 160) / 3;
			Fl_Button *btn_yes = new Fl_Button(btw, Xdialog.ysize - 25 - 5, 80, 25, 0);
			Fl_Button *btn_help = new Fl_Button(btw * 2 + 80, Xdialog.ysize - 25 - 5, 80, 25, 0);
			btn_yes->label(Xdialog.ok_label);
			btn_help->label(HELP);
			btn_yes->callback(click_yes);
			btn_help->callback(click_help);
		}
		else
		{
			int btw = (Xdialog.xsize - 80) / 2;
			Fl_Button *btn_yes = new Fl_Button(btw, Xdialog.ysize - 25 - 5, 80, 25, 0);
			btn_yes->label(Xdialog.ok_label);
			btn_yes->callback(click_yes);
		}
	}
	set_timeout();
}

//-------------------------------------------------------------------------

void create_infobox(gchar *optarg, gint timeout)
{
  fl_font(FL_BOLD,14);
  char *_optarg=new char[MAX_LABEL_LENGTH];
	trim_string(optarg,_optarg,MAX_LABEL_LENGTH);
      if(Xdialog.xsize==0&&Xdialog.ysize==0)
      {
	int _x,_y;
	fl_measure(_optarg,_x,_y);
	if(_x>80)
	  Xdialog.xsize=_x+10;
	else
	  Xdialog.xsize=90;
	Xdialog.ysize=_y+BUTTON_HEIGHT;
	Xdialog.size_in_pixels=true;
      }
      open_window();
	int check_height = CHECK_BUTTON_HEIGHT;
	int button_height = BUTTON_HEIGHT;
	int backtitle_height = BACKTITLE_HEIGHT;
	if (!Xdialog.check)
		check_height = 0;
	if (!Xdialog.buttons)
		button_height = 0;
	if (strlen(Xdialog.backtitle) == 0)
		backtitle_height = 0;
	set_backtitle(true);
	set_label(optarg, true, 0, backtitle_height, Xdialog.xsize, Xdialog.ysize - check_height - button_height - backtitle_height);
	if (Xdialog.buttons&&!dialog_compat)
	{
		if (timeout>0)
		{
			int btw = (Xdialog.xsize - 80) / 2;
			Fl_Button *btn_yes = new Fl_Button(btw, Xdialog.ysize - 25 - 5, 80, 25, 0);
			btn_yes->label(OK);
			btn_yes->callback(click_yes);
		}
		else
		{

			int btw = (Xdialog.xsize - 80) / 2;
			Fl_Button *btn_yes = new Fl_Button(btw, Xdialog.ysize - 25 - 5, 80, 25, 0);
			btn_yes->label(CANCEL);
			btn_yes->callback(click_no);
		}
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
  int check_height = CHECK_BUTTON_HEIGHT;
	int button_height = BUTTON_HEIGHT;
	int backtitle_height = BACKTITLE_HEIGHT;
	if (!Xdialog.check)
		check_height = 0;
	if (!Xdialog.buttons)
		button_height = 0;
	if (strlen(Xdialog.backtitle) == 0)
		backtitle_height = 0;
	char *label_value=new char[5];
	sprintf(label_value,"%d%%",percent);
	open_window();
	set_backtitle(true);
	Xdialog.widget4=(Fl_Widget *)set_label(optarg,true,0,backtitle_height,Xdialog.xsize,Xdialog.ysize-check_height-backtitle_height-30);
	int btw=Xdialog.xsize/10;
	Fl_Progress *progress=new Fl_Progress(btw,Xdialog.ysize-30,btw*8,25);
	progress->minimum(0);
	progress->maximum(100);
	progress->color(0x88888800);
	progress->selection_color(0x4444ff00);
	progress->labelcolor(FL_WHITE);
	progress->value(percent);
	progress->label(label_value);
	Xdialog.widget1=(Fl_Widget *)progress;
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

	FILE *infile;

	int i, n = 0, llen = 0, lcnt = 0;

	infile=fopen(optarg,"r");
	if(!infile)
	{
	    Xdialog.exit_code=255;
	  printf("Xdialog:cann't open %s",optarg);
	  exit(0);
	}
	fseek(infile, 0L, SEEK_END);
	int sz = ftell(infile);
	fseek(infile, 0L, SEEK_SET);
	char *file_content=new char[sz];
	if (infile) {
		char buffer[1024];
		int nchars;

		do {
			nchars = fread(buffer, 1, 1024, infile);
			/* Calculate the maximum line length and lines count */
			for (i = 0; i < nchars; i++)
				if (buffer[i] != '\n') {
					if (buffer[i] == '\t')
						n += 8;
					else
						n++;
				} else {
					if (n > llen)
						llen = n;
					n = 0;
					lcnt++;
				}
		strcatsafe(file_content,buffer,sz);
		} while (nchars == 1024);
	}
	else
	{
	  Xdialog.exit_code=255;
	  printf("Xdialog:cann't open %s",optarg);
	  printf("\n");
	  exit(0);
	}
	fclose(infile);
	open_window();
	char *p=new char[sz+sz];
	char *fir=p;
	char *mid=p;
	fl_font(FL_BOLD,10);
	while(*file_content!='\0')
	{
	  *p=*file_content;
	  int _width=0;
	  int _height=0;
	  *(p+1)='\0';
	  fl_measure(mid,_width,_height,0);
	  if(_width>Xdialog.ysize+10)
	  {
	    p++;
	    *p='\n';
	    mid=(p+1);
	  }
	  p++;
	  file_content++;
	}
	*p='\0';
	int size=strlen(fir);
	char *_file_content=new char[size];
	strcpysafe(_file_content,fir,size);
	Fl_Text_Display *textView = new Fl_Text_Display(5, 5, Xdialog.xsize - 10, Xdialog.ysize - 60,0);
	textView->scrollbar_align(FL_ALIGN_RIGHT);
	Fl_Text_Buffer *buffer = new Fl_Text_Buffer();
	textView->textsize(12);
	buffer->append(_file_content);
	textView->buffer(buffer);

	textView->cursor_style(Fl_Text_Display::NORMAL_CURSOR);
	textView->scroll(strlen(_file_content),strlen(_file_content));
	create_buttons();
}

//-------------------------------------------------------------------------

void create_logbox(gchar *optarg)
{
}

//-------------------------------------------------------------------------

void create_textbox(gchar *optarg, gboolean editable)
{
	int check_height = CHECK_BUTTON_HEIGHT;
	int button_height = BUTTON_HEIGHT;
	int backtitle_height = BACKTITLE_HEIGHT;
	Fl_Text_Editor *editor_box;
	if (!Xdialog.check)
		check_height = 0;
	if (!Xdialog.buttons)
		button_height = 0;
	if (strlen(Xdialog.backtitle) == 0)
		backtitle_height = 0;
	FILE *infile;
	gint i, n = 0, llen = 0, lcnt = 0;

	open_window();
	set_backtitle(false);
	//editor_box=new Fl_Text_Editor(5,backtitle_height,Xdialog.xsize-10,Xdialog.ysize-button_height-check_height-backtitle_height,0);
	if (strcmp(optarg, "-") == 0)
		infile = stdin;
	else
		infile = fopen(optarg, "r");
	fseek(infile, 0L, SEEK_END);
	int sz = ftell(infile);
	fseek(infile, 0L, SEEK_SET);
	char *file_content=new char[sz];
	file_content[0]=0;
	if (infile) {
		char buffer[1024];
		int nchars;

		do {
			nchars = fread(buffer, 1, 1024, infile);
			/* Calculate the maximum line length and lines count */
			for (i = 0; i < nchars; i++)
				if (buffer[i] != '\n') {
					if (buffer[i] == '\t')
						n += 8;
					else
						n++;
				} else {
					if (n > llen)
						llen = n;
					n = 0;
					lcnt++;
				}
		strcatsafe(file_content,buffer,nchars);
		} while (nchars == 1024);

		if (infile != stdin)
			fclose(infile);
	}
	if (dialog_compat && !editable)
		Xdialog.cancel_button = false;
	if (!editable)
	{
		Fl_Multiline_Output *output = new Fl_Multiline_Output(5, 5, Xdialog.xsize - 10, Xdialog.ysize - 60);
		output->value(file_content);
		Xdialog.type=2;
		Xdialog.widget1=(Fl_Widget*)output;
	}
	else
	{
		Fl_Text_Editor *output = new Fl_Text_Editor(5, 5, Xdialog.xsize - 10, Xdialog.ysize - 60);
		Fl_Text_Buffer *buffer = new Fl_Text_Buffer();
		buffer->append(file_content);
		Xdialog.type=3;
		output->buffer(buffer);
		Xdialog.widget1=(Fl_Widget*)output;
	}
	create_buttons();
	set_timeout();
}

//-------------------------------------------------------------------------

void create_inputbox(gchar *optarg, gchar *options[], gint entries)
{
    char deftext[MAX_INPUT_DEFAULT_LENGTH];
    int check_height = CHECK_BUTTON_HEIGHT;
	int button_height = BUTTON_HEIGHT;
	int backtitle_height = BACKTITLE_HEIGHT;
	if (!Xdialog.check)
		check_height = 0;
	if (!Xdialog.buttons)
		button_height = 0;
	if (strlen(Xdialog.backtitle) == 0)
		backtitle_height = 0;
	open_window();
	Fl_Input *input;
	if(entries==1)
	{
	set_backtitle(true);
	int row_hight=(Xdialog.ysize-backtitle_height-button_height-10);
	set_label(optarg,true,0,backtitle_height,Xdialog.xsize,row_hight);
	input=new Fl_Input(10,Xdialog.ysize-button_height-10,Xdialog.xsize-20,25,0);
	input->value(options[0]);
	create_buttons();
	if ((Xdialog.passwd > 0 && Xdialog.passwd < 10) ||
				(Xdialog.passwd > 10 && Xdialog.passwd <= entries + 10))
	{
		Fl_Check_Button *typing_hide = new Fl_Check_Button(10, row_hight + row_hight*0.5+30, 40, 25, 0);
		typing_hide->label("Hide typing");
		typing_hide->callback(HidetypingCallback);
	}
	}
	Xdialog.type=1;
	Xdialog.widget1=(Fl_Widget *)input;
}

//-------------------------------------------------------------------------

void create_combobox(gchar *optarg, gchar *options[], gint list_size)
{
}

//-------------------------------------------------------------------------

void create_rangebox(gchar *optarg, gchar *options[], gint ranges)
{
    int check_height = CHECK_BUTTON_HEIGHT;
    int button_height = BUTTON_HEIGHT;
    int backtitle_height = BACKTITLE_HEIGHT;
	if (!Xdialog.check)
		check_height = 0;
	if (!Xdialog.buttons)
		button_height = 0;
	if (strlen(Xdialog.backtitle) == 0)
		backtitle_height = 0;

     int min,max,deflt;
     open_window();
     Fl_Hor_Slider *slider=new Fl_Hor_Slider(10,Xdialog.ysize-button_height-30,Xdialog.xsize-20,30);
     set_backtitle(true);
     if(ranges==1)
     {
	set_label(optarg,true,0,backtitle_height,Xdialog.xsize,Xdialog.ysize-button_height-30);

	slider->minimum(atoi(options[0]));
	slider->maximum(atoi(options[1]));
	slider->value(atoi(options[2]));
	slider->label(options[2]);
	slider->callback(sliderCallback);
     }
     Xdialog.type=9;
     create_buttons();
     if (Xdialog.interval>0)
	{
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
	int i;
	open_window();
	int row_height=0;
	int label_h=0;
	int labeltitle_y=0;
	int params = 3 + Xdialog.tips;
	if(strlen(Xdialog.backtitle)!=0)
	{
	  row_height+=BACKTITLE_HEIGHT-10;

	}
	if(strlen(optarg)!=0)
	{
	  int _x=0,_y=0;

	  fl_measure(optarg,_x,_y);
	  label_h=(_x/Xdialog.xsize+1)*_y;
	  label_h+=countBackSlash(_optarg)*_y;
	   labeltitle_y=row_height;
	  row_height+=label_h;
	}
	int browser_h=Xdialog.ysize-row_height-BUTTON_HEIGHT;
	//row_height+=10;
	Fl_Check_Browser *browser = new Fl_Check_Browser(5, row_height, Xdialog.xsize - 10, browser_h);
	if(strlen(Xdialog.backtitle)!=0)
	{
	  set_backtitle(false);
	}
	if(strlen(optarg)!=0)
	{
 	  set_label(optarg,true,0,labeltitle_y,Xdialog.xsize,label_h);
	}
	Xdialog_array(list_size);
	browser->type(FL_MULTI_BROWSER);

	if(Xdialog.list_height>15)
	{
	    browser->textsize(Xdialog.list_height);
	}
	else
	{
	  browser->textsize(15);
	}
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
		if(Xdialog.array[i].state!=1)
		{
		  browser->set_checked(i+1);
		}
	}
	Xdialog.window->resizable(browser);
	create_buttons();
	if (Xdialog.interval>0)
	{
		Fl::add_timeout(Xdialog.interval / 1000, OutputCallback_ItemList);
	}
	Xdialog.window->show();
	Xdialog.type=8;
	set_timeout();
}

//-------------------------------------------------------------------------

void create_buildlist(gchar *optarg, gchar *options[], gint list_size)
{
  glist_size=list_size;
  if(Xdialog.xsize==0&&Xdialog.ysize==0)
  {
    Xdialog.xsize=50;
    Xdialog.ysize=12;
  }
  char *_optarg=new char[MAX_LABEL_LENGTH];
  trim_string(optarg,_optarg,MAX_LABEL_LENGTH);
  int params=3+Xdialog.tips;
  char temp[MAX_LABEL_LENGTH];
  int label_h;
  int labeltitle_y;
  open_window();
  int row_height = 0;
	if(strlen(Xdialog.backtitle)!=0)
	{
//	  row_height+=BACKTITLE_HEIGHT;
	  row_height+=10;
	}
	if(strlen(optarg)!=0)
	{
	  int _x=0,_y=0;

	  fl_measure(optarg,_x,_y);
	  label_h=(_x/Xdialog.xsize+1)*_y;
	  label_h+=countBackSlash(_optarg)*_y;
	   labeltitle_y=row_height;
	  row_height+=label_h;
	}
	int browser_h=Xdialog.ysize-row_height-BUTTON_HEIGHT;
	row_height+=10;
	Fl_Browser *first_Browser = new Fl_Browser(5, row_height, Xdialog.xsize*0.4 - 10, browser_h);
	Fl_Browser *second_Browser = new Fl_Browser(5 + Xdialog.xsize*0.6,row_height, Xdialog.xsize*0.4 - 10, browser_h);

	Fl_Button *add = new Fl_Button(Xdialog.xsize*0.4, row_height+browser_h*0.3, Xdialog.xsize*0.2,25);
	add->label("ADD");
	Fl_Button *remove = new Fl_Button(Xdialog.xsize*0.4, row_height+browser_h*0.6, Xdialog.xsize*0.2, 25);
	if(strlen(Xdialog.backtitle)!=0)
	{
	//  set_backtitle(true);
	}
	if(strlen(optarg)!=0)
	{
	  set_label(Xdialog.backtitle,true,0,labeltitle_y,Xdialog.xsize,label_h);

	}
	first_Browser->has_scrollbar(Fl_Check_Browser::VERTICAL);
	first_Browser->type(FL_MULTI_BROWSER);
	second_Browser->has_scrollbar(Fl_Check_Browser::VERTICAL);
	second_Browser->type(FL_MULTI_BROWSER);
	Xdialog_array(list_size);
	//////
	for (int i = 0; i < list_size;i++)
	{

		strcpysafe(Xdialog.array[i].tag, options[params*i], MAX_ITEM_LENGTH);
		Xdialog.array[i].state=item_status(options[params*i+2], Xdialog.array[i].tag);
		temp[0] = 0;
		/*if (Xdialog.tags && strlen(options[params*i]) != 0) {
			strcpysafe(temp, options[params*i], MAX_ITEM_LENGTH);
			strcatsafe(temp, ":", MAX_ITEM_LENGTH);
		}*/
		strcpysafe(temp, options[params*i + 1], MAX_ITEM_LENGTH);
		if(Xdialog.array[i].state==1)
		    second_Browser->add(temp);
		else
		    first_Browser->add(temp);
		strcpysafe(Xdialog.array[i].name,temp,MAX_LABEL_LENGTH);
		/*if(Xdialog.array[i].state!=0)
		{
		  first_Browser->set_checked(i+1);
		}*/
	}
	/////
	first_Browser->select(1);
	second_Browser->select(1);
	remove->label("REMOVE");
	remove->deactivate();
	add->callback(AddCallback);
	remove->callback(RemoveCallback);
	Xdialog.type=7;
	create_buttons();
	set_timeout();
		if(first_Browser->size()==0)
	  add->deactivate();
	if(first_Browser->size()>0)
	  add->activate();
	if(second_Browser->size()==0)
	  remove->deactivate();
	if(second_Browser->size()>0)
	  remove->activate();
}

//-------------------------------------------------------------------------

void create_menubox(gchar *optarg, gchar *options[], gint list_size)
{
	open_window();
	int row_height=0;
	int label_h=0;
	int labeltitle_y=0;
	int params = 3 + Xdialog.tips;
	char *_optarg=new char[MAX_LABEL_LENGTH];
	trim_string(optarg,_optarg,MAX_LABEL_LENGTH);
	if(strlen(Xdialog.backtitle)!=0)
	{
	  row_height+=BACKTITLE_HEIGHT;

	}
	if(strlen(optarg)!=0)
	{
	  int _x=0,_y=0;

	  fl_measure(optarg,_x,_y);
	  label_h=(_x/Xdialog.xsize+1)*_y;
	  label_h+=countBackSlash(_optarg)*_y;
	  labeltitle_y=row_height;
	  row_height+=label_h;
	}
	int browser_h=Xdialog.ysize-row_height-BUTTON_HEIGHT;
	row_height+=10;
	Fl_Browser *browser = new Fl_Browser(5, row_height, Xdialog.xsize - 10, browser_h,0);

	if(strlen(Xdialog.backtitle)!=0)
	{
	  set_backtitle(true);
	}
	if(strlen(optarg)!=0)
	{
	  set_label(optarg,true,0,labeltitle_y,Xdialog.xsize,label_h);

	}
      Xdialog_array(list_size);
	for (int i = 0; i < list_size; i++)
	{
		strcpysafe(Xdialog.array[i].tag, options[2*i], MAX_ITEM_LENGTH);

	}
	int old_tag=0;
	int old_menu=0;
	int maxtag_x = 0;
	int maxtag_y = 0;
	int maxmenu_x = 0;
	int maxmenu_y = 0;
	for (int i = 0; i < 2*list_size;i+=2)
	{
	  maxtag_x=0;
	  maxmenu_x=0;
	  fl_measure(options[i],maxtag_x,maxtag_y);
	  printf("%s\n",options[i]);
	  fl_measure(options[i+1],maxmenu_x,maxmenu_y);
	  if(maxtag_x>old_tag)
	    old_tag=maxtag_x;
	  if(maxmenu_x>old_menu)
	    old_menu=maxmenu_x;

	}
	static int widths[] = { maxtag_x+5,maxmenu_x+5,0 };
	browser->column_widths(widths);
	browser->column_char('\t');
	browser->type(2);
	//browser->has_scrollbar(Fl_Browser::VERTICAL);

	Xdialog.widget1=(Fl_Widget *)browser;
	int default_index = 1;
	for (int i = 0; i < 2 * list_size;i+=2)
	{
		char *cell = new char[strlen(options[i]) + strlen(options[i + 1])+10];
		cell[0] = 0;
		if (Xdialog.tags)
		{
			strcpysafe(cell, options[i],MAX_LABEL_LENGTH);
			cell = strcat(cell, "\t");
			//cell[strlen(cell)]='#';
			//cell[strlen(cell)]='\0';
		}
		if (strcmp(options[i],Xdialog.default_item)==0)
		{
			default_index = i*0.5;
		}
		cell = strcat(cell,options[i+1]);
		browser->add(cell);
	}
	//browser->select(default_index+1);
	browser->select(1);
	Xdialog.window->resizable(browser);
	Xdialog.type=4;
	create_buttons();
	Xdialog.window->end();
	if (Xdialog.interval>0)
	{
	Fl::add_timeout(Xdialog.interval/1000,OutputCallback_menubox);
	}
	Xdialog.window->show();
	set_timeout();
}

//-------------------------------------------------------------------------

void create_treeview(char *optarg, char *options[], int list_size)
{
 int oldlevel=-1;
	char *item = new char[128];
	item[0] = 0;
	char *name = new char[128];
	int depth = 0;
	int level, i;
	int params = 4 + Xdialog.tips;
  int check_height = CHECK_BUTTON_HEIGHT;
      int button_height = BUTTON_HEIGHT;
      int backtitle_height = BACKTITLE_HEIGHT;

	char temp[MAX_ITEM_LENGTH];
	char *_optarg=new char[MAX_LABEL_LENGTH];
	trim_string(optarg,_optarg,MAX_LABEL_LENGTH);
	open_window();
	int row_height=0;
	int label_h=0;
	int labeltitle_y=0;
	if(strlen(Xdialog.backtitle)!=0)
	{
	  row_height+=BACKTITLE_HEIGHT;

	}
	if(strlen(optarg)!=0)
	{
	  int _x=0,_y=0;

	  fl_measure(optarg,_x,_y);
	  label_h=(_x/Xdialog.xsize+1)*_y;
	  label_h+=countBackSlash(_optarg)*_y;
	   labeltitle_y=row_height;
	  row_height+=label_h;
	}
	int browser_h=Xdialog.ysize-row_height-BUTTON_HEIGHT;
	row_height+=10;
	Fl_Tree *tree = new Fl_Tree(5, row_height, Xdialog.xsize - 10, browser_h);
	if(strlen(Xdialog.backtitle)!=0)
	{
	  set_backtitle(true);
	}
	if(strlen(optarg)!=0)
	{

 	  set_label(optarg,true,0,labeltitle_y+5,Xdialog.xsize,label_h);
	}



	Xdialog_array(list_size);
	for (int i = 0; i < list_size; i++)
	{
		strcpysafe(Xdialog.array[i].tag, options[2*i], MAX_ITEM_LENGTH);
		if (item_status( options[params*i+2], Xdialog.array[i].tag) == 1) {
			Xdialog.array[0].state = i;
		}
	}
	Xdialog.widget1=(Fl_Widget *)tree;
	for (i = 0; i < list_size; i++) {
		int state=item_status(options[params*i+2], Xdialog.array[i].tag);
		strcpysafe(Xdialog.array[i].tag, options[params*i], MAX_ITEM_LENGTH);
		strcpysafe(Xdialog.array[i].name, options[params*i + 1], MAX_ITEM_LENGTH);
		strcpysafe(name, options[params*i + 1], MAX_ITEM_LENGTH);
		level = atoi(options[params*i + 3]);
		Xdialog.array[i].state=level;
		if (i > 0) {
			if (atoi(options[params*(i - 1) + 3]) > level) {
				depth = level;
			}
		}
		if (i + 1 < list_size) {
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
		for(int k=i;k>=0;k--)
		{
		  if(Xdialog.array[k].state==Xdialog.array[i].state-1)
		  {
		    char *temp=new char[MAX_LABEL_LENGTH];
		    strcpysafe(temp,Xdialog.array[k].tips,MAX_LABEL_LENGTH);
		    strcatsafe(temp,"/",MAX_LABEL_LENGTH);
		    strcatsafe(temp,Xdialog.array[i].name,MAX_LABEL_LENGTH);
		    strcpysafe(Xdialog.array[i].tips,temp,MAX_LABEL_LENGTH);
		    break;
		  }
		  if(Xdialog.array[k].state==0)
		  {
		    strcpysafe(Xdialog.array[i].tips,Xdialog.array[i].name,MAX_LABEL_LENGTH);
		    break;
		  }
		}
		tree->add(Xdialog.array[i].tips);
		if(state==1)
		  tree->select(Xdialog.array[i].tips);

	}

	/*for(int i=0;i<list_size;i++)
	{
	  tree->add(Xdialog.array[i].tips);
	}*/
	glist_size=list_size;
	Xdialog.type=5;
	create_buttons();
	Xdialog.window->end();
	Xdialog.window->show();
	set_timeout();
}

//-------------------------------------------------------------------------

void create_filesel(gchar *optarg, gboolean dsel_flag)
{
  int check_height = CHECK_BUTTON_HEIGHT;
  int button_height = BUTTON_HEIGHT;
  int backtitle_height = BACKTITLE_HEIGHT;

  font_init();

	if (!Xdialog.check)
		check_height = 0;

	if (!Xdialog.buttons)
		button_height = 0;

	if (strlen(Xdialog.backtitle) == 0)
		backtitle_height = 10;

	//open_window();
	set_backtitle(true);
	char *output=new char[MAX_LABEL_LENGTH];
	output[0]=0;

	if(!dsel_flag) {
	  Fl_File_Chooser *chooser=new Fl_File_Chooser(optarg,"*.*",Fl_File_Chooser::SINGLE,Xdialog.title);
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
	  Fl_File_Chooser *chooser=new Fl_File_Chooser(optarg,"*.*",Fl_File_Chooser::DIRECTORY,Xdialog.title);
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
  if(Xdialog.xsize==0&&Xdialog.ysize==0)
  {
    Xdialog.xsize=40;
    Xdialog.ysize=10;
  }
  open_window();
  int row_hight = 0;
	if (Xdialog.ysize>=60)
	{
		row_hight = (Xdialog.ysize - BUTTON_HEIGHT)*0.5;
	}
	Fl_Box *box = new Fl_Box(5, row_hight, Xdialog.xsize - 10, row_hight);
	box->box(FL_BORDER_BOX);
	Fl_Spinner *spinner1 = new Fl_Spinner(Xdialog.xsize*0.16, row_hight*1.5, 50, 20);
	spinner1->range(0,23);
	spinner1->type(FL_FLOAT_INPUT);
	spinner1->value(hours);
	Fl_Spinner *spinner2 = new Fl_Spinner(Xdialog.xsize*0.48, row_hight*1.5, 50, 20);
	spinner2->range(0, 59);
	spinner2->type(FL_FLOAT_INPUT);
	spinner2->value(minutes);
	Fl_Spinner *spinner3 = new Fl_Spinner(Xdialog.xsize*0.80, row_hight*1.5, 50, 20);
	spinner3->range(0, 59);
	spinner3->type(FL_FLOAT_INPUT);
	spinner3->value(seconds);
	set_label(":",true,Xdialog.xsize*0.33,row_hight*1.5,20,20);
	set_label(":",true,Xdialog.xsize*0.66, row_hight*1.5, 20, 20);
	Fl_Box *box1 = new Fl_Box(Xdialog.xsize*0.5 - 100, row_hight, 200, 1);
	set_label( "Hours:Minutes:Seconds",true,Xdialog.xsize*0.5 - 50, row_hight, 100, 0);
	box1->box(FL_FLAT_BOX);
	set_label(optarg,true, 0,0, Xdialog.xsize, row_hight);
	set_backtitle(true);
	create_buttons();
	if (Xdialog.interval>0)
	{
		Fl::add_timeout(Xdialog.interval/1000,OutputCallback_timebox);
	}
		set_timeout();
	Xdialog.type=6;
	Xdialog.widget1=(Fl_Widget *)spinner1;
	Xdialog.widget2=(Fl_Widget *)spinner2;
	Xdialog.widget3=(Fl_Widget *)spinner3;
}



