/*
 * Callback functions for Xdialog.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#ifdef STDC_HEADERS
#	include <stdlib.h>
#	include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#	include <unistd.h>
#endif
#include <time.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

extern Xdialog_data Xdialog;
extern int glist_size;


//-------------------------------------------------------------------------

void timeout_exit(void *)
{
	Xdialog.exit_code = 255;
	Xdialog.window->hide();
}

//-------------------------------------------------------------------------

static char *getTag(char *input)
{
  for(int i=0;i<glist_size;i++) {
    if(strcmp(Xdialog.array[i].name,input)==0) {
      return Xdialog.array[i].tag;
    }
  }
}

//-------------------------------------------------------------------------

static void OutputTree()
{
	Fl_Tree *tree=(Fl_Tree *)Xdialog.widget1;
	for(int i=0;i<glist_size;i++) {
		if(tree->is_selected(Xdialog.array[i].tips)==1)
			fprintf(Xdialog.output, "%s\n",Xdialog.array[i].tag);
	}
}

//-------------------------------------------------------------------------

static void OutputChecklist()
{
	Fl_Check_Browser *check_browser=(Fl_Check_Browser *)Xdialog.widget1;
	bool firstprint=true;
	for(int i=0; i<check_browser->nitems(); ) {
		i++;
		if (check_browser->checked(i)==1) {
			if (firstprint)
				firstprint=false;
			else
				fprintf(Xdialog.output, " ");
			fprintf(Xdialog.output, "%s", Xdialog.array[i-1].tag);
		}
	}
}

//-------------------------------------------------------------------------

static void OutputTimeBox()
{
	Fl_Spinner *spinner1=(Fl_Spinner *)Xdialog.widget1;
	Fl_Spinner *spinner2=(Fl_Spinner *)Xdialog.widget2;
	Fl_Spinner *spinner3=(Fl_Spinner *)Xdialog.widget3;
	int hours=spinner1->value();
	int mins=spinner2->value();
	int seconds=spinner3->value();
	fprintf(Xdialog.output,"time %d:%d:%d\n",hours,mins,seconds);
}

//-------------------------------------------------------------------------

static void OutputBuildList()
{
	Fl_Browser *second = (Fl_Browser *)Xdialog.widget2;
	for(int i=1;i<second->size()+1;i++) {
		// find what tag it is by looking for name
		ssize_t index=(ssize_t) second->data(i);
		fprintf(Xdialog.output,"|%s", Xdialog.array[index].tag);
	}
}

//-------------------------------------------------------------------------

static void OutputMenuBox()
{
	Fl_Browser *input=(Fl_Browser *)Xdialog.widget1;
	for(int i=1;i<input->size()+1;i++) {
		if(input->selected(i)==1) {
			fprintf(Xdialog.output,"%s",Xdialog.array[i-1].tag);
		}
	}
}

//-------------------------------------------------------------------------

void click_yes(Fl_Widget *o, void*)
{
	if (Xdialog.check) {
		if (Xdialog.checked)
			fprintf(Xdialog.output, "checked\n");
		else
			fprintf(Xdialog.output, "unchecked\n");
	}

	Xdialog.exit_code = XDIALOG_PRESS_YES;

	if(Xdialog.type==XDIALOG_TYPE_INPUT)	{
		Fl_Input *input=(Fl_Input *)Xdialog.widget1;
		fprintf(Xdialog.output,"%s",input->value());
		fprintf(Xdialog.output,"\n");
	}

	/* No output for textbox
	if(Xdialog.type==XDIALOG_TYPE_EDIT_READONLY)	{
	  Fl_Multiline_Output *input;
	  input=(Fl_Multiline_Output *)Xdialog.widget1;
	 fprintf(Xdialog.output,"%s",input->value());
	 fprintf(Xdialog.output,"\n");
	}
	*/

	if(Xdialog.type==XDIALOG_TYPE_EDIT)	{
	  Fl_Text_Editor *input;
	  input=(Fl_Text_Editor *)Xdialog.widget1;
	  Fl_Text_Buffer *buffer=input->buffer();
	  fprintf(Xdialog.output,"%s",buffer->text());
	  fprintf(Xdialog.output,"\n");
	}

	if(Xdialog.type==XDIALOG_TYPE_MENU)	{
		OutputMenuBox();
	}

	if(Xdialog.type==XDIALOG_TYPE_TREE) {
			OutputTree();
	}

	if(Xdialog.type==XDIALOG_TYPE_TIME)	{
		OutputTimeBox();
	}

	if(Xdialog.type==XDIALOG_TYPE_BROWSERLIST)	{
		OutputBuildList();
	}

	if(Xdialog.type==XDIALOG_TYPE_CHECKLIST)	{
		OutputChecklist();
	}

	if(Xdialog.type==XDIALOG_TYPE_RANGEBOX) {
	  Fl_Slider *slider=(Fl_Slider *)Xdialog.widget1;
	  fprintf(Xdialog.output, "%d\n",(int)slider->value());
	}

	Xdialog.window->hide();
}

