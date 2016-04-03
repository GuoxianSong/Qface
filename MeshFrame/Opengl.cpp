#include "Opengl.h"
#include <QKeyEvent>
#include <QColorDialog>
#include <QFileDialog>
#include <iostream>
#include <QtWidgets/QMenu>
#include <QtWidgets/QAction>
#include <QtWidgets/QMessageBox>
#include <QTextCodec>
#include <gl/GLU.h>
#include <glut.h>
#include <algorithm>
#include "meshframe.h"
#include "ArcBall.h"
#include "globalFunctions.h"
#include "HE_mesh/Mesh3D.h"


using namespace std;

COpenGL::COpenGL(QWidget *parent,GLMainWindow *mainwindow)
    : QGLWidget(parent),pMainWindow(mainwindow),m_eyeDist(5.0),
	m_bLight(false),m_bDrawPoint(true),m_bDrawEdge(false),m_bDrawFace(false),m_bDrawTex(false)
{	
    m_pArcBall = new CArcBall(width(), height());
	m_pMesh = new Mesh3D();

	m_bLoadTex = false;
	m_bAxes = false;
	
	m_eyeGoal[0] = m_eyeGoal[1] = m_eyeGoal[2] = 0.0;
	m_eyeDirection[0] = m_eyeDirection[1] = 0.0;
	m_eyeDirection[2] = 1.0;
}

COpenGL::~COpenGL()
{
    SafeDelete(m_pArcBall);
	SafeDelete(m_pMesh);
}

void COpenGL::initializeGL()
{
   // glClearColor(0.3,0.3,0.3,0.0);
	glClearColor(0.9, 0.9, 0.9, 0.0);
	glShadeModel(GL_SMOOTH);

	glEnable(GL_DOUBLEBUFFER);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1);

	setLight();

}

