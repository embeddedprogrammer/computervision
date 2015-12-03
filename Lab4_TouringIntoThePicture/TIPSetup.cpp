#include <cv.h>
#include <highgui.h>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

Mat originalImage, imageBillboards;

#define ENTER 10
#define ESC 27

// User parameters
Point points[5]; //Vanishing and corner points
float focalLength = 700; //In pixels

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

Point2f mapBackwards(Point3f voxel)
{
	float x = voxel.x * focalLength / voxel.z;
	float y = voxel.y * focalLength / voxel.z;
	return Point2f(x + points[0].x, y + points[0].y);
}

Point3f mapForwardsFromDepth(Point2f pixel, float depth)
{
	float x = (pixel.x - points[0].x) * depth / focalLength;
	float y = (pixel.y - points[0].y) * depth / focalLength;
	return Point3f(x, y, depth);
}

float getDepth(float xw_or_yw, float xc_or_yc, bool vertical)
{
	if(vertical)
		return focalLength * xw_or_yw / (xc_or_yc - points[0].x);
	else
		return focalLength * xw_or_yw / (xc_or_yc - points[0].y);
}

Point3f mapForwards(Point2f pixel, float xw_or_yw, bool vertical)
{
	float z = getDepth(xw_or_yw, vertical ? pixel.x : pixel.y, vertical);
	return mapForwardsFromDepth(pixel, z);
}

float mixValues(float c1, float c2, float c3, float c4, float f1, float f2, float f3, float f4)
{
	return c1*f1 + c2*f2 + c3*f3 + c4*f4;
}

Vec3b mixPixels(Vec3b c1, Vec3b c2, Vec3b c3, Vec3b c4, float f1, float f2, float f3, float f4)
{
	return Vec3b(mixValues(c1[0], c2[0], c3[0], c4[0], f1, f2, f3, f4),
				 mixValues(c1[1], c2[1], c3[1], c4[1], f1, f2, f3, f4),
				 mixValues(c1[2], c2[2], c3[2], c4[2], f1, f2, f3, f4));
}

Vec3b GetPixelInterpolated(Point2f p)
{
	int xTruncated = p.x;
	int yTruncated = p.y;
	float xFrac = p.x - xTruncated;
	float yFrac = p.y - yTruncated;
	Vec3b color11 = originalImage.at<Vec3b>(yTruncated,     xTruncated);
	Vec3b color12 = originalImage.at<Vec3b>(yTruncated,     xTruncated + 1);
	Vec3b color21 = originalImage.at<Vec3b>(yTruncated + 1, xTruncated);
	Vec3b color22 = originalImage.at<Vec3b>(yTruncated + 1, xTruncated + 1);
	Vec3b mix = mixPixels(color11, color12, color21, color22, (1 - xFrac)*(1 - yFrac), xFrac*(1 - yFrac), (1 - xFrac)*yFrac, xFrac*yFrac);
	return mix;
}

typedef struct
{
	Point p1;
	Point p2;
	Point p3;
	Point p4;
	Point3f vx1;
	Point3f vx2;
	Point3f vx3;
	Point3f vx4;
	Size size;
	bool vertical;
	float wallDistance;
	float minXorY;
	float minZ;
	float deltaXorY;
	float deltaZ;
} MappingInfo;

Point3f mapWallPixelBackwards(Point wallPoint, MappingInfo info)
{
	float x, y, z;
	if(info.vertical)
	{
		x = info.wallDistance;
		y = info.minXorY + info.deltaXorY * wallPoint.y;
		z = info.minZ + info.deltaZ * wallPoint.x;
	}
	else
	{
		x = info.minXorY + info.deltaXorY * wallPoint.y;
		y = info.wallDistance;
		z = info.minZ + info.deltaZ * wallPoint.x;
	}
	return Point3f(x, y, z);
}

