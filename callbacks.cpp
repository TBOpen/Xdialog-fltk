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

void click_yes(Fl_Widget *o, void*)
{
	if (Xdialog.check) {
		if (Xdialog.checked)
			fprintf(Xdialog.output, "checked\n");
		else
			fprintf(Xdialog.output, "unchecked\n");
	}

	Xdialog.exit_code = 0;
	if(Xdialog.type==1)	{
	  Fl_Input *input=(Fl_Input *)Xdialog.widget1;
	 fprintf(Xdialog.output,"%s",input->value());
	 fprintf(Xdialog.output,"\n");
	}

	if(Xdialog.type==2)	{
	  Fl_Multiline_Output *input;
	  input=(Fl_Multiline_Output *)Xdialog.widget1;
	 fprintf(Xdialog.output,"%s",input->value());
	 fprintf(Xdialog.output,"\n");
	}

	if(Xdialog.type==3)	{
	  Fl_Text_Editor *input;
	  input=(Fl_Text_Editor *)Xdialog.widget1;
	  Fl_Text_Buffer *buffer=input->buffer();
	  fprintf(Xdialog.output,"%s",buffer->text());
	  fprintf(Xdialog.output,"\n");
	}

	if(Xdialog.type==4)	{
	  Fl_Browser *input=(Fl_Browser *)Xdialog.widget1;
	  for(int i=0;i<input->size()+1;i++) {
	    if(input->selected(i)==1) {
	      fprintf(Xdialog.output,"%s",Xdialog.array[i-1].tag);
	      printf("tag::%s\n",Xdialog.array[i-1].tag);
	    }
	  }
	}

	if(Xdialog.type==5) {
	  Fl_Tree *tree=(Fl_Tree *)Xdialog.window->child(0);
	  for(int i=0;i<glist_size;i++) {
	    if(tree->is_selected(Xdialog.array[i].tips)==1)
	      printf("%s\n",Xdialog.array[i].tag);
    }
	}

	if(Xdialog.type==6)	{
	  Fl_Spinner *spinner1=(Fl_Spinner *)Xdialog.widget1;
	  Fl_Spinner *spinner2=(Fl_Spinner *)Xdialog.widget2;
	  Fl_Spinner *spinner3=(Fl_Spinner *)Xdialog.widget3;
	  int hours=spinner1->value();
	  int mins=spinner2->value();
	  int seconds=spinner3->value();
	  fprintf(Xdialog.output,"time %d:%d:%d\n",hours,mins,seconds);
	}

	if(Xdialog.type==7)	{
	  char *outString=new char[MAX_LABEL_LENGTH];
	  outString[0]=0;
	  Fl_Browser *second = (Fl_Browser *)Xdialog.window->child(1);
	  for(int i=1;i<second->size()+1;i++) {
	    char *temp=new char[50];
	    temp[0]=0;
	    strcpysafe(temp,(char*)second->text(i),MAX_LABEL_LENGTH);
	    char *tag=getTag(temp);
	    strcat(outString,tag);
	    strcat(outString,"|");
	  }
	  outString[strlen(outString)-1]='\0';
	  fprintf(Xdialog.output,"%s",outString);
	}

	if(Xdialog.type==8)	{
	  Fl_Check_Browser *check_browser=(Fl_Check_Browser *)Xdialog.window->child(0);
	  char *outString=new char[MAX_LABEL_LENGTH];
	  outString[0]=0;
	  for(int i=1;i<check_browser->nitems()+1;i++) {
	    if(check_browser->checked(i)==1) {
	    strcat(outString,check_browser->text(i));
	    strcat(outString,"/");

	    }
	  }
	  outString[strlen(outString)-1]='\0';
	  fprintf(Xdialog.output,"%s",outString);
	}

	if(Xdialog.type==9) {
	  Fl_Slider *slider=(Fl_Slider *)Xdialog.window->child(0);
	  printf("%d\n",(int)slider->value());
	}

	Xdialog.window->hide();
}

//-------------------------------------------------------------------------

void click_no(Fl_Widget *o, void*)
{
	Xdialog.exit_code = 1;
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
	Xdialog.exit_code = 2;
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
	if (o->parent()->child(0)->type()!= FL_SECRET_INPUT) {
		o->parent()->child(0)->type(FL_SECRET_INPUT);
		o->parent()->child(0)->redraw();
	}
	else
	{
		o->parent()->child(0)->type(FL_NORMAL_INPUT);
		o->parent()->child(0)->redraw();
	}
}

//-------------------------------------------------------------------------

