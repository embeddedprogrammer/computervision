#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <string>
#include <iostream>
#include "graph.h"
#include <math.h>
#include <vector>

using namespace cv;
using namespace std;

// GRAPH CUT

typedef Graph<int,int,int> GraphType;

typedef struct
{
	int x, y;
	bool foreground;
	cv::Vec<unsigned char, 3> color;
} Seed;

typedef std::vector<Seed> SeedType;

int getNodeId(Mat image, int x, int y)
{
	return (image.cols * y) + x;
}

int maxVal(int a, int b)
{
	return (a > b) ? a : b;
}

int minVal(int a, int b)
{
	return (a < b) ? a : b;
}

int maxVal3(int val1, int val2, int val3)
{
	return maxVal(maxVal(val1, val2), val3);
}

int getDiffRGB(cv::Vec<unsigned char, 3> val1, cv::Vec<unsigned char, 3> val2)
{
	return maxVal3(abs(val1[0] - val2[0]), abs(val1[1] - val2[1]), abs(val1[2] - val2[2]));
}

int getCost(cv::Vec<unsigned char, 3> val1, cv::Vec<unsigned char, 3> val2)
{
	return 255 - getDiffRGB(val1, val2);
}

int addEdge(GraphType* g, Mat image, int x, int y, int dx, int dy)
{
	int cost = getCost(image.at<Vec3b>(y, x), image.at<Vec3b>(y + dy, x + dx));
	g->add_edge(getNodeId(image, x, y), getNodeId(image, x + dx, y + dy), cost, cost);
	return cost;
}

typedef struct
{
	int df, db;
} DistFB;

int print = 0;

DistFB findDistFB(cv::Vec<unsigned char, 3> val, SeedType seeds)
{
	int minDistForeground = 255;
	int minDistBackground = 255;
	for(int i = 0; i < seeds.size(); i++)
	{
		Seed s = seeds.at(i);
		int dist = getDiffRGB(val, s.color);
		if(s.foreground)
		{
			if(dist < minDistForeground)
				minDistForeground = dist;
		}
		else
		{
			if(dist < minDistBackground)
				minDistBackground = dist;
		}
	}
	return (DistFB){minDistForeground, minDistBackground};
}