MappingInfo setupBackwardsMapping(int direction)
{
	MappingInfo info;

	// Parameters to setup spidery mesh
	float depth = 10;

	// From direction, choose two initial points
	if(direction == 0)
	{
		info.p1 = points[1];
		info.p2 = points[4];
		info.vertical = false;
	}
	else if(direction == 1)
	{
		info.p1 = points[4];
		info.p2 = points[3];
		info.vertical = true;
	}
	else if(direction == 2)
	{
		info.p1 = points[2];
		info.p2 = points[3];
		info.vertical = false;
	}
	else if(direction == 3)
	{
		info.p1 = points[1];
		info.p2 = points[2];
		info.vertical = true;
	}

	// From initial two points, calculate the other two points that form the wall.
	info.p3 = calcLineIntersection(info.p1, info.vertical);
	info.p4 = calcLineIntersection(info.p2, info.vertical);

	// Find the corresponding real world coordinates for the corners of the wall.
	info.vx1 = mapForwardsFromDepth(info.p1, depth);
	info.vx2 = mapForwardsFromDepth(info.p2, depth);
	info.wallDistance = info.vertical ? info.vx1.x : info.vx1.y;
	info.vx3 = mapForwards(Point2f(info.p3.x, info.p3.y), info.wallDistance, info.vertical);
	info.vx4 = mapForwards(Point2f(info.p4.x, info.p4.y), info.wallDistance, info.vertical);

	// Find the wall's min and max real world coordinates (ie. it's real-world bounding box).
	info.minXorY = info.vertical ? info.vx3.y : info.vx3.x;
	float maxXorY = info.vertical ? info.vx2.y : info.vx2.x;
	info.minZ = info.vx3.z;
	float maxZ = info.vx2.z;

	// Calculate the size of the wall in pixels.
	int wallHeight = info.vertical ? info.p4.y - info.p3.y : info.p4.x - info.p3.x;
	float wallHeight_w = maxXorY - info.minXorY;
	float wallDepth_w = maxZ - info.minZ;
	int wallDepth = wallDepth_w * wallHeight / wallHeight_w;
	info.deltaXorY = (maxXorY - info.minXorY) / wallHeight;
	info.deltaZ = (maxZ - info.minZ) / wallDepth;

	// Return information as a struct.
	info.size = Size(wallDepth, wallHeight);
	return info;
}

void mapWallBackwards(string outputImageFilename, int direction)
{
	// Backwards map each wall pixel and calculate it's color using linear interpolation
	MappingInfo mappingInfo = setupBackwardsMapping(direction);
	Mat wallDrawing = Mat::zeros(mappingInfo.size, CV_8UC3);
	for(int wallX = 0; wallX < mappingInfo.size.width; wallX++)
	{
		for(int wallY = 0; wallY <  mappingInfo.size.height; wallY++)
		{
			Point wallPoint = Point(wallX, wallY);
			Point3f p3f = mapWallPixelBackwards(wallPoint, mappingInfo);
			Point2f p = mapBackwards(p3f);
			if(p.x >= 0 && p.y >= 0 && p.x < originalImage.cols - 1 && p.y < originalImage.rows - 1)
			{
				wallDrawing.at<Vec3b>(wallY, wallX) = GetPixelInterpolated(p);
			}
		}
	}

	// Write image to file. Record wall's real world coordinates and camera coordinates.
	imwrite(outputImageFilename, wallDrawing);
	printf("SHAPE 4\n");
	printf("%.2f %.2f %.2f %.1f %.1f\n", mappingInfo.vx3.x, -mappingInfo.vx3.y, mappingInfo.vx3.z, 0.0f, 0.0f);
	printf("%.2f %.2f %.2f %.1f %.1f\n", mappingInfo.vx4.x, -mappingInfo.vx4.y, mappingInfo.vx4.z, 0.0f, 1.0f);
	printf("%.2f %.2f %.2f %.1f %.1f\n", mappingInfo.vx2.x, -mappingInfo.vx2.y, mappingInfo.vx2.z, 1.0f, 1.0f);
	printf("%.2f %.2f %.2f %.1f %.1f\n", mappingInfo.vx1.x, -mappingInfo.vx1.y, mappingInfo.vx1.z, 1.0f, 0.0f);
	printf("%s\n\n", outputImageFilename.c_str());
}

float getDepth(float XorYc, int wallDirection)
{
	MappingInfo mappingInfo = setupBackwardsMapping(wallDirection);

	if((wallDirection == 0 && XorYc < points[0].y)
	 || (wallDirection == 1 && XorYc > points[0].x)
	 || (wallDirection == 2 && XorYc > points[0].y)
	 || (wallDirection == 3 && XorYc < points[0].x))
		return getDepth(mappingInfo.wallDistance, XorYc, mappingInfo.vertical);
	else
		return -1;
}

