#ifndef DEV_WINDOW_H
#define DEV_WINDOW_H
void DevWindow_Init(void);
void DevWindow_Show(int visible);
int DevWindow_IsVisible(void);
void DevWindow_SetText(const char *title, const char lines[][128], int count);
void DevWindow_Poll(void);
void DevWindow_Shutdown(void);
#endif
