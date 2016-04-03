#include "meshframe.h"
#include "ui_meshframe.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QMessageBox>
#include<QtWidgets/QComboBox>
#include <QKeyEvent>
#include "OpenGL.h"
#include "qmessagebox.h"

#include <QFileDialog>
#include <iostream>
#include <QtWidgets/QMenu>
#include <QtWidgets/QAction>
#include <QTextCodec>
#include <gl/GLU.h>
#include <glut.h>
#include <algorithm>
#include "ArcBall.h"
#include "globalFunctions.h"
#include "HE_mesh/Mesh3D.h"


bool tracked[NUI_SKELETON_COUNT] = { FALSE };
CvPoint skeletonPoint[NUI_SKELETON_COUNT][NUI_SKELETON_POSITION_COUNT] = { cvPoint(0, 0) };
CvPoint colorPoint[NUI_SKELETON_COUNT][NUI_SKELETON_POSITION_COUNT] = { cvPoint(0, 0) };


using namespace std;


GLMainWindow::GLMainWindow(QWidget *parent) :
QMainWindow(parent),
	ui(new Ui::MeshFrameClass)
{
	ui->setupUi(this);

	m_pGLViewer = new COpenGL(this, this);

	QStringList shapes;
	shapes << tr("Tom") << tr("Woman");
	combobox_shape_ = new QComboBox(this);
	combobox_shape_->addItems(shapes);
	connect(combobox_shape_, SIGNAL(activated(int)), this, SLOT(Choose()));

	createBasicTools();


	//基于摄像头的实时面部表情驱动
	realtimeAnimattionAction = new QAction(tr("Realtime_Animation"),this);
	connect(realtimeAnimattionAction, SIGNAL(triggered()), this, SLOT(realtimeAnimation()));

	realtimeAnimationToolBar = addToolBar(tr("realtimeAnimation"));

	realtimeAnimationToolBar->addAction(realtimeAnimattionAction);

	//读取AVI视频进行表情驱动
	readAVIAction = new QAction(tr("Read_AVI"),this);
	connect(readAVIAction, SIGNAL(triggered()), this, SLOT(readAVI()));

	readAVIToolBar = addToolBar(tr("readAVI"));

	readAVIToolBar->addAction(readAVIAction);

	//基于摄像头录制表情驱动的AVI视频
	tapeAVIAction = new QAction(tr("Tape_AVI"),this);
	connect(tapeAVIAction, SIGNAL(triggered()), this, SLOT(tapeAVI()));//面部表情驱动

	tapeAVIToolBar = addToolBar(tr("tapeAVI"));

	tapeAVIToolBar->addAction(tapeAVIAction);

	//设置kinect接口
	realtime_from_kinet_Action=new QAction(tr("Kinect_Animation"), this);
	connect(realtime_from_kinet_Action, SIGNAL(triggered()), this, SLOT(from_kinect()));//面部表情驱动

	realtime_from_kinet_ToolBar = addToolBar(tr("data_from_kinect"));

	realtime_from_kinet_ToolBar->addAction(realtime_from_kinet_Action);


	//设置simulate
	simulate_friend_Action = new QAction(tr("Get_your_friend_face"), this);
	connect(simulate_friend_Action, SIGNAL(triggered()), this, SLOT(simulate()));//面部表情驱动

	simulate_friend_ToorBar = addToolBar(tr("simulate"));

	simulate_friend_ToorBar->addAction(simulate_friend_Action);

	//Simulate_friend_face_Action = new QAction(tr("Simulate_your_friend"), this);
	//connect(Simulate_friend_face_Action, SIGNAL(triggered()), this, SLOT(simulate_face()));//面部表情驱动
	//simulate_friend_ToorBar->addAction(Simulate_friend_face_Action);



   createStatusBar ();
	createActions ();
	createMenus ();
	createToolBars ();
	

	createGroupRender ();

	//createBasicTools ();

	QVBoxLayout *leftLayout = new QVBoxLayout;

	leftLayout->addWidget (groupRender);
	leftLayout->addStretch (4);

	QHBoxLayout *mainLayout = new QHBoxLayout;


	mainLayout->addLayout (leftLayout);
	mainLayout->addWidget (m_pGLViewer);
	mainLayout->setStretch (1,1);
	this->centralWidget ()->setLayout (mainLayout);

	m_fileToolBar->setVisible(false);

	m_pSourceMesh = NULL;
	m_pTargetMesh = NULL;
	m_pTargetCopy = new Mesh3D;

	left_group_1 = 0;
	left_group_2 = 0;
	left_group_3 = 0;
	right_group_1 = 0;
	right_group_2 = 0;
	right_group_3 = 0;
	
	m_tapeAVIMark = 0;
}
void GLMainWindow::openFile()
{
	m_pGLViewer->ReadMesh();
}

void GLMainWindow::about()
{
	QMessageBox::information(this, "About QtMeshFrame-1.0.1",
		
		QString("<h3>This MeshFrame provides some operations about *.obj files sunch as") + 
				" IO, render with points , edges, triangles or textures and some interactions with mouse." 
				" A fix light source is provided for you."
				"This is a basic and raw frame for handling meshes. The mesh is of half_edge struct.\n"
				"Please contact" "<font color=blue> sgx2012@mail.ustc.edu.cn<\font><font color=black>, Guoxian Song if you has any questions.<\font><\n3>"
				,
		QMessageBox::Ok);
}

GLMainWindow::~GLMainWindow()
{
	delete m_pTargetCopy;
	delete ui;
}

void GLMainWindow::createGroupRender()
{
	checkRenderPoint = new QCheckBox(tr("Point"),this);
	connect(checkRenderPoint, SIGNAL(clicked(bool)),
		m_pGLViewer, SLOT(checkDrawPoint(bool)));

	checkRenderPoint->setChecked(true);

	checkRenderEdge  = new QCheckBox(tr("Edge"),this);
	connect(checkRenderEdge, SIGNAL(clicked(bool)),
		m_pGLViewer, SLOT(checkDrawEdge(bool)));

	checkRenderTriangle = new QCheckBox(tr("Triangle"),this);
	connect(checkRenderTriangle, SIGNAL(clicked(bool)),
		m_pGLViewer, SLOT(checkDrawFace(bool)));

	checkRenderLight = new QCheckBox(tr("Light"), this);
	connect(checkRenderLight, SIGNAL(clicked(bool)),
		m_pGLViewer, SLOT(checkLight(bool)));

	checkRenderTexture = new QCheckBox(tr("Texture"),this);
	connect(checkRenderTexture, SIGNAL(clicked(bool)),
		m_pGLViewer, SLOT(checkDrawTex(bool)));

//	checkRenderTexture->setChecked(true);

	checkRenderAxes = new QCheckBox(tr("Axes"),this);
	connect(checkRenderAxes, SIGNAL(clicked(bool)),
		m_pGLViewer, SLOT(checkDrawAxes(bool)));

	groupRender = new QGroupBox(tr("Render"),this);

	QVBoxLayout * renderLayout = new QVBoxLayout(groupRender);
	renderLayout->addWidget (checkRenderPoint);
	renderLayout->addWidget (checkRenderEdge);
	renderLayout->addWidget (checkRenderTriangle);
	renderLayout->addWidget (checkRenderTexture);

	renderLayout->addWidget (checkRenderLight);
	renderLayout->addWidget (checkRenderAxes);
}

void GLMainWindow::createBasicTools()
{
	basicToolBar = addToolBar (tr("basicToolBar"));
	actionReadOBJ = new QAction(tr("Read_AvatarMesh"), this);
	actionChangeBackground = new QAction(tr("Change_Background"), this);
	actionLoadTex = new QAction(tr("Load_Texture"), this);

	confirm_shape_action = new QAction(tr("Confirm "), this);
	connect(confirm_shape_action, SIGNAL(triggered()), this, SLOT(Choose()));//面部表情驱动
	//confirm_shape_action->setIcon(QIcon(":right_3.ico"));
	

	
confirm_shape_toolbar = addToolBar(tr("Confirm"));
	connect(actionReadOBJ, SIGNAL(triggered()),m_pGLViewer, SLOT(ReadMesh()));
	connect(actionChangeBackground, SIGNAL(triggered()),
		this->m_pGLViewer, SLOT(SetBackground()));
	connect(actionLoadTex, SIGNAL(triggered()),m_pGLViewer, SLOT(LoadTexture()));
basicToolBar->addWidget(combobox_shape_);
basicToolBar->addAction(confirm_shape_action);

basicToolBar_set = addToolBar(tr("basicToolBar_set_by_file"));
basicToolBar_set->addAction(actionReadOBJ);
basicToolBar_set->addAction(actionLoadTex);
basicToolBar_set->addAction(actionChangeBackground);

	

	/*pecede=new QGroupBox(tr("Pecede"), this);
	QVBoxLayout * pecedeLayout = new QVBoxLayout(pecede);
	*/
	
}

void GLMainWindow::createActions()
{
	/*-------------File Actions-----------------------*/

	openAction = new QAction(tr("&Open"), this);
	openAction->setIcon (QIcon(":/images/open.png"));
	openAction->setShortcut (tr("Ctrl+O"));
	openAction->setStatusTip (tr("Open a new file"));
	connect(openAction, SIGNAL(triggered()),
		this, SLOT(openFile()));

	saveAction = new QAction(tr("&Save"),this);
	saveAction->setIcon (QIcon(":/images/save.png"));
	saveAction->setShortcut (tr("Ctrl+S"));
	connect(saveAction, SIGNAL(triggered()),
		m_pGLViewer, SLOT(WriteMesh()));

	exitAction = new QAction(tr("&Exit"),this);

	/*-------------------About Actions-------------------------------*/

	aboutAction = new QAction(tr("About"),this);
	aboutAction->setStatusTip ("About this");
	connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

	aboutQtAction = new QAction(tr("About &Qt"),this);
	aboutQtAction->setStatusTip ("Show this Qt Library's About box");
	connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	
}

void GLMainWindow::createMenus()
{
	/*-------------------file menu-----------------------------*/
	m_fileMenu = menuBar ()->addMenu (tr("&File"));
	m_fileMenu->addAction (openAction);
	m_fileMenu->addAction (saveAction);

	separatorAction = m_fileMenu->addSeparator ();

	m_fileMenu->addAction(exitAction);
	menuBar()->addSeparator ();
	helpMenu = menuBar()->addMenu (tr("&Help"));
	helpMenu->addAction(aboutAction);
	helpMenu->addAction(aboutQtAction);
}

void GLMainWindow::createToolBars ()
{
	m_fileToolBar = addToolBar (tr("&File"));
	m_fileToolBar->addAction(openAction);
	m_fileToolBar->addAction(saveAction);
}

void GLMainWindow::createStatusBar ()
{
	meshInfoLabel = new QLabel(QString("MeshInfo: p: %1 e: %2 f: %3").arg(0).arg(0).arg(0));
	meshInfoLabel->setAlignment(Qt::AlignCenter);
	meshInfoLabel->setMinimumSize(meshInfoLabel->sizeHint());

	operatorInfoLabel = new QLabel(/*QString(100,' ')*/);
	operatorInfoLabel->setAlignment(Qt::AlignVCenter);


	//   statusBar ()->addWidget (locationlabel);

	statusBar()->addWidget(meshInfoLabel);
	connect(m_pGLViewer, SIGNAL(meshInfo(int ,int ,int)),
		this, SLOT(showMeshInfo(int ,int ,int)));

	statusBar()->addWidget(operatorInfoLabel);
	connect(m_pGLViewer, SIGNAL(operatorInfo(QString)),
		operatorInfoLabel, SLOT(setText(QString)));
}

void GLMainWindow::showMeshInfo(int npoint, int nedge, int nface)
{
	meshInfoLabel->setText(QString("MeshInfo: p: %1 e: %2 f: %3").arg(npoint).arg(nedge).arg(nface));
}
void GLMainWindow::keyPressEvent(QKeyEvent* e)
{
	m_pGLViewer->keyPressEvent(e);
}

void GLMainWindow::keyReleaseEvent(QKeyEvent* e)
{
	m_pGLViewer->keyReleaseEvent(e);
}