float getDepth(Rect rect)
{
	float d[4];
	d[0] = getDepth(rect.y, 0);
	d[1] = getDepth(rect.x + rect.width, 1);
	d[2] = getDepth(rect.y + rect.height, 2);
	d[3] = getDepth(rect.x, 3);
	float minDepth;
	int minDepthIndex = -1;
	for(int i = 0; i < 4; i++)
	{
		float depth = d[i];
		if(depth > 0 && (minDepthIndex == -1 || depth < minDepth))
		{
			minDepth = depth;
			minDepthIndex = i;
		}
	}
	return minDepth;
}

void glueBillboardsToImage()
{
	// Threshold image
	Mat thresholdedImage;
	inRange(imageBillboards, Scalar(0, 0, 0), Scalar(20, 20, 20), thresholdedImage);
	bitwise_not(thresholdedImage, thresholdedImage);

	imshow("Thresholded image", thresholdedImage);

	// Find contours
	Mat img_copy = thresholdedImage.clone();

	vector<Vec4i> hierarchy;
	vector<vector<Point> > contours;
	findContours(img_copy, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	//Find bounding box
	for(int i = 0; i < contours.size(); i++)
	{
		Rect rect = boundingRect(contours[i]);
		float depth = getDepth(rect);

		Point3f vx1 = mapForwardsFromDepth(Point(rect.x,              rect.y),               depth);
		Point3f vx2 = mapForwardsFromDepth(Point(rect.x,              rect.y + rect.height), depth);
		Point3f vx3 = mapForwardsFromDepth(Point(rect.x + rect.width, rect.y + rect.height), depth);
		Point3f vx4 = mapForwardsFromDepth(Point(rect.x + rect.width, rect.y),               depth);

		// Write image to file. Record wall's real world coordinates and camera coordinates.
		Mat croppedImage = imageBillboards(rect);
		char outputImageFilename[20];
		sprintf(outputImageFilename, "billboard%d.jpg", i);
		imwrite(outputImageFilename, croppedImage);

		printf("SHAPE 4\n");
		printf("%.2f %.2f %.2f %.1f %.1f\n", vx1.x, -vx1.y, vx1.z, 0.0f, 0.0f);
		printf("%.2f %.2f %.2f %.1f %.1f\n", vx2.x, -vx2.y, vx2.z, 0.0f, 1.0f);
		printf("%.2f %.2f %.2f %.1f %.1f\n", vx3.x, -vx3.y, vx3.z, 1.0f, 1.0f);
		printf("%.2f %.2f %.2f %.1f %.1f\n", vx4.x, -vx4.y, vx4.z, 1.0f, 0.0f);
		printf("%s\n\n", outputImageFilename);
	}
}

void applyTransformations()
{
	Mat drawing = originalImage.clone();
	circle(drawing, points[0], 2, Scalar(255, 255, 255), CV_FILLED);
	rectangle(drawing, points[1], points[3], Scalar(255, 255, 255), 1);
	for(int i = 1; i <= 4; i++)
	{
		Point pt2 = calcRectIntersection(points[i]);
		line(drawing, points[i], pt2, Scalar(255, 255, 255), 1);
	}
	mapWallBackwards("top.jpg", 0);
	mapWallBackwards("right.jpg", 1);
	mapWallBackwards("bottom.jpg", 2);
	mapWallBackwards("left.jpg", 3);
	glueBillboardsToImage();
}

bool mouseDown;

void moveClosestPoint(Point clickPoint)
{
	int minDistSquared;
	int minDistIndex = -1;
	for(int i = 0; i < 6; i++)
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
	if ( argc < 4 )
	{
		printf("usage: DisplayImage.out <Image_Path> <Billboard Path> <Settings_Path>\n");
		return -1;
	}

	printf("Welcome to TIP Setup. Click to move points.\n");

	originalImage = imread(argv[1], 1);
	imageBillboards = imread(argv[2], 1);

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
		readSettingsFile(argv[3]);
	drawPoints();

	std::string lineInput = "";

	while(true)
	{
		char c = waitKey(0);
		if(c == ESC)
			break;
		else if(c == ENTER)
		{
			applyTransformations();
		}
	}
	if(argc >= 3)
		saveSettingsFile(argv[3]);
	return 0;
}
