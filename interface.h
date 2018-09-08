/*
 * Command line parsing and main routines for Xdialog.

 g++ -std=c++11 `fltk-config --cxxflags` main.cpp  `fltk-config --ldflags` -o Xdialog

 */

// #define NO_FLTK_PATCH

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif
#include <iostream>
#include <stdio.h>
#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef ENABLE_NLS
#include <locale.h>
#endif
#include<stdlib.h>
#include<errno.h>
#include<stdio.h>
#include <fcntl.h>
#include <time.h>
#include <getopt.h>
#include <unistd.h>

#include<FL/Fl.H>
#include <FL/Fl_Spinner.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Pack.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_File_Browser.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Select_Browser.H>
#include <FL/Fl_Check_Browser.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Hor_Slider.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Multiline_Output.H>

#define GTK_WIN_POS_MOUSE 10
#define GTK_WIN_POS_CENTER 11
#define GTK_WIN_POS_NONE 12
#define GTK_JUSTIFY_LEFT 13
#define GTK_JUSTIFY_RIGHT 14
#define GTK_JUSTIFY_CENTER 15
#define GTK_JUSTIFY_FILL 16
////////
#define XDIALOG "Xdialog"	/* Default Xdialog window title */

#define INFO_TIME 1000		/* the number of ms an infobox should stay up*/

/* Names for environment variables */
#define HIGH_DIALOG_COMPAT	"XDIALOG_HIGH_DIALOG_COMPAT"
#define FORCE_AUTOSIZE		"XDIALOG_FORCE_AUTOSIZE"
#define INFOBOX_TIMEOUT		"XDIALOG_INFOBOX_TIMEOUT"
/* Temporary pipe filename used for printing */
#define TEMP_FILE               "/tmp/Xdialog.tmp"

/* You may change these as well IOT change the Xdialog limitations... */

#define MAX_TREE_DEPTH 24
#define MAX_LABEL_LENGTH 2048
#define MAX_INPUT_DEFAULT_LENGTH 1024
#define MAX_BUTTON_LABEL_LENGTH 32
#define MAX_TITLE_LENGTH 64
#define MAX_BACKTITLE_LENGTH 256
#define MAX_CHECK_LABEL_LENGTH 256
#define MAX_WMCLASS_LENGTH 32
#define MAX_PRTNAME_LENGTH 64
#define MAX_ITEM_LENGTH 128
#define MAX_FILENAME_LENGTH 256
#define MAX_PRTNAME_LENGTH 64
#define MAX_PRTCMD_LENGTH MAX_PRTNAME_LENGTH+32
#define CHECK_BUTTON_HEIGHT 10
#define SLIDER_HEIGHT 30
#define SPINNER_HEIGHT 20
#define SPINNER_WIDTH 50
#define CLIENT_BORDER_SIZE 10
#define BORDER_SIZE 5
#define LABEL_TEXT_HEIGHT 14
#define XDIALOG_BUTTON_HEIGHT 25
#define XDIALOG_BUTTON_WIDTH 80


#define XDIALOG_MAX_BUTTONS 4

/* The following defines should be changed via the "configure" options, type:
*    ./configure --help
* to learn more about these options.
*/

#ifndef FIXED_FONT
#define FIXED_FONT "-*-*-medium-r-normal-*-*-*-*-*-m-70-*-*"
#endif

#ifndef PRINTER_CMD
#define PRINTER_CMD "lpr" 		/* Command to be used IOT print text */
#endif
#ifndef PRINTER_CMD_OPTION
#define PRINTER_CMD_OPTION "-P"		/* The option to specify the prt queue */
#endif
#ifdef HAVE_CATGETS			/* We don't support catgets */
#error Xdialog does not get catgets support, try: ./configure --enable-nls
#endif

#ifdef ENABLE_NLS			/* NLS is supported by default */
#include <libintl.h>
#define _(s) gettext(s)
#else
#undef _
#define _(s) s
#endif

