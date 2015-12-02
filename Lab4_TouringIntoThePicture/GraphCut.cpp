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

int scale = 1;

void imshow2(string name, Mat image)
{
	Mat resizedImage;
	scale = image.cols / 400;
    resize(image, resizedImage, Size(image.cols / scale, image.rows / scale));
	imshow(name, resizedImage);
}

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

int getDiffHSV(cv::Vec<unsigned char, 3> val1, cv::Vec<unsigned char, 3> val2)
{
	//For HSV, Hue range is [0,179], Saturation range is [0,255] and Value range is [0,255]
	return abs(val1[0] - val2[0]) + abs(val1[1] - val2[1])/4 + abs(val1[2] - val2[2])/4;
}

int getCost(cv::Vec<unsigned char, 3> val1, cv::Vec<unsigned char, 3> val2)
{
	return 254 / (getDiffHSV(val1, val2) + 1);
}

int distanceToNearestSeed(SeedType seeds, Mat image, int x, int y)
{
	int minDist = image.rows + image.cols;
	for(int i = 0; i < seeds.size(); i++)
	{
		Seed s = seeds.at(i);
		int dx = abs(s.x - x);
		int dy = abs(s.y - y);
		int dist = sqrt(dx*dx + dy*dy);

		if(dist < minDist)
			minDist = dist;
	}
	return minDist;
}