//-------------------------------------------------------------------------

void click_no(Fl_Widget *o, void*)
{
	Xdialog.exit_code = XDIALOG_PRESS_NO;
	Xdialog.window->hide();
}

//-------------------------------------------------------------------------

void click_pre(Fl_Widget *o, void*)
{
	Xdialog.exit_code = 3;
	Xdialog.window->hide();
}

//-------------------------------------------------------------------------

void click_next(Fl_Widget *o, void*)
{
	if (Xdialog.check) {
		if (Xdialog.checked)
			fprintf(Xdialog.output, "checked\n");
		else
			fprintf(Xdialog.output, "unchecked\n");
	}
	Xdialog.exit_code = 0;
	Xdialog.window->hide();
}

//-------------------------------------------------------------------------

void click_help(Fl_Widget *o, void*)
{
	Xdialog.exit_code = XDIALOG_PRESS_HELP;
	Xdialog.window->hide();
}

//-------------------------------------------------------------------------

void click_print(Fl_Widget *o,void *)
{
	int length=0, i;
	char * buffer = new char[0];
	char cmd[MAX_PRTCMD_LENGTH];
	FILE * temp;

	strcpysafe(cmd, PRINTER_CMD, MAX_PRTCMD_LENGTH);
	if (strlen(Xdialog.printer) != 0) {
		strcatsafe(cmd, " " PRINTER_CMD_OPTION, MAX_PRTCMD_LENGTH);
		strcatsafe(cmd, Xdialog.printer, MAX_PRTCMD_LENGTH);
	}

	temp = popen(cmd, "w");
	if (temp != NULL) {
		i = fwrite(buffer, sizeof(char), length, temp);
		pclose(temp);
	}
	free(buffer);
}

//-------------------------------------------------------------------------

void CheckCallback(Fl_Widget *o, void *)
{
	Fl_Check_Button *checker = (Fl_Check_Button *)o;
	checker->redraw();
	if (checker->value() == 1)
		Xdialog.checked = true;
	else
		Xdialog.checked = false;
}

//-------------------------------------------------------------------------

void HidetypingCallback(Fl_Widget *o,void *data)
{
	int type_mode = o->type();
	Fl_Input *input=(Fl_Input*) Xdialog.widget1;
	if (input->type()!= FL_SECRET_INPUT) {
		input->type(FL_SECRET_INPUT);
		input->redraw();
	}
	else
	{
		input->type(FL_NORMAL_INPUT);
		input->redraw();
	}
}

//-------------------------------------------------------------------------

void OutputCallback_rangebox(void *)
{
	Fl_Box *slider = (Fl_Box *)Xdialog.widget1;
	fprintf(Xdialog.output, "%s\n", slider->label());
	Fl::repeat_timeout(Xdialog.interval/1000,OutputCallback_rangebox);
}

//-------------------------------------------------------------------------

void sliderCallback(Fl_Object *o,void *data)
{
  Fl_Slider *slider=(Fl_Slider *)o;
  char *value = new char[10];
  sprintf(value, "%d", (int)slider->value());
  slider->redraw_label();
  slider->redraw();
  slider->label(value);

}

//-------------------------------------------------------------------------

void OutputCallback_CheckList(void *)
{
	OutputChecklist();
	Fl::repeat_timeout(Xdialog.interval / 1000, OutputCallback_CheckList);
}

//-------------------------------------------------------------------------


void OutputCallback_menubox(void *)
{
	OutputMenuBox();
	Fl::repeat_timeout(Xdialog.interval/1000,OutputCallback_menubox);
}

//-------------------------------------------------------------------------

void OutputCallback_timebox(void *)
{
	OutputTimeBox();
	Fl::repeat_timeout(Xdialog.interval/1000,OutputCallback_timebox);
}

//-------------------------------------------------------------------------

void OutputCallback_BuildList(void *)
{
	OutputBuildList();
	Fl::repeat_timeout(Xdialog.interval / 1000, OutputCallback_BuildList);
}

//-------------------------------------------------------------------------

void OutputCallback_treeview(void *)
{
	OutputTree();
	Fl::repeat_timeout(Xdialog.interval / 1000, OutputCallback_treeview);
}