#ifdef FRENCH				/* french translations without NLS */
#	define OK "OK"
#	define CANCEL "Annuler"
#	define YES "Oui"
#	define NO "Non"
#	define HELP "Aide"
#	define PREVIOUS "Précédent"
#	define NEXT "Suivant"
#	define PRINT "Imprimer"
#	define ADD "Ajouter"
#	define REMOVE "Retirer"
#	define TIME_FRAME_LABEL "Heures : Minutes : Secondes"
#	define TIME_STAMP "Heure"
#	define DATE_STAMP "Date - Heure"
#	define LOG_MESSAGE "Message"
#	define HIDE_TYPING "Masquer la saisie"
#else
#	define OK _("OK")
#	define CANCEL _("Cancel")
#	define YES _("Yes")
#	define NO _("No")
#	define HELP _("Help")
#	define PREVIOUS _("Previous")
#	define NEXT _("Next")
#	define PRINT _("Print")
#	define ADD _("Add")
#	define REMOVE _("Remove")
#	define TIME_FRAME_LABEL _("Hours : Minutes : Seconds")
#	define TIME_STAMP _("Time stamp")
#	define DATE_STAMP _("Date - Time")
#	define LOG_MESSAGE _("Log message")
#	define HIDE_TYPING _("Hide typing")
#endif

/* The following defines should not be changed. */

#define XSIZE_MULT 8
#define YSIZE_MULT 12

#define ALPHANUM_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"

#define FIXED_FONT_RC_STRING "style 'fixed_font' { font = \"" FIXED_FONT "\" }\nwidget '*' style 'fixed_font'"

#define ICON_AND_TEXT -1
#define ICON_ONLY 0
#define TEXT_ONLY 1

#define RADIOLIST 1
#define CHECKLIST 2

#define BEEP_BEFORE 1
#define BEEP_AFTER  2

#define XDIALOG_PRESS_YES           0
#define XDIALOG_PRESS_NO						1
#define XDIALOG_PRESS_HELP          2


#define XDIALOG_TYPE_INPUT          1
#define XDIALOG_TYPE_EDIT_READONLY 	2
#define XDIALOG_TYPE_EDIT						3
#define XDIALOG_TYPE_MENU           4
#define XDIALOG_TYPE_TREE					  5
#define XDIALOG_TYPE_TIME           6
#define XDIALOG_TYPE_BROWSERLIST 		7
#define XDIALOG_TYPE_CHECKLIST 			8
#define XDIALOG_TYPE_RANGEBOX 			9


///////////

#define FALSE 0
#define TRUE  1

typedef int gboolean;
typedef int gint;
typedef double gdouble;
typedef char gchar;
typedef void* gpointer;


/* Structure definitions. */

typedef struct {
	int state;
	//GtkWidget *widget;
	Fl_Widget *widget;
	char tag[MAX_ITEM_LENGTH];
	char name[MAX_ITEM_LENGTH];
	char tips[MAX_ITEM_LENGTH];
} listname;