void GLMainWindow::getNewMeshVertex(AAM_POINT_2D *meshvertex, Mesh3D *mesh)
{
	if (mesh->num_of_vertex_list() != 0)
	{
		mesh->ClearData();
	}

	for (int i = 0; i<87; i++)
	{
		mesh->InsertVertex(Vec3f(meshvertex[i].x, 0 - meshvertex[i].y, 0));
	}

	if (m_tapeAVIMark == 3)
	{
		double y_div = min(meshvertex[86].y - meshvertex[85].y, meshvertex[68].y - meshvertex[67].y) / 4;
		double x_87_2_69_div = (meshvertex[86].x - meshvertex[68].x) / 12;
		x_88_2_100.clear();
		y_88_2_100.clear();
        for (int i = 99; i >= 97; i--)
		{
			if (i == 98)
			{
				mesh->InsertVertex(Vec3f(meshvertex[86].x - (i - 86.5)*x_87_2_69_div, 0 - (meshvertex[68].y + y_div*(100 - i)), 0));
				x_88_2_100.push_back(meshvertex[86].x - (i - 86.5)*x_87_2_69_div);
				y_88_2_100.push_back((meshvertex[68].y + y_div*(100 - i)));
			}
			else
			{
				mesh->InsertVertex(Vec3f(meshvertex[86].x - (i - 87)*x_87_2_69_div, 0 - (meshvertex[68].y + y_div*(100 - i)), 0));
				x_88_2_100.push_back(meshvertex[86].x - (i - 87)*x_87_2_69_div);
				y_88_2_100.push_back((meshvertex[68].y + y_div*(100 - i)));
			}
		}
        for (int i = 96; i >= 90; i--)
		{
			mesh->InsertVertex(Vec3f(meshvertex[86].x - (i - 87)*x_87_2_69_div, 0 - (meshvertex[86].y + y_div*(90 - 86)), 0));
			x_88_2_100.push_back(meshvertex[86].x - (i - 87)*x_87_2_69_div);
			y_88_2_100.push_back((meshvertex[86].y + y_div*(90 - 86)));
		}

		for (int i = 89; i >=87; i--)
		{
			if (i == 88)
			{
				mesh->InsertVertex(Vec3f(meshvertex[86].x - (i - 87.5)*x_87_2_69_div, 0 - (meshvertex[86].y + y_div*(i - 86)), 0));
				x_88_2_100.push_back(meshvertex[86].x - (i - 87.5)*x_87_2_69_div);
				y_88_2_100.push_back((meshvertex[86].y + y_div*(i - 86)));
			}
			else
			{
				mesh->InsertVertex(Vec3f(meshvertex[86].x - (i - 87)*x_87_2_69_div, 0 - (meshvertex[86].y + y_div*(i - 86)), 0));
				x_88_2_100.push_back(meshvertex[86].x - (i - 87)*x_87_2_69_div);
				y_88_2_100.push_back((meshvertex[86].y + y_div*(i - 86)));
			}
			
		}
		
		
	}
	mesh->ComputeBoundingBox();
	mesh->Unify(2.f);
}

void GLMainWindow::facialAnimation(CvCapture *pCapture)
{
	QMessageBox::information(NULL, "Information", "You can tap 'Spacebar'to stop this animation. ", QMessageBox::Yes, QMessageBox::Yes);
	CvVideoWriter *writer = NULL;	//当录制视频时用到
	static int index = 1;
	char fname[128];

	if ( pCapture==NULL )
	{
		QMessageBox::warning(this, tr("Failed!!!"), tr("Capture is NULL!"));
		return;
	}

	if ( m_pTargetMesh==NULL )
	{
		QMessageBox::warning(this, tr("Read Failed!!!"), tr("There is No Mesh!"));
		return;
	}

	Mesh3D *sourcemesh = new Mesh3D;	//copy源网格最初的网格数据
	vector<vector<Vec3f>> list_of_meshpoints_after_animation;	//存储target mesh每次变形后网格点的位置

	static int nImgWidth = 0;
	static int nImgHeight = 0;
	static int nImgSize = 0;
	bool bReset = true;

	nImgWidth   = cvGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_WIDTH);
	nImgHeight  = cvGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_HEIGHT);

	cvSetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_WIDTH, nImgWidth);
	cvSetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_HEIGHT, nImgHeight);

	char pcMdlPath[512];
	sprintf(pcMdlPath, "./data/model_map_2d_data.bin");
	FILE* fp = fopen(pcMdlPath, "rb");
	if (fp == NULL)
	{
		QMessageBox::warning(this, tr("Failed!!!"), tr("file is NULL!"));
		return;
	}
	fseek(fp, 0, SEEK_SET);
	fseek(fp, 0, SEEK_END);
	long len_bytes = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	unsigned char* pLoadData = (unsigned char*)malloc(len_bytes);
	fread(pLoadData, sizeof(unsigned char), len_bytes, fp);
	if (!EiInitialize(nImgWidth, nImgHeight, pLoadData, len_bytes) ||
		!EiInitialize_NewReso(nImgWidth, nImgHeight))
	{
		printf("initialize failed\r\n");
		free(pLoadData);
		if (pCapture)
			cvReleaseCapture(&pCapture);
		exit(-1);
	}
	free(pLoadData);

	nImgSize = nImgWidth * nImgHeight;

	IplImage *pImg = cvCreateImage(cvSize(nImgWidth, nImgHeight), 8, 3);
	IplImage *outImg = cvCreateImage(cvSize(nImgWidth, nImgHeight), 8, 3);
	                    
	unsigned char * pRGBSrc = new unsigned char[nImgSize * 3 * sizeof(unsigned char)];
	unsigned char * pRGBDest = new unsigned char[nImgSize * 3 * sizeof(unsigned char)];

	cvWaitKey(2000);	//等待1.3s，用于准备（保持正脸，标准脸）

	if ( m_tapeAVIMark==1 )		//if m_tapeAVIMark=1,tape the video by camera
	{
		sprintf(fname, "video_%d.avi", index);

		writer = cvCreateVideoWriter(fname, CV_FOURCC('P','I','M','1'),30,cvGetSize(pImg),1);

		if ( writer==NULL )
		{
			QMessageBox::warning(this, tr("Create Failed!!!"), tr("VideoWriter is NULL!"));
			return;
		}
	} 

	//眼部特征点组纵坐标之差
	float left_g1 = 0;
	float left_g2 = 0;
	float left_g3 = 0;

	float right_g1 = 0;
	float right_g2 = 0;
	float right_g3 = 0;

	float mouse_1_ = 0;
	float mouse_2_ = 0;
	float mouse_3_ = 0;



	Transfer transformation;	
	
	for (;;)	// 实时镜头视频处理 //
	{

		m_pGLViewer->getmouthshut();

  		pImg = cvQueryFrame(pCapture);

		if (!pImg) break;

		if ( writer!=NULL )		//保存视频
		{
			cvWriteFrame(writer,pImg);
		}

		memcpy((unsigned char *)pRGBSrc, (unsigned char *)pImg->imageData, nImgSize * 3);
		memcpy((unsigned char *)outImg->imageData, (unsigned char *)pRGBSrc, nImgSize * 3);

		IplImage* pGrayImg = cvCreateImage(cvGetSize(pImg), 8, 1);

		cvCvtColor(pImg, pGrayImg, CV_BGR2GRAY);
		AAM_OUTPUT_STRUCT aam_ret = {0};
		aam_ret.nstate = -1;
		aam_ret.nstate = EiGetExpression((unsigned char*)pGrayImg->imageData, &aam_ret, bReset);

		if (aam_ret.nstate == 1)
		{ 
			for (int i = 0; i < aam_ret.n2DNum; ++i)  //aam_ret.n2DNum
			{
				cvDrawCircle(pImg, cvPoint(int(aam_ret.pKeyPoint2DOut[i].x + 0.5f), int(aam_ret.pKeyPoint2DOut[i].y + 0.5f)), 1, cvScalar(0, 255, 0), -1);
				
			}
// 
// 			for (int i = 0; i < aam_ret.n3DPNum; ++i)
// 				cvDrawCircle(pImg, cvPoint(int(aam_ret.pKeyPoint3DProjTo2D[i].x + 0.5f), int(aam_ret.pKeyPoint3DProjTo2D[i].y + 0.5f)), 1, cvScalar(0, 0, 255), -1);
		
			if ( m_pSourceMesh==NULL )
			{
				m_pSourceMesh = new Mesh3D;
				getNewMeshVertex(aam_ret.pKeyPoint2DOut,m_pSourceMesh);

				left_group_1 = len(m_pSourceMesh->get_vertex(1)->position_-m_pSourceMesh->get_vertex(7)->position_);
				left_group_2 = len(m_pSourceMesh->get_vertex(2)->position_-m_pSourceMesh->get_vertex(6)->position_);
				left_group_3 = len(m_pSourceMesh->get_vertex(3)->position_-m_pSourceMesh->get_vertex(5)->position_);

				right_group_1 = len(m_pSourceMesh->get_vertex(9)->position_-m_pSourceMesh->get_vertex(15)->position_);
				right_group_2 = len(m_pSourceMesh->get_vertex(10)->position_-m_pSourceMesh->get_vertex(14)->position_);
				right_group_3 = len(m_pSourceMesh->get_vertex(11)->position_-m_pSourceMesh->get_vertex(13)->position_);
			}
			else
			{
				Mesh3D *mesh = new Mesh3D;
				getNewMeshVertex(aam_ret.pKeyPoint2DOut,mesh);

				//deformation transform
				vector<Vec3f> pList;
				pList = transformation.getTransformMesh(m_pSourceMesh,m_pTargetCopy,mesh);

				left_g1 = len(mesh->get_vertex(1)->position_-mesh->get_vertex(7)->position_);
				left_g2 = len(mesh->get_vertex(2)->position_-mesh->get_vertex(6)->position_);
				left_g3 = len(mesh->get_vertex(3)->position_-mesh->get_vertex(5)->position_);

				right_g1 = len(mesh->get_vertex(9)->position_-mesh->get_vertex(15)->position_);
				right_g2 = len(mesh->get_vertex(10)->position_-mesh->get_vertex(14)->position_);
				right_g3 = len(mesh->get_vertex(11)->position_-mesh->get_vertex(13)->position_);

				mouse_1_ = len(mesh->get_vertex(61)->position_ - mesh->get_vertex(67)->position_);
				mouse_2_ = len(mesh->get_vertex(62)->position_ - mesh->get_vertex(66)->position_);
				mouse_3_ = len(mesh->get_vertex(63)->position_ - mesh->get_vertex(65)->position_);


				float left_change_scale = left_g2-left_group_2;
				float right_change_scale = right_g2-right_group_2;
				
				float mouse_change = mouse_2_ - mouse_2;

				if (fabs(mouse_change)>2*mouse_2)
				{
					m_pGLViewer->getmouthopen();
					
				}
				else
				{
					m_pGLViewer->getmouthshut();
	
				}

				//cout << mouse_change << "    " << mouse_2<<endl;


				//当左眼变小的幅度大于原来的0.19时，判定为闭眼（由于特征点检测不准）
				if ( fabs(left_change_scale)>0.19*left_group_2 && left_change_scale<0 )
				{
					pList[2][1] = (pList[0][1]+pList[4][1])/2;
					pList[6][1] = (pList[0][1]+pList[4][1])/2;

					pList[1][1] = (pList[0][1]+pList[2][1])/2;
					pList[7][1] = (pList[0][1]+pList[2][1])/2;

					pList[3][1] = (pList[2][1]+pList[4][1])/2;
					pList[5][1] = (pList[2][1]+pList[4][1])/2;
				}
				else	//左眼放大变形效果
				{
					float left_group_1_y = m_pSourceMesh->get_vertex(1)->position_[1]-m_pSourceMesh->get_vertex(7)->position_[1];
					float left_group_2_y = m_pSourceMesh->get_vertex(2)->position_[1]-m_pSourceMesh->get_vertex(6)->position_[1];
					float left_group_3_y = m_pSourceMesh->get_vertex(3)->position_[1]-m_pSourceMesh->get_vertex(5)->position_[1];

					float left_g1_y = mesh->get_vertex(1)->position_[1]-mesh->get_vertex(7)->position_[1];
					float left_g2_y = mesh->get_vertex(2)->position_[1]-mesh->get_vertex(6)->position_[1];
					float left_g3_y = mesh->get_vertex(3)->position_[1]-mesh->get_vertex(5)->position_[1];

					//适当放大左眼变形
					pList[1][1] += (left_g1_y-left_group_1_y);
					pList[2][1] += (left_g2_y-left_group_2_y);
					pList[3][1] += (left_g3_y-left_group_3_y);

					pList[5][1] -= (left_g3_y-left_group_3_y);
					pList[6][1] -= (left_g2_y-left_group_2_y);
					pList[7][1] -= (left_g1_y-left_group_1_y);
				}

				//当右眼变小的幅度大于原来的0.19时，判定为闭眼（由于特征点检测不准）
				if ( fabs(right_change_scale)>0.19*right_group_2 && right_change_scale<0 )
				{
					pList[10][1] = (pList[8][1]+pList[12][1])/2;
					pList[14][1] = (pList[8][1]+pList[12][1])/2;

					pList[9][1] = (pList[8][1]+pList[10][1])/2;
					pList[15][1] = (pList[8][1]+pList[10][1])/2;

					pList[11][1] = (pList[10][1]+pList[12][1])/2;
					pList[13][1] = (pList[10][1]+pList[12][1])/2;
				}
				else	//右眼放大变形效果
				{
					float right_group_1_y = m_pSourceMesh->get_vertex(9)->position_[1]-m_pSourceMesh->get_vertex(15)->position_[1];
					float right_group_2_y = m_pSourceMesh->get_vertex(10)->position_[1]-m_pSourceMesh->get_vertex(14)->position_[1];
					float right_group_3_y = m_pSourceMesh->get_vertex(11)->position_[1]-m_pSourceMesh->get_vertex(13)->position_[1];

					float right_g1_y = mesh->get_vertex(9)->position_[1]-mesh->get_vertex(15)->position_[1];
					float right_g2_y = mesh->get_vertex(10)->position_[1]-mesh->get_vertex(14)->position_[1];
					float right_g3_y = mesh->get_vertex(11)->position_[1]-mesh->get_vertex(13)->position_[1];

					//适当放大右眼变形
					pList[9][1] += (right_g1_y-right_group_1_y);
					pList[10][1] += (right_g2_y-right_group_2_y);
					pList[11][1] += (right_g3_y-right_group_3_y);

					pList[13][1] -= (right_g3_y-right_group_3_y);
					pList[14][1] -= (right_g2_y-right_group_2_y);
					pList[15][1] -= (right_g1_y-right_group_1_y);
				}

// 				if ( pList[17][1]<0 || pList[27][1]<0 || pList[72][1]>0 || pList[82][1]>0 || pList[42][1]<pList[51][1] || pList[42][1]<pList[57][1])	 // 
// 				{	/* 无论如何变形，第18、28个特征点（眉毛最高点）的y值总是应该大于0；第73、83个特征点（下巴）的y值总是应该小于0(第78个特征点为下巴最低点)； */																		 
// 					/* 第52个特征点（上嘴唇）、58个特征点（下嘴唇）y值总是应该大于第43个特征点（鼻子下部）的y值；否则说明检测特征点出错或者计算矩阵得到的解出错 */
// 					//出错情况下，以上次正确结果作为本次结果
// 					int num_of_animation_frame = list_of_meshpoints_after_animation.size();
// 					if ( num_of_animation_frame!=0 )
// 					{
// 						int num_of_vertices = m_pTargetMesh->num_of_vertex_list();
// 						for ( size_t i=0; i!=num_of_vertices; i++ )		//当未检测到人脸（特征点）时，目标图像保持最后检测到的脸相对应的图像
// 						{
// 							m_pTargetMesh->get_vertex(i)->set_position(list_of_meshpoints_after_animation[num_of_animation_frame-1][i]);
// 						}
// 
// 						list_of_meshpoints_after_animation.push_back(list_of_meshpoints_after_animation[num_of_animation_frame-1]);
// 					}
// 				}
// 				else
// 				{
					//正常情况下


				/*-------------------------------------------Shape And Limit process----------------------------------------------*/
				if (list_of_meshpoints_after_animation.size() != 0)
				{
					vector<Vec3f> temp;
					temp = list_of_meshpoints_after_animation[list_of_meshpoints_after_animation.size() - 1];
					double sum_length_move = 0;
					for (int i = 0; i < 87; i++)
					{
						Vec3f list_temp = pList[i];
						Vec3f temp_temp = temp[i];
						double shakemove = pow(list_temp.x() - temp_temp.x(), 2) + pow(list_temp.y() - temp_temp.y(), 2);
						//qDebug("my window handle = %f", shakemove);
						sum_length_move = sum_length_move + shakemove;
						if (shakemove < 0.0001)
							pList[i] = temp[i];
					}
					if (sum_length_move>10)
					{
						pList = temp;
					}
					temp.clear();
				}



					int num_of_vertices = m_pTargetMesh->num_of_vertex_list();
					for ( size_t i=0; i!=num_of_vertices; i++ )
					{
						m_pTargetMesh->get_vertex(i)->set_position(pList[i]);
					}

					list_of_meshpoints_after_animation.push_back(pList);	//存储Target变形后的网格点位置
//				}

				m_pGLViewer->updateGL();
			}

			bReset = false;
		}
		else
		{
			int num_of_animation_frame = list_of_meshpoints_after_animation.size();
			if ( num_of_animation_frame!=0 )
			{
				int num_of_vertices = m_pTargetMesh->num_of_vertex_list();
				for ( size_t i=0; i!=num_of_vertices; i++ )		//当未检测到人脸（特征点）时，目标图像保持最后检测到的脸相对应的图像
				{
					m_pTargetMesh->get_vertex(i)->set_position(list_of_meshpoints_after_animation[num_of_animation_frame-1][i]);
				}
			}

			bReset = true;
		}

		cvShowImage("original video",pImg);

		cvReleaseImage(&pGrayImg);

		char c = cvWaitKey(2);
		switch ( c )
		{
		case 27:
			break;
		}
		if (char(c) == ' ')
		{
			if ( m_tapeAVIMark==1 )
			{
				cvReleaseVideoWriter(&writer);
				m_tapeAVIMark = 0;

				QMessageBox::StandardButton rb = QMessageBox::question(NULL, "Save Video", 
					"Do You Want to Save the Video as AVI", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

				if ( rb == QMessageBox::No )	//若选择No，删除前面录制的视频
				{
					remove(fname);
				}
				else
				{
					index++;
				}
			}
			
			break;
		}
	}

	delete [] pRGBSrc;
	delete [] pRGBDest;

	delete sourcemesh;

//	cvReleaseImage(&pImg);
	cvReleaseImage(&outImg);
	cvDestroyWindow("original video");
	EiDestroy_NewReso();
	EiDestroy();

	int vertex_number = m_pTargetMesh->num_of_vertex_list(); 
	for ( size_t i=0; i!=vertex_number; i++ )		//初始化目标网格点位置
	{
		m_pTargetMesh->get_vertex(i)->set_position(m_pTargetCopy->get_vertex(i)->position_);
	}
	m_pGLViewer->updateGL();
	
	delete m_pSourceMesh;
	m_pSourceMesh = NULL;
}

