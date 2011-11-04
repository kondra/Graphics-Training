#include <QGLWidget>
#include <QGLShader>
#include <QMatrix4x4>
#include <QVector3D>
#include <QVector2D>
#include <QTime>

#include <qmath.h>
#include <cmath>
#include <cassert>

#include "clock.h"

//#define SHADERS

#ifdef SHADERS

#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_TEXCOORD_ATTRIBUTE 1

QGLShaderProgram *program;
#endif

inline void qSetColor(float colorVec[], QColor c)
{
    colorVec[0] = c.redF();
    colorVec[1] = c.greenF();
    colorVec[2] = c.blueF();
    colorVec[3] = c.alphaF();
}

struct Geometry
{
    QVector<GLushort> faces;
    QVector<QVector3D> vertices;
    QVector<QVector3D> normals;
    QVector<QVector2D> texCoords;
    void appendSmooth(const QVector3D &a, const QVector3D &n, int from);
    void appendFaceted(const QVector3D &a, const QVector3D &n);
    void finalize();
    void loadArrays() const;
};

void Geometry::loadArrays() const
{
#ifdef SHADERS
//    program->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
//    program->enableAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE);
//
//    program->setAttributeArray(PROGRAM_VERTEX_ATTRIBUTE, vertices.constData());
//    program->setAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE, texCoords.constData());
//
//    program->disableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
//    program->disableAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE);

    glVertexPointer(3, GL_FLOAT, 0, vertices.constData());
    glNormalPointer(GL_FLOAT, 0, normals.constData());
    glTexCoordPointer(2, GL_FLOAT, 0, texCoords.constData());
#else
    glVertexPointer(3, GL_FLOAT, 0, vertices.constData());
    glNormalPointer(GL_FLOAT, 0, normals.constData());
    glTexCoordPointer(2, GL_FLOAT, 0, texCoords.constData());
#endif
}

void Geometry::finalize()
{
    // TODO: add vertex buffer uploading here

    // Finish smoothing normals by ensuring accumulated normals are returned
    // to length 1.0.
    for (int i = 0; i < normals.count(); ++i)
        normals[i].normalize();
}

void Geometry::appendSmooth(const QVector3D &a, const QVector3D &n, int from)
{
    // Smooth normals are acheived by averaging the normals for faces meeting
    // at a point.  First find the point in geometry already generated
    // (working backwards, since most often the points shared are between faces
    // recently added).
    int v = vertices.count() - 1;
    for ( ; v >= from; --v)
        if (qFuzzyCompare(vertices[v], a))
            break;
    if (v < from)
    {
        // The vert was not found so add it as a new one, and initialize
        // its corresponding normal
        v = vertices.count();
        vertices.append(a);
        texCoords.append(a.toVector2D());
        normals.append(n);
    }
    else
    {
        // Vert found, accumulate normals into corresponding normal slot.
        // Must call finalize once finished accumulating normals
        normals[v] += n;
    }
    // In both cases (found or not) reference the vert via its index
    faces.append(v);
}

void Geometry::appendFaceted(const QVector3D &a, const QVector3D &n)
{
    // Faceted normals are achieved by duplicating the vert for every
    // normal, so that faces meeting at a vert get a sharp edge.
    int v = vertices.count();
    vertices.append(a);
    normals.append(n);
    texCoords.append(a.toVector2D());
    faces.append(v);
}

class Patch
{
public:
    enum Smoothing { Faceted, Smooth };
    Patch(Geometry *);
    void setSmoothing(Smoothing s) { sm = s; }
    void translate(const QVector3D &t);
    void rotate(qreal deg, QVector3D axis);
    void draw() const;
    void addTri(const QVector3D &a, const QVector3D &b, const QVector3D &c, const QVector3D &n);
    void addQuad(const QVector3D &a, const QVector3D &b,  const QVector3D &c, const QVector3D &d);

    GLushort start;
    GLushort count;
    GLushort initv;

    GLfloat faceColor[4];
    QMatrix4x4 mat;
    Smoothing sm;
    bool metal;
    Geometry *geom;
};

Patch::Patch(Geometry *g)
   : start(g->faces.count())
   , count(0)
   , initv(g->vertices.count())
   , sm(Patch::Smooth)
   , metal(false)
   , geom(g)
{
    qSetColor(faceColor, QColor(Qt::darkGray));
}

void Patch::rotate(qreal deg, QVector3D axis)
{
    mat.rotate(deg, axis);
}

void Patch::translate(const QVector3D &t)
{
    mat.translate(t);
}

static inline void qMultMatrix(const QMatrix4x4 &mat)
{
    if (sizeof(qreal) == sizeof(GLfloat))
        glMultMatrixf((GLfloat*)mat.constData());
    else
    {
        GLfloat fmat[16];
        qreal const *r = mat.constData();
        for (int i = 0; i < 16; ++i)
            fmat[i] = r[i];
        glMultMatrixf(fmat);
    }
}

