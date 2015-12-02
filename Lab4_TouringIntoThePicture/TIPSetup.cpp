#include <cv.h>
#include <highgui.h>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

Mat originalImage;

#define ENTER 10
#define ESC 27

Point points[5]; //Vanishing and corner points

void initPoints()
{
	points[0] = Point(originalImage.rows * 1 / 2, originalImage.rows * 1 / 2);
	points[1] = Point(originalImage.rows * 1 / 3, originalImage.rows * 1 / 3);
	points[2] = Point(originalImage.rows * 1 / 3, originalImage.rows * 2 / 3);
	points[3] = Point(originalImage.rows * 2 / 3, originalImage.rows * 2 / 3);
	points[4] = Point(originalImage.rows * 2 / 3, originalImage.rows * 1 / 3);
}

void readSettingsFile(string filename)
{
    ifstream settingsFile;
    settingsFile.open(filename.c_str());

    int x, y;
    for(int i = 0; i < 5; i++)
    {
    	settingsFile >> x >> y;
    	if(x < 0 || y < 0 || x >= originalImage.cols || y >= originalImage.rows)
    	{
    		printf("Invalid point in point file.\n");
    		initPoints();
    		return;
    	}
    	points[i] = Point(x, y);
    }
    settingsFile.close();
}

void saveSettingsFile(string filename)
{
	FILE* file = fopen(filename.c_str(), "w");
    for(int i = 0; i < 5; i++)
    {
    	fprintf(file, "%d %d\n", points[i].x, points[i].y);
    }
	fclose(file);
}

Point calcLineIntersection(Point center, Point farAwayCorner, bool vertical)
{
	float dy1 = farAwayCorner.y - center.y;
	float dx1 = farAwayCorner.x - center.x;
	if(vertical)
	{
		float dx2 = (dx1 < 0 ? 0 : originalImage.cols) - farAwayCorner.x;
		int dy2 = dy1 * dx2/dx1;
		return Point(farAwayCorner.x + dx2, farAwayCorner.y + dy2);
	}
	else
	{
		float dy2 = (dy1 < 0 ? 0 : originalImage.rows) - farAwayCorner.y;
		int dx2 = dx1 * dy2/dy1;
		return Point(farAwayCorner.x + dx2, farAwayCorner.y + dy2);
	}
}

Point calcRectIntersection(Point center, Point farAwayCorner)
{
	Point p1 = calcLineIntersection(center, farAwayCorner, true);
	if(p1.x >= 0 && p1.y >= 0 && p1.x <= originalImage.cols && p1.y <= originalImage.rows)
		return p1;
	else
		return calcLineIntersection(center, farAwayCorner, false);
}

void drawPoints()
{
	Mat drawing = originalImage.clone();
	circle(drawing, points[0], 2, Scalar(255, 255, 255), CV_FILLED);
	rectangle(drawing, points[1], points[3], Scalar(255, 255, 255), 1);
	for(int i = 1; i <= 4; i++)
	{
		Point pt2 = calcRectIntersection(points[0], points[i]);
		line(drawing, points[i], pt2, Scalar(255, 255, 255), 1);
	}
	for(int x = 0; x < originalImage.cols; x+=10)
	{
		for(int y = 0; y < originalImage.rows; y+=10)
		{
			circle(drawing, Point(x, y), 2, Scalar(255, 255, 255), CV_FILLED);
		}
	}
	imshow("Display Image", drawing);
}

//void addSeed(int x, int y, bool foreground, cv::Vec<unsigned char, 3> color)
//{
//	seeds.push_back((Seed){x, y, foreground, color});
//}

bool mouseDown;

void moveClosestPoint(Point clickPoint)
{
	int minDistSquared;
	int minDistIndex = -1;
	for(int i = 0; i < 5; i++)
	{
		Point p = points[i];
		int distSquared = (p.x - clickPoint.x) * (p.x - clickPoint.x) + (p.y - clickPoint.y) * (p.y - clickPoint.y);
		if(minDistIndex == -1 || distSquared < minDistSquared)
		{
			minDistIndex = i;
			minDistSquared = distSquared;
		}
	}
	points[minDistIndex] = clickPoint;
	if(minDistIndex == 1)
	{
		points[2].x = clickPoint.x;
		points[4].y = clickPoint.y;
	}
	else if(minDistIndex == 2)
	{
		points[1].x = clickPoint.x;
		points[3].y = clickPoint.y;
	}
	else if(minDistIndex == 3)
	{
		points[4].x = clickPoint.x;
		points[2].y = clickPoint.y;
	}
	else if(minDistIndex == 4)
	{
		points[3].x = clickPoint.x;
		points[1].y = clickPoint.y;
	}
}

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
		moveClosestPoint(Point(x, y));
		drawPoints();
	}
}

int main(int argc, char** argv )
{
	if ( argc < 2 )
	{
		printf("usage: DisplayImage.out <Image_Path> <Settings_Path>\n");
		return -1;
	}
	printf("Welcome to TIP Setup. Click to move points.\n");

	originalImage = imread(argv[1], 1);
	if ( !originalImage.data )
	{
		printf("No image data \n");
		return -1;
	}
	namedWindow("Display Image", WINDOW_AUTOSIZE );
	setMouseCallback("Display Image", CallBackFunc, NULL);
	if(argc < 3)
		initPoints();
	else
		readSettingsFile(argv[2]);
	drawPoints();

	std::string lineInput = "";

	while(true)
	{
		char c = waitKey(0);
		if(c == ESC)
			break;
		else
		{

		}
	}
	if(argc >= 3)
		saveSettingsFile(argv[2]);
	return 0;
}
