/*
 * defines for the support functions.
 */

extern void strcpysafe(char *dest, const char *source, int maxchars);
extern void strcatsafe(char *dest, const char *source, int maxchars);
extern void backslash_n_to_linefeed(const char *s0, char *s, int max_len);
extern void trim_string(const char *s0, char *s, int max_len);
extern void Xdialog_array(gint elements);
#ifndef USE_SCANF
#define MY_SCANF_BUFSIZE 256
extern int my_scanf(char *buf);
extern int my_fgetc(void);
#endif
