#ifndef __DARNIT_MENUTK_H__
#define	__DARNIT_MENUTK_H__

typedef void DARNIT_MENU;

DARNIT_MENU *d_menu_horizontal_new(const char *options, int x, int y, void *font, int max_w);
DARNIT_MENU *d_menu_vertical_new(const char *options, int x, int y, void *font, int menuw, int textskip_x, int max_h);
DARNIT_MENU *d_menu_vertical_shade_new(int x, int y, int shade_w, int shade_h, int y_advance, int options, int skip_option, int max_h);
DARNIT_MENU *d_menu_spinbutton_new(const char *label, int x, int y, void *font, int step, int min, int max);
DARNIT_MENU *d_menu_textinput_new(int x, int y, void *font, char *buff, int buff_len, int field_len);
void d_menu_vertical_shade_selections_reduce(DARNIT_MENU *menu);
void d_menu_vertical_shade_selections_increase(DARNIT_MENU *menu);
void d_menu_shade_color(DARNIT_MENU *menu, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
int d_menu_loop(DARNIT_MENU *menu);
void d_menu_selection_wait(DARNIT_MENU *menu);
int d_menu_peek(DARNIT_MENU *menu, int *top_sel);
int d_menu_changed(DARNIT_MENU *menu);
void d_menu_hide(DARNIT_MENU *menu);
void d_menu_show(DARNIT_MENU *menu);
void *d_menu_destroy(DARNIT_MENU *menu);

#endif