typedef struct	{
	char		title[MAX_TITLE_LENGTH];		/* Xdialog window title */
	char		backtitle[MAX_BACKTITLE_LENGTH];	/* Backtitle label */
	char		wmclass[MAX_WMCLASS_LENGTH];		/* Xdialog wmclass name */
	char		label_text[MAX_LABEL_LENGTH];		/* New label text (for infobox and gauge) */
	char		check_label[MAX_CHECK_LABEL_LENGTH];	/* Check button label text */
	char		ok_label[MAX_BUTTON_LABEL_LENGTH];	/* OK button label text */
	char		cancel_label[MAX_BUTTON_LABEL_LENGTH];	/* CANCEL button label text */
	char		default_item[MAX_ITEM_LENGTH];		/* Tag of the default item */
	char		separator[2];				/* Xdialog output result separator */
	int		xsize;					/* Xdialog window X size */
	int		ysize;					/* Xdialog window Y size */
	int		xorg;					/* Xdialog window X origin */
	int		yorg;					/* Xdialog window Y origin */
	int		list_height;				/* Xdialog menu/list height (in lines) */
	int		placement;				/* Xdialog window placement method */
	int		justify;				/* Xdialog labels justification method */
	int		buttons_style;				/* Xdialog buttons style */
	int		interval;				/* Xdialog output result time interval */
	int		timeout;				/* Xdialog user input timeout (in seconds) */
	int		timer;					/* Xdialog timer routine */
	int		timer2;					/* Xdialog timer routine #2 (for user timeout) */
	int		passwd;					/* Password flags for text entries */
	int		tips;					/* Tips flag (0 or 1) for tips in menu/list */
	int		beep;					/* Beep flag */
	bool	size_in_pixels;				/* TRUE if xsize/ysize are in pixels */
	bool	set_origin;				/* TRUE if window origin to be set */
	bool	new_label;				/* TRUE if a start label delimiter received */
	bool	wrap;					/* TRUE if label text is to be auto-wrapped */
	bool	cr_wrap;				/* TRUE to wrap at linefeeds */
	bool	fixed_font;				/* TRUE if fixed font to be used in text */
	bool	tags;					/* TRUE if tags to be displayed in menu/list */
	int		buttons;			/* TRUE if display buttons */
	bool	ok_button;				/* FALSE to prevent setting up OK button in tailbox/logbox */
	bool	cancel_button;		/* FALSE to prevent setting up Cancel button */
	bool  yesno_button;		/* TRUE if ok/cancel are yes/no */
	bool	help;					/* TRUE to setup the Help button */
	bool	default_no;				/* When TRUE No/Cancel is the default button */
	bool	wizard;					/* TRUE to setup Wizard buttons */
	bool	print;					/* TRUE to setup the Print button */
	bool	check;					/* TRUE to setup the check button */
	bool	checked;				/* TRUE when check button is checked */
	bool	icon;					/* TRUE to setup an icon */
	bool	no_close;				/* TRUE to forbid Xdialog window closing */
	bool	editable;				/* TRUE for an editable combobox */
	bool	time_stamp;				/* TRUE for time stamps in logbox */
	bool	date_stamp;				/* TRUE for date stamps in logbox */
	bool	reverse;				/* TRUE for reverse order in logbox */
	bool	keep_colors;				/* TRUE to remember colors in logbox */
	bool	ignore_eof;				/* TRUE to ignore EOF in infobox/gauge */
	bool	smooth;					/* TRUE for smooth scrolling in tail/log */
	/*GtkWidget *	window;
	GtkBox *	vbox;
	GtkWidget *	widget1;
	GtkWidget *	widget2;
	GtkWidget *	widget3;
	GtkWidget *	widget4;
	guint		status_id;
	*/
	Fl_Window *	window;
	Fl_Pack *vbox;
	Fl_Widget *	widget1;
	Fl_Widget *	widget2;
	Fl_Widget *	widget3;
	Fl_Widget *	widget4;
	int		status_id;
	FILE *		output;
	FILE *		file;
	long		file_init_size;				/* for logbox and tailbox */
	char		icon_file[MAX_FILENAME_LENGTH];
	char		rc_file[MAX_FILENAME_LENGTH];
	char		printer[MAX_PRTNAME_LENGTH];
	int		exit_code;
	int 		type;
	char * 		saveFileName;
	char *		content;
	listname *	array;
} Xdialog_data;

///////////

/* interface.c functions prototypes */

void open_window(void);

void get_maxsize(int *x, int *y);

void create_msgbox(gchar *optarg, gboolean yesno);

void create_infobox(gchar *optarg, gint timeout);

void create_gauge(gchar *optarg, gint percent);

void create_progress(gchar *optarg, gint leading, gint maxdots);

void create_tailbox(gchar *optarg);

void create_logbox(gchar *optarg);

void create_textbox(gchar *optarg, gboolean editable);

void create_inputbox(gchar *optarg, gchar *options[], gint entries);

void create_combobox(gchar *optarg, gchar *options[], gint list_size);

void create_rangebox(gchar *optarg, gchar *options[], gint ranges);

void create_spinbox(gchar *optarg, gchar *options[], gint spins);

void create_itemlist(gchar *optarg, gint type, gchar *options[], gint list_size);

void create_buildlist(gchar *optarg, gchar *options[], gint list_size);

void create_treeview(gchar *optarg, gchar *options[], gint list_size);

void create_menubox(gchar *optarg, gchar *options[], gint list_size);

void create_filesel(gchar *optarg, gboolean dsel_flag);

void create_colorsel(gchar *optarg, gdouble *colors);

void create_fontsel(gchar *optarg);

void create_calendar(gchar *optarg, gint day, gint month, gint year);

void create_timebox(gchar *optarg, gint hours, gint minutes, gint seconds);