void COpenGL::resizeGL(int w, int h)
{
	h = (h==0)?1:h;

    m_pArcBall->reSetBound (w, h);

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

    gluPerspective (45.0, GLdouble(w)/GLdouble(h), 0.001, 1000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void COpenGL::paintGL()
{
	glShadeModel(GL_SMOOTH);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (m_bLight)
	{
		setLight();
	}
	else
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	register vec eyepos = m_eyeDist*m_eyeDirection;
	gluLookAt (eyepos[0], eyepos[1], eyepos[2],
		m_eyeGoal[0],m_eyeGoal[1],m_eyeGoal[2],
			0.0,1.0,0.0);
	glPushMatrix();

    glMultMatrixf(m_pArcBall->GetBallMatrix ());

	Render();
	glPopMatrix();
}

void COpenGL::Render()
{
	DrawAxes(m_bAxes);

//	testRender();
	DrawPoints(m_bDrawPoint);
	DrawEdge(m_bDrawEdge);
	DrawFace(m_bDrawFace);
	DrawTexture(m_bDrawTex);
}
void COpenGL::testRender()
{
	if ( m_pMesh==NULL )
		return;

	if (m_pMesh->num_of_vertex_list() == 0)
	{
		return;
	}

	const std::vector<HE_vert*>& verts = *(m_pMesh->get_vertex_list());
	glPointSize(8);
	glBegin(GL_POINTS);
	glColor3f(1.0,0.0,0.0);
	glVertex3fv(verts.at(10)->position().data());

	for (size_t i=0; i != verts.at(10)->neighborIdx.size(); ++i)
	{	glColor3f(0.0,1.0*i/verts.at(10)->neighborIdx.size(),0.0);
	glVertex3fv(verts.at(verts.at(10)->neighborIdx[i])->position().data());
	}
	glEnd();

	glPointSize(1.0);
	glColor3f(0.0,0.0,1.0);

	glBegin(GL_LINES);

	for (size_t i=0; i != verts.at(10)->neighborIdx.size(); ++i)
	{
		glColor3f(0.0,1.0*i/verts.at(10)->neighborIdx.size(),0.0);
		glLineWidth(i+1);
		glVertex3fv(verts.at(10)->position().data());		
		glVertex3fv(verts.at(verts.at(10)->neighborIdx[i])->position().data());
	}
	glEnd();

	glColor3f(1.0,1.0,1.0);
}
void COpenGL::mousePressEvent (QMouseEvent *e)
{
    switch(e->button ())
    {
    case Qt::LeftButton:
        m_pArcBall->MouseDown(e->pos());
       break;
	case Qt::MidButton:
		m_curPos = e->pos();
		break;
    default:
        break;
    }

    updateGL();
}
void COpenGL::mouseMoveEvent (QMouseEvent *e)
{
    switch (e->buttons ())
    {
		setCursor(Qt::ClosedHandCursor);
    case Qt::LeftButton:        
        m_pArcBall->MouseMove(e->pos());        
        break;
	case Qt::MidButton:
		m_eyeGoal[0] -= 4.0*GLfloat(e->x() - m_curPos.x())/GLfloat(width());
		m_eyeGoal[1] += 4.0*GLfloat(e->y() - m_curPos.y())/GLfloat(height());
		m_curPos = e->pos();
		break;
    default:
        break;
    }

    updateGL();
}
void COpenGL::mouseDoubleClickEvent(QMouseEvent *e)
{
	switch (e->button())
	{
	case Qt::LeftButton:
		break;
	default:
		break;
	}
	updateGL();
}
void COpenGL::mouseReleaseEvent(QMouseEvent *e)
{
    switch (e->button ())
    {
    case Qt::LeftButton:
        m_pArcBall->MouseUp(e->pos());
        setCursor(Qt::ArrowCursor);
        break;

    case Qt::RightButton:
        break;
    default:
        break;
    }
}
void COpenGL::keyPressEvent(QKeyEvent *e)
{
	switch (e->key())
	{
	case Qt::Key_A:
		break;
	default:
		break;
	}
}

void COpenGL::keyReleaseEvent(QKeyEvent *e)
{
	switch (e->key())
	{
	case Qt::Key_A:
		break;
	default:
		break;
	}
}
void COpenGL::wheelEvent(QWheelEvent *e)
{
    m_eyeDist += e->delta ()*0.001;
    m_eyeDist = m_eyeDist<0?0:m_eyeDist;

    updateGL();
}

void COpenGL::SetBackground()
{
	QColor color = QColorDialog::getColor(Qt::white, this,tr("background color"));
	GLfloat r = (color.red())/255.0f;
	GLfloat g = (color.green())/255.0f;
	GLfloat b = (color.blue())/255.0f;
	GLfloat alpha = color.alpha()/255.0f;
	glClearColor(r,g,b,alpha);
    updateGL();
}

void COpenGL::DrawAxes(bool bV)
{
	if (!bV)
		return;
	//x axis
	glColor3f(1.0,0.0,0.0);
	glBegin(GL_LINES);
	glVertex3f(0,0,0);
	glVertex3f(0.7,0.0,0.0);
	glEnd();
	glPushMatrix();
	glTranslatef(0.7,0,0);
	glRotatef(90,0.0,1.0,0.0);
	glutSolidCone(0.02,0.06,20,10);
	glPopMatrix();

	//y axis
	glColor3f(0.0,1.0,0.0);
	glBegin(GL_LINES);
	glVertex3f(0,0,0);
	glVertex3f(0.0,0.7,0.0);
	glEnd();

	glPushMatrix();
	glTranslatef(0.0,0.7,0);
	glRotatef(90,-1.0,0.0,0.0);
	glutSolidCone(0.02,0.06,20,10);
	glPopMatrix();

	//z axis
	glColor3f(0.0,0.0,1.0);
	glBegin(GL_LINES);
	glVertex3f(0,0,0);
	glVertex3f(0.0,0.0,0.7);
	glEnd();
	glPushMatrix();
	glTranslatef(0.0,0,0.7);
	glutSolidCone(0.02,0.06,20,10);
	glPopMatrix();

	glColor3f(1.0,1.0,1.0);
}

void COpenGL::ReadMesh()	
{
	QString filename = QFileDialog::
		getOpenFileName(this,tr("Read Mesh"),
		"..",tr("Meshes (*.obj)"));

 	if (filename.isEmpty())
	{
		emit(operatorInfo(QString("Read Mesh Failed!")));
		return;
	}
		//中文路径支持
	QTextCodec *code = QTextCodec::codecForName("gd18030");
	QTextCodec::setCodecForLocale(code);

	QByteArray byfilename = filename.toLocal8Bit();
	m_pMesh->LoadFromOBJFile(byfilename.data());

	for ( int i=0; i<m_pMesh->num_of_vertex_list(); i++ )	//使网格正放
	{
		m_pMesh->get_vertex(i)->position_[1] = 0-m_pMesh->get_vertex(i)->position_[1];
	}
	
	emit(operatorInfo(QString("Read Mesh from")+filename+QString("Done")));
	emit(meshInfo(m_pMesh->num_of_vertex_list(),m_pMesh->num_of_edge_list(),m_pMesh->num_of_face_list()));

	pMainWindow->m_pTargetMesh = m_pMesh;

	if ( pMainWindow->m_pTargetMesh!=NULL )	//copy the vertices of the target mesh
	{
		if ( pMainWindow->m_pTargetCopy->num_of_vertex_list() != 0 )
		{
			pMainWindow->m_pTargetCopy->ClearData();
		}

		int vertex_number = pMainWindow->m_pTargetMesh->num_of_vertex_list(); 
		for ( size_t i=0; i!=vertex_number; i++ )	//复制顶点
		{
			pMainWindow->m_pTargetCopy->InsertVertex(pMainWindow->m_pTargetMesh->get_vertex(i)->position_);
		}
	}
	checkDrawTex(TRUE);
updateGL();
	mouth_open = FALSE;
}

void COpenGL::ReadMesh_simulate()
{
	m_pMesh->LoadFromOBJFile("./data/Woman_2.obj");
	if (m_pMesh->num_of_vertex_list() == 0)
	{
		cout << "Woman Mesh missing!!!" << endl;
		exit(0);
	}

	for (int i = 0; i<m_pMesh->num_of_vertex_list(); i++)	//使网格正放
	{
		m_pMesh->get_vertex(i)->position_[1] = 0 - m_pMesh->get_vertex(i)->position_[1];
		//m_pMesh->get_vertex(i)->position_[0] = 0 - m_pMesh->get_vertex(i)->position_[0];
	}


	emit(meshInfo(m_pMesh->num_of_vertex_list(), m_pMesh->num_of_edge_list(), m_pMesh->num_of_face_list()));

	pMainWindow->m_pTargetMesh = m_pMesh;

	if (pMainWindow->m_pTargetMesh != NULL)	//copy the vertices of the target mesh
	{
		if (pMainWindow->m_pTargetCopy->num_of_vertex_list() != 0)
		{
			pMainWindow->m_pTargetCopy->ClearData();
		}

		int vertex_number = pMainWindow->m_pTargetMesh->num_of_vertex_list();
		for (size_t i = 0; i != vertex_number; i++)	//复制顶点
		{
			pMainWindow->m_pTargetCopy->InsertVertex(pMainWindow->m_pTargetMesh->get_vertex(i)->position_);
		}
	}

	mouth_open = FALSE;
	updateGL();
}

void COpenGL::WriteMesh()
{
	if (m_pMesh->num_of_vertex_list() == 0)
	{
		emit(QString("The Mesh is Empty !"));
		return;
	}
	QString filename = QFileDialog::
		getSaveFileName(this,tr("Write Mesh"),
		"..",tr("Meshes (*.obj)"));

	if (filename.isEmpty())
		return;

	m_pMesh->WriteToOBJFile(filename.toLatin1().data());

	emit(operatorInfo(QString("Write Mesh to ") + filename + QString(" Done")));
}


void COpenGL::LoadTexture()
{	
	if ( m_pMesh==NULL )
	{
		QMessageBox::information(NULL,"Read Mesh","Please Read a Mesh.",QMessageBox::Yes);
		m_pMesh = new Mesh3D;
		ReadMesh();
	}
	else if ( m_pMesh->get_vertex_list()==NULL )
	{
		QMessageBox::information(NULL,"Read Mesh","Please Read a Mesh.",QMessageBox::Yes);
		ReadMesh();
	}

    QString filename = QFileDialog::getOpenFileName(this,tr("Load Texture"),
                                                    "..",tr("Images(*.bmp *.jpg *.png *.jpeg)"));
    if (filename.isEmpty())
	{
		emit(operatorInfo(QString("Load Texture Failed!")));
        return;
	}

    glGenTextures(1, &texture[0]);
    QImage tex1, buf;
    if (!buf.load(filename))
    {
//        QMessageBox::warning(this, tr("Load Fialed!"), tr("Cannot Load Image %1").arg(filenames.at(0)));
		emit(operatorInfo(QString("Load Texture Failed!")));
		return;
	
//         QImage dummy(128, 128, QImage::Format_ARGB32);
//         dummy.fill(Qt::green);
//         buf = dummy;

    }

    tex1 = QGLWidget::convertToGLFormat(buf);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGB, tex1.width(), tex1.height(),
                       GL_RGBA, GL_UNSIGNED_BYTE, tex1.bits());

    m_bLoadTex = true;
	emit(operatorInfo(QString("Load Texture from ")+filename+QString(" Done")));
	
	float width = buf.width()*1.0;
	float height = buf.height()*1.0;
	std::vector<Vec3f> mesh_point;
	m_pMesh->DeUnify(2.0,mesh_point);

	for ( int i=0; i!=m_pMesh->num_of_vertex_list(); i++ )
	{
		m_pMesh->get_vertex(i)->texCoord_[0] = mesh_point[i][0]/width;
		m_pMesh->get_vertex(i)->texCoord_[1] = 1-(mesh_point[i][1]/height);
	}
	DrawTexture(true);

	updateGL();		//added by D.D
}

void COpenGL::DrawPoints(bool bv)
{
	if (!bv || m_pMesh == NULL)
		return;
	if (m_pMesh->num_of_vertex_list() == 0)
	{
		return;
	}

	const std::vector<HE_vert*>& verts = *(m_pMesh->get_vertex_list());
	glBegin(GL_POINTS);
	for (size_t i=0; i != m_pMesh->num_of_vertex_list(); ++i)
	{
		glNormal3fv(verts[i]->normal().data());
		glVertex3fv(verts[i]->position().data());
	}
	/*for (size_t i = 0; i !=88; ++i)
	{
		glNormal3fv(verts[i]->normal().data());
		glVertex3fv(verts[i]->position().data());
	}*/
	glEnd();
}

void COpenGL::DrawEdge(bool bv)
{
	if (!bv || m_pMesh == NULL)
		return;

	if (m_pMesh->num_of_face_list() == 0)
	{
		return;
	}

	const std::vector<HE_face *>& faces = *(m_pMesh->get_faces_list());
	for (size_t i=0; i!= faces.size(); ++i)
	{
		glBegin(GL_LINE_LOOP);
		HE_edge *pedge(faces.at(i)->pedge_);
		do 
		{
			glNormal3fv(pedge->pvert_->normal().data());
			glVertex3fv(pedge->pvert_->position().data());
		
			pedge = pedge->pnext_;

		} while (pedge != faces.at(i)->pedge_);		
		glEnd();
	}
}

void COpenGL::DrawFace(bool bv)
{
	if (!bv || m_pMesh == NULL)
		return;

	if (m_pMesh->num_of_face_list() == 0)
	{
		return;
	}
	
	const std::vector<HE_face *>& faces = *(m_pMesh->get_faces_list());
	
	glBegin(GL_TRIANGLES);

	for (size_t i=0; i!= faces.size(); ++i)
	{
		HE_edge *pedge(faces.at(i)->pedge_);
		do 
		{
			glNormal3fv(pedge->pvert_->normal().data());
			glVertex3fv(pedge->pvert_->position().data());

			pedge = pedge->pnext_;

		} while (pedge != faces.at(i)->pedge_);		
	}

	glEnd();
}
void COpenGL::timerEvent(QTimerEvent * e)
{
	updateGL();
}

void COpenGL::setLight()
{
	static GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
	static GLfloat mat_shininess[] = {50.0};
	static GLfloat light_position[] = {1.0,1.0,1.0,0.0};
	static GLfloat white_light[] = {0.8,0.8, 0.8, 1.0};
	static GLfloat lmodel_ambient[] = {0.3,0.3,0.3,1.0};

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}

void COpenGL::checkDrawPoint(bool bv)
{
	m_bDrawPoint = bv;
	updateGL();
}
void COpenGL::checkDrawEdge(bool bv)
{
	m_bDrawEdge = bv;
	updateGL();
}
void COpenGL::checkDrawFace(bool bv)
{
	m_bDrawFace = bv;
	updateGL();
}
void COpenGL::checkLight(bool bv)
{
	m_bLight = bv;
	updateGL();
}
void COpenGL::checkDrawTex(bool bv)
{
	m_bDrawTex = bv;
	if(m_bDrawTex)
		glEnable(GL_TEXTURE_2D);
	else
		glDisable(GL_TEXTURE_2D);

	updateGL();
}
void COpenGL::checkDrawAxes(bool bV)
{
	m_bAxes = bV;
	updateGL();
}

void COpenGL::DrawTexture(bool bv)
{
	if (!bv)
		return;
	if (m_pMesh->num_of_face_list() == 0 || !m_bLoadTex)
		return;

	//默认使用球面纹理映射，效果不好
//	m_pMesh->SphereTex();

	const std::vector<HE_face *>& faces = *(m_pMesh->get_faces_list());
	glBindTexture( GL_TEXTURE_2D, texture[0] );
	glBegin(GL_TRIANGLES);

	

	for (size_t i = 0; i != faces.size(); ++i) // faces.size()
	{
		/*if (i == 155 || i == 157 || i == 82||i==86)
		{
			if (mouth_open==TRUE)
			continue;
		}*/
		/*if (i == 0 || i == 1 || i == 2 || i == 4 || i == 5 || i == 6 || i == 8 || i == 9 || i == 15 || i == 21 || i == 22 || i == 23 || i == 24 || i == 39 || i == 83 || i == 84 || i == 85 || i == 89 || i == 91 || i == 106 || i == 110 || i == 111 || i == 112 || i == 118 || i == 120 || i == 121 || i == 122 || i == 124 || i == 166)
		{
			continue;
		}*/

			HE_edge *pedge(faces.at(i)->pedge_);
			do
			{
				/*请在此处绘制纹理，添加纹理坐标即可*/
				glTexCoord2fv(pedge->pvert_->texCoord_.data());
				glNormal3fv(pedge->pvert_->normal().data());
				glVertex3fv(pedge->pvert_->position().data());

				pedge = pedge->pnext_;

			} while (pedge != faces.at(i)->pedge_);
		
	}

glEnd();


/*测试点用--------------Guoxian-------------------*/
//HE_vert* vert;
//int id;
//for (size_t i = 0; i != faces.size(); ++i) // faces.size()
//{
//	HE_edge *pedge(faces.at(i)->pedge_);
//	do
//	{
//		HE_vert* vert = pedge->pvert_;
//		id = vert->id_;
//		if (id == 87)
//			cout << "point:87   " << i<< endl; 
//		if (id == 88)
//			cout << "point:88   " << i<< endl;
//		if (id == 89)
//			cout << "point:89   " << i << endl;
//		if (id == 90)
//			cout << "point:90   " << i<< endl;
//		if (id == 91)
//			cout << "point:91   " << i << endl;
//		if (id == 92)
//			cout << "point:92   " << i << endl;
//		if (id == 93)
//			cout << "point:93   " << i << endl;
//		if (id == 94)
//			cout << "point:94   " << i<< endl;
//		if (id == 95)
//			cout << "point:95   " << i << endl;
//		if (id == 96)
//			cout << "point:96   " << i << endl;
//		if (id == 97)
//			cout << "point:97   " << i << endl;
//		if (id == 98)
//			cout << "point:98   " << i << endl;
//		if (id == 99)
//			cout << "point:99   " << i << endl;
//		
//		pedge = pedge->pnext_;
//
//	} while (pedge != faces.at(i)->pedge_);
//}

}

void COpenGL::getmouthopen()
{
	mouth_open = TRUE;
}

void COpenGL::getmouthshut()
{
	mouth_open = FALSE;
}


void COpenGL::Simulate_tex(IplImage pFrame, AAM_OUTPUT_STRUCT aam_ret, vector<double>x_88_2_100, vector<double>y_88_2_100)
{
	IplImage* iplImg = &pFrame;
	QImage qImage;
	int nChannel = iplImg->nChannels;
	if (nChannel == 3)
	{
		cvConvertImage(iplImg, iplImg, CV_CVTIMG_SWAP_RB);
		qImage = QImage((const unsigned char*)iplImg->imageData, iplImg->width, iplImg->height, QImage::Format_RGB888);
	}
	else if (nChannel == 4 || nChannel == 1)
	{
		qImage = QImage((const unsigned char*)iplImg->imageData, iplImg->width, iplImg->height, QImage::Format_ARGB32);
	}

	//QImage qImage((unsigned char *)pFrame.imageData, pFrame.width, pFrame.height, pFrame.widthStep, QImage::Format_RGB888);
	glGenTextures(1, &texture[0]);
	QImage tex1, buf;
	buf = qImage;

	tex1 = QGLWidget::convertToGLFormat(buf);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, tex1.width(), tex1.height(),
		GL_RGBA, GL_UNSIGNED_BYTE, tex1.bits());

	m_bLoadTex = true;


	float width = buf.width()*1.0;
	float height = buf.height()*1.0;
	std::vector<Vec3f> mesh_point;
	m_pMesh->DeUnify(2.0, mesh_point);

	for (int i = 0; i !=87; i++)
	{
		m_pMesh->get_vertex(i)->texCoord_[0] = int(aam_ret.pKeyPoint2DOut[i].x + 0.5f) / width;
		m_pMesh->get_vertex(i)->texCoord_[1] = 1-int(aam_ret.pKeyPoint2DOut[i].y + 0.5f) / height;

	}

	for (int i = 87; i != 100; i++)
	{
		m_pMesh->get_vertex(i)->texCoord_[0] = int(x_88_2_100[i-87] + 0.5f) / width;
		m_pMesh->get_vertex(i)->texCoord_[1] = 1 - int(y_88_2_100[i-87] + 0.5f) / height;

	}

	DrawTexture(true);

	updateGL();		


}