void Patch::draw() const
{
#ifdef SHADERS
    glPushMatrix();
    qMultMatrix(mat);

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, faceColor);
    if (metal) {
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 128);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, faceColor);
    } else {
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0);
        GLfloat def_spec[] = {0, 0, 0, 1};
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, def_spec);
    }

    const GLushort *indices = geom->faces.constData();
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, indices + start);

    glPopMatrix();
#else
    glPushMatrix();
    qMultMatrix(mat);

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, faceColor);
    if (metal) {
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 128);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, faceColor);
    } else {
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0);
        GLfloat def_spec[] = {0, 0, 0, 1};
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, def_spec);
    }

    const GLushort *indices = geom->faces.constData();
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, indices + start);

    glPopMatrix();
#endif
}

void Patch::addTri(const QVector3D &a, const QVector3D &b, const QVector3D &c, const QVector3D &n)
{
    QVector3D norm = n.isNull() ? QVector3D::normal(a, b, c) : n;
    if (sm == Smooth)
    {
        geom->appendSmooth(a, norm, initv);
        geom->appendSmooth(b, norm, initv);
        geom->appendSmooth(c, norm, initv);
    }
    else
    {
        geom->appendFaceted(a, norm);
        geom->appendFaceted(b, norm);
        geom->appendFaceted(c, norm);
    }
    count += 3;
}

void Patch::addQuad(const QVector3D &a, const QVector3D &b,  const QVector3D &c, const QVector3D &d)
{
    QVector3D norm = QVector3D::normal(a, b, c);
    if (sm == Smooth)
    {
        addTri(a, b, c, norm);
        addTri(a, c, d, norm);
    }
    else
    {
        // If faceted share the two common verts
        addTri(a, b, c, norm);
        int k = geom->vertices.count();
        geom->appendSmooth(a, norm, k);
        geom->appendSmooth(c, norm, k);
        geom->appendFaceted(d, norm);
        count += 3;
    }
}

static inline QVector<QVector3D> extrude(const QVector<QVector3D> &verts, qreal depth)
{
    QVector<QVector3D> extr = verts;
    for (int v = 0; v < extr.count(); ++v)
        extr[v].setZ(extr[v].z() - depth);
    return extr;
}

class Rectoid
{
public:
    void translate(const QVector3D &t)
    {
        for (int i = 0; i < parts.count(); ++i)
            parts[i]->translate(t);
    }
    void rotate(qreal deg, QVector3D axis)
    {
        for (int i = 0; i < parts.count(); ++i)
            parts[i]->rotate(deg, axis);
    }
    void setColor(QColor c)
    {
        for (int i = 0; i < parts.count(); ++i)
            qSetColor(parts[i]->faceColor, c);
    }
    void setMetal()
    {
        for (int i = 0; i < parts.count(); ++i)
            parts[i]->metal = true;
    }

    // No special Rectoid destructor - the parts are fetched out of this member
    // variable, and destroyed by the new owner
    QList<Patch*> parts;
};

class RectPrism : public Rectoid
{
public:
    RectPrism() {}
    RectPrism(Geometry *g, qreal width, qreal height, qreal depth);
};

RectPrism::RectPrism(Geometry *g, qreal width, qreal height, qreal depth)
{
    enum { bl, br, tr, tl };
    Patch *fb = new Patch(g);
    fb->setSmoothing(Patch::Faceted);

    // front face
    QVector<QVector3D> r(4);
    r[br].setX(width);
    r[tr].setX(width);
    r[tr].setY(height);
    r[tl].setY(height);
    QVector3D adjToCenter(-width / 2.0, -height / 2.0, depth / 2.0);
    for (int i = 0; i < 4; ++i)
        r[i] += adjToCenter;
    fb->addQuad(r[bl], r[br], r[tr], r[tl]);

    // back face
    QVector<QVector3D> s = extrude(r, depth);
    fb->addQuad(s[tl], s[tr], s[br], s[bl]);

    // side faces
    Patch *sides = new Patch(g);
    sides->setSmoothing(Patch::Faceted);
    sides->addQuad(s[bl], s[br], r[br], r[bl]);
    sides->addQuad(s[br], s[tr], r[tr], r[br]);
    sides->addQuad(s[tr], s[tl], r[tl], r[tr]);
    sides->addQuad(s[tl], s[bl], r[bl], r[tl]);

    parts << fb << sides;
}

class RectTorus : public Rectoid
{
public:
    RectTorus();
    RectTorus(Geometry *g, qreal iRad, qreal oRad, qreal depth, int numSectors);
};

