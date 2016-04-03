#include "Transfer.h"

Transfer::Transfer(void)
{
	A.resize(0,0);
	B.resize(0,0);

	m_pMesh_template = new Mesh3D;	//载入网格模板信息
	m_pMesh_template->LoadFromOBJFile("./data/mesh_template.obj");	
	if ( m_pMesh_template->num_of_vertex_list()==0 )
	{
		cout<<"Read Mesh Template Wrong!!!"<<endl;
		exit(0);
	}
}

Transfer::~Transfer(void)
{
	delete m_pMesh_template;
}

void Transfer::getRverseV(Mesh3D *mesh,SparseMatrix<double> &V)	
{
	int num_of_vertx = m_pMesh_template->num_of_vertex_list();
	int num_of_face = m_pMesh_template->num_of_face_list();

	if ( V.rows()==2*num_of_face && V.cols()==num_of_vertx )	//
	{
		MatrixXd A(2,2);
		MatrixXd B(2,2);

		vector<Triplet<double>> triplets;
		vector<int> row;
		vector<int> col;
		vector<double> val;

		int k = 0;
		vector<HE_vert *> verts;
		vector<int> id(3);
		Vec3f p1,p2;
		for ( size_t i=0; i!=num_of_face; i++ )
		{
			m_pMesh_template->get_face(i)->face_verts(verts);

			for ( size_t j=0; j!=3; j++ )
			{
				id[j] = verts[j]->id_;		//获取源网格图三角形顶点id
			}

			for ( size_t j=0; j!=3; j++ )
			{
				verts[j] = mesh->get_vertex(id[j]);		//获取对应点的坐标
			}

			p1 = verts[1]->position_-verts[0]->position_;
			p2 = verts[2]->position_-verts[0]->position_;

			A(0,0) = p1[0];
			A(1,0) = p1[1];
			A(0,1) = p2[0];
			A(1,1) = p2[1];

			B = A.inverse();    //求逆
			A = B.transpose();	//转置

			row.push_back(k);
			col.push_back(verts[0]->id_);
			val.push_back((-A(0,0)-A(0,1)));

			row.push_back(k);
			col.push_back(verts[1]->id_);
			val.push_back(A(0,0));

			row.push_back(k);
			col.push_back(verts[2]->id_);
			val.push_back(A(0,1));

			k++;

			row.push_back(k);
			col.push_back(verts[0]->id_);
			val.push_back((-A(1,0)-A(1,1)));

			row.push_back(k);
			col.push_back(verts[1]->id_);
			val.push_back(A(1,0));

			row.push_back(k);
			col.push_back(verts[2]->id_);
			val.push_back(A(1,1));

			k++;
		}

		int num = val.size();

		for ( size_t i=0; i!=num; i++ )
		{
			Triplet<double> tri(row[i],col[i],val[i]);    
			triplets.push_back(tri);
		}

		V.setFromTriplets(triplets.begin(),triplets.end());
	}
}

MatrixXd Transfer::getSDeformationMx(Mesh3D *source,Mesh3D *deform)
{
	int num_of_vertx = m_pMesh_template->num_of_vertex_list();
	int num_of_face = m_pMesh_template->num_of_face_list();
	
	if ( num_of_vertx!=0 && num_of_face!=0 )
	{
		SparseMatrix<double> V(2*num_of_face,num_of_vertx);
		
		getRverseV(source,V);

		Vec3f p;
		MatrixXd XY(num_of_vertx,2);

		for ( size_t i=0; i!=num_of_vertx; i++ )
		{
			p = deform->get_vertex(i)->position_;
			XY(i,0) = p[0];
			XY(i,1) = p[1];
		}

		MatrixXd S(2*num_of_face,2);
		S = V*XY;

		return S;
	}
	else
	{
		MatrixXd S(0,0);
		return S;
	}
}

