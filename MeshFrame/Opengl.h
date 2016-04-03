#ifndef OPENGL_H
#define OPENGL_H
#include <QGLWidget>
#include <QEvent>
#include "HE_mesh/Vec.h"
#include "meshframe.h"
#include<vector>

using trimesh::vec;
using trimesh::point;

class GLMainWindow;
class CArcBall;
class Mesh3D;

class COpenGL : public QGLWidget
{
	Q_OBJECT
public:
    COpenGL(QWidget *parent, GLMainWindow* mainwindow = 0);
	~COpenGL();

public:
	GLMainWindow    *pMainWindow;
	CArcBall        *m_pArcBall;
	Mesh3D			*m_pMesh;	//´æ´¢Ô´Í¼ÏñµÄ³õÊ¼Íø¸ñ£¨³õÊ¼×´Ì¬£©
	
	//texture
	GLuint			texture[1];
	bool			m_bLoadTex;

	//eye
	GLfloat         m_eyeDist;
	point			m_eyeGoal;
	vec				m_eyeDirection;
	QPoint			m_curPos;

	//render
	bool			m_bDrawPoint;
	bool			m_bDrawEdge;
	bool			m_bDrawFace;
	bool			m_bDrawTex;
	bool			m_bLight;
	bool			m_bAxes;

	bool            mouth_open;

protected:
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();
	void timerEvent(QTimerEvent * e);

	//mouse events
    void mousePressEvent (QMouseEvent *e);
    void mouseMoveEvent (QMouseEvent *e);
    void mouseReleaseEvent (QMouseEvent *e);
	void mouseDoubleClickEvent(QMouseEvent *e);
    void wheelEvent (QWheelEvent *);

public:
	//key events
	void keyPressEvent(QKeyEvent *e);
	void keyReleaseEvent(QKeyEvent *e);

signals:
	void meshInfo(int, int , int);
	void operatorInfo(QString);

private:
	void Render();
	void setLight();

public slots:
	void SetBackground();
	void ReadMesh();
	void WriteMesh();
	void LoadTexture();

	//--------------------------------simulate---------------------
	void Simulate_tex(IplImage pFrame, AAM_OUTPUT_STRUCT aam_ret, vector<double>x_88_2_100,vector<double>y_88_2_100);

	void checkDrawPoint(bool bv);
	void checkDrawEdge(bool bv);
	void checkDrawFace(bool bv);
	void checkLight(bool bv);
	void checkDrawTex(bool bv);
	void checkDrawAxes(bool bV);

	void getmouthopen();
	void getmouthshut();
	void choose_shape(int shape);

private:
    void DrawAxes(bool bV);
	void testRender();
private:
	void DrawPoints(bool);
	void DrawEdge(bool);
	void DrawFace(bool);
	void DrawTexture(bool);

public:
	void ReadMesh_simulate();
};

#endif // OPENGL_H