void OutputCallback_rangebox(void *)
{
	Fl_Box *slider = (Fl_Box *)Xdialog.window->child(0);
	printf("%s\n", slider->label());
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

void OutputCallback_ItemList(void *)
{
	Fl_Check_Browser *browser = (Fl_Check_Browser *)Xdialog.window->child(0);
	char *temp = new char[128];
	temp[0] = 0;
	for (int i = 1; i <= browser->nitems(); i++) {
		if (browser->checked(i) == 1)	{
			strcatsafe(temp, Xdialog.array[i - 1].tag, 128);
			strcatsafe(temp, "/", 128);
		}
	}

	if (temp!="")	{
		temp[strlen(temp) - 1] = 0;
		printf("%s\n", temp);
	}

	Fl::repeat_timeout(Xdialog.interval / 1000, OutputCallback_ItemList);
}

//-------------------------------------------------------------------------

void OutputCallback_menubox(void *)
{
	Fl_Browser *browser = (Fl_Browser *)Xdialog.widget1;
	int a = browser->size();
	for (int i = 0; i<browser->size();i++)
	{
		if (browser->selected(i+1)==1)
		{
			printf("%s\n",Xdialog.array[i].tag);
		}
	}
	Fl::repeat_timeout(Xdialog.interval/1000,OutputCallback_menubox);
}

//-------------------------------------------------------------------------

void OutputCallback_timebox(void *)
{
	int hours = 0;
	int mins = 0;
	int seconds = 0;
	Fl_Spinner *hour = (Fl_Spinner *)Xdialog.window->child(1);
	Fl_Spinner *min = (Fl_Spinner *)Xdialog.window->child(2);
	Fl_Spinner *second = (Fl_Spinner *)Xdialog.window->child(3);
	hours = hour->value();
	mins = min->value();
	seconds = second->value();
	printf("%d:%d:%d\n",hours,mins,seconds);
	Fl::repeat_timeout(Xdialog.interval/1000,OutputCallback_timebox);
}

//-------------------------------------------------------------------------

void AddCallback(Fl_Widget *o,void *)
{
	int check_state=0;
	int check_index=0;
	char *selected_item=new char[MAX_LABEL_LENGTH];
	Fl_Button *add = (Fl_Button *)o;
	Fl_Button *remove = (Fl_Button *)o->parent()->child(3);
	Fl_Browser *first = (Fl_Browser *)o->parent()->child(0);
	Fl_Browser *second = (Fl_Browser *)o->parent()->child(1);
	int size=first->size();
	for(int i=1;i<=size;i++) {
	  if(first->selected(i)==1) {
	    const char *temp=first->text(i);
	    second->add(temp);
	  }
	}

	for(int i=1;i<=size;i++) {
	  if(first->selected(i)==1) {
	    first->remove(i);
	    i=0;
	  }
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
	Fl_Button *add = (Fl_Button *)o->parent()->child(2);
	Fl_Browser *first = (Fl_Browser *)o->parent()->child(0);
	Fl_Browser *second = (Fl_Browser *)o->parent()->child(1);
	int size=second->size();
	for(int i=1;i<=size;i++) {
	  if(second->selected(i)==1) {
	    const char *temp=second->text(i);
	    first->add(temp);
	  }
	}
	for(int i=1;i<=size;i++) {
	  if(second->selected(i)==1) {
	    second->remove(i);
	    i=0;
	  }
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
  ret=my_scanf(temp);
  int new_val=0;
  if (ret == EOF && !Xdialog.ignore_eof)
	return click_yes(NULL, NULL);
  if (ret != 1)
	return ;
  if (!Xdialog.new_label && strcmp(temp, "XXX")) {
	new_val = (float) atoi(temp);
	Fl_Progress *temp_pro=(Fl_Progress *)Xdialog.widget1;
	temp_pro->value(new_val);
	char *label_value=new char[5];
	sprintf(label_value,"%d%%",new_val);
	temp_pro->label(label_value);
	//printf("log1\n");
  } else {
	if (strcmp(temp, "XXX") == 0) {
		if (Xdialog.new_label) {
			//printf("new labe %s\n",Xdialog.label_text);
			Fl_Box *temp_box=(Fl_Box *)Xdialog.widget4;
			temp_box->labelfont(0);
			temp_box->labelcolor(FL_BLACK);
			temp_box->align(FL_ALIGN_WRAP);
			temp_box->labelsize(17);
			temp_box->label(Xdialog.label_text);
			temp_box->redraw();
			//Xdialog.label_text[0] = 0 ;
			Xdialog.new_label = false;
		} else {
			Xdialog.new_label = true;
			//printf("log2\n");
		}
	} else {

		if (strlen(Xdialog.label_text)+strlen(temp)+2 < MAX_LABEL_LENGTH) {
			if (strcmp(temp, "\\n") == 0) {
				strcat(Xdialog.label_text, "\n");
				//printf("log3\n");
				} else {
					strcat(Xdialog.label_text, " ");
					strcat(Xdialog.label_text, temp);
					//printf("log4\n");
				}
			}
		}
	}
  Fl::repeat_timeout(1,gauge_timeout);

}
