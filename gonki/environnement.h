void drawSky()
{
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glBegin(GL_QUADS);
    glColor3f(51.0f / 255, 153.0f / 255, 255.0f / 255);
    glVertex2f(-1, 1);
    glVertex2f(1, 1);

    glColor3f(0.0f, 128.0f / 255, 255.0f / 255);
    glVertex2f(1, -1);
    glVertex2f(-1, -1);
    glEnd();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);
}
void drawGround(float camX, float camZ)
{
    const float size = 10000.0f;

    float x = floor(camX / size) * size;
    float z = floor(camZ / size) * size;

    glColor3f(0.0f, 102.0f / 255, 0.0f);
    glBegin(GL_QUADS);
    glVertex3f(x - size, 0.0f, z - size);
    glVertex3f(x + size, 0.0f, z - size);
    glVertex3f(x + size, 0.0f, z + size);
    glVertex3f(x - size, 0.0f, z + size);
    glEnd();
}

