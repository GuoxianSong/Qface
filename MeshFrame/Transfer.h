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
	SparseMatrix<double> A;		//ϡ��ϵ������A��Ax = b��
	SparseMatrix<double> B;		//���ڹ���ϡ�����A, B'B = A
	SparseLU<SparseMatrix<double>>   solver;
	Mesh3D *m_pMesh_template;	//��������ģ��

// private:
// 	vector<Vec3f> m_RTvertices;	//���ڴ洢�����㣬���һ����任����תR+ƽ��T������ά��

public:
	vector<Vec3f> list_of_meshpoints_after_animation;	//�洢TargetMesh Animation��������

public:
	Transfer(void);
	virtual ~Transfer(void);

public:			
	void			getRverseV(Mesh3D *mesh,SparseMatrix<double> &V);	//�������������Ӧ����
	void			getRTdeformation(Mesh3D *mesh,vector<Vec3f> pList,MatrixXd &R,VectorXd &d);	//��ñ��κ�ĸ��Ա任����תR+ƽ��d��
	MatrixXd		getSDeformationMx(Mesh3D *source,Mesh3D *deform);	//���������ξ���
	vector<Vec3f>	getTransformMesh(Mesh3D *source,Mesh3D *target,Mesh3D *deform);	//m_source:SourceMesh; m_target:Animationǰ��TargetMesh; 
																						//m_deform:�洢TargetMesh Animation��������
	void			unitifyMesh(vector<Vec3f> &pList);		//ƽ�����꣬ʹ������[-1,1]��
};

