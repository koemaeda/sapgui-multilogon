#ifndef DLLMAIN_H_INCLUDED
#define DLLMAIN_H_INCLUDED

#define APPLICATION_NAME "SAPGUI MultiLogon"
#define APPLICATION_VERSION "v1.0.0"

#define SPLASH_WINDOW_LOGO_X 250
#define SPLASH_WINDOW_LOGO_Y 0

#define LOGO_BITMAP_WIDTH 200
#define LOGO_BITMAP_HEIGHT 200


BOOL WINAPI DllMain (HINSTANCE hInstance, DWORD iReason, void * pReserved);

void LoadLogoBitmap (void);
void PaintLogoBitmap (HWND hWindow, int iX, int iY);

DWORD ShowLastErrorMessage (void);


#endif // DLLMAIN_H_INCLUDED