RectTorus::RectTorus(Geometry *g, qreal iRad, qreal oRad, qreal depth, int k)
{
    QVector<QVector3D> inside;
    QVector<QVector3D> outside;
    for (int i = 0; i < k; ++i) {
        qreal angle = (i * 2 * M_PI) / k;
        inside << QVector3D(iRad * qSin(angle), iRad * qCos(angle), depth / 2.0);
        outside << QVector3D(oRad * qSin(angle), oRad * qCos(angle), depth / 2.0);
    }
    inside << QVector3D(0.0, iRad, 0.0);
    outside << QVector3D(0.0, oRad, 0.0);
    QVector<QVector3D> in_back = extrude(inside, depth);
    QVector<QVector3D> out_back = extrude(outside, depth);

    // Create front, back and sides as separate patches so that smooth normals
    // are generated for the curving sides, but a faceted edge is created between
    // sides and front/back
    Patch *front = new Patch(g);
    for (int i = 0; i < k; ++i)
        front->addQuad(outside[i], inside[i],
                       inside[(i + 1) % k], outside[(i + 1) % k]);
    Patch *back = new Patch(g);
    for (int i = 0; i < k; ++i)
        back->addQuad(in_back[i], out_back[i],
                      out_back[(i + 1) % k], in_back[(i + 1) % k]);
    Patch *is = new Patch(g);
    for (int i = 0; i < k; ++i)
        is->addQuad(in_back[i], in_back[(i + 1) % k],
                    inside[(i + 1) % k], inside[i]);
    Patch *os = new Patch(g);
    for (int i = 0; i < k; ++i)
        os->addQuad(out_back[(i + 1) % k], out_back[i],
                    outside[i], outside[(i + 1) % k]);
    parts << front << back << is << os;
}

Clock::Clock(QObject *parent, int divisions, qreal scale)
    : QObject(parent)
    , secondAngle(0)
    , minuteAngle(0)
    , hourAngle(0)
    , geom(new Geometry())
{
    buildGeometry(divisions, scale);

#ifdef SHADERS
    QGLShader *vshader = new QGLShader(QGLShader::Vertex, this);
//    const char *vsrc =
//        "attribute highp vec4 vertex;\n"
//        "attribute mediump vec4 texCoord;\n"
//        "varying mediump vec4 texc;\n"
//        "uniform mediump mat4 matrix;\n"
//        "void main(void)\n"
//        "{\n"
//        "    gl_Position = matrix * vertex;\n"
//        "    texc = texCoord;\n"
//        "}\n";
//    const char *vsrc =
//        "void main(void)\n"
//        "{\n"
//        "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
//        "}\n";
//    vshader->compileSourceCode(vsrc);
    vshader->compileSourceFile("test.vsh");

    QGLShader *fshader = new QGLShader(QGLShader::Fragment, this);
//    const char *fsrc =
//        "uniform sampler2D texture;\n"
//        "varying mediump vec4 texc;\n"
//        "void main(void)\n"
//        "{\n"
//        "    gl_FragColor = texture2D(texture, texc.st);\n"
//        "}\n";
//    const char *fsrc =
//        "void main(void)\n"
//        "{\n"
//        "    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
//        "}\n";
//    fshader->compileSourceCode(fsrc);
    fshader->compileSourceFile("test.fsh");

    program = new QGLShaderProgram(this);
    program->addShader(vshader);
    program->addShader(fshader);
//    program->bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
//    program->bindAttributeLocation("texCoord", PROGRAM_TEXCOORD_ATTRIBUTE);
    program->link();
#endif
}

Clock::~Clock()
{
    delete geom;

    qDeleteAll(body->parts);
    qDeleteAll(hourPointer->parts);
    qDeleteAll(minutePointer->parts);
    qDeleteAll(secondPointer->parts);

    for (int i = 0; i < 12; ++i) {
        qDeleteAll(hourMarks[i]->parts);
        delete hourMarks[i];
    }
    delete hourMarks;

    delete hourPointer;
    delete minutePointer;
    delete secondPointer;
    delete body;
}

void Clock::buildGeometry(int divisions, qreal scale)
{
    scale = 0;

    secondPointer = new RectPrism(geom, 0.005, 0.21, 0.005);
    secondPointer->setColor(Qt::red);
    secondPointer->setMetal();

    minutePointer = new RectPrism(geom, 0.008, 0.18, 0.008);
    minutePointer->setColor(Qt::black);
    minutePointer->setMetal();

    hourPointer = new RectPrism(geom, 0.01, 0.13, 0.01);
    hourPointer->setColor(Qt::black);
    hourPointer->setMetal();

    translatePointers(true);

    body = new RectTorus(geom, 0.05, 0.30, 0.02, divisions);
    body->translate(QVector3D(0.0, 0.0, -0.016));
    body->setColor(qRgba(100, 75, 255, 70));

    center = new RectTorus(geom, 0.0, 0.01, 0.05, divisions);
    center->setColor(Qt::black);
    center->setMetal();

    center2 = new RectTorus(geom, 0.0, 0.05, 0.02, divisions);
    center2->translate(QVector3D(0.0, 0.0, -0.016));
    center2->setColor(Qt::white);
    center2->setMetal();

    qreal angle;
    QVector3D z(0.0, 0.0, 1.0);
    QVector3D shift(0.0, 0.2, -0.007);
    hourMarks = new RectPrism*[12];
    for (int i = 0; i < 12; ++i) {
        angle = (360.0 / 12.0) * i;
        hourMarks[i] = new RectPrism(geom, 0.005, 0.02, 0.01);
        hourMarks[i]->rotate(angle, z);
        hourMarks[i]->translate(shift);
        hourMarks[i]->setColor(Qt::black);
        hourMarks[i]->setMetal();
    }

    rotatePointers();

    geom->finalize();
}