void displayResults(Mat image, SeedType seeds)
{
	GraphType* g = new GraphType(image.rows * image.cols, 2 * image.rows * image.cols - image.rows - image.cols);

	Mat priors1 = Mat::zeros(image.size(), image.type());
	Mat priors2 = Mat::zeros(image.size(), image.type());
	// Create nodes and initialize the tn links based on prior estimation
	for(int y = 0; y < image.rows; y++)
		for(int x = 0; x < image.cols; x++)
		{
			g->add_node();
			DistFB d = findDistFB(image.at<Vec3b>(y,x), seeds);
			g->add_tweights(getNodeId(image, x, y), 10*d.db/(d.df + d.db), 10*d.df/(d.df + d.db));
			priors1.at<Vec3b>(y,x)[0] = d.df;
			priors1.at<Vec3b>(y,x)[1] = d.df;
			priors1.at<Vec3b>(y,x)[2] = d.df;
			priors2.at<Vec3b>(y,x)[0] = d.db;
			priors2.at<Vec3b>(y,x)[1] = d.db;
			priors2.at<Vec3b>(y,x)[2] = d.db;
		}
//	imshow("Dist to Foreground", priors1);
//	imshow("Dist to Background", priors2);

	// Add edges with corresponding cost
	Mat newImage = Mat::zeros(image.size(), image.type());
	for(int y = 0; y < image.rows; y++)
		for(int x = 0; x < image.cols; x++)
		{
			int c1, c2 = 0;
			if(x < image.cols - 1)
				c1 = addEdge(g, image, x, y, 1, 0);
			if(y < image.rows - 1)
				c2 = addEdge(g, image, x, y, 0, 1);
			int c = minVal(c1, c2);
			newImage.at<Vec3b>(y,x)[0] = c;
			newImage.at<Vec3b>(y,x)[1] = c;
			newImage.at<Vec3b>(y,x)[2] = c;
		}
	//imshow("Cost", newImage);

	// Add user specified seeds
	for(int i = 0; i < seeds.size(); i++)
	{
		Seed s = seeds.at(i);
		if(s.foreground	== 1)
			g->add_tweights(getNodeId(image, s.x, s.y), 1000000, 0);
		else
			g->add_tweights(getNodeId(image, s.x, s.y), 0, 1000000);
	}



	int flow = g->maxflow();

	printf("Flow = %d\n", flow);

	Mat result = Mat::zeros(image.size(), image.type());
	for( int y = 0; y < image.rows; y++ )
		for( int x = 0; x < image.cols; x++ )
		{
			if(g->what_segment(getNodeId(image, x, y)) == GraphType::SOURCE)
			{
//				result.at<Vec3b>(y,x)[0] = saturate_cast<uchar>(image.at<Vec3b>(y,x)[0]);
//				result.at<Vec3b>(y,x)[1] = saturate_cast<uchar>(image.at<Vec3b>(y,x)[1]);
//				result.at<Vec3b>(y,x)[2] = saturate_cast<uchar>(image.at<Vec3b>(y,x)[2]);
				result.at<Vec3b>(y,x)[0] = 255;
				result.at<Vec3b>(y,x)[1] = 255;
				result.at<Vec3b>(y,x)[2] = 255;
			}
			else
			{
				result.at<Vec3b>(y,x)[0] = 0;
				result.at<Vec3b>(y,x)[1] = 0;
				result.at<Vec3b>(y,x)[2] = 0;
//				result.at<Vec3b>(y,x)[0] = saturate_cast<uchar>(image.at<Vec3b>(y,x)[0]) / 3;
//				result.at<Vec3b>(y,x)[1] = saturate_cast<uchar>(image.at<Vec3b>(y,x)[1]) / 3;
//				result.at<Vec3b>(y,x)[2] = saturate_cast<uchar>(image.at<Vec3b>(y,x)[2]) / 3;
			}
		}

	imshow("New Image", result);
}

// Priors

SeedType seeds;

void addSeed(GraphType* g, Mat image, int x, int y, bool foreground, cv::Vec<unsigned char, 3> color)
{
	seeds.push_back((Seed){x, y, foreground, color});
}

//OPEN CV GUI INTERACTION

#define ENTER 10
#define ESC 27

int mouseDown;
Mat image2;
Mat originalImage;
GraphType* graph;

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
	if(event == EVENT_LBUTTONDOWN)
		mouseDown = 1;
	else if(event == EVENT_RBUTTONDOWN)
		mouseDown = 2;
	else if(event == EVENT_LBUTTONUP)
		mouseDown = 0;
	else if(event == EVENT_RBUTTONUP)
		mouseDown = 0;
	if (mouseDown)
	{
		if(mouseDown == 1)
			addSeed(graph, originalImage, x, y, true, image2.at<Vec3b>(y,x));
		else
			addSeed(graph, originalImage, x, y, false, image2.at<Vec3b>(y,x));

		Scalar color = (mouseDown == 1) ? Scalar(255, 0, 0) : Scalar(0, 0, 255);
		circle(image2, Point(x, y), 3, color, CV_FILLED);
		imshow("Display Image", image2);
	}
}

int main(int argc, char** argv )
{
	if ( argc != 2 )
	{
		printf("usage: DisplayImage.out <Image_Path>\n");
		return -1;
	}

	originalImage = imread( argv[1], 1 );
	image2 = originalImage.clone();

	if ( !image2.data )
	{
		printf("No image data \n");
		return -1;
	}
	namedWindow("Display Image", WINDOW_AUTOSIZE );
	imshow("Display Image", image2);
	setMouseCallback("Display Image", CallBackFunc, NULL);

	std::string lineInput = "";

	while(true)
	{
		char c = waitKey(0);
		displayResults(originalImage, seeds);
		if(c == ESC)
			break;
	}
	delete graph;
	return 0;
}



