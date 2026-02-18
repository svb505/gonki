#include <windows.h>
#include <GL/gl.h>
#include "text.h"

GLuint fontBase = 0;

void BuildFont()
{
    HFONT font;
    HDC hDC = wglGetCurrentDC();

    fontBase = glGenLists(256);

    font = CreateFontA(
        -16, 0, 0, 0,
        FW_NORMAL,
        FALSE, FALSE, FALSE,
        ANSI_CHARSET,
        OUT_TT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY,
        FF_DONTCARE | DEFAULT_PITCH,
        "Consolas"
    );

    SelectObject(hDC, font);
    wglUseFontBitmapsA(hDC, 0, 256, fontBase);

    DeleteObject(font);
}
void PrintText(const char* text)
{
    if (!text) return;

    glPushAttrib(GL_LIST_BIT);
    glListBase(fontBase);
    glCallLists((GLsizei)strlen(text), GL_UNSIGNED_BYTE, text);
    glPopAttrib();
}
void RenderTextHUD(float x, float y, float r, float g, float b, const char* text, int screenW, int screenH)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, screenW, screenH, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    PrintText(text);

    glEnable(GL_DEPTH_TEST);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}
void RenderTextWorld(float x, float y, float z, float r, float g, float b, const char* text)
{
    glPushMatrix();

    glTranslatef(x, y, z);
    glScalef(0.002f, 0.002f, 0.002f);

    glColor3f(r, g, b);
    glRasterPos3f(0, 0, 0);
    PrintText(text);

    glPopMatrix();
}

