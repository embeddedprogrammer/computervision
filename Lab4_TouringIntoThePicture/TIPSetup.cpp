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

Point calcLineIntersection(Point farAwayCorner, bool vertical)
{
	float dy1 = farAwayCorner.y - points[0].y;
	float dx1 = farAwayCorner.x - points[0].x;
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

Point calcRectIntersection(Point farAwayCorner)
{
	Point p1 = calcLineIntersection(farAwayCorner, true);
	if(p1.x >= 0 && p1.y >= 0 && p1.x <= originalImage.cols && p1.y <= originalImage.rows)
		return p1;
	else
		return calcLineIntersection(farAwayCorner, false);
}

void drawPoints()
{
	Mat drawing = originalImage.clone();
	circle(drawing, points[0], 2, Scalar(255, 255, 255), CV_FILLED);
	rectangle(drawing, points[1], points[3], Scalar(255, 255, 255), 1);
	for(int i = 1; i <= 4; i++)
	{
		Point pt2 = calcRectIntersection(points[i]);
		line(drawing, points[i], pt2, Scalar(255, 255, 255), 1);
	}
	imshow("Display Image", drawing);
}

Point2f mapBackwards(Point3f voxel, float focalLength)
{
	float x = voxel.x * focalLength / voxel.z;
	float y = voxel.y * focalLength / voxel.z;
	return Point2f(x + points[0].x, y + points[0].y);
}

Point3f mapForwardsFromDepth(Point2f pixel, float focalLength, float depth)
{
	float x = (pixel.x - points[0].x) * depth / focalLength;
	float y = (pixel.y - points[0].y) * depth / focalLength;
	printf("Point: %f %f %f\n", x, y, depth);
	return Point3f(x, y, depth);
}

Point3f mapForwards(Point2f pixel, float focalLength, float xw)
{
	float z = focalLength * xw / (pixel.x - points[0].x);
	return mapForwardsFromDepth(pixel, focalLength, z);
}

void drawPoints2()
{
	Mat drawing = originalImage.clone();
	circle(drawing, points[0], 2, Scalar(255, 255, 255), CV_FILLED);
	rectangle(drawing, points[1], points[3], Scalar(255, 255, 255), 1);
	for(int i = 1; i <= 4; i++)
	{
		Point pt2 = calcRectIntersection(points[i]);
		line(drawing, points[i], pt2, Scalar(255, 255, 255), 1);
	}

	//Init 3d world
	float depth = 10;
	float focalLength = 700; //pixels
	Point3f vx1 = mapForwardsFromDepth(points[4], focalLength, depth);
	Point3f vx2 = mapForwardsFromDepth(points[3], focalLength, depth);
	float wallDistance = vx1.x;
	Point p1 = calcLineIntersection(points[4], true);
	Point p2 = calcLineIntersection(points[3], true);
	circle(drawing, points[4], 3, Scalar(255, 255, 255), CV_FILLED);
	circle(drawing, points[3], 3, Scalar(255, 255, 255), CV_FILLED);
	circle(drawing, p1, 3, Scalar(255, 255, 255), CV_FILLED);
	circle(drawing, p2, 3, Scalar(255, 255, 255), CV_FILLED);
	Point3f vx3 = mapForwards(Point2f(p1.x, p1.y), focalLength, wallDistance);
	Point3f vx4 = mapForwards(Point2f(p2.x, p2.y), focalLength, wallDistance);

	float x = wallDistance;
	float minY = vx3.y;
	float maxY = vx2.y;
	float minZ = vx3.z;
	float maxZ = vx2.z;

	int wallHeight = p2.y - p1.y;
	int wallLength = (maxZ - minZ) * (p2.y - p1.y) / (maxY - minY);

	printf("Wall size: %d %d\n", wallHeight, wallLength);

	float deltaY = (maxY - minY) / wallHeight;
	float deltaZ = (maxZ - minZ) / wallLength;

	printf("%f to %f, +=%f\n", minY, maxY, deltaY);
	printf("%f to %f, +=%f\n", minZ, maxZ, deltaZ);

	Mat wallDrawing = Mat::zeros(wallHeight, wallLength, CV_8UC3);

	for(int wallX = 0; wallX < wallLength; wallX++)
	{
		for(int wallY = 0; wallY < wallHeight; wallY++)
		{
			float y = minY + deltaY * wallY;
			float z = minZ + deltaZ * wallX;
			Point2f p = mapBackwards(Point3f(x, y, z), focalLength);
			if(p.x >= 0 && p.y >= 0 && p.x < originalImage.cols && p.y < originalImage.rows)
			{
//				drawing.at<Vec3b>(p.y, p.x)[0] = 255;
//				drawing.at<Vec3b>(p.y, p.x)[1] = 0;
//				drawing.at<Vec3b>(p.y, p.x)[2] = 0; //TODO: Interpolate.
				wallDrawing.at<Vec3b>(wallY, wallX)[0] = saturate_cast<uchar>(originalImage.at<Vec3b>(p.y, p.x)[0]);
				wallDrawing.at<Vec3b>(wallY, wallX)[1] = saturate_cast<uchar>(originalImage.at<Vec3b>(p.y, p.x)[1]);
				wallDrawing.at<Vec3b>(wallY, wallX)[2] = saturate_cast<uchar>(originalImage.at<Vec3b>(p.y, p.x)[2]);
			}
		}
	}
	imshow("Display Image", drawing);
	imshow("Wall", wallDrawing);
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
			drawPoints2();
		}
	}
	if(argc >= 3)
		saveSettingsFile(argv[2]);
	return 0;
}