vector<Vec3f> Transfer::getTransformMesh(Mesh3D *source,Mesh3D *target,Mesh3D *deform)
{
	int num_of_vertx = m_pMesh_template->num_of_vertex_list();
	int num_of_face = m_pMesh_template->num_of_face_list();

// 	if ( target->num_of_vertex_list()>num_of_vertx )
// 	{
// 		//当目标网格点数大于模板点数时，超过的点先随特征点做一个刚性变换（旋转+平移），而根据模板和试验结果第37、38、48个点（id值减一）所做的变换几乎是刚性变换
// 		m_RTvertices.push_back(target->get_vertex(36)->position_);
// 		m_RTvertices.push_back(target->get_vertex(37)->position_);
// 		m_RTvertices.push_back(target->get_vertex(47)->position_);
// 	}

	list_of_meshpoints_after_animation.clear();

	if ( A.size()==0 && B.size()==0 )
	{
		A.resize(num_of_vertx,num_of_vertx);
		B.resize(2*num_of_face,num_of_vertx);

		getRverseV(target,B);

		A = B.transpose()*B;

		solver.analyzePattern(A); 
		// Compute the ordering permutation vector from the structural pattern of A
		solver.analyzePattern(A); 
		// Compute the numerical factorization 
		solver.factorize(A); 
	}

	VectorXd X(num_of_vertx);
	VectorXd Y(num_of_vertx);

	VectorXd b_x(num_of_vertx);
	VectorXd b_y(num_of_vertx);

	MatrixXd C(num_of_vertx,2);
	MatrixXd S(2*num_of_face,2);

	S = getSDeformationMx(source,deform);
	C = B.transpose()*S;

	for ( size_t j=0; j!=num_of_vertx; j++ )
	{
		b_x[j] = C(j,0);
		b_y[j] = C(j,1);
	}

	X = solver.solve(b_x);
	Y = solver.solve(b_y);

	Vec3f p(0,0,0);
	for ( size_t j=0; j!=num_of_vertx; j++ )
	{
		p[0] = X[j];
		p[1] = Y[j];

		list_of_meshpoints_after_animation.push_back(p);
	}

	if ( target->num_of_vertex_list()>num_of_vertx )
	{
		int num_of_vertex_of_target = target->num_of_vertex_list();

		MatrixXd R(2,2);	//旋转矩阵
		VectorXd d(2);		//平移向量

		getRTdeformation(target,list_of_meshpoints_after_animation,R,d);

		VectorXd V(2);
		for ( size_t i=num_of_vertx; i!=num_of_vertex_of_target; i++ )
		{
			V[0] = target->get_vertex(i)->position_[0];
			V[1] = target->get_vertex(i)->position_[1];

			V = R*V+d ;

			p[0] = V[0];
			p[1] = V[1];

			list_of_meshpoints_after_animation.push_back(p);
		}
	}

	unitifyMesh(list_of_meshpoints_after_animation);

	return list_of_meshpoints_after_animation;
}

void Transfer::getRTdeformation(Mesh3D *mesh,vector<Vec3f> pList,MatrixXd &R,VectorXd &d)
{
	if ( mesh->num_of_vertex_list()>m_pMesh_template->num_of_vertex_list() && pList.size()==m_pMesh_template->num_of_vertex_list() )
	{
		MatrixXd A(2,2);
		MatrixXd B(2,2);

		A(0,0) = mesh->get_vertex(37)->position_[0]-mesh->get_vertex(36)->position_[0];
		A(1,0) = mesh->get_vertex(37)->position_[1]-mesh->get_vertex(36)->position_[1];
		A(0,1) = mesh->get_vertex(47)->position_[0]-mesh->get_vertex(36)->position_[0];
		A(1,1) = mesh->get_vertex(47)->position_[1]-mesh->get_vertex(36)->position_[1];

		B(0,0) = pList[37][0]-pList[36][0];
		B(1,0) = pList[37][1]-pList[36][1];
		B(0,1) = pList[47][0]-pList[36][0];
		B(1,1) = pList[47][1]-pList[36][1];

		R = B*(A.inverse());	//求出旋转矩阵R

		VectorXd V1(2);
		VectorXd V2(2);

		V1[0] = mesh->get_vertex(36)->position_[0];
		V1[1] = mesh->get_vertex(36)->position_[1];
		V2[0] = pList[36][0];
		V2[1] = pList[36][1];

		d = V2 - R*V1;	//得到平移向量	
	}
}

void Transfer::unitifyMesh(vector<Vec3f> &pList)	//平移网格点坐标，使得其在[-1,1]内
{
	double max_x = -1;
	double min_x = 1;
	double max_y = -1;
	double min_y = 1;

	int size = pList.size();

	for ( size_t i=0; i!=size; i++ )	
	{
		if ( pList[i][0]>max_x )
		{
			max_x = pList[i][0];
		}
		else if ( pList[i][0]<min_x )
		{
			min_x = pList[i][0];
		}

		if ( pList[i][1]>max_y )
		{
			max_y = pList[i][1];
		}
		else if ( pList[i][1]<min_y )
		{
			min_y = pList[i][1];
		}
	}

	double scaleX = max_x-min_x;
	double scaleY = max_y-min_y;
	double scaleMax = 0;

	if ( scaleX < scaleY )
	{
		scaleMax = scaleY;
	}
	else
	{
		scaleMax = scaleX;
	}

	double scaleV = 2 / scaleMax;
	double centerPos_x = (min_x+max_x)/2.0;
	double centerPos_y = (min_y+max_y)/2.0;

	for ( size_t i=0; i!=size; i++ )
	{
		pList[i][0] = ( pList[i][0]-centerPos_x ) * scaleV;
		pList[i][1] = ( pList[i][1]-centerPos_y ) * scaleV;
	}
}