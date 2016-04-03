#ifndef MESHFRAME_H
#define MESHFRAME_H
#include "Windows.h"   
#include <QtWidgets/QMainWindow>
#include "ui_meshframe.h"
#include "Transfer.h"
#include"NuiApi.h"
#include <stdlib.h>
#include <stdio.h>
#include "cv.h"
#include "highgui.h"
#include "FaceTrackingDll.h"


#include  "math.h" 
    
#pragma comment(lib,"../bin/FaceTrackingDll.lib")

class QLabel;
class QPushButton;
class QCheckBox;
class COpenGL;
class QRadioButton;
class QCheckBox;
class QGroupBox;
class Mesh3D;
class Transfer;


enum RegionShape
{
	Tom,				//绘制tom
	Woman,				//绘制woman
	kother				//绘制other
};




//class GLViewer;

class GLMainWindow : public QMainWindow
{
	Q_OBJECT

	QToolBar	*realtimeAnimationToolBar;
	QAction		*realtimeAnimattionAction;

	QToolBar	*readAVIToolBar;
	QAction		*readAVIAction;

	QToolBar	*tapeAVIToolBar;
	QAction		*tapeAVIAction;

	QToolBar	*realtime_from_kinet_ToolBar;
	QAction		*realtime_from_kinet_Action;

	QToolBar     *simulate_friend_ToorBar;
	QAction      *simulate_friend_Action;
	QAction      *Simulate_friend_face_Action;


	QComboBox					*combobox_shape_;					//下拉框选择形状
	QComboBox					*combobox_paste_;					//下拉框选择粘贴方法

	QToolBar     *confirm_shape_toolbar;
	QAction      *confirm_shape_action;

public:
	explicit GLMainWindow(QWidget *parent = 0);
	~GLMainWindow();

	enum {dragPoint = 0, addPoint, delPoint, changeTagent} radioState;

private:
	Ui::MeshFrameClass* ui;



	/*-----------------Render RadioButtons-------------------------*/
private:
	QCheckBox *checkRenderPoint;
	QCheckBox *checkRenderEdge;
	QCheckBox *checkRenderTriangle;
	QCheckBox *checkRenderLight;
	QCheckBox *checkRenderTexture;
	QCheckBox *checkRenderAxes;

	QGroupBox *groupRender;
	QGroupBox *pecede;

	void createGroupRender();

	/*--------------------Basic Operator Tool-----------------*/
private:
	QAction *actionReadOBJ;
	QAction *actionLoadTex;
	QAction *actionChangeBackground;
	QToolBar *basicToolBar;
	QToolBar *basicToolBar_set;

	void createBasicTools();

private:

	QMenu   *m_fileMenu;
	QMenu   *helpMenu;

	QToolBar *m_fileToolBar;

	QStatusBar *m_statusBar;

	QAction *openAction;
	QAction *saveAction;
	QAction *exitAction;



	QAction *separatorAction;
	QAction *aboutQtAction;
	QAction *aboutAction;

	QLabel  *meshInfoLabel;
	QLabel  *operatorInfoLabel;

protected:
	void keyPressEvent(QKeyEvent* e);
	void keyReleaseEvent(QKeyEvent* e);
private:
	void createActions();
	void createMenus();
	void createToolBars();
	void createStatusBar();

	public slots:
		void showMeshInfo(int npoint, int nedge, int nface);
		void openFile();
		void about();
		void realtimeAnimation();
		void readAVI();
		void tapeAVI();
		void from_kinect();
		void simulate();
		void simulate_face(CvCapture *pCapture);
		void Choose();//Tom Woman_face obj
		


public:
	void facialAnimation(CvCapture *pCapture);
	void getNewMeshVertex(AAM_POINT_2D *meshvertex,Mesh3D *mesh);
	void getstandface(CvCapture *pCapture);

	void getyourfriend_data(CvCapture *pCapture);
	void kinect_process_standface();

public:
	Mesh3D			*m_pSourceMesh;		//变形前的源网格图
	Mesh3D			*m_pTargetMesh;		//随源网格而变形的目标网格
	Mesh3D			*m_pTargetCopy;		//复制目标网格
	COpenGL			*m_pGLViewer;
	/*vector<Mesh3D *> m_pMeshDeformList;		//存储变形网格（变形过程）*/

//眼部特征点纵坐标之差，配对：（左眼：2-8,3-7，4-6； 右眼：10-16,11-15,12-14），
//分别令为left_group_1,left_group_2,left_group_3;right_group_1,right_group_2,right_group_3
public:
	float left_group_1;
	float left_group_2;
	float left_group_3;

	float right_group_1;
	float right_group_2;
	float right_group_3;

	float  mouse_1;
	float mouse_2;
	float mouse_3;

	vector<double>x_88_2_100;
	vector<double>y_88_2_100;
private:
	int m_tapeAVIMark;

	void getColorImage(HANDLE &colorEvent, HANDLE &colorStreamHandle, IplImage *colorImage);
	void getDepthImage(HANDLE &depthEvent, HANDLE &depthStreamHandle, IplImage *depthImage);//kinect的API
	void getSkeletonImage(HANDLE &skeletonEvent, IplImage *skeletonImage, IplImage *colorImage, IplImage *depthImage);//预留开发API
	void drawSkeleton(IplImage *image, CvPoint pointSet[], int witchone);
	void getTheContour(IplImage *image, int whichone, IplImage *mask);//得到各个人物的轮廓 

	int getDepthImage_depthdata(HANDLE &depthEvent, HANDLE &depthStreamHandle, IplImage *depthImage,int row,int col);//kinect的一个点的深度数据
};

#endif // MESHFRAME_H
