#include "stdafx.h"
#include "OpenGLControl.h"
#include ".\openglcontrol.h"
#include "MeshOperation.h"

#include <math.h>
#include <stdio.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <ANN/ANN.h>

// make std:: accessible
using namespace std;

//Queue for all meshes
vector<MyMesh>  meshQueue;

bool NOISE_CONTROL = false;
bool NORMALIZE_CONTROL = false;
bool SKETCH_CONTROL = false;
bool RETRIEVAL_CONTROL = false;

//add noise variable
double noise_standard_deviation = 0.01; 

//retrieval variables
vector<double> sketchpoint_x;
vector<double> sketchpoint_y;
vector<double> grid_id_x;
vector<double> grid_id_y;
double scaling_x = 7.0,scaling_y = 4.5;
MyMesh projected_mesh;

//shading parameters
GLfloat mat_specular[]={1.0f, 0.0f, 1.0f, 1.0f};
GLfloat mat_diffuse[]={0.0f, 0.0f, 1.0f, 1.0f};
GLfloat mat_ambient[]={0.8f, 0.0f, 0.8f, 1.0f};
GLfloat mat_shininess[]={80.0};

COpenGLControl::COpenGLControl(void)
{
	m_fPosX = 0.0f;						 // X position of model in camera view
	m_fPosY = -0.1f;					 // Y position of model in camera view
	m_fZoom = 1.0f;						 // Zoom on model in camera view
	m_fRotX = 0.0f;						 // Rotation on model in camera view
	m_fRotY	= 0.0f;						 // Rotation on model in camera view
	m_bIsMaximized = false;
}

COpenGLControl::~COpenGLControl(void)
{
}

BEGIN_MESSAGE_MAP(COpenGLControl, CWnd)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

void COpenGLControl::OnPaint()
{
	//CPaintDC dc(this); // device context for painting
	ValidateRect(NULL);
}

void COpenGLControl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	if (0 >= cx || 0 >= cy || nType == SIZE_MINIMIZED) return;

	// Map the OpenGL coordinates.
	glViewport(0, 0, cx, cy);

	// Projection view
	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	// Set our current view perspective
	gluPerspective(35.0f, (float)cx / (float)cy, 0.01f, 2000.0f);

	// Model view
	glMatrixMode(GL_MODELVIEW);

	switch (nType)
	{
		// If window resize token is "maximize"
	case SIZE_MAXIMIZED:
		{
			// Get the current window rect
			GetWindowRect(m_rect);

			// Move the window accordingly
			MoveWindow(6, 6, cx - 14, cy - 14);

			// Get the new window rect
			GetWindowRect(m_rect);

			// Store our old window as the new rect
			m_oldWindow = m_rect;

			break;
		}

		// If window resize token is "restore"
	case SIZE_RESTORED:
		{
			// If the window is currently maximized
			if (m_bIsMaximized)
			{
				// Get the current window rect
				GetWindowRect(m_rect);

				// Move the window accordingly (to our stored old window)
				MoveWindow(m_oldWindow.left, m_oldWindow.top - 18, m_originalRect.Width() - 4, m_originalRect.Height() - 4);

				// Get the new window rect
				GetWindowRect(m_rect);

				// Store our old window as the new rect
				m_oldWindow = m_rect;
			}
			break;
		}
	}
}

int COpenGLControl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1) return -1;

	oglInitialize();

	return 0;
}

void COpenGLControl::OnDraw(CDC *pDC)
{
	// If the current view is perspective...
	glLoadIdentity();
	gluLookAt(0.0,0.0,5.0,0.0,0.0,0.0,0.0,1.0,0.0);
	//glFrustum(-1, 1, -1, 1, 0.0, 40.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glTranslatef(0.0f, 0.0f, -m_fZoom);
	glTranslatef(m_fPosX, m_fPosY, 0.0f);
	glRotatef(m_fRotX, 1.0f, 0.0f, 0.0f);
	glRotatef(m_fRotY, 0.0f, 1.0f, 0.0f);
}

void COpenGLControl::OnTimer(UINT nIDEvent)
{
	switch (nIDEvent)
	{
	case 1:
		{
			// Clear color and depth buffer bits
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
			// Draw OpenGL scene
			oglDrawScene();

			// Swap buffers
			SwapBuffers(hdc);

			break;
		}

	default:
		break;
	}

	CWnd::OnTimer(nIDEvent);
}