void GLMainWindow::realtimeAnimation()
{
	while( m_pTargetMesh == NULL )			//提示读取目标网格图和纹理图
	{
		QMessageBox::StandardButton rb = QMessageBox::question(NULL, "Read Mesh", 
			"Please read a mesh firstly!", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
		if ( rb == QMessageBox::Yes )	//若选择Yes，读取网格
		{
			m_pGLViewer->ReadMesh();
		}
		
		if ( m_pTargetMesh != NULL )
		{
			QMessageBox::StandardButton rb = QMessageBox::question(NULL, "Load Texture", 
				"Please load a texture firstly!", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
			if ( rb == QMessageBox::Yes )	//若选择Yes，载入纹理图
			{
				m_pGLViewer->LoadTexture();
			}
		}
	}

	// Camera Load
	CvCapture *pCapture = cvCaptureFromCAM(0) ;
	
	facialAnimation(pCapture);

	if (pCapture)
	{
		cvReleaseCapture(&pCapture);
	}
}

void GLMainWindow::readAVI()
{
	while( m_pTargetMesh == NULL )			//提示读取目标网格图和纹理图
	{
		QMessageBox::StandardButton rb = QMessageBox::question(NULL, "Read Mesh", 
			"Please read a mesh firstly!", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
		if ( rb == QMessageBox::Yes )	//若选择Yes，读取网格
		{
			m_pGLViewer->ReadMesh();
		}

		if ( m_pTargetMesh != NULL )
		{
			QMessageBox::StandardButton rb = QMessageBox::question(NULL, "Load Texture", 
				"Please load a texture firstly!", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
			if ( rb == QMessageBox::Yes )	//若选择Yes，载入纹理图
			{
				m_pGLViewer->LoadTexture();
			}
		}
	}
	
	// AVI Video Load
	QString filename = QFileDialog::
		getOpenFileName(this,tr("Read AVI Video"),
		"..",tr("Video(*.AVI)"));

	if (filename.isEmpty())
	{
		QMessageBox::warning(this, tr("Failed!!!"), tr("Cannot Load Video"));
		return;
	}
	//中文路径支持
	QTextCodec *code = QTextCodec::codecForName("gd18030");
	QTextCodec::setCodecForLocale(code);
	QByteArray byfilename = filename.toLocal8Bit();

	CvCapture *pCapture = cvCreateFileCapture(byfilename.data()) ;
	
	facialAnimation(pCapture);

	if (pCapture)
	{
		cvReleaseCapture(&pCapture);
	}
}

void GLMainWindow::tapeAVI()
{
	m_tapeAVIMark = 1;	 //录制视频时，m_tapeAVIMark标记为1

	while( m_pTargetMesh == NULL )		//提示读取目标网格图和纹理图
	{
		QMessageBox::StandardButton rb = QMessageBox::question(NULL, "Read Mesh", 
			"Please read a mesh firstly!", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
		if ( rb == QMessageBox::Yes )	//若选择Yes，读取网格
		{
			m_pGLViewer->ReadMesh();
		}

		if ( m_pTargetMesh != NULL )
		{
			QMessageBox::StandardButton rb = QMessageBox::question(NULL, "Load Texture", 
				"Please load a texture firstly!", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
			if ( rb == QMessageBox::Yes )	//若选择Yes，载入纹理图
			{
				m_pGLViewer->LoadTexture();
			}
		}
	}

	// Camera Load
	CvCapture *pCapture = cvCaptureFromCAM(0) ;
	getstandface(pCapture);
	facialAnimation(pCapture);

	if (pCapture)
	{
		cvReleaseCapture(&pCapture);
	}
}

void GLMainWindow::getstandface(CvCapture *pCapture)
{
	QMessageBox::information(NULL, "Information", "I want to tape your face as a standard face. Please tap 'e', when you think this face are ready.", QMessageBox::Yes, QMessageBox::Yes);
	if (pCapture == NULL)
	{
		QMessageBox::warning(this, tr("Failed!!!"), tr("Capture is NULL!"));
		return;
	}
	IplImage* pFrame = NULL;

	static int nImgWidth = 0;
	static int nImgHeight = 0;
	static int nImgSize = 0;
	bool bReset = true;

	nImgWidth = cvGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_WIDTH);
	nImgHeight = cvGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_HEIGHT);

	cvSetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_WIDTH, nImgWidth);
	cvSetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_HEIGHT, nImgHeight);

	char pcMdlPath[512];
	sprintf(pcMdlPath, "./data/model_map_2d_data.bin");
	FILE* fp = fopen(pcMdlPath, "rb");
	if (fp == NULL)
	{
		QMessageBox::warning(this, tr("Failed!!!"), tr("file is NULL!"));
		return;
	}
	fseek(fp, 0, SEEK_SET);
	fseek(fp, 0, SEEK_END);
	long len_bytes = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	unsigned char* pLoadData = (unsigned char*)malloc(len_bytes);
	fread(pLoadData, sizeof(unsigned char), len_bytes, fp);
	if (!EiInitialize(nImgWidth, nImgHeight, pLoadData, len_bytes) ||
		!EiInitialize_NewReso(nImgWidth, nImgHeight))
	{
		printf("initialize failed\r\n");
		free(pLoadData);
		if (pCapture)
			cvReleaseCapture(&pCapture);
		exit(-1);
	}
	free(pLoadData);


	//显示视屏  
	while (1)
	{
		pFrame = cvQueryFrame(pCapture);
		IplImage* pGrayImg = cvCreateImage(cvGetSize(pFrame), 8, 1);
		cvCvtColor(pFrame, pGrayImg, CV_BGR2GRAY);
		AAM_OUTPUT_STRUCT aam_ret = { 0 };
		aam_ret.nstate = -1;
		aam_ret.nstate = EiGetExpression((unsigned char*)pGrayImg->imageData, &aam_ret, bReset);

		if (aam_ret.nstate == 1)
		{
			for (int i = 0; i < aam_ret.n2DNum; ++i)  //aam_ret.n2DNum
			{
				cvDrawCircle(pFrame, cvPoint(int(aam_ret.pKeyPoint2DOut[i].x + 0.5f), int(aam_ret.pKeyPoint2DOut[i].y + 0.5f)), 1, cvScalar(0, 255, 0), -1);
			}
		}
		if (!pFrame)break;
		cvShowImage("video", pFrame);
		char c = cvWaitKey(33);
		if (c == 27)break;
		if (c == 'e')
		{
			QMessageBox::StandardButton rb = QMessageBox::question(NULL, "Standard Face",
				"Do You Want to Use this picture as a standard face ?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

			if (rb == QMessageBox::Yes)	//若选择No，则重新选择标准脸
			{
				m_pSourceMesh = new Mesh3D;
				getNewMeshVertex(aam_ret.pKeyPoint2DOut, m_pSourceMesh);
				break;
			}
			else
			{
				continue;
			}
		}
	}
	left_group_1 = len(m_pSourceMesh->get_vertex(1)->position_ - m_pSourceMesh->get_vertex(7)->position_);
	left_group_2 = len(m_pSourceMesh->get_vertex(2)->position_ - m_pSourceMesh->get_vertex(6)->position_);
	left_group_3 = len(m_pSourceMesh->get_vertex(3)->position_ - m_pSourceMesh->get_vertex(5)->position_);

	right_group_1 = len(m_pSourceMesh->get_vertex(9)->position_ - m_pSourceMesh->get_vertex(15)->position_);
	right_group_2 = len(m_pSourceMesh->get_vertex(10)->position_ - m_pSourceMesh->get_vertex(14)->position_);
	right_group_3 = len(m_pSourceMesh->get_vertex(11)->position_ - m_pSourceMesh->get_vertex(13)->position_);


	mouse_1 = len(m_pSourceMesh->get_vertex(61)->position_ - m_pSourceMesh->get_vertex(67)->position_);
	mouse_2 = len(m_pSourceMesh->get_vertex(62)->position_ - m_pSourceMesh->get_vertex(66)->position_);
	mouse_3 = len(m_pSourceMesh->get_vertex(63)->position_ - m_pSourceMesh->get_vertex(65)->position_);

	cvDestroyWindow("video");
}

void GLMainWindow::from_kinect()
{
	m_tapeAVIMark = 2;	 //kinect时，m_tapeAVIMark标记为2

	while (m_pTargetMesh == NULL)		//提示读取目标网格图和纹理图
	{
		QMessageBox::StandardButton rb = QMessageBox::question(NULL, "Read Mesh",
			"Please read a mesh firstly!", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
		if (rb == QMessageBox::Yes)	//若选择Yes，读取网格
		{
			m_pGLViewer->ReadMesh();
		}

		if (m_pTargetMesh != NULL)
		{
			QMessageBox::StandardButton rb = QMessageBox::question(NULL, "Load Texture",
				"Please load a texture firstly!", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
			if (rb == QMessageBox::Yes)	//若选择Yes，载入纹理图
			{
				m_pGLViewer->LoadTexture();
			}
		}
	}
	
	kinect_process_standface();
	cv::Mat image;
	image.create(480, 640, CV_8UC3);

	
	//1、初始化NUI 
	HRESULT hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR);
	if (FAILED(hr))
	{
		cout << "NuiInitialize failed" << endl;
	}

	//2、定义事件句柄 
	//创建读取下一帧的信号事件句柄，控制KINECT是否可以开始读取下一帧数据
	HANDLE nextColorFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	HANDLE colorStreamHandle = NULL; //保存图像数据流的句柄，用以提取数据 

	//3、打开KINECT设备的彩色图信息通道，并用colorStreamHandle保存该流的句柄，以便于以后读取
	hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480,
		0, 2, nextColorFrameEvent, &colorStreamHandle);
	if (FAILED(hr))//判断是否提取正确 
	{
		cout << "Could not open color image stream video" << endl;
		NuiShutdown();
	
	}

	//---------------------------------------------------------------
	static int index = 1;
	char fname[128];
	if (m_pTargetMesh == NULL)
	{
		QMessageBox::warning(this, tr("Read Failed!!!"), tr("There is No Mesh!"));
		return;
	}

	Mesh3D *sourcemesh = new Mesh3D;	//copy源网格最初的网格数据
	vector<vector<Vec3f>> list_of_meshpoints_after_animation;	//存储target mesh每次变形后网格点的位置

	static int nImgWidth = 0;
	static int nImgHeight = 0;
	static int nImgSize = 0;
	bool bReset = true;

	nImgWidth = image.cols;
	nImgHeight = image.rows;



	char pcMdlPath[512];
	sprintf(pcMdlPath, "./data/model_map_2d_data.bin");
	FILE* fp = fopen(pcMdlPath, "rb");
	if (fp == NULL)
	{
		QMessageBox::warning(this, tr("Failed!!!"), tr("file is NULL!"));
		return;
	}
	fseek(fp, 0, SEEK_SET);
	fseek(fp, 0, SEEK_END);
	long len_bytes = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	unsigned char* pLoadData = (unsigned char*)malloc(len_bytes);
	fread(pLoadData, sizeof(unsigned char), len_bytes, fp);
	if (!EiInitialize(nImgWidth, nImgHeight, pLoadData, len_bytes) ||
		!EiInitialize_NewReso(nImgWidth, nImgHeight))
	{
		printf("initialize failed\r\n");
		free(pLoadData);
		exit(-1);
	}
	free(pLoadData);

	nImgSize = nImgWidth * nImgHeight;

	IplImage *pImg = cvCreateImage(cvSize(nImgWidth, nImgHeight), 8, 3);
	IplImage *outImg = cvCreateImage(cvSize(nImgWidth, nImgHeight), 8, 3);

	unsigned char * pRGBSrc = new unsigned char[nImgSize * 3 * sizeof(unsigned char)];
	unsigned char * pRGBDest = new unsigned char[nImgSize * 3 * sizeof(unsigned char)];

	cvWaitKey(1300);	//等待1.3s，用于准备（保持正脸，标准脸）



	//眼部特征点组纵坐标之差
	float left_g1 = 0;
	float left_g2 = 0;
	float left_g3 = 0;

	float right_g1 = 0;
	float right_g2 = 0;
	float right_g3 = 0;

	float mouse_1_ = 0;
	float mouse_2_ = 0;
	float mouse_3_ = 0;



	Transfer transformation;
	//------------------------------------------------------------------------------------


	//4、开始读取彩色图数据 
	while (1)
	{
		const NUI_IMAGE_FRAME * pImageFrame = NULL;

		//4.1、无限等待新的数据，等到后返回
		if (WaitForSingleObject(nextColorFrameEvent, INFINITE) == 0)
		{
			//4.2、从刚才打开数据流的流句柄中得到该帧数据，读取到的数据地址存于pImageFrame
			hr = NuiImageStreamGetNextFrame(colorStreamHandle, 0, &pImageFrame);
			if (FAILED(hr))
			{
				cout << "Could not get color image" << endl;
				NuiShutdown();
			}

			INuiFrameTexture * pTexture = pImageFrame->pFrameTexture;
			NUI_LOCKED_RECT LockedRect;

			//4.3、提取数据帧到LockedRect，它包括两个数据对象：pitch每行字节数，pBits第一个字节地址
			//并锁定数据，这样当我们读数据的时候，kinect就不会去修改它
			pTexture->LockRect(0, &LockedRect, NULL, 0);
			//4.4、确认获得的数据是否有效
			if (LockedRect.Pitch != 0)
			{
				//4.5、将数据转换为OpenCV的Mat格式
				for (int i = 0; i<image.rows; i++)
				{
					uchar *ptr = image.ptr<uchar>(i);  //第i行的指针

					//每个字节代表一个颜色信息，直接使用uchar
					uchar *pBuffer = (uchar*)(LockedRect.pBits) + i * LockedRect.Pitch;
					for (int j = 0; j<image.cols; j++)
					{
						ptr[3 * j] = pBuffer[4 * j];  //内部数据是4个字节，0-1-2是BGR，第4个现在未使用 
						ptr[3 * j + 1] = pBuffer[4 * j + 1];
						ptr[3 * j + 2] = pBuffer[4 * j + 2];
					}
				}

				m_pGLViewer->getmouthshut();

				pImg = &IplImage(image);

				if (!pImg) break;


				memcpy((unsigned char *)pRGBSrc, (unsigned char *)pImg->imageData, nImgSize * 3);
				memcpy((unsigned char *)outImg->imageData, (unsigned char *)pRGBSrc, nImgSize * 3);

				IplImage* pGrayImg = cvCreateImage(cvGetSize(pImg), 8, 1);

				cvCvtColor(pImg, pGrayImg, CV_BGR2GRAY);
				AAM_OUTPUT_STRUCT aam_ret = { 0 };
				aam_ret.nstate = -1;
				aam_ret.nstate = EiGetExpression((unsigned char*)pGrayImg->imageData, &aam_ret, bReset);

				if (aam_ret.nstate == 1)
				{
					for (int i = 0; i < aam_ret.n2DNum; ++i)  //aam_ret.n2DNum
					{
						cvDrawCircle(pImg, cvPoint(int(aam_ret.pKeyPoint2DOut[i].x + 0.5f), int(aam_ret.pKeyPoint2DOut[i].y + 0.5f)), 1, cvScalar(0, 255, 0), -1);

					}
					if (m_pSourceMesh == NULL)
					{
						m_pSourceMesh = new Mesh3D;
						getNewMeshVertex(aam_ret.pKeyPoint2DOut, m_pSourceMesh);

						left_group_1 = len(m_pSourceMesh->get_vertex(1)->position_ - m_pSourceMesh->get_vertex(7)->position_);
						left_group_2 = len(m_pSourceMesh->get_vertex(2)->position_ - m_pSourceMesh->get_vertex(6)->position_);
						left_group_3 = len(m_pSourceMesh->get_vertex(3)->position_ - m_pSourceMesh->get_vertex(5)->position_);

						right_group_1 = len(m_pSourceMesh->get_vertex(9)->position_ - m_pSourceMesh->get_vertex(15)->position_);
						right_group_2 = len(m_pSourceMesh->get_vertex(10)->position_ - m_pSourceMesh->get_vertex(14)->position_);
						right_group_3 = len(m_pSourceMesh->get_vertex(11)->position_ - m_pSourceMesh->get_vertex(13)->position_);
					}
					else
					{
						Mesh3D *mesh = new Mesh3D;
						getNewMeshVertex(aam_ret.pKeyPoint2DOut, mesh);

						//deformation transform
						vector<Vec3f> pList;
						pList = transformation.getTransformMesh(m_pSourceMesh, m_pTargetCopy, mesh);

						left_g1 = len(mesh->get_vertex(1)->position_ - mesh->get_vertex(7)->position_);
						left_g2 = len(mesh->get_vertex(2)->position_ - mesh->get_vertex(6)->position_);
						left_g3 = len(mesh->get_vertex(3)->position_ - mesh->get_vertex(5)->position_);

						right_g1 = len(mesh->get_vertex(9)->position_ - mesh->get_vertex(15)->position_);
						right_g2 = len(mesh->get_vertex(10)->position_ - mesh->get_vertex(14)->position_);
						right_g3 = len(mesh->get_vertex(11)->position_ - mesh->get_vertex(13)->position_);

						mouse_1_ = len(mesh->get_vertex(61)->position_ - mesh->get_vertex(67)->position_);
						mouse_2_ = len(mesh->get_vertex(62)->position_ - mesh->get_vertex(66)->position_);
						mouse_3_ = len(mesh->get_vertex(63)->position_ - mesh->get_vertex(65)->position_);


						float left_change_scale = left_g2 - left_group_2;
						float right_change_scale = right_g2 - right_group_2;

						float mouse_change = mouse_2_ - mouse_2;

						if (fabs(mouse_change)>2 * mouse_2)
						{
							m_pGLViewer->getmouthopen();

						}
						else
						{
							m_pGLViewer->getmouthshut();

						}

						//cout << mouse_change << "    " << mouse_2<<endl;


						//当左眼变小的幅度大于原来的0.19时，判定为闭眼（由于特征点检测不准）
						if (fabs(left_change_scale)>0.19*left_group_2 && left_change_scale<0)
						{
							pList[2][1] = (pList[0][1] + pList[4][1]) / 2;
							pList[6][1] = (pList[0][1] + pList[4][1]) / 2;

							pList[1][1] = (pList[0][1] + pList[2][1]) / 2;
							pList[7][1] = (pList[0][1] + pList[2][1]) / 2;

							pList[3][1] = (pList[2][1] + pList[4][1]) / 2;
							pList[5][1] = (pList[2][1] + pList[4][1]) / 2;
						}
						else	//左眼放大变形效果
						{
							float left_group_1_y = m_pSourceMesh->get_vertex(1)->position_[1] - m_pSourceMesh->get_vertex(7)->position_[1];
							float left_group_2_y = m_pSourceMesh->get_vertex(2)->position_[1] - m_pSourceMesh->get_vertex(6)->position_[1];
							float left_group_3_y = m_pSourceMesh->get_vertex(3)->position_[1] - m_pSourceMesh->get_vertex(5)->position_[1];

							float left_g1_y = mesh->get_vertex(1)->position_[1] - mesh->get_vertex(7)->position_[1];
							float left_g2_y = mesh->get_vertex(2)->position_[1] - mesh->get_vertex(6)->position_[1];
							float left_g3_y = mesh->get_vertex(3)->position_[1] - mesh->get_vertex(5)->position_[1];

							//适当放大左眼变形
							pList[1][1] += (left_g1_y - left_group_1_y);
							pList[2][1] += (left_g2_y - left_group_2_y);
							pList[3][1] += (left_g3_y - left_group_3_y);

							pList[5][1] -= (left_g3_y - left_group_3_y);
							pList[6][1] -= (left_g2_y - left_group_2_y);
							pList[7][1] -= (left_g1_y - left_group_1_y);
						}

						//当右眼变小的幅度大于原来的0.19时，判定为闭眼（由于特征点检测不准）
						if (fabs(right_change_scale)>0.19*right_group_2 && right_change_scale<0)
						{
							pList[10][1] = (pList[8][1] + pList[12][1]) / 2;
							pList[14][1] = (pList[8][1] + pList[12][1]) / 2;

							pList[9][1] = (pList[8][1] + pList[10][1]) / 2;
							pList[15][1] = (pList[8][1] + pList[10][1]) / 2;

							pList[11][1] = (pList[10][1] + pList[12][1]) / 2;
							pList[13][1] = (pList[10][1] + pList[12][1]) / 2;
						}
						else	//右眼放大变形效果
						{
							float right_group_1_y = m_pSourceMesh->get_vertex(9)->position_[1] - m_pSourceMesh->get_vertex(15)->position_[1];
							float right_group_2_y = m_pSourceMesh->get_vertex(10)->position_[1] - m_pSourceMesh->get_vertex(14)->position_[1];
							float right_group_3_y = m_pSourceMesh->get_vertex(11)->position_[1] - m_pSourceMesh->get_vertex(13)->position_[1];

							float right_g1_y = mesh->get_vertex(9)->position_[1] - mesh->get_vertex(15)->position_[1];
							float right_g2_y = mesh->get_vertex(10)->position_[1] - mesh->get_vertex(14)->position_[1];
							float right_g3_y = mesh->get_vertex(11)->position_[1] - mesh->get_vertex(13)->position_[1];

							//适当放大右眼变形
							pList[9][1] += (right_g1_y - right_group_1_y);
							pList[10][1] += (right_g2_y - right_group_2_y);
							pList[11][1] += (right_g3_y - right_group_3_y);

							pList[13][1] -= (right_g3_y - right_group_3_y);
							pList[14][1] -= (right_g2_y - right_group_2_y);
							pList[15][1] -= (right_g1_y - right_group_1_y);
						}

						/*--------------------------------Shake处理和极限变形的处理----------------------------------*/
						if (list_of_meshpoints_after_animation.size() != 0)
						{
							vector<Vec3f> temp;
							temp = list_of_meshpoints_after_animation[list_of_meshpoints_after_animation.size() - 1];
							double sum_length_move = 0;
							for (int i = 0; i < 87; i++)
							{
								Vec3f list_temp = pList[i];
								Vec3f temp_temp = temp[i];
								double shakemove = pow(list_temp.x() - temp_temp.x(), 2) + pow(list_temp.y() - temp_temp.y(), 2);
								//qDebug("my window handle = %f", shakemove);
								sum_length_move=sum_length_move + shakemove;
								if (shakemove < 0.0001)
									pList[i] = temp[i];
							}
							if (sum_length_move>10)
							{
								pList = temp;
							}
							temp.clear();
						}
						int num_of_vertices = m_pTargetMesh->num_of_vertex_list();
						for (size_t i = 0; i != num_of_vertices; i++)
						{
							m_pTargetMesh->get_vertex(i)->set_position(pList[i]);
						}

						list_of_meshpoints_after_animation.push_back(pList);	//存储Target变形后的网格点位置
						//				}

						m_pGLViewer->updateGL();
					}

					bReset = false;
				}
				else
				{
					int num_of_animation_frame = list_of_meshpoints_after_animation.size();
					if (num_of_animation_frame != 0)
					{
						int num_of_vertices = m_pTargetMesh->num_of_vertex_list();
						for (size_t i = 0; i != num_of_vertices; i++)		//当未检测到人脸（特征点）时，目标图像保持最后检测到的脸相对应的图像
						{
							m_pTargetMesh->get_vertex(i)->set_position(list_of_meshpoints_after_animation[num_of_animation_frame - 1][i]);
						}
					}
					bReset = true;
				}

				cvShowImage("original video", pImg);

				cvReleaseImage(&pGrayImg);

				char c = cvWaitKey(2);
				if (c == ' ')
				{
					break;
				}
				/*switch (c)
				{
				case 27:
					break;
				}*/
			}
			else
			{
				cout << "Buffer length of received texture is bogus\r\n" << endl;
			}

			//5、这帧已经处理完了，所以将其解锁
			pTexture->UnlockRect(0);
			//6、释放本帧数据，准备迎接下一帧 
			NuiImageStreamReleaseFrame(colorStreamHandle, pImageFrame);
		}
		if (cvWaitKey(20) == ' ')
			break;
	}
	////7、关闭NUI链接 
	delete[] pRGBSrc;
	delete[] pRGBDest;

	delete sourcemesh;

	//	cvReleaseImage(&pImg);
	cvReleaseImage(&outImg);

	EiDestroy_NewReso();
	EiDestroy();

	int vertex_number = m_pTargetMesh->num_of_vertex_list();
	for (size_t i = 0; i != vertex_number; i++)		//初始化目标网格点位置
	{
		m_pTargetMesh->get_vertex(i)->set_position(m_pTargetCopy->get_vertex(i)->position_);
	}
	m_pGLViewer->updateGL();
	delete m_pSourceMesh;
	m_pSourceMesh = NULL;
	NuiShutdown();
	cvDestroyWindow("original video");
	
}

void GLMainWindow::simulate()
{
	// Camera Load
	CvCapture *pCapture = cvCaptureFromCAM(0);
	QMessageBox::information(NULL, "First", "Please tape your friend's face, get him here. Please tap 'e', when you think this face would be ok.", QMessageBox::Yes, QMessageBox::Yes);
	getyourfriend_data(pCapture);
	QMessageBox::information(NULL, "Second", "I want to tape your face as a standard face. Please tap 'e', when you think this face are already.", QMessageBox::Yes, QMessageBox::Yes);
	getstandface(pCapture);
	
	m_pGLViewer->checkDrawPoint(FALSE);
	m_pGLViewer->checkDrawTex(TRUE);
	//facialAnimation(pCapture);
	simulate_face(pCapture);
}

void GLMainWindow::getyourfriend_data(CvCapture *pCapture)
{
	m_tapeAVIMark = 3;	 //simulate，m_tapeAVIMark标记为3
	if (pCapture == NULL)
	{
		QMessageBox::warning(this, tr("Failed!!!"), tr("Capture is NULL!"));
		return;
	}
	IplImage* pFrame = NULL;
	IplImage pFrame_copy;      //texture
	static int nImgWidth = 0;
	static int nImgHeight = 0;
	static int nImgSize = 0;
	bool bReset = true;

	nImgWidth = cvGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_WIDTH);
	nImgHeight = cvGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_HEIGHT);

	cvSetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_WIDTH, nImgWidth);
	cvSetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_HEIGHT, nImgHeight);

	char pcMdlPath[512];
	sprintf(pcMdlPath, "./data/model_map_2d_data.bin");
	FILE* fp = fopen(pcMdlPath, "rb");
	if (fp == NULL)
	{
		QMessageBox::warning(this, tr("Failed!!!"), tr("file is NULL!"));
		return;
	}
	fseek(fp, 0, SEEK_SET);
	fseek(fp, 0, SEEK_END);
	long len_bytes = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	unsigned char* pLoadData = (unsigned char*)malloc(len_bytes);
	fread(pLoadData, sizeof(unsigned char), len_bytes, fp);
	if (!EiInitialize(nImgWidth, nImgHeight, pLoadData, len_bytes) ||
		!EiInitialize_NewReso(nImgWidth, nImgHeight))
	{
		printf("initialize failed\r\n");
		free(pLoadData);
		if (pCapture)
			cvReleaseCapture(&pCapture);
		exit(-1);
	}
	free(pLoadData);
	AAM_OUTPUT_STRUCT aam_ret_temp;

	//显示视屏  
	while (1)
	{
		pFrame = cvQueryFrame(pCapture);
		IplImage* pGrayImg = cvCreateImage(cvGetSize(pFrame), 8, 1);
		cvCvtColor(pFrame, pGrayImg, CV_BGR2GRAY);
		AAM_OUTPUT_STRUCT aam_ret = { 0 };
		aam_ret.nstate = -1;
		aam_ret.nstate = EiGetExpression((unsigned char*)pGrayImg->imageData, &aam_ret, bReset);

		if (aam_ret.nstate == 1)
		{
			for (int i = 0; i < aam_ret.n2DNum; ++i)  //aam_ret.n2DNum
			{
				cvDrawCircle(pFrame, cvPoint(int(aam_ret.pKeyPoint2DOut[i].x + 0.5f), int(aam_ret.pKeyPoint2DOut[i].y + 0.5f)), 1, cvScalar(0, 255, 0), -1);
			}
		}
		if (!pFrame)break;
		cvShowImage("video", pFrame);
		char c = cvWaitKey(33);
		if (c == 27)break;
		if (c == 'e')
		{
			QMessageBox::StandardButton rb = QMessageBox::question(NULL, "Standard Face",
				"Do You Want to Use this picture as your face ?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

			if (rb == QMessageBox::Yes)	//若选择No，则重新选择标准脸
			{
				m_pSourceMesh = new Mesh3D;
				getNewMeshVertex(aam_ret.pKeyPoint2DOut, m_pSourceMesh);
				aam_ret_temp = aam_ret;
				break;
			}
			else
			{
				continue;
			}
		}
		
	}
	
	m_pGLViewer->ReadMesh_simulate();
	for (int i = 0; i < 100; i++)
	{
	m_pGLViewer->m_pMesh->get_vertex(i)->position_[0] = m_pSourceMesh->get_vertex(i)->position_[0];
	m_pGLViewer->m_pMesh->get_vertex(i)->position_[1] = m_pSourceMesh->get_vertex(i)->position_[1];
	}
	pFrame_copy = *cvQueryFrame(pCapture);
	m_pGLViewer->Simulate_tex(pFrame_copy, aam_ret_temp,x_88_2_100,y_88_2_100);
	cvDestroyWindow("video");
}

