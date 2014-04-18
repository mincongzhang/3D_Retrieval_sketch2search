#include "stdafx.h"
#include "MeshOperation.h"

#include <math.h>
#include <cmath>
#include <stdio.h>
#include <random>

double candidate_index_array[DATASIZE] = {}; 
using namespace std; // make std:: accessible

/*Add random Gaussian Noise to verteices*/
void AddNoise(double noise_standard_deviation,MyMesh &mesh)
{
	std::default_random_engine generator;
	std::normal_distribution<double> distribution(0.0,noise_standard_deviation); //Gaussian distribution: mean value = 0.0

	for (auto it = mesh.vertices_begin(); it != mesh.vertices_end(); ++it)
	{
		double Pt[3] = {};
		for (int d=0;d<3;d++)
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
void MeshSketchRetrieval(vector<double> &sketchpoint_x,vector<double> &sketchpoint_y,
						 vector<double> &grid_id_x,vector<double> &grid_id_y)
{
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
	/*base section*/
	//similarity_halfcircle = sum(hist_test.*hist)/(norm(hist_test)*norm(hist))
	//double similarity_vector[DATASIZE] = {};
	//for (int i = 0; i < DATASIZE; i++)
	//{ 
	//	double historgram_object[143]={};
	//	//number to string
	//	string num = static_cast<ostringstream*>( &(ostringstream() << (i)) )->str();
	//	string filname;
	//	if(RETRIEVAL_CONTROL==1)
	//		filname = "./MeshHistData/back_"+num+"_hist_front.txt";
	//	if(RETRIEVAL_CONTROL==2)
	//		filname = "./MeshHistData/seat"+num+"_hist_top.txt";
	//	loadHistogram (filname,historgram_object);

	//	double similarity_tem  = similarity(historgram_object,histogram_sketch);
	//	similarity_vector[i] = similarity_tem;
	//}

	/*Calculate similarity with database*/
	/*advance method 1*/
	double similarity_vector[DATASIZE] = {};
	for (int i = 0; i < DATASIZE; i++)
	{ 
		double historgram_object[143]={};
		//number to string
		string num = static_cast<ostringstream*>( &(ostringstream() << (i)) )->str();
		string filname,filname_front,filname_up15,filname_down15,filname_side15,filname_up30,filname_down30,filname_side30,filname_side45;
		if(RETRIEVAL_CONTROL==1)
		{
			filname_front = "./MeshHistData/back_"+num+"_hist_front.txt"; //hist_front/up30/down30/side30/side45
			loadHistogram (filname_front,historgram_object);
			double similarity_tem_front  = similarity(historgram_object,histogram_sketch);

			filname_up15 = "./MeshHistData/back_"+num+"_hist_up15.txt"; 
			loadHistogram (filname_up15,historgram_object);
			double similarity_tem_up15  = similarity(historgram_object,histogram_sketch);

			filname_down15 = "./MeshHistData/back_"+num+"_hist_down15.txt";
			loadHistogram (filname_down15,historgram_object);
			double similarity_tem_down15  = similarity(historgram_object,histogram_sketch);

			filname_side15 = "./MeshHistData/back_"+num+"_hist_side15.txt";
			loadHistogram (filname_side15,historgram_object);
			double similarity_tem_side15  = similarity(historgram_object,histogram_sketch);

			filname_up30 = "./MeshHistData/back_"+num+"_hist_up30.txt"; 
			loadHistogram (filname_up30,historgram_object);
			double similarity_tem_up30  = similarity(historgram_object,histogram_sketch);

			filname_down30 = "./MeshHistData/back_"+num+"_hist_down30.txt";
			loadHistogram (filname_down30,historgram_object);
			double similarity_tem_down30  = similarity(historgram_object,histogram_sketch);

			filname_side30 = "./MeshHistData/back_"+num+"_hist_side30.txt";
			loadHistogram (filname_side30,historgram_object);
			double similarity_tem_side30  = similarity(historgram_object,histogram_sketch);

			filname_side45 = "./MeshHistData/back_"+num+"_hist_side45.txt";
			loadHistogram (filname_side45,historgram_object);
			double similarity_tem_side45  = similarity(historgram_object,histogram_sketch);

			similarity_vector[i] = (similarity_tem_front*abs(cos(theta_y))+similarity_tem_front*abs(cos(theta_x))
				+similarity_tem_up15*abs(cos(15*2*M_PI/360.0-theta_x))+similarity_tem_down15*abs(cos(15*2*M_PI/360.0-theta_x))
				+similarity_tem_side15*abs(cos(15*2*M_PI/360.0-theta_y))+similarity_tem_side15*abs(cos(15*2*M_PI/360.0-theta_y))
				+similarity_tem_up30*abs(cos(30*2*M_PI/360.0-theta_x))+similarity_tem_down30*abs(cos(30*2*M_PI/360.0-theta_x))
				+similarity_tem_side30*abs(cos(30*2*M_PI/360.0-theta_y))+similarity_tem_side30*abs(cos(30*2*M_PI/360.0)-theta_y)
				+similarity_tem_side45*abs(cos(45*2*M_PI/360.0-theta_y))+similarity_tem_side45*abs(cos(45*2*M_PI/360.0)-theta_y))
				/(abs(cos(theta_y))+abs(cos(theta_x))
				+abs(cos(15*2*M_PI/360.0-theta_x))+abs(cos(15*2*M_PI/360.0-theta_x))+abs(cos(15*2*M_PI/360.0-theta_y))+abs(cos(15*2*M_PI/360.0-theta_y))
				+abs(cos(30*2*M_PI/360.0-theta_x))+abs(cos(30*2*M_PI/360.0-theta_x))+abs(cos(30*2*M_PI/360.0-theta_y))+abs(cos(30*2*M_PI/360.0-theta_y))
				+abs(cos(45*2*M_PI/360.0-theta_y))+abs(cos(45*2*M_PI/360.0-theta_y)));
		}
		if(RETRIEVAL_CONTROL==2)
		{
			filname = "./MeshHistData/seat"+num+"_hist_top.txt";
			loadHistogram (filname,historgram_object);
			double similarity_tem  = similarity(historgram_object,histogram_sketch);
			similarity_vector[i] = similarity_tem;
		}
	}



	///*Calculate similarity with database*/
	///*advance method 2*/
	////similarity_halfcircle = sum(hist_test.*hist)/(norm(hist_test)*norm(hist))
	//double similarity_vector[DATASIZE] = {};
	//for (int i = 0; i < DATASIZE; i++)
	//{ 
	//	double historgram_object[143]={};
	//	//number to string
	//	string num = static_cast<ostringstream*>( &(ostringstream() << (i)) )->str();
	//	string filname,filname_front,filname_up15,filname_down15,filname_side15,filname_side30;
	//	if(RETRIEVAL_CONTROL==1)
	//	{
	//		filname_front = "./MeshHistData/back_"+num+"_hist_front.txt"; 
	//		loadHistogram (filname_front,historgram_object);
	//		double similarity_tem_front  = similarity(historgram_object,histogram_sketch);

	//		filname_up15 = "./MeshHistData/back_"+num+"_hist_up15.txt"; 
	//		loadHistogram (filname_up15,historgram_object);
	//		double similarity_tem_up15  = similarity(historgram_object,histogram_sketch);

	//		filname_down15 = "./MeshHistData/back_"+num+"_hist_down15.txt";
	//		loadHistogram (filname_down15,historgram_object);
	//		double similarity_tem_down15  = similarity(historgram_object,histogram_sketch);

	//		filname_side15 = "./MeshHistData/back_"+num+"_hist_side15.txt";
	//		loadHistogram (filname_side15,historgram_object);
	//		double similarity_tem_side15  = similarity(historgram_object,histogram_sketch);

	//		filname_side30 = "./MeshHistData/back_"+num+"_hist_side30.txt";
	//		loadHistogram (filname_side30,historgram_object);
	//		double similarity_tem_side30  = similarity(historgram_object,histogram_sketch);

	//		similarity_vector[i] = (similarity_tem_front+similarity_tem_up15*cos(15*2*M_PI/360.0)+similarity_tem_down15*cos(15*2*M_PI/360.0)
	//			+similarity_tem_side15*cos(15*2*M_PI/360.0)+similarity_tem_side30*cos(30*2*M_PI/360.0)
	//			+similarity_tem_side15*cos(15*2*M_PI/360.0)+similarity_tem_side30*cos(30*2*M_PI/360.0))
	//			/(1+cos(15*2*M_PI/360.0)+cos(15*2*M_PI/360.0)+cos(15*2*M_PI/360.0)+cos(30*2*M_PI/360.0)+cos(15*2*M_PI/360.0)+cos(30*2*M_PI/360.0));
	//	}
	//	if(RETRIEVAL_CONTROL==2)
	//	{
	//		filname = "./MeshHistData/seat"+num+"_hist_top.txt";
	//		loadHistogram (filname,historgram_object);
	//		double similarity_tem  = similarity(historgram_object,histogram_sketch);
	//		similarity_vector[i] = similarity_tem;
	//	}
	//}


	//initial candidate_index_array[DATASIZE] form 0 to (DATASIZE-1)
	for (int i = 0; i < DATASIZE; i++)
	{
		candidate_index_array[i] = double(i);
	}

	//sort candidate similarity and return candidate id; sort from the 0th to (DATASIZE-1)th
	qsort_getid(similarity_vector,candidate_index_array, 0, DATASIZE-1);

	//Choose the first one with highest similarity
	if(RETRIEVAL_CONTROL==1)
		ChooseCandidate(candidate_index_array,0);
	if(RETRIEVAL_CONTROL==2)
		ChooseCandidate(candidate_index_array,10);

	RETRIEVAL_CONTROL = 0;
}

void ChooseCandidate(double candidate_index_array[],int candidateIndx)
{
	int index = candidateIndx;

	if(index>=10) index = index-10;

	int CandidateIdx = int(candidate_index_array[DATASIZE-index-1]);
	//number to string
	string CandidateIdx_S = static_cast<ostringstream*>( &(ostringstream() << CandidateIdx) )->str();
	string back_filname = "./MeshData/back/back"+CandidateIdx_S+".obj";
	string seat_filname = "./MeshData/seat/seat"+CandidateIdx_S+".obj";
	string leg_filname = "./MeshData/leg/leg"+CandidateIdx_S+".obj";

	//load candidate mesh 
	MyMesh back_mesh,seat_mesh,leg_mesh;
	OpenMesh::IO::read_mesh(back_mesh, back_filname);
	OpenMesh::IO::read_mesh(seat_mesh, seat_filname);
	OpenMesh::IO::read_mesh(leg_mesh,  leg_filname);
	meshQueue.clear();
	if(candidateIndx<10)
	{
		meshQueue.push_back(seat_mesh);
		meshQueue.push_back(back_mesh);	
		meshQueue.push_back(leg_mesh);
	}
	if(candidateIndx>=10)
	{
		meshQueue.push_back(back_mesh);	
		meshQueue.push_back(seat_mesh);
		meshQueue.push_back(leg_mesh);
	}
}