void COpenGLControl::OnMouseMove(UINT nFlags, CPoint point)
{
	int diffX = (int)(point.x - m_fLastX);
	int diffY = (int)(point.y - m_fLastY);
	m_fLastX  = (float)point.x;
	m_fLastY  = (float)point.y;

	// Left mouse button
	if (nFlags & MK_LBUTTON)
	{
		if(SKETCH_CONTROL)
		{
			sketchpoint_x.push_back((double)point.x);
			sketchpoint_y.push_back((double)point.y);
		}
		else
		{
			m_fRotX += (float)0.5f * diffY;

			if ((m_fRotX > 360.0f) || (m_fRotX < -360.0f))
			{
				m_fRotX = 0.0f;
			}

			m_fRotY += (float)0.5f * diffX;

			if ((m_fRotY > 360.0f) || (m_fRotY < -360.0f))
			{
				m_fRotY = 0.0f;
			}
		}
	}
	// Middle mouse button
	else if (nFlags & MK_MBUTTON)
	{
		m_fZoom -= (float)0.01f * diffY;
	}
	// Right mouse button
	else if (nFlags & MK_RBUTTON)
	{
		m_fPosX += (float)0.0005f * diffX;
		m_fPosY -= (float)0.0005f * diffY;
	}

	OnDraw(NULL);

	CWnd::OnMouseMove(nFlags, point);
}

void COpenGLControl::oglCreate(CRect rect, CWnd *parent)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_OWNDC, NULL, (HBRUSH)GetStockObject(BLACK_BRUSH), NULL);

	CreateEx(0, className, "OpenGL", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, rect, parent, 0);

	// Set initial variables' values
	m_oldWindow	   = rect;
	m_originalRect = rect;

	hWnd = parent;
}

void COpenGLControl::oglInitialize(void)
{
	// Initial Setup:
	//
	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		32, // bit depth
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		16, // z-buffer depth
		0, 0, 0, 0, 0, 0, 0,
	};

	// Get device context only once.
	hdc = GetDC()->m_hDC;

	// Pixel format.
	m_nPixelFormat = ChoosePixelFormat(hdc, &pfd);
	SetPixelFormat(hdc, m_nPixelFormat, &pfd);

	// Create the OpenGL Rendering Context.
	hrc = wglCreateContext(hdc);
	wglMakeCurrent(hdc, hrc);

	// Basic Setup:
	//
	// Set color to use when clearing the background.
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	////////////////////////////////////////

	GLfloat light_ambient1[]={0.0f, 0.0f, 0.0f, 1.0f};
	GLfloat light_diffuse1[]={1.0, 1.0, 1.0, 1.0};
	GLfloat light_specular1[]={0.8f, 0.8f, 0.8f, 1.0f};
	GLfloat light_position1[]={11.0, 11.0, 11.0, 0.0};

	glLightfv(GL_LIGHT0, GL_POSITION, light_position1);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient1);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse1);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular1);
	glEnable(GL_LIGHT0);

	glMaterialfv(GL_FRONT,GL_AMBIENT,mat_ambient);
	glMaterialfv(GL_FRONT,GL_DIFFUSE,mat_diffuse);
	glMaterialfv(GL_FRONT,GL_SPECULAR,mat_specular);
	glMaterialfv(GL_FRONT,GL_SHININESS,mat_shininess);


	glEnable(GL_LIGHTING);

	glShadeModel(GL_FLAT);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
	///////////////////////////////////////
	// Turn on backface culling
	//glFrontFace(GL_CCW);
	//glCullFace(GL_BACK);

	// Turn on depth testing

	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 0.0, 5.0,  /* eye is at (0,0,5) */
		0.0, 0.0, 0.0,      /* center is at (0,0,0) */
		0.0, 1.0, 0.);      /* up is in positive Y direction */


	// Send draw request
	OnDraw(NULL);


}