void Clock::translatePointers(bool direction)
{
    int coef = direction ? 1 : -1;
    secondPointer->translate(QVector3D(0.0, coef * (0.21 / 2.0 - 0.006), coef * (0.005 + 0.008 + 0.007)));
    minutePointer->translate(QVector3D(0.0, coef * (0.18 / 2.0 - 0.006), coef * (0.005 + 0.007)));
    hourPointer->translate(QVector3D(0.0, coef * (0.13 / 2.0 - 0.006), coef * (0.002)));
}

void Clock::rotatePointers()
{
    QTime t(QTime::currentTime());

    qreal s = (360.0 / 60.0) * t.second();
    qreal m = (360.0 / 60.0) * t.minute() + (360.0 / 3600.0) * t.second();
    qreal h = (360.0 / 12.0) * (t.hour() % 12) + (360.0 / 720.0) * t.minute();

    if (qFuzzyIsNull(s))
        s = 360;
    if (qFuzzyIsNull(m))
        m = 360;
    if (qFuzzyIsNull(h))
        h = 360;

    qreal ds = -fabs(s - secondAngle);
    qreal dm = -fabs(m - minuteAngle);
    qreal dh = -fabs(h - hourAngle);

    secondAngle = qFuzzyCompare(s, 360) ? 0 : s;
    minuteAngle = qFuzzyCompare(m, 360) ? 0 : m;
    hourAngle = qFuzzyCompare(h, 360) ? 0 : h;

    QVector3D z(0.0, 0.0, 1.0);

    translatePointers(false);
    secondPointer->rotate(ds, z);
    minutePointer->rotate(dm, z);
    hourPointer->rotate(dh, z);
    translatePointers(true);
}

void Clock::draw()
{
    rotatePointers();

    geom->finalize();

    geom->loadArrays();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    int i, j;

    for (i = 0; i < center->parts.count(); ++i) {
        center->parts[i]->draw();
    }

    glBindTexture(GL_TEXTURE_2D, metalTexture);
    glEnable(GL_TEXTURE_2D);
    for (i = 0; i < center2->parts.count(); ++i) {
        center2->parts[i]->draw();
    }
    glDisable(GL_TEXTURE_2D);

    for (i = 0; i < 12; ++i) {
        for (j = 0; j < hourMarks[i]->parts.count(); ++j)
            hourMarks[i]->parts[j]->draw();
    }

    for (i = 0; i < hourPointer->parts.count(); ++i)
        hourPointer->parts[i]->draw();
    for (i = 0; i < minutePointer->parts.count(); ++i)
        minutePointer->parts[i]->draw();
    for (i = 0; i < secondPointer->parts.count(); ++i)
        secondPointer->parts[i]->draw();

#ifdef SHADERS
//    program->bind();
//    program->setUniformValue("ScaleFactor", GLfloat(0.5));
//    program->setUniformValue("BrickColor", QVector3D(1.0, 0.3, 0.2));
//    program->setUniformValue("MortarColor", QVector3D(0.85, 0.86, 0.84));
//    program->setUniformValue("BrickSize", QVector2D(0.030, 0.015));
//    program->setUniformValue("BrickPct", QVector2D(0.090, 0.085));
//    program->setUniformValue("MortarPct", QVector2D(0.2, 0.2));
//    program->setUniformValue("SkyColor", QVector3D(1.0, 0.3, 0.2));
//    program->setUniformValue("GroundColor", QVector3D(0.85, 0.86, 0.84));
//    program->setUniformValue("LightPosition", QVector4D(0.5, 15.0, 7.0, 1.0).toVector3D());
#endif
    glBindTexture(GL_TEXTURE_2D, glassTexture);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    for (i = 0; i < body->parts.count(); ++i)
        body->parts[i]->draw();
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
#ifdef SHADERS
//    program->release();
#endif

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void Clock::loadTex(GLuint _glassTexture, GLuint _metalTexture)
{
    glassTexture = _glassTexture;
    metalTexture = _metalTexture;
}