void GLMainWindow::kinect_process_standface()
{
	QMessageBox::information(NULL, "Standard face", "I want to tap your face as a standard face. Please tap 'e', when your face is ready.", QMessageBox::Yes, QMessageBox::Yes);
IplImage *colorImage = cvCreateImage(cvSize(640, 480), 8, 3);
IplImage *depthImage = cvCreateImage(cvSize(320, 240), 8, 3);


HANDLE colorEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
HANDLE depthEvent = CreateEvent(NULL, TRUE, FALSE, NULL);


HANDLE colorStreamHandle = NULL;
HANDLE depthStreamHandle = NULL;

HRESULT hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX );
if (hr != S_OK)
{
	cout << "NuiInitialize failed" << endl;

}

hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480, NULL, 4, colorEvent, &colorStreamHandle);
if (hr != S_OK)
{
	cout << "Open the color Stream failed" << endl;
	NuiShutdown();

}
hr = NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX, NUI_IMAGE_RESOLUTION_320x240, NULL, 2, depthEvent, &depthStreamHandle);
if (hr != S_OK)
{
	cout << "Open the depth Stream failed" << endl;
	NuiShutdown();

}

IplImage* pFrame = NULL;

static int nImgWidth = 0;
static int nImgHeight = 0;
static int nImgSize = 0;
bool bReset = true;

