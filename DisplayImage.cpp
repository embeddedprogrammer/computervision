#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <string>
#include <iostream>
#include "graph.h"
#include <math.h>

using namespace cv;
using namespace std;

// GRAPH CUT

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

int getCost(cv::Vec<unsigned char, 3> val1, cv::Vec<unsigned char, 3> val2)
{
	int diff = maxVal3(abs(val1[0] - val2[0]), abs(val1[1] - val2[1]), abs(val1[2] - val2[2]));
	return 255 - diff;
}

typedef Graph<int,int,int> GraphType;

int addEdge(GraphType* g, Mat image, int x, int y, int dx, int dy)
{
	int cost = getCost(image.at<Vec3b>(y, x), image.at<Vec3b>(y + dy, x + dx));
	g->add_edge(getNodeId(image, x, y), getNodeId(image, x + dx, y + dy), cost, cost);
	return cost;
}

// Create the graph. Add nodes and edges (this part can be done without any user interaction)
GraphType* createGraph(Mat image)
{
	GraphType* g = new GraphType(image.rows * image.cols, 2 * image.rows * image.cols - image.rows - image.cols);

	for(int y = 0; y < image.rows; y++)
		for(int x = 0; x < image.cols; x++)
			g->add_node();

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
	imshow("Cost", newImage);
	return g;
}

void displayResults(Mat image, GraphType* g)
{
	int flow = g->maxflow();

	printf("Flow = %d\n", flow);

	Mat newImage = Mat::zeros(image.size(), image.type());
	for( int y = 0; y < image.rows; y++ )
		for( int x = 0; x < image.cols; x++ )
		{
			if(g->what_segment(getNodeId(image, x, y)) == GraphType::SOURCE)
			{
//				newImage.at<Vec3b>(y,x)[0] = saturate_cast<uchar>(image.at<Vec3b>(y,x)[0]);
//				newImage.at<Vec3b>(y,x)[1] = saturate_cast<uchar>(image.at<Vec3b>(y,x)[1]);
//				newImage.at<Vec3b>(y,x)[2] = saturate_cast<uchar>(image.at<Vec3b>(y,x)[2]);
				newImage.at<Vec3b>(y,x)[0] = 255;
				newImage.at<Vec3b>(y,x)[1] = 255;
				newImage.at<Vec3b>(y,x)[2] = 255;
			}
			else
			{
				newImage.at<Vec3b>(y,x)[0] = 0;
				newImage.at<Vec3b>(y,x)[1] = 0;
				newImage.at<Vec3b>(y,x)[2] = 0;
//				newImage.at<Vec3b>(y,x)[0] = saturate_cast<uchar>(image.at<Vec3b>(y,x)[0]) / 3;
//				newImage.at<Vec3b>(y,x)[1] = saturate_cast<uchar>(image.at<Vec3b>(y,x)[1]) / 3;
//				newImage.at<Vec3b>(y,x)[2] = saturate_cast<uchar>(image.at<Vec3b>(y,x)[2]) / 3;
			}
		}

	imshow("New Image", newImage);
}

// Priors

cv::Vec<unsigned char, 3> f, b;

void addSeed(GraphType* g, Mat image, int x, int y, bool foreground)
{
	if(foreground == 1)
	{
		g->add_tweights(getNodeId(image, x, y), 10000, 0);
		f = image.at<Vec3b>(y,x);
	}
	else
	{
		g->add_tweights(getNodeId(image, x, y), 0, 10000);
		b = image.at<Vec3b>(y,x);
	}
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
//		cout <<   "B: " << (int)image2.at<Vec3b>(y,x)[0]
//			 << ", G: " << (int)image2.at<Vec3b>(y,x)[1]
//			 << ", R: " << (int)image2.at<Vec3b>(y,x)[2] << endl;
		mouseDown = 1;
	else if(event == EVENT_RBUTTONDOWN)
		mouseDown = 2;
	else if(event == EVENT_LBUTTONUP)
		mouseDown = 0;
	else if(event == EVENT_RBUTTONUP)
		mouseDown = 0;
	if (mouseDown) //EVENT_MOUSEMOVE
	{
		if(mouseDown == 1)
			addSeed(graph, originalImage, x, y, true);
		else
			addSeed(graph, originalImage, x, y, false);

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

	graph = createGraph(originalImage);

	while(true)
	{
		char c = waitKey(0);
		displayResults(originalImage, graph);
		if(c == ESC)
			break;
	}
	delete graph;


//	while(true)
//	{
//		char c = waitKey(0);
//		if(c == 'p')
//			createGraph(originalImage);
//
//		else
//		{
//			printf("Character %c %d\n", c, (int)c);
//			break;
//		}
//
//
////		if(c == ENTER)
////		{
////			cout << "Line: " << lineInput << endl;
////			lineInput = "";
////		}
////		else
////		{
////			lineInput += c;
////		}
//	}

	return 0;

}



