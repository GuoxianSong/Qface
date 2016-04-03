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
	Tom,				//����tom
	Woman,				//����woman
	kother				//����other
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


	QComboBox					*combobox_shape_;					//������ѡ����״
	QComboBox					*combobox_paste_;					//������ѡ��ճ������

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
	Mesh3D			*m_pSourceMesh;		//����ǰ��Դ����ͼ
	Mesh3D			*m_pTargetMesh;		//��Դ��������ε�Ŀ������
	Mesh3D			*m_pTargetCopy;		//����Ŀ������
	COpenGL			*m_pGLViewer;
	/*vector<Mesh3D *> m_pMeshDeformList;		//�洢�������񣨱��ι��̣�*/

//�۲�������������֮���ԣ������ۣ�2-8,3-7��4-6�� ���ۣ�10-16,11-15,12-14����
//�ֱ���Ϊleft_group_1,left_group_2,left_group_3;right_group_1,right_group_2,right_group_3
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
	void getDepthImage(HANDLE &depthEvent, HANDLE &depthStreamHandle, IplImage *depthImage);//kinect��API
	void getSkeletonImage(HANDLE &skeletonEvent, IplImage *skeletonImage, IplImage *colorImage, IplImage *depthImage);//Ԥ������API
	void drawSkeleton(IplImage *image, CvPoint pointSet[], int witchone);
	void getTheContour(IplImage *image, int whichone, IplImage *mask);//�õ�������������� 

	int getDepthImage_depthdata(HANDLE &depthEvent, HANDLE &depthStreamHandle, IplImage *depthImage,int row,int col);//kinect��һ������������
};

#endif // MESHFRAME_H