nImgWidth = 640;
nImgHeight = 480;
char pcMdlPath[512];
sprintf(pcMdlPath, "./data/model_map_2d_data.bin");
FILE* fp = fopen(pcMdlPath, "rb");
if (fp == NULL)
{
	QMessageBox::warning(this, tr("Failed!!!"), tr("file is NULL!"));
	return;
}
fseek(fp, 0, SEEK_SET);
fseek(fp, 0, SEEK_END);
long len_bytes = ftell(fp);
fseek(fp, 0, SEEK_SET);
unsigned char* pLoadData = (unsigned char*)malloc(len_bytes);
fread(pLoadData, sizeof(unsigned char), len_bytes, fp);
if (!EiInitialize(nImgWidth, nImgHeight, pLoadData, len_bytes) ||
	!EiInitialize_NewReso(nImgWidth, nImgHeight))
{
	printf("initialize failed\r\n");
	free(pLoadData);
	exit(-1);
}
free(pLoadData);
IplImage depthImage_temp ;

double eye_distance = 0;
int eye_row = 0;
int eye_col = 0;
while (1)
{
	
	if (WaitForSingleObject(colorEvent, 0) == 0)
		getColorImage(colorEvent, colorStreamHandle, colorImage);
	if (WaitForSingleObject(depthEvent, 0) == 0)
		getDepthImage(depthEvent, depthStreamHandle, depthImage);
	pFrame = colorImage;
	IplImage* pGrayImg = cvCreateImage(cvGetSize(pFrame), 8, 1);
	cvCvtColor(pFrame, pGrayImg, CV_BGR2GRAY);
	AAM_OUTPUT_STRUCT aam_ret = { 0 };
	aam_ret.nstate = -1;
	aam_ret.nstate = EiGetExpression((unsigned char*)pGrayImg->imageData, &aam_ret, bReset);

	if (aam_ret.nstate == 1)
	{
		for (int i = 0; i < aam_ret.n2DNum; ++i)  //aam_ret.n2DNum
		{
			cvDrawCircle(pFrame, cvPoint(int(aam_ret.pKeyPoint2DOut[i].x + 0.5f), int(aam_ret.pKeyPoint2DOut[i].y + 0.5f)), 1, cvScalar(0, 255, 0), -1);
		}
	}


	cvShowImage("colorImage", pFrame);
	depthImage_temp = *depthImage;
	if (aam_ret.nstate == 1)
	{
		eye_col = (int(aam_ret.pKeyPoint2DOut[0].x + 0.5f) + int(aam_ret.pKeyPoint2DOut[8].x + 0.5f)) / 4;
		eye_row = (int(aam_ret.pKeyPoint2DOut[0].y + 0.5f) + int(aam_ret.pKeyPoint2DOut[8].y + 0.5f)) / 4;

		//eye_distance = getDepthImage_depthdata(depthEvent, depthStreamHandle, &depthImage_temp, 160, 120);
		
		//cout << eye_distance << endl;
		//cout << eye_col <<endl ;
	}
	cvShowImage("depthImage", depthImage);
	
	char c = cvWaitKey(33);
	if (c == ' ')break;
	if (c == 'e')
	{
		QMessageBox::StandardButton rb = QMessageBox::question(NULL, "Standard Face",
			"Do You Want to Use this picture as a standard face ?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

		if (rb == QMessageBox::Yes)	//若选择No，则重新选择标准脸
		{
			m_pSourceMesh = new Mesh3D;
			getNewMeshVertex(aam_ret.pKeyPoint2DOut, m_pSourceMesh);
			break;
		}
		else
		{
			continue;
		}
	}
	
}
cvReleaseImage(&colorImage);
cvReleaseImage(&depthImage);
NuiShutdown();

left_group_1 = len(m_pSourceMesh->get_vertex(1)->position_ - m_pSourceMesh->get_vertex(7)->position_);
left_group_2 = len(m_pSourceMesh->get_vertex(2)->position_ - m_pSourceMesh->get_vertex(6)->position_);
left_group_3 = len(m_pSourceMesh->get_vertex(3)->position_ - m_pSourceMesh->get_vertex(5)->position_);

right_group_1 = len(m_pSourceMesh->get_vertex(9)->position_ - m_pSourceMesh->get_vertex(15)->position_);
right_group_2 = len(m_pSourceMesh->get_vertex(10)->position_ - m_pSourceMesh->get_vertex(14)->position_);
right_group_3 = len(m_pSourceMesh->get_vertex(11)->position_ - m_pSourceMesh->get_vertex(13)->position_);


mouse_1 = len(m_pSourceMesh->get_vertex(61)->position_ - m_pSourceMesh->get_vertex(67)->position_);
mouse_2 = len(m_pSourceMesh->get_vertex(62)->position_ - m_pSourceMesh->get_vertex(66)->position_);
mouse_3 = len(m_pSourceMesh->get_vertex(63)->position_ - m_pSourceMesh->get_vertex(65)->position_);
cvDestroyWindow("colorImage");
cvDestroyWindow("depthImage");
}

