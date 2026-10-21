#ifndef DEV_MODE_H
#define DEV_MODE_H

#include <stdbool.h>

/* GUI-only developer diagnostics. No search or UCI behavior is changed. */
void DevMode_Init(void);
void DevMode_HandleHotkey(void);
void DevMode_Update(bool humanPlaysWhite);
void DevMode_DrawOverlay(void);
void DevMode_Shutdown(void);

#endif