int addEdge(GraphType* g, SeedType seeds, Mat image, int x, int y, int dx, int dy, int lambda)
{
	//int fractPerimeter = (image.rows + image.cols)/8;
	//int distToSeed = distanceToNearestSeed(seeds, image, x, y);
	int cost = getCost(image.at<Vec3b>(y, x), image.at<Vec3b>(y + dy, x + dx)); // * fractPerimeter / (distToSeed + 1);
	int modifiedCost = cost * 10 * (100 + lambda) / 100;
	g->add_edge(getNodeId(image, x, y), getNodeId(image, x + dx, y + dy), modifiedCost, modifiedCost);
	return cost / 10;
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
		int dist = getDiffHSV(val, s.color);
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

void displayResults(Mat image, Mat hsv, SeedType seeds, int lambda)
{
	GraphType* g = new GraphType(image.rows * image.cols, 2 * image.rows * image.cols - image.rows - image.cols);

	printf("Calculating sink and source weights\n");
	Mat priors1 = Mat::zeros(image.size(), image.type());
	Mat priors2 = Mat::zeros(image.size(), image.type());
	// Create nodes and initialize the tn links based on prior estimation
	for(int y = 0; y < image.rows; y++)
		for(int x = 0; x < image.cols; x++)
		{
			g->add_node();
			DistFB d = findDistFB(hsv.at<Vec3b>(y,x), seeds);
			int sourceWeight = 100*d.db/(d.df + d.db + 1);
			int sinkWeight = 100*d.df/(d.df + d.db + 1);
			g->add_tweights(getNodeId(image, x, y), sourceWeight * (100 - lambda) / 1000, sinkWeight * (100 - lambda) / 1000);
			priors1.at<Vec3b>(y,x)[0] = sourceWeight;
			priors1.at<Vec3b>(y,x)[1] = sourceWeight;
			priors1.at<Vec3b>(y,x)[2] = sourceWeight;
			priors2.at<Vec3b>(y,x)[0] = sinkWeight;
			priors2.at<Vec3b>(y,x)[1] = sinkWeight;
			priors2.at<Vec3b>(y,x)[2] = sinkWeight;
		}

	// Add edges with corresponding cost
	printf("Calculating edge weights\n");
	Mat cost = Mat::zeros(image.size(), image.type());
	for(int y = 0; y < image.rows; y++)
		for(int x = 0; x < image.cols; x++)
		{
			int c1, c2 = 0;
			if(x < image.cols - 1)
				c1 = addEdge(g, seeds, hsv, x, y, 1, 0, lambda);
			if(y < image.rows - 1)
				c2 = addEdge(g, seeds, hsv, x, y, 0, 1, lambda);
			int c = (c1 + c2) / 2;
			cost.at<Vec3b>(y,x)[0] = saturate_cast<uchar>(c);
			cost.at<Vec3b>(y,x)[1] = saturate_cast<uchar>(c);
			cost.at<Vec3b>(y,x)[2] = saturate_cast<uchar>(c);
		}

	// Add user specified seeds
	for(int i = 0; i < seeds.size(); i++)
	{
		Seed s = seeds.at(i);
		if(s.foreground	== 1)
			g->add_tweights(getNodeId(image, s.x, s.y), 1000000, 0);
		else
			g->add_tweights(getNodeId(image, s.x, s.y), 0, 1000000);
	}

	// Calculate max-flow min-cut.
	printf("Calculating max-flow min-cut\n");
	int flow = g->maxflow();
	Mat result = Mat::zeros(image.size(), image.type());
	Mat resultInv = Mat::zeros(image.size(), image.type());
	for( int y = 0; y < image.rows; y++ )
		for( int x = 0; x < image.cols; x++ )
		{
			if(g->what_segment(getNodeId(image, x, y)) == GraphType::SOURCE) //Foreground
			{
				result.at<Vec3b>(y,x)[0] = saturate_cast<uchar>(image.at<Vec3b>(y,x)[0]);
				result.at<Vec3b>(y,x)[1] = saturate_cast<uchar>(image.at<Vec3b>(y,x)[1]);
				result.at<Vec3b>(y,x)[2] = saturate_cast<uchar>(image.at<Vec3b>(y,x)[2]);
				resultInv.at<Vec3b>(y,x)[0] = 0;
				resultInv.at<Vec3b>(y,x)[1] = 0;
				resultInv.at<Vec3b>(y,x)[2] = 0;
			}
			else
			{
				result.at<Vec3b>(y,x)[0] = 0;
				result.at<Vec3b>(y,x)[1] = 0;
				result.at<Vec3b>(y,x)[2] = 0;
				resultInv.at<Vec3b>(y,x)[0] = saturate_cast<uchar>(image.at<Vec3b>(y,x)[0]);
				resultInv.at<Vec3b>(y,x)[1] = saturate_cast<uchar>(image.at<Vec3b>(y,x)[1]);
				resultInv.at<Vec3b>(y,x)[2] = saturate_cast<uchar>(image.at<Vec3b>(y,x)[2]);
			}
		}

	imshow2("result", result);
	imwrite("result.jpg", result);
	imwrite("resultInv.jpg", resultInv);
}

// Priors

SeedType seeds;

void addSeed(int x, int y, bool foreground, cv::Vec<unsigned char, 3> color)
{
	seeds.push_back((Seed){x, y, foreground, color});
}

//OPEN CV GUI INTERACTION

#define ENTER 10
#define ESC 27

int mouseDown;
Mat image2;
Mat originalImage;
Mat hsvImage;
GraphType* graph;

int lambdaSlider = 100;

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
	x *= scale;
	y *= scale;
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
			addSeed(x, y, true, hsvImage.at<Vec3b>(y,x));
		else
			addSeed(x, y, false, hsvImage.at<Vec3b>(y,x));

		Scalar color = (mouseDown == 1) ? Scalar(255, 0, 0) : Scalar(0, 0, 255);
		circle(image2, Point(x, y), 3 * scale, color, CV_FILLED);
		imshow2("Display Image", image2);
	}
}

int main(int argc, char** argv )
{
	if ( argc != 2 )
	{
		printf("usage: DisplayImage.out <Image_Path>\n");
		return -1;
	}
	printf("Welcome to Graph Cut. Press any key to cut graph. Press ESC once you are happy with the results.\n");

	originalImage = imread(argv[1], 1);
	cvtColor(originalImage, hsvImage, COLOR_BGR2HSV);
	image2 = originalImage.clone();

	if ( !image2.data )
	{
		printf("No image data \n");
		return -1;
	}
	namedWindow("Display Image", WINDOW_AUTOSIZE );
	createTrackbar("Lambda", "Display Image", &lambdaSlider, 200);
	imshow2("Display Image", image2);
	setMouseCallback("Display Image", CallBackFunc, NULL);

	std::string lineInput = "";

	while(true)
	{
		char c = waitKey(0);
		if(c == ESC)
			break;
		else
		{
			displayResults(originalImage, hsvImage, seeds, lambdaSlider - 100);
		}
	}
	delete graph;
	return 0;
}
