#pragma once
#include <vector>
#include "HE_mesh/Mesh3D.h"
#include "eigen/Eigen/Eigen"
#include "eigen/Eigen/SparseCore"
#include <eigen/Eigen/SparseLU>

using namespace Eigen;
using namespace std;

class Transfer
{
private:
	SparseMatrix<double> A;		//稀疏系数矩阵A（Ax = b）
	SparseMatrix<double> B;		//用于构造稀疏矩阵A, B'B = A
	SparseLU<SparseMatrix<double>>   solver;
	Mesh3D *m_pMesh_template;	//三角网格模板

// private:
// 	vector<Vec3f> m_RTvertices;	//用于存储三个点，求出一仿射变换（旋转R+平移T）（二维）

public:
	vector<Vec3f> list_of_meshpoints_after_animation;	//存储TargetMesh Animation后的网格点

public:
	Transfer(void);
	virtual ~Transfer(void);

public:			
	void			getRverseV(Mesh3D *mesh,SparseMatrix<double> &V);	//根据网格点获得相应矩阵
	void			getRTdeformation(Mesh3D *mesh,vector<Vec3f> pList,MatrixXd &R,VectorXd &d);	//获得变形后的刚性变换（旋转R+平移d）
	MatrixXd		getSDeformationMx(Mesh3D *source,Mesh3D *deform);	//获得网格变形矩阵
	vector<Vec3f>	getTransformMesh(Mesh3D *source,Mesh3D *target,Mesh3D *deform);	//m_source:SourceMesh; m_target:Animation前的TargetMesh; 
																						//m_deform:存储TargetMesh Animation后的网格点
	void			unitifyMesh(vector<Vec3f> &pList);		//平移坐标，使得其在[-1,1]内
};

