#ifndef DEV_WINDOW_H
#define DEV_WINDOW_H
void DevWindow_Init(void);
void DevWindow_Show(int visible);
void DevWindow_SetText(const char *title, const char *const *lines, int count);
void DevWindow_Poll(void);
void DevWindow_Shutdown(void);
#endif