void COpenGL::choose_shape(int shape)
{
	if (shape == 0)
	{
		m_pMesh->LoadFromOBJFile("./data/Tom.obj");
		if (m_pMesh->num_of_vertex_list() == 0)
		{
			cout << "Tom Mesh missing!!!" << endl;
			exit(0);
		}

		for (int i = 0; i<m_pMesh->num_of_vertex_list(); i++)	//使网格正放
		{
			m_pMesh->get_vertex(i)->position_[1] = 0 - m_pMesh->get_vertex(i)->position_[1];
		}

		
		emit(meshInfo(m_pMesh->num_of_vertex_list(), m_pMesh->num_of_edge_list(), m_pMesh->num_of_face_list()));

		pMainWindow->m_pTargetMesh = m_pMesh;

		if (pMainWindow->m_pTargetMesh != NULL)	//copy the vertices of the target mesh
		{
			if (pMainWindow->m_pTargetCopy->num_of_vertex_list() != 0)
			{
				pMainWindow->m_pTargetCopy->ClearData();
			}

			int vertex_number = pMainWindow->m_pTargetMesh->num_of_vertex_list();
			for (size_t i = 0; i != vertex_number; i++)	//复制顶点
			{
				pMainWindow->m_pTargetCopy->InsertVertex(pMainWindow->m_pTargetMesh->get_vertex(i)->position_);
			}
		}
		
		mouth_open = FALSE;

		

		glGenTextures(1, &texture[0]);
		QImage tex1, buf;
		if (!buf.load("./data/Tom.jpg"))
		{
			//        QMessageBox::warning(this, tr("Load Fialed!"), tr("Cannot Load Image %1").arg(filenames.at(0)));
			emit(operatorInfo(QString("Load Tom Texture Failed!")));
			return;

			//         QImage dummy(128, 128, QImage::Format_ARGB32);
			//         dummy.fill(Qt::green);
			//         buf = dummy;

		}

		tex1 = QGLWidget::convertToGLFormat(buf);
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, tex1.width(), tex1.height(),
			GL_RGBA, GL_UNSIGNED_BYTE, tex1.bits());

		m_bLoadTex = true;
		
		float width = buf.width()*1.0;
		float height = buf.height()*1.0;
		std::vector<Vec3f> mesh_point;
		m_pMesh->DeUnify(2.0, mesh_point);

		for (int i = 0; i != m_pMesh->num_of_vertex_list(); i++)
		{
			m_pMesh->get_vertex(i)->texCoord_[0] = mesh_point[i][0] / width;
			m_pMesh->get_vertex(i)->texCoord_[1] = 1 - (mesh_point[i][1] / height);
		}
		DrawTexture(true);
		updateGL();	

	}

	if (shape == 1)
	{

		m_pMesh->LoadFromOBJFile("./data/Woman.obj");
		if (m_pMesh->num_of_vertex_list() == 0)
		{
			cout << "Woman Mesh missing!!!" << endl;
			exit(0);
		}

		for (int i = 0; i<m_pMesh->num_of_vertex_list(); i++)	//使网格正放
		{
			m_pMesh->get_vertex(i)->position_[1] = 0 - m_pMesh->get_vertex(i)->position_[1];
		}


		emit(meshInfo(m_pMesh->num_of_vertex_list(), m_pMesh->num_of_edge_list(), m_pMesh->num_of_face_list()));

		pMainWindow->m_pTargetMesh = m_pMesh;

		if (pMainWindow->m_pTargetMesh != NULL)	//copy the vertices of the target mesh
		{
			if (pMainWindow->m_pTargetCopy->num_of_vertex_list() != 0)
			{
				pMainWindow->m_pTargetCopy->ClearData();
			}

			int vertex_number = pMainWindow->m_pTargetMesh->num_of_vertex_list();
			for (size_t i = 0; i != vertex_number; i++)	//复制顶点
			{
				pMainWindow->m_pTargetCopy->InsertVertex(pMainWindow->m_pTargetMesh->get_vertex(i)->position_);
			}
		}

		mouth_open = FALSE;



		glGenTextures(1, &texture[0]);
		QImage tex1, buf;
		if (!buf.load("./data/Woman.bmp"))
		{
			//        QMessageBox::warning(this, tr("Load Fialed!"), tr("Cannot Load Image %1").arg(filenames.at(0)));
			emit(operatorInfo(QString("Load Woman Texture Failed!")));
			return;

			//         QImage dummy(128, 128, QImage::Format_ARGB32);
			//         dummy.fill(Qt::green);
			//         buf = dummy;

		}

		tex1 = QGLWidget::convertToGLFormat(buf);
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, tex1.width(), tex1.height(),
			GL_RGBA, GL_UNSIGNED_BYTE, tex1.bits());

		m_bLoadTex = true;

		float width = buf.width()*1.0;
		float height = buf.height()*1.0;
		std::vector<Vec3f> mesh_point;
		m_pMesh->DeUnify(2.0, mesh_point);

		for (int i = 0; i != m_pMesh->num_of_vertex_list(); i++)
		{
			m_pMesh->get_vertex(i)->texCoord_[0] = mesh_point[i][0] / width;
			m_pMesh->get_vertex(i)->texCoord_[1] = 1 - (mesh_point[i][1] / height);
		}
		DrawTexture(true);
		updateGL();

	}



}
