#ifndef UTILS_H

#define UTILS_H

#include <QtOpenGL>

void perspective(GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar);

void lookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez,
        GLfloat centerx, GLfloat centery, GLfloat centerz,
        GLfloat upx, GLfloat upy, GLfloat upz);

#endif