void GLMainWindow::Choose()
{
	switch (combobox_shape_->currentIndex())
	{
	case 0:
		m_pGLViewer->choose_shape(0);
		m_pGLViewer->checkDrawTex(TRUE);
		break;
	case 1:
		m_pGLViewer->choose_shape(1);
		m_pGLViewer->checkDrawTex(TRUE);
		break;
	default:
		break;
	}

}

void GLMainWindow::getColorImage(HANDLE &colorEvent, HANDLE &colorStreamHandle, IplImage *colorImage)
{
	const NUI_IMAGE_FRAME *colorFrame = NULL;

	NuiImageStreamGetNextFrame(colorStreamHandle, 0, &colorFrame);
	INuiFrameTexture *pTexture = colorFrame->pFrameTexture;

	NUI_LOCKED_RECT LockedRect;
	pTexture->LockRect(0, &LockedRect, NULL, 0);

	if (LockedRect.Pitch != 0)
	{
		//cvZero(colorImage); 
		for (int i = 0; i<480; i++)
		{
			uchar* ptr = (uchar*)(colorImage->imageData + i*colorImage->widthStep);
			BYTE * pBuffer = (BYTE*)(LockedRect.pBits) + i*LockedRect.Pitch;//每个字节代表一个颜色信息，直接使用BYTE   
			for (int j = 0; j<640; j++)
			{
				ptr[3 * j] = pBuffer[4 * j];//内部数据是4个字节，0-1-2是BGR，第4个现在未使用   
				ptr[3 * j + 1] = pBuffer[4 * j + 1];
				ptr[3 * j + 2] = pBuffer[4 * j + 2];
			}
		}

		//cvShowImage("colorImage", colorImage);//显示图像 
		//cvWaitKey(1); 

	}
	else
	{
		cout << "捕捉视频帧时发生错误" << endl;
	}

	NuiImageStreamReleaseFrame(colorStreamHandle, colorFrame);
}

void GLMainWindow::getDepthImage(HANDLE &depthEvent, HANDLE &depthStreamHandle, IplImage *depthImage)
{
	const NUI_IMAGE_FRAME *depthFrame = NULL;

	NuiImageStreamGetNextFrame(depthStreamHandle, 0, &depthFrame);
	INuiFrameTexture *pTexture = depthFrame->pFrameTexture;

	NUI_LOCKED_RECT LockedRect;
	pTexture->LockRect(0, &LockedRect, NULL, 0);

	RGBQUAD q;
	//q.rgbBlue = q.rgbGreen = q.rgbRed = 0; 
	//cvZero(depthImage); 
	if (LockedRect.Pitch != 0)
	{
		for (int i = 0; i<240; i++)
		{
			uchar *ptr = (uchar*)(depthImage->imageData + i*depthImage->widthStep);
			BYTE *buffer = (BYTE*)(LockedRect.pBits) + i*LockedRect.Pitch;
			USHORT *bufferRun = (USHORT*)buffer;
			for (int j = 0; j<320; j++)
			{
				int player = bufferRun[j] & 7;
				int data = (bufferRun[j] & 0xfff8) >> 3;

				uchar imageData = 255 - (uchar)(256 * data / 0x0fff);
				q.rgbBlue = q.rgbGreen = q.rgbRed = 0;

				switch (player)
				{
				case 0:
					q.rgbRed = imageData / 2;
					q.rgbBlue = imageData / 2;
					q.rgbGreen = imageData / 2;
					break;
				case 1:
					q.rgbRed = imageData;
					break;
				case 2:
					q.rgbGreen = imageData;
					break;
				case 3:
					q.rgbRed = imageData / 4;
					q.rgbGreen = q.rgbRed * 4;  //这里利用乘的方法，而不用原来的方法可以避免不整除的情况 
					q.rgbBlue = q.rgbRed * 4;  //可以在后面的getTheContour()中配合使用，避免遗漏一些情况 
					break;
				case 4:
					q.rgbBlue = imageData / 4;
					q.rgbRed = q.rgbBlue * 4;
					q.rgbGreen = q.rgbBlue * 4;
					break;
				case 5:
					q.rgbGreen = imageData / 4;
					q.rgbRed = q.rgbGreen * 4;
					q.rgbBlue = q.rgbGreen * 4;
					break;
				case 6:
					q.rgbRed = imageData / 2;
					q.rgbGreen = imageData / 2;
					q.rgbBlue = q.rgbGreen * 2;
					break;
				case 7:
					q.rgbRed = 255 - (imageData / 2);
					q.rgbGreen = 255 - (imageData / 2);
					q.rgbBlue = 255 - (imageData / 2);
				}

				ptr[3 * j] = q.rgbBlue;
				ptr[3 * j + 1] = q.rgbGreen;
				ptr[3 * j + 2] = q.rgbRed;
			}
		}
	}
	else
	{
		cout << "捕捉深度图像出现错误" << endl;
	}

	NuiImageStreamReleaseFrame(depthStreamHandle, depthFrame);

}

