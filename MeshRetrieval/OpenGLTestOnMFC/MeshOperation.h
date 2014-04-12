#pragma once
#include "afxwin.h"

#include <gl/gl.h>
#include <gl/glu.h>
#include <vector>
#include <ANN/ANN.h>
#include <fstream>
#include <iostream>
#include <string>
//#include <stdlib.h>
using namespace std;

#undef min
#undef max
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Mesh/Status.hh>
#include <OpenMesh/Core/IO/exporter/ExporterT.hh>

struct MyTraits : public OpenMesh::DefaultTraits
{
	VertexAttributes(OpenMesh::Attributes::Status);
	FaceAttributes(OpenMesh::Attributes::Status);
	EdgeAttributes(OpenMesh::Attributes::Status);
};

using namespace std;

typedef OpenMesh::PolyMesh_ArrayKernelT<>  MyMesh;

double round(double number);
double FindMaxDistance(MyMesh &mesh);
double similarity(double *histogram_test,double *histogram_sketch);
void Normalizer(MyMesh &mesh);
void AddNoise(double noise_standard_deviation,MyMesh &mesh);
void loadHistogram(string filname,double *histogram);
void MeshSketchRetrieval(MyMesh &mesh,double scaling_x,double scaling_y,
						 vector<double> &sketchpoint_x,vector<double> &sketchpoint_y,
						 vector<double> &grid_id_x,vector<double> &grid_id_y);