//-------------------------------------------------------------------------

void AddCallback(Fl_Widget *o,void *)
{
	int check_state=0;
	int check_index=0;
	char *selected_item=new char[MAX_LABEL_LENGTH];

	Fl_Button *add = (Fl_Button *)o;
	Fl_Button *remove = (Fl_Button*) Xdialog.widget4;
	Fl_Browser *first = (Fl_Browser*) Xdialog.widget1;
	Fl_Browser *second = (Fl_Browser*) Xdialog.widget2;

	for(int i=1;i<=first->size();) {
	  if(first->selected(i)==1) {
	    const char *temp=first->text(i);
	    second->add(temp, first->data(i));
	    first->remove(i);
	  }
	  else i++;
	}

	if(first->size()==0)
	  add->deactivate();

	if(first->size()>0)
	  add->activate();

	if(second->size()==0)
	  remove->deactivate();

	if(second->size()>0)
	  remove->activate();
}

//-------------------------------------------------------------------------

void RemoveCallback(Fl_Widget *o, void *)
{
	int check_state=0;
	int check_index=0;
	char *selected_item=new char[MAX_LABEL_LENGTH];

	Fl_Button *remove = (Fl_Button *)o;
	Fl_Button *add = (Fl_Button*) Xdialog.widget3;
	Fl_Browser *first = (Fl_Browser*) Xdialog.widget1;
	Fl_Browser *second = (Fl_Browser*) Xdialog.widget2;

	for(int i=1;i<=second->size();) {
	  if(second->selected(i)==1) {
	    const char *temp=second->text(i);
	    first->add(temp, second->data(i));
	    second->remove(i);
	  }
	  else i++;
	}

	if(first->size()==0)
	  add->deactivate();

	if(first->size()>0)
	  add->activate();

	if(second->size()==0)
	  remove->deactivate();

	if(second->size()>0)
	  remove->activate();
}

//-------------------------------------------------------------------------

void info_timeout(void *)
{
	click_yes(NULL, NULL);
}

//-------------------------------------------------------------------------

void gauge_timeout(void *)
{
  char temp[MY_SCANF_BUFSIZE];
  int ret;

  do {
		ret=my_scanf(temp);

		if (ret == EOF && !Xdialog.ignore_eof)
			return click_yes(NULL, NULL);

		if (ret == 1) {
			int new_val=0;
			bool toggle=(strcmp(temp, "XXX")==0);

			if (!Xdialog.new_label && !toggle) {
				new_val = (float) atoi(temp);
				Fl_Progress *temp_pro=(Fl_Progress *)Xdialog.widget1;
				temp_pro->value(new_val);
				char *label_value=new char[5];
				sprintf(label_value,"%d%%",new_val);
				temp_pro->label(label_value);
			}
			else {
				if (toggle) {
					if (Xdialog.new_label) {
						Fl_Box *temp_box=(Fl_Box *)Xdialog.widget4;
						char *oldlabel=(char*) temp_box->label();
						char *newlabel=new char[MAX_LABEL_LENGTH];
						backslash_n_to_linefeed(Xdialog.label_text, newlabel, MAX_LABEL_LENGTH);
						temp_box->label(newlabel);
						Xdialog.label_text[0]=0;
						delete[] oldlabel;
						temp_box->redraw();
						Xdialog.new_label = false;
					}
					else {
						Xdialog.new_label = true;
					}
				}
				else {
					// myscanf seems to skip "\n" input by putting "\" on one line and "n" on another works.
					strcatsafe(Xdialog.label_text, temp, MAX_LABEL_LENGTH);
				}
			}
		}
  } while(ret==1);

	Fl::repeat_timeout(1, gauge_timeout);
}

//-------------------------------------------------------------------------

void BrowserCallback(Fl_Widget *o, void *)
{
	// check for double click
	if (Fl::event_clicks()) {
		// yes, so consider it a click on ok/yes
		if(Xdialog.type==XDIALOG_TYPE_MENU)	{
			Xdialog.exit_code = XDIALOG_PRESS_YES;
			OutputMenuBox();
			Xdialog.window->hide();
		}

		if(Xdialog.type==XDIALOG_TYPE_CHECKLIST)	{
			Xdialog.exit_code = XDIALOG_PRESS_YES;
			OutputChecklist();
			Xdialog.window->hide();
		}
	}
	// for this type we get the callback when enter pressed (get call back on double click too so use else)
	else if(Xdialog.type==XDIALOG_TYPE_BROWSERLIST)	{
		Xdialog.exit_code = XDIALOG_PRESS_YES;
		OutputBuildList();
		Xdialog.window->hide();
	}

}