void COpenGLControl::oglDrawScene(void)
{

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	unsigned int meshsize = meshQueue.size();

	//add noise to current mesh
	if(NOISE_CONTROL && meshsize>=1)
	{
		AddNoise(noise_standard_deviation,meshQueue.at(meshsize-1));
	}
	//normalize the current mesh
	if(NORMALIZE_CONTROL && meshsize>=1)
	{
		Normalizer(meshQueue.at(meshsize-1));
	}
	//retrieval
	if(RETRIEVAL_CONTROL && !sketchpoint_x.empty())
	{
		grid_id_x.clear();
		grid_id_y.clear();
		//MeshSketchRetrieval(meshQueue.at(meshsize-1),sketchpoint_x,sketchpoint_y,grid_id_x,grid_id_y);
		//DEBUG for grid
		MeshSketchRetrieval(meshQueue.at(meshsize-1),scaling_x,scaling_y,
						    sketchpoint_x,sketchpoint_y,
						    grid_id_x,grid_id_y);
		sketchpoint_x.clear();
		sketchpoint_y.clear();
	}
	//draw meshes
	for (unsigned int i=0;i<meshsize;i++)
	{
		if(meshsize>0)
		{
			//sketch
			if(!SKETCH_CONTROL)
			{
				//Restore rotated 3D object
				projected_mesh = meshQueue.at(i);
			}
			//Get 2D projected view of rotated 3D object
			if(SKETCH_CONTROL)
			{
				glPushMatrix();
				double theta_x = m_fRotX*2*M_PI/360.0;
				double theta_y = m_fRotY*2*M_PI/360.0;

				//draw the projection of the current mesh at xy plane 
				glColor3f(GLfloat(1.0), GLfloat(1.0), GLfloat(1.0));
				glBegin(GL_POINTS);
				for(MyMesh::VertexIter v_it = projected_mesh.vertices_begin();v_it!=projected_mesh.vertices_end();++v_it)
				{
					//rotate the mesh and show in 2D projection
					MyMesh::Point point = projected_mesh.point(v_it.handle());
					double temp_x = point.data()[0],temp_y = point.data()[1],temp_z = point.data()[2];
					(float)*(projected_mesh.point(v_it).data()+0) =  temp_x*cos(theta_y)+temp_y*sin(theta_x)*sin(theta_y)+temp_z*cos(theta_x)*sin(theta_y);
					(float)*(projected_mesh.point(v_it).data()+1) =  temp_y*cos(theta_x)+temp_z*sin(theta_x);
					(float)*(projected_mesh.point(v_it).data()+2) = -temp_x*sin(theta_y)+temp_y*sin(theta_x)*cos(theta_y)+temp_z*cos(theta_x)*cos(theta_y);
					glVertex3f((float)*(projected_mesh.point(v_it).data()+0),(float)*(projected_mesh.point(v_it).data()+1),0.0f);
				}
				glEnd();

				//draw the sketch
				glColor3f(GLfloat(1.0), GLfloat(1.0), GLfloat(0.0));
				glPointSize(2.0);
				glBegin(GL_POINTS);
				for(unsigned int i = 0;i<sketchpoint_x.size();i++)
				{
					glVertex3f((float)(sketchpoint_x.at(i)/(scaling_x*100.0)-0.5)*5,(float)(0.5-sketchpoint_y.at(i)/(scaling_y*100.0))*5,0.0f);
				}
				glEnd();

				glPopMatrix();

				//initial the factors
				m_fPosX =  0.0f;						 // X position of model in camera view
				m_fPosY = -0.1f;					     // Y position of model in camera view
				m_fRotX =  0.0f;						 // Rotation on model in camera view
				m_fRotY	=  0.0f;						 // Rotation on model in camera view
			}
			else
			{ 
				MyMesh meshcurrent=meshQueue.at(i);
				MyMesh::FaceIter f_it, vi_end(meshcurrent.faces_end());
				MyMesh::FaceVertexIter v_it;

				//x axis
				glColor3f(GLfloat(1.0), GLfloat(0.0), GLfloat(0.0));
				glBegin(GL_LINES);
				glVertex3f(0.0,0.0,0.0);
				glVertex3f(1.0,0.0,0.0);
				glEnd();
				//y axis
				glColor3f(GLfloat(0.0), GLfloat(1.0), GLfloat(0.0));
				glBegin(GL_LINES);
				glVertex3f(0.0,0.0,0.0);
				glVertex3f(0.0,1.0,0.0);
				glEnd();
				//z axis
				glColor3f(GLfloat(0.0), GLfloat(0.0), GLfloat(1.0));
				glBegin(GL_LINES);
				glVertex3f(0.0,0.0,0.0);
				glVertex3f(0.0,0.0,1.0);
				glEnd();

				//change the colour for each mesh
				switch (i) 
				{
				case 0:
					glColor3f(GLfloat(1.0), GLfloat(1.0), GLfloat(1.0));
					break;
				case 1:
					glColor3f(GLfloat(0.7), GLfloat(0.5), GLfloat(1.0));
					break;
				case 2:
					glColor3f(GLfloat(0.6), GLfloat(1.0), GLfloat(0.5));
					break;
				case 3:
					glColor3f(GLfloat(0.6), GLfloat(1.0), GLfloat(1.0));
					break;
				default:
					glColor3f(GLfloat(0.5), GLfloat(0.5), GLfloat(0.5));
				};

				meshcurrent.request_face_normals();
				meshcurrent.update_normals();

				GLdouble norms[3]={0.0,0.0,0.0};
				for(f_it=meshcurrent.faces_begin();f_it!=vi_end;++f_it)
				{
					for(int n=0;n<3;n++){
						norms[n]=(GLdouble)*(meshcurrent.normal(f_it).data()+n);
					}
					glNormal3dv(norms);

					glPushMatrix();
					glBegin(GL_POLYGON);
					for(v_it=meshcurrent.fv_iter(f_it);v_it;++v_it)
					{
						glVertex3fv(meshcurrent.point(v_it).data());
					}
					glEnd();
					glPopMatrix();
				}

				//draw grid
				if(grid_id_x.size()>0 )
				{
					//grid
					glColor3f(GLfloat(0.0), GLfloat(1.0), GLfloat(1.0));
					glPointSize(2.0);
					glBegin(GL_POINTS);
					for(unsigned int i = 0 ;i<grid_id_x.size();i++){
						glVertex3f(float(grid_id_x.at(i))/31,float(grid_id_y.at(i))/31,0.0);
					}
					glEnd();
				}//end draw grid
			}//end else
		} //end if(meshsize>0)
	}//end for (unsigned int i=0;i<meshsize;i++)
}//end void COpenGLControl::oglDrawScene(void)