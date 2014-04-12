#include "stdafx.h"
#include "Toolbox.h"
#include "MeshOperation.h"
#include "OpenGLControl.h"
#include ".\openglcontrol.h"

#include <math.h>
#include <stdio.h>
#include <random>

//GSL and ANN library
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include <ANN/ANN.h>

#define DATASIZE 3

using namespace std; // make std:: accessible

//kd tree
// Global variables
//
int k = 2;					// number of nearest neighbors
int dim = 3;				// dimension
double eps = 0;				// error bound
istream* dataIn = NULL;		// input for data points
istream* queryIn = NULL;	// input for query points

/*Add random Gaussian Noise to verteices*/
void AddNoise(double noise_standard_deviation,MyMesh &mesh)
{
	std::default_random_engine generator;
	std::normal_distribution<double> distribution(0.0,noise_standard_deviation); //Gaussian distribution: mean value = 0.0

	for (auto it = mesh.vertices_begin(); it != mesh.vertices_end(); ++it)
	{
		double Pt[3] = {};
		for (int d=0;d<dim;d++)
		{
			Pt[d]=*(mesh.point(it).data()+d);
			double randn = distribution(generator);
			if ((randn>=-1.0)&&(randn<=1.0))//Gaussian distribution range [-1.0,1.0]							        
			{
				Pt[d]= Pt[d]*(1.0+randn);
				*(mesh.point(it).data()+d)=float(Pt[d]);
			}
		}
	}
	NOISE_CONTROL = false;
}


/*normalize the model inside the unit cube*/
void Normalizer(MyMesh &mesh)
{
	double max_distance = FindMaxDistance(mesh);

	for (MyMesh::VertexIter v_it = mesh.vertices_begin();v_it!=mesh.vertices_end(); ++v_it)
	{
		for (int d = 0; d < 3; d++)
		{
			*(mesh.point(v_it).data()+d) /= max_distance;
		}

	}
	NORMALIZE_CONTROL = FALSE;
}

/*Retrieval mesh*/
void MeshSketchRetrieval(MyMesh &mesh,double scaling_x,double scaling_y,
						 vector<double> &sketchpoint_x,vector<double> &sketchpoint_y,
						 vector<double> &grid_id_x,vector<double> &grid_id_y)
{
	///*Interpolate in sketch points*/
	//vector<double> inter_x,inter_y;
	//inter_x.push_back(sketchpoint_x.at(0)/scaling_x);
	//inter_y.push_back(sketchpoint_y.at(0)/scaling_y);
	////start from the second point in sketch
	//for(int i=1;i<sketchpoint_x.size();i++)
	//{
	//	double vector_x = sketchpoint_x.at(i)-sketchpoint_x.at(i-1);
	//	double vector_y = sketchpoint_y.at(i)-sketchpoint_y.at(i-1);
	//	int dist = int(sqrt(pow(vector_x,2)+pow(vector_y,2))*1.0);

	//	if(dist!=0) // interpolate
	//	{
	//		for (int j=1;j<dist;j++)
	//		{
	//			inter_x.push_back((sketchpoint_x.at(i-1)+(double)j*vector_x/(double)dist)/scaling_x);
	//			inter_y.push_back((sketchpoint_y.at(i-1)+(double)j*vector_y/(double)dist)/scaling_y);
	//		}
	//	}
	//	else
	//	{
	//		inter_x.push_back(sketchpoint_x.at(i)/scaling_x);
	//		inter_y.push_back(sketchpoint_y.at(i)/scaling_y);
	//	}
	//}

	/*normalize sketch points*/
	//find min of sketch points
	double min_x = sketchpoint_x.at(0), min_y = sketchpoint_y.at(0);
	for(unsigned int i = 0;i<sketchpoint_x.size();i++)
	{
		if(sketchpoint_x.at(i)<min_x) min_x = sketchpoint_x.at(i);
		if(sketchpoint_y.at(i)<min_y) min_y = sketchpoint_y.at(i);
	}

	//move to the origin
	for (unsigned int i = 0; i < sketchpoint_x.size(); i++)
	{
		sketchpoint_x.at(i) -= min_x;
		sketchpoint_y.at(i) -= min_y;
	}

	//find max of moved sketch points
	double max_x = sketchpoint_x.at(0), max_y = sketchpoint_y.at(0);
	for(unsigned int i = 0;i<sketchpoint_x.size();i++)
	{
		if(sketchpoint_x.at(i)>max_x) max_x = sketchpoint_x.at(i);
		if(sketchpoint_y.at(i)>max_y) max_y = sketchpoint_y.at(i);
	}

	//find max of max_x and max_y as normal factor
	double normalfactor = max_x;
	if(max_y>normalfactor) normalfactor = max_y;

	//normalize
	for(unsigned int i = 0;i<sketchpoint_x.size();i++)
	{
		sketchpoint_x.at(i) /= normalfactor;
		sketchpoint_y.at(i)  = max_y - sketchpoint_y.at(i); //inverse coordinate
		sketchpoint_y.at(i) /= normalfactor;
	}

	/*map to 32*32 grid and get centroid*/
	//map
	int grid [32*32] = {};
	for(unsigned int i=0;i<sketchpoint_x.size();i++)
	{
		int row = int(round(sketchpoint_y.at(i)*31.0));
		int col = int(round(sketchpoint_x.at(i)*31.0));
		grid[row*32+col] = 1;
	}

	//get coordinate of grid when the value equals to 1 and calculate mean of grid_x and grid_y
	//vector<double> grid_id_x,grid_id_y;	
	double centroid_x = 0.0,centroid_y = 0.0;
	for(int m=0;m<32;m++)//row
	{
		for(int n=0;n<32;n++)//column
		{
			if(grid[m*32+n]==1)
			{
				grid_id_y.push_back(double(m));
				centroid_y += double(m);
				grid_id_x.push_back(double(n));
				centroid_x += double(n);
			}
		}
	}
	centroid_x/=grid_id_x.size();
	centroid_y/=grid_id_y.size();

	/*Histogram of sketch points*/
	vector<double> diff;
	for (unsigned int i = 0; i < grid_id_x.size(); i++)
	{
		double different = sqrt(pow( (grid_id_x.at(i)-centroid_x),2 )+pow( (grid_id_y.at(i)-centroid_y),2 ));
		//normalize different
		different/= (32*sqrt(2));
		diff.push_back(different);
	}

	//142 = ceil(100*sqrt(2)); use 143 because there are 143 data in the file(the last one is zero)
	double histogram_sketch [143]={};
	for(unsigned int i =0;i<diff.size();i++)
	{
		int index_hist = int(round(diff.at(i)*100.0));
		histogram_sketch[index_hist]+=1.0;
	}

	/*Calculate similarity with database*/
	//similarity_halfcircle = sum(hist_test.*hist)/(norm(hist_test)*norm(hist))
	double similarity_vector[DATASIZE] = {};
	for (int i = 0; i < DATASIZE; i++)
	{ 
		double historgram_object[143]={};
		string num = static_cast<ostringstream*>( &(ostringstream() << i) )->str();
		string filname = "./MeshHistData/back_"+num+"_hist_front.txt";
		loadHistogram (filname,historgram_object);

		double similarity_tem  = similarity(historgram_object,histogram_sketch);
		similarity_vector[i] = similarity_tem;
	}

	double candidate_index[DATASIZE] = {}; 
	for (int i = 0; i < DATASIZE; i++)
	{
		candidate_index[i] = i;
	}

	//sort candidate similarity and return candidate id
	qsort_getid(similarity_vector,candidate_index, 0, DATASIZE-1);

	int first_CandidateIdx = candidate_index[DATASIZE-1];
	string first_CandidateIdx_S = static_cast<ostringstream*>( &(ostringstream() << first_CandidateIdx) )->str();
	string first_filname = "./MeshData/back_"+first_CandidateIdx_S+".obj";

	//load candidate mesh 
	MyMesh first_mesh;
	try
	{
		if ( !OpenMesh::IO::read_mesh(first_mesh, first_filname) )
		{
			std::cerr << "Cannot read mesh from the directory" << std::endl;
		}
	}
	catch( std::exception& x )
	{
		std::cerr << x.what() << std::endl;
	}
	meshQueue.pop_back();
	meshQueue.push_back(first_mesh);

	//double historgram_object2[143]={};
	//string filname2 = "./MeshHitsData/back_2_hist_front.txt";
	//loadHistogram (filname2,historgram_object2);

	/*double historgram_object6[143]={};
	string filname6 = "./MeshHitsData/back_6_hist_front.txt";
	loadHistogram (filname6,historgram_object6);*/

	/*double historgram_object36[143]={};
	string filname36 = "./MeshHitsData/back_36_hist_front.txt";
	loadHistogram (filname36,historgram_object36);*/

	/*
	double similarity2  = similarity(historgram_object2,histogram_sketch);
	double similarity6  = similarity(historgram_object6,histogram_sketch);
	double similarity36 = similarity(historgram_object36,histogram_sketch);*/

	RETRIEVAL_CONTROL = false;
}