void GLMainWindow::getSkeletonImage(HANDLE &skeletonEvent, IplImage *skeletonImage, IplImage *colorImage, IplImage *depthImage)
{
	

	NUI_SKELETON_FRAME skeletonFrame;
	bool bFoundSkeleton = false;

	if (NuiSkeletonGetNextFrame(0, &skeletonFrame) == S_OK)
	{
		for (int i = 0; i < NUI_SKELETON_COUNT; i++)
		{
			if (skeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED)
			{
				bFoundSkeleton = true;
				//cout << "ok" << endl; 
				break;
			}
		}
	}
	else
	{
		cout << "没有找到合适的骨骼帧" << endl;
		return;
	}

	if (!bFoundSkeleton)
	{
		return;
	}

	NuiTransformSmooth(&skeletonFrame, NULL);//平滑骨骼帧,消除抖动   

	cvZero(skeletonImage);
	for (int i = 0; i < NUI_SKELETON_COUNT; i++)
	{
		if (skeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED &&
			skeletonFrame.SkeletonData[i].eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_SHOULDER_CENTER] != NUI_SKELETON_POSITION_NOT_TRACKED)
		{
			float fx, fy;

			for (int j = 0; j < NUI_SKELETON_POSITION_COUNT; j++)//所有的坐标转化为深度图的坐标   
			{
				NuiTransformSkeletonToDepthImage(skeletonFrame.SkeletonData[i].SkeletonPositions[j], &fx, &fy);
				skeletonPoint[i][j].x = (int)(fx * 320 + 0.5f);
				skeletonPoint[i][j].y = (int)(fy * 240 + 0.5f);
			}

			for (int j = 0; j<NUI_SKELETON_POSITION_COUNT; j++)
			{
				if (skeletonFrame.SkeletonData[i].eSkeletonPositionTrackingState[j] != NUI_SKELETON_POSITION_NOT_TRACKED)//跟踪点一用有三种状态：1没有被跟踪到，2跟踪到，3根据跟踪到的估计到   
				{
					LONG colorx, colory;
					NuiImageGetColorPixelCoordinatesFromDepthPixel(NUI_IMAGE_RESOLUTION_640x480, 0,
						skeletonPoint[i][j].x, skeletonPoint[i][j].y, 0, &colorx, &colory);
					colorPoint[i][j].x = int(colorx); colorPoint[i][j].y = int(colory);//存储坐标点 
					cvCircle(colorImage, colorPoint[i][j], 4, cvScalar(0, 255, 255), -1, 8, 0);
					//cvCircle(depthImage, skeletonPoint[i][j], 3, cvScalar(0, 255, 255), -1, 8, 0); 
					cvCircle(skeletonImage, skeletonPoint[i][j], 3, cvScalar(0, 255, 255), -1, 8, 0);

					tracked[i] = TRUE;
				}
			}

			drawSkeleton(colorImage, colorPoint[i], i);
			//drawSkeleton(depthImage, skeletonPoint[i], i); 
			drawSkeleton(skeletonImage, skeletonPoint[i], i);
		}
	}

	//cvShowImage("skeletonImage", skeletonImage);   
	//cvShowImage("skeletsdfonImage", colorImage);   
	//cvWaitKey(1);   
}

void GLMainWindow::drawSkeleton(IplImage *image, CvPoint pointSet[], int witchone)
{
	CvScalar color;
	switch (witchone)//跟踪不同的人显示不同的颜色 
	{
	case 0:
		color = cvScalar(255);
		break;
	case 1:
		color = cvScalar(0, 255);
		break;
	case 2:
		color = cvScalar(0, 0, 255);
		break;
	case 3:
		color = cvScalar(255, 255, 0);
		break;
	case 4:
		color = cvScalar(255, 0, 255);
		break;
	case 5:
		color = cvScalar(0, 255, 255);
		break;
	}

	if ((pointSet[NUI_SKELETON_POSITION_HEAD].x != 0 || pointSet[NUI_SKELETON_POSITION_HEAD].y != 0) &&
		(pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].x != 0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].y != 0))
		cvLine(image, pointSet[NUI_SKELETON_POSITION_HEAD], pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER], color, 2);
	if ((pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].x != 0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].y != 0) &&
		(pointSet[NUI_SKELETON_POSITION_SPINE].x != 0 || pointSet[NUI_SKELETON_POSITION_SPINE].y != 0))
		cvLine(image, pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER], pointSet[NUI_SKELETON_POSITION_SPINE], color, 2);
	if ((pointSet[NUI_SKELETON_POSITION_SPINE].x != 0 || pointSet[NUI_SKELETON_POSITION_SPINE].y != 0) &&
		(pointSet[NUI_SKELETON_POSITION_HIP_CENTER].x != 0 || pointSet[NUI_SKELETON_POSITION_HIP_CENTER].y != 0))
		cvLine(image, pointSet[NUI_SKELETON_POSITION_SPINE], pointSet[NUI_SKELETON_POSITION_HIP_CENTER], color, 2);

	//左上肢 
	if ((pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].x != 0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].y != 0) &&
		(pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT].y != 0))
		cvLine(image, pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER], pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT], color, 2);
	if ((pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT].y != 0) &&
		(pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT].y != 0))
		cvLine(image, pointSet[NUI_SKELETON_POSITION_SHOULDER_LEFT], pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT], color, 2);
	if ((pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT].y != 0) &&
		(pointSet[NUI_SKELETON_POSITION_WRIST_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_WRIST_LEFT].y != 0))
		cvLine(image, pointSet[NUI_SKELETON_POSITION_ELBOW_LEFT], pointSet[NUI_SKELETON_POSITION_WRIST_LEFT], color, 2);
	if ((pointSet[NUI_SKELETON_POSITION_WRIST_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_WRIST_LEFT].y != 0) &&
		(pointSet[NUI_SKELETON_POSITION_HAND_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_HAND_LEFT].y != 0))
		cvLine(image, pointSet[NUI_SKELETON_POSITION_WRIST_LEFT], pointSet[NUI_SKELETON_POSITION_HAND_LEFT], color, 2);

	//右上肢 
	if ((pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].x != 0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER].y != 0) &&
		(pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT].y != 0))
		cvLine(image, pointSet[NUI_SKELETON_POSITION_SHOULDER_CENTER], pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT], color, 2);
	if ((pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT].y != 0) &&
		(pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT].y != 0))
		cvLine(image, pointSet[NUI_SKELETON_POSITION_SHOULDER_RIGHT], pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT], color, 2);
	if ((pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT].y != 0) &&
		(pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT].y != 0))
		cvLine(image, pointSet[NUI_SKELETON_POSITION_ELBOW_RIGHT], pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT], color, 2);
	if ((pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT].y != 0) &&
		(pointSet[NUI_SKELETON_POSITION_HAND_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_HAND_RIGHT].y != 0))
		cvLine(image, pointSet[NUI_SKELETON_POSITION_WRIST_RIGHT], pointSet[NUI_SKELETON_POSITION_HAND_RIGHT], color, 2);

	//左下肢 
	if ((pointSet[NUI_SKELETON_POSITION_HIP_CENTER].x != 0 || pointSet[NUI_SKELETON_POSITION_HIP_CENTER].y != 0) &&
		(pointSet[NUI_SKELETON_POSITION_HIP_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_HIP_LEFT].y != 0))
		cvLine(image, pointSet[NUI_SKELETON_POSITION_HIP_CENTER], pointSet[NUI_SKELETON_POSITION_HIP_LEFT], color, 2);
	if ((pointSet[NUI_SKELETON_POSITION_HIP_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_HIP_LEFT].y != 0) &&
		(pointSet[NUI_SKELETON_POSITION_KNEE_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_KNEE_LEFT].y != 0))
		cvLine(image, pointSet[NUI_SKELETON_POSITION_HIP_LEFT], pointSet[NUI_SKELETON_POSITION_KNEE_LEFT], color, 2);
	if ((pointSet[NUI_SKELETON_POSITION_KNEE_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_KNEE_LEFT].y != 0) &&
		(pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT].y != 0))
		cvLine(image, pointSet[NUI_SKELETON_POSITION_KNEE_LEFT], pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT], color, 2);
	if ((pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT].y != 0) &&
		(pointSet[NUI_SKELETON_POSITION_FOOT_LEFT].x != 0 || pointSet[NUI_SKELETON_POSITION_FOOT_LEFT].y != 0))
		cvLine(image, pointSet[NUI_SKELETON_POSITION_ANKLE_LEFT], pointSet[NUI_SKELETON_POSITION_FOOT_LEFT], color, 2);

	//右下肢 
	if ((pointSet[NUI_SKELETON_POSITION_HIP_CENTER].x != 0 || pointSet[NUI_SKELETON_POSITION_HIP_CENTER].y != 0) &&
		(pointSet[NUI_SKELETON_POSITION_HIP_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_HIP_RIGHT].y != 0))
		cvLine(image, pointSet[NUI_SKELETON_POSITION_HIP_CENTER], pointSet[NUI_SKELETON_POSITION_HIP_RIGHT], color, 2);
	if ((pointSet[NUI_SKELETON_POSITION_HIP_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_HIP_RIGHT].y != 0) &&
		(pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT].y != 0))
		cvLine(image, pointSet[NUI_SKELETON_POSITION_HIP_RIGHT], pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT], color, 2);
	if ((pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT].y != 0) &&
		(pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT].y != 0))
		cvLine(image, pointSet[NUI_SKELETON_POSITION_KNEE_RIGHT], pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT], color, 2);
	if ((pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT].y != 0) &&
		(pointSet[NUI_SKELETON_POSITION_FOOT_RIGHT].x != 0 || pointSet[NUI_SKELETON_POSITION_FOOT_RIGHT].y != 0))
		cvLine(image, pointSet[NUI_SKELETON_POSITION_ANKLE_RIGHT], pointSet[NUI_SKELETON_POSITION_FOOT_RIGHT], color, 2);
}

void GLMainWindow::getTheContour(IplImage *image, int whichone, IplImage *mask)//根据给定的深度数据的关系（在getDepthImage()中的）确定不同的跟踪目标 
{
	for (int i = 0; i<240; i++)
	{
		uchar *ptr = (uchar*)(image->imageData + i*image->widthStep);
		uchar *ptrmask = (uchar*)(mask->imageData + i*mask->widthStep);
		for (int j = 0; j<320; j++)
		{
			if (ptr[3 * j] == 0 && ptr[3 * j + 1] == 0 && ptr[3 * j + 2] == 0)//都为0的时候予以忽略 
			{
				ptrmask[3 * j] = ptrmask[3 * j + 1] = ptrmask[3 * j + 2] = 0;
			}
			else if (ptr[3 * j] == 0 && ptr[3 * j + 1] == 0 && ptr[3 * j + 2] != 0)//ID为1的时候，显示绿色 
			{
				ptrmask[3 * j] = 0;
				ptrmask[3 * j + 1] = 255;
				ptrmask[3 * j + 2] = 0;
			}
			else if (ptr[3 * j] == 0 && ptr[3 * j + 1] != 0 && ptr[3 * j + 2] == 0)//ID为2的时候，显示红色 
			{
				ptrmask[3 * j] = 0;
				ptrmask[3 * j + 1] = 0;
				ptrmask[3 * j + 2] = 255;
			}
			else if (ptr[3 * j] == ptr[3 * j + 1] && ptr[3 * j] == 4 * ptr[3 * j + 2])//ID为3的时候 
			{
				ptrmask[3 * j] = 255;
				ptrmask[3 * j + 1] = 255;
				ptrmask[3 * j + 2] = 0;
			}
			else if (4 * ptr[3 * j] == ptr[3 * j + 1] && ptr[3 * j + 1] == ptr[3 * j + 2])//ID为4的时候 
			{
				ptrmask[3 * j] = 255;
				ptrmask[3 * j + 1] = 0;
				ptrmask[3 * j + 2] = 255;
			}
			else if (ptr[3 * j] == 4 * ptr[3 * j + 1] && ptr[3 * j] == ptr[3 * j + 2])//ID为5的时候 
			{
				ptrmask[3 * j] = 0;
				ptrmask[3 * j + 1] = 255;
				ptrmask[3 * j + 2] = 255;
			}
			else if (ptr[3 * j] == 2 * ptr[3 * j + 1] && ptr[3 * j + 1] == ptr[3 * j + 2])//ID为6的时候 
			{
				ptrmask[3 * j] = 255;
				ptrmask[3 * j + 1] = 255;
				ptrmask[3 * j + 2] = 255;
			}
			else if (ptr[3 * j] == ptr[3 * j + 1] && ptr[3 * j] == ptr[3 * j + 2])//ID为7的时候或者ID为0的时候，显示蓝色 
			{
				ptrmask[3 * j] = 255;
				ptrmask[3 * j + 1] = 0;
				ptrmask[3 * j + 2] = 0;
			}
			else
			{
				cout << "如果输出这段代码，说明有遗漏的情况，请查询getTheContour函数" << endl;
			}
		}
	}
}

int GLMainWindow::getDepthImage_depthdata(HANDLE &depthEvent, HANDLE &depthStreamHandle, IplImage *depthImage, int row, int col)
{
	const NUI_IMAGE_FRAME *depthFrame = NULL;

	NuiImageStreamGetNextFrame(depthStreamHandle, 0, &depthFrame);
	INuiFrameTexture *pTexture = depthFrame->pFrameTexture;

	NUI_LOCKED_RECT LockedRect;
	pTexture->LockRect(0, &LockedRect, NULL, 0);

	RGBQUAD q;

	if (LockedRect.Pitch != 0)
	{
		int i = row;
		int j = col;
			uchar *ptr = (uchar*)(depthImage->imageData + i*depthImage->widthStep);
			BYTE *buffer = (BYTE*)(LockedRect.pBits) + i*LockedRect.Pitch;
			USHORT *bufferRun = (USHORT*)buffer;
			
				int player = bufferRun[j] & 7;
				int data = (bufferRun[j] & 0xfff8) >> 3;

				uchar imageData = 255 - (uchar)(256 * data / 0x0fff);
				NuiImageStreamReleaseFrame(depthStreamHandle, depthFrame);
				return int(imageData);
		
	}
	else
	{
		cout << "捕捉深度图像出现错误" << endl;
		NuiImageStreamReleaseFrame(depthStreamHandle, depthFrame);
		return -1;
	}
}

void GLMainWindow::simulate_face(CvCapture *pCapture)
{
	QMessageBox::information(NULL, "Information", "You can tap 'Spacebar'to stop this animation. ", QMessageBox::Yes, QMessageBox::Yes);
	if (pCapture == NULL)
	{
		QMessageBox::warning(this, tr("Failed!!!"), tr("Capture is NULL!"));
		return;
	}

	Mesh3D *sourcemesh = new Mesh3D;	//copy源网格最初的网格数据
	vector<vector<Vec3f>> list_of_meshpoints_after_animation;	//存储target mesh每次变形后网格点的位置

	static int nImgWidth = 0;
	static int nImgHeight = 0;
	static int nImgSize = 0;
	bool bReset = true;

	nImgWidth = cvGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_WIDTH);
	nImgHeight = cvGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_HEIGHT);
	cvSetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_WIDTH, nImgWidth);
	cvSetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_HEIGHT, nImgHeight);
	char pcMdlPath[512];
	sprintf(pcMdlPath, "./data/model_map_2d_data.bin");
	FILE* fp = fopen(pcMdlPath, "rb");
	if (fp == NULL)
	{
		QMessageBox::warning(this, tr("Failed!!!"), tr("file is NULL!"));
		return;
	}
	fseek(fp, 0, SEEK_SET);
	fseek(fp, 0, SEEK_END);
	long len_bytes = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	unsigned char* pLoadData = (unsigned char*)malloc(len_bytes);
	fread(pLoadData, sizeof(unsigned char), len_bytes, fp);
	if (!EiInitialize(nImgWidth, nImgHeight, pLoadData, len_bytes) ||
		!EiInitialize_NewReso(nImgWidth, nImgHeight))
	{
		printf("initialize failed\r\n");
		free(pLoadData);
		if (pCapture)
			cvReleaseCapture(&pCapture);
		exit(-1);
	}
	free(pLoadData);
	nImgSize = nImgWidth * nImgHeight;
	IplImage *pImg = cvCreateImage(cvSize(nImgWidth, nImgHeight), 8, 3);
	IplImage *outImg = cvCreateImage(cvSize(nImgWidth, nImgHeight), 8, 3);
	cvWaitKey(1300);	//等待1.3s，用于准备（保持正脸，标准脸）

	//眼部特征点组纵坐标之差
	float left_g1 = 0;
	float left_g2 = 0;
	float left_g3 = 0;

	float right_g1 = 0;
	float right_g2 = 0;
	float right_g3 = 0;

	float mouse_1_ = 0;
	float mouse_2_ = 0;
	float mouse_3_ = 0;



	Transfer transformation;

	for (;;)	// 实时镜头视频处理 //
	{

		m_pGLViewer->getmouthshut();
		pImg = cvQueryFrame(pCapture);
		if (!pImg) break;
		IplImage* pGrayImg = cvCreateImage(cvGetSize(pImg), 8, 1);
		cvCvtColor(pImg, pGrayImg, CV_BGR2GRAY);
		AAM_OUTPUT_STRUCT aam_ret = { 0 };
		aam_ret.nstate = -1;
		aam_ret.nstate = EiGetExpression((unsigned char*)pGrayImg->imageData, &aam_ret, bReset);

		if (aam_ret.nstate == 1)
		{
			for (int i = 0; i < aam_ret.n2DNum; ++i)  //aam_ret.n2DNum
			{
				cvDrawCircle(pImg, cvPoint(int(aam_ret.pKeyPoint2DOut[i].x + 0.5f), int(aam_ret.pKeyPoint2DOut[i].y + 0.5f)), 1, cvScalar(0, 255, 0), -1);

			}
			
				Mesh3D *mesh = new Mesh3D;
				getNewMeshVertex(aam_ret.pKeyPoint2DOut, mesh);

				//deformation transform
				vector<Vec3f> pList;
				pList = transformation.getTransformMesh(m_pSourceMesh, m_pTargetCopy, mesh);

				left_g1 = len(mesh->get_vertex(1)->position_ - mesh->get_vertex(7)->position_);
				left_g2 = len(mesh->get_vertex(2)->position_ - mesh->get_vertex(6)->position_);
				left_g3 = len(mesh->get_vertex(3)->position_ - mesh->get_vertex(5)->position_);

				right_g1 = len(mesh->get_vertex(9)->position_ - mesh->get_vertex(15)->position_);
				right_g2 = len(mesh->get_vertex(10)->position_ - mesh->get_vertex(14)->position_);
				right_g3 = len(mesh->get_vertex(11)->position_ - mesh->get_vertex(13)->position_);

				mouse_1_ = len(mesh->get_vertex(61)->position_ - mesh->get_vertex(67)->position_);
				mouse_2_ = len(mesh->get_vertex(62)->position_ - mesh->get_vertex(66)->position_);
				mouse_3_ = len(mesh->get_vertex(63)->position_ - mesh->get_vertex(65)->position_);
				float left_change_scale = left_g2 - left_group_2;
				float right_change_scale = right_g2 - right_group_2;
				float mouse_change = mouse_2_ - mouse_2;
				if (fabs(mouse_change)>2 * mouse_2)
				{
					m_pGLViewer->getmouthopen();
				}
				else
				{
					m_pGLViewer->getmouthshut();
				}
				//当左眼变小的幅度大于原来的0.19时，判定为闭眼（由于特征点检测不准）
				if (fabs(left_change_scale)>0.19*left_group_2 && left_change_scale<0)
				{
					pList[2][1] = (pList[0][1] + pList[4][1]) / 2;
					pList[6][1] = (pList[0][1] + pList[4][1]) / 2;

					pList[1][1] = (pList[0][1] + pList[2][1]) / 2;
					pList[7][1] = (pList[0][1] + pList[2][1]) / 2;

					pList[3][1] = (pList[2][1] + pList[4][1]) / 2;
					pList[5][1] = (pList[2][1] + pList[4][1]) / 2;
				}
				else	//左眼放大变形效果
				{
					float left_group_1_y = m_pSourceMesh->get_vertex(1)->position_[1] - m_pSourceMesh->get_vertex(7)->position_[1];
					float left_group_2_y = m_pSourceMesh->get_vertex(2)->position_[1] - m_pSourceMesh->get_vertex(6)->position_[1];
					float left_group_3_y = m_pSourceMesh->get_vertex(3)->position_[1] - m_pSourceMesh->get_vertex(5)->position_[1];

					float left_g1_y = mesh->get_vertex(1)->position_[1] - mesh->get_vertex(7)->position_[1];
					float left_g2_y = mesh->get_vertex(2)->position_[1] - mesh->get_vertex(6)->position_[1];
					float left_g3_y = mesh->get_vertex(3)->position_[1] - mesh->get_vertex(5)->position_[1];

					//适当放大左眼变形
					pList[1][1] += (left_g1_y - left_group_1_y);
					pList[2][1] += (left_g2_y - left_group_2_y);
					pList[3][1] += (left_g3_y - left_group_3_y);

					pList[5][1] -= (left_g3_y - left_group_3_y);
					pList[6][1] -= (left_g2_y - left_group_2_y);
					pList[7][1] -= (left_g1_y - left_group_1_y);
				}

				//当右眼变小的幅度大于原来的0.19时，判定为闭眼（由于特征点检测不准）
				if (fabs(right_change_scale)>0.19*right_group_2 && right_change_scale<0)
				{
					pList[10][1] = (pList[8][1] + pList[12][1]) / 2;
					pList[14][1] = (pList[8][1] + pList[12][1]) / 2;

					pList[9][1] = (pList[8][1] + pList[10][1]) / 2;
					pList[15][1] = (pList[8][1] + pList[10][1]) / 2;

					pList[11][1] = (pList[10][1] + pList[12][1]) / 2;
					pList[13][1] = (pList[10][1] + pList[12][1]) / 2;
				}
				else	//右眼放大变形效果
				{
					float right_group_1_y = m_pSourceMesh->get_vertex(9)->position_[1] - m_pSourceMesh->get_vertex(15)->position_[1];
					float right_group_2_y = m_pSourceMesh->get_vertex(10)->position_[1] - m_pSourceMesh->get_vertex(14)->position_[1];
					float right_group_3_y = m_pSourceMesh->get_vertex(11)->position_[1] - m_pSourceMesh->get_vertex(13)->position_[1];

					float right_g1_y = mesh->get_vertex(9)->position_[1] - mesh->get_vertex(15)->position_[1];
					float right_g2_y = mesh->get_vertex(10)->position_[1] - mesh->get_vertex(14)->position_[1];
					float right_g3_y = mesh->get_vertex(11)->position_[1] - mesh->get_vertex(13)->position_[1];

					//适当放大右眼变形
					pList[9][1] += (right_g1_y - right_group_1_y);
					pList[10][1] += (right_g2_y - right_group_2_y);
					pList[11][1] += (right_g3_y - right_group_3_y);

					pList[13][1] -= (right_g3_y - right_group_3_y);
					pList[14][1] -= (right_g2_y - right_group_2_y);
					pList[15][1] -= (right_g1_y - right_group_1_y);
				}
			

				/*-------------------------------------------Shape And Limit process----------------------------------------------*/
				if (list_of_meshpoints_after_animation.size() != 0)
				{
					vector<Vec3f> temp;
					temp = list_of_meshpoints_after_animation[list_of_meshpoints_after_animation.size() - 1];
					double sum_length_move = 0;
					for (int i = 0; i < 87; i++)
					{
						Vec3f list_temp = pList[i];
						Vec3f temp_temp = temp[i];
						double shakemove = pow(list_temp.x() - temp_temp.x(), 2) + pow(list_temp.y() - temp_temp.y(), 2);
						//qDebug("my window handle = %f", shakemove);
						sum_length_move = sum_length_move + shakemove;
						if (shakemove < 0.0001)
							pList[i] = temp[i];
					}
					if (sum_length_move>10)
					{
						pList = temp;
					}
					temp.clear();
				}



				int num_of_vertices = m_pTargetMesh->num_of_vertex_list();
				for (size_t i = 0; i != num_of_vertices; i++)
				{
					m_pTargetMesh->get_vertex(i)->set_position(pList[i]);
				}

				list_of_meshpoints_after_animation.push_back(pList);	//存储Target变形后的网格点位置
				

				m_pGLViewer->updateGL();
			bReset = false;
		}
		else
		{
			int num_of_animation_frame = list_of_meshpoints_after_animation.size();
			if (num_of_animation_frame != 0)
			{
				int num_of_vertices = m_pTargetMesh->num_of_vertex_list();
				for (size_t i = 0; i != num_of_vertices; i++)		//当未检测到人脸（特征点）时，目标图像保持最后检测到的脸相对应的图像
				{
					m_pTargetMesh->get_vertex(i)->set_position(list_of_meshpoints_after_animation[num_of_animation_frame - 1][i]);
				}
			}

			bReset = true;
		}

		cvShowImage("original video", pImg);
		cvReleaseImage(&pGrayImg);
		char c = cvWaitKey(2);
		switch (c)
		{
		case 27:
			break;
		}
		if (char(c) == ' ')
		{
			break;
		}
	}

	cvReleaseImage(&outImg);
	EiDestroy_NewReso();
	EiDestroy();
	int vertex_number = m_pTargetMesh->num_of_vertex_list();
	for (size_t i = 0; i != vertex_number; i++)		//初始化目标网格点位置
	{
		m_pTargetMesh->get_vertex(i)->set_position(m_pTargetCopy->get_vertex(i)->position_);
	}
	m_pGLViewer->updateGL();

	delete m_pSourceMesh;
	m_pSourceMesh = NULL;
	cvDestroyWindow("original video");
}