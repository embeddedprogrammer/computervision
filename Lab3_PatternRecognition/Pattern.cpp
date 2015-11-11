#include <cv.h>
#include <highgui.h>

using namespace cv;

#define ENTER 10
#define ESC 27

RNG rng(12345);

Scalar getColor(int i)
{
	Scalar c0 = Scalar(64, 64, 64);
	Scalar c1 = Scalar(0, 64, 128);
	Scalar c2 = Scalar(0, 0, 255);
	Scalar c3 = Scalar(0, 128, 255);
	Scalar c4 = Scalar(0, 255, 255);
	Scalar c5 = Scalar(0, 255, 0);
	Scalar c6 = Scalar(255, 0, 0);
	Scalar c7 = Scalar(128, 0, 128);
	Scalar c8 = Scalar(128, 128, 128);
	Scalar c9 = Scalar(255, 255, 255);
	Scalar c[] = {c0, c1, c2, c3, c4, c5, c6, c7, c8, c9};
	if(i <= 9)
		return c[i];
	else
		return Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
}

int getColorNumber(Vec3b v)
{
	for(int i = 0; i <= 9; i++)
	{
		Scalar c = getColor(i);
		if(c.val[0] == v[0] && c.val[1] == v[1] && c.val[2] == v[2])
			return i;
	}
	return 0;
}

string getShape(int i)
{
	string s[] = {"Rectangle", "Triangle", "Circle", "Millstone", "Oval", "??", "+", "Oval", "E", "F"};
	if(i <= 9)
		return s[i];
	else
		return "?";
}

typedef struct
{
	double scaledPerimeter;
	double eccentricity;
	double rectDensity;
	double hullDensity;
	double density;
	vector<Point> hull;
	Point center;
	Vec3b shapeColor;
	int shapeNumber;
	string shape;
} features_t;

features_t getContourFeatures(vector<vector<Point> > contours, vector<Vec4i> hierarchy, Mat ans, int i)
{
	Moments mm = moments((Mat)contours[i]);
	double m00 = mm.m00;
	double m10 = mm.m10;
	double m01 = mm.m01;
	double m20 = mm.m20;
	double m02 = mm.m02;
	double outsideContourMass = m00;

	int densityA = 0;

	//Subtract holes from mass.
	for(int j = 0; j < contours.size(); j++)
		if(hierarchy[j][3] == i)
		{
			mm = moments((Mat)contours[j]);
			m00 -= mm.m00;
			m10 -= mm.m10;
			m01 -= mm.m01;
			m20 -= mm.m20;
			m02 -= mm.m02;
		}

	double mass = m00;
	double centerX = (m10 / m00);
	double centerY = (m01 / m00);
	double sigma_x = sqrt(m20 / m00 - centerX * centerX);
	double sigma_y = sqrt(m02 / m00 - centerY * centerY);
	double perimeter = arcLength((Mat)contours[i], true);
	features_t features;
	features.scaledPerimeter = perimeter / sqrt(outsideContourMass);
	features.center = Point(centerX, centerY);
	features.shapeColor = ans.at<Vec3b>(features.center.y, features.center.x);
	features.shapeNumber = getColorNumber(features.shapeColor);
	features.shape = getShape(features.shapeNumber);
	features.density = mass / outsideContourMass;

	// Find the convex hull object
	vector<Point> hull;
	convexHull(Mat(contours[i]), hull);
	features.hull = hull;
	features.hullDensity = outsideContourMass / moments((Mat)hull).m00;

	// Find the rectangle object
    RotatedRect rect = minAreaRect(contours[i]);
	features.rectDensity = moments((Mat)hull).m00 / rect.size.area();
	double eccentricity = (rect.size.width > rect.size.height) ? (rect.size.width / rect.size.height) : (rect.size.height / rect.size.width);
	features.eccentricity = eccentricity;

	return features;
}

#define VERTICAL_INNER_SPACING 150
#define HORIZONTAL_INNER_SPACING 150
#define VERTICAL_OUTER_SPACING 200
#define HORIZONTAL_OUTER_SPACING 200
#define MAX_OBJECTS_PER_CLASS 15
#define MAX_OBJECT_CLASSES 10
#define ORGANIZATION_SPACE_WIDTH (HORIZONTAL_OUTER_SPACING*2 + HORIZONTAL_INNER_SPACING*(MAX_OBJECTS_PER_CLASS - 1))
#define ORGANIZATION_SPACE_HEIGHT (VERTICAL_OUTER_SPACING*2 + VERTICAL_INNER_SPACING*(MAX_OBJECT_CLASSES - 1))

void processImage(string imageFileName, string answerFileName, string matlabLabel, bool invert, FILE *file, int* classCount, Mat drawing)
{
	// Read image
	Mat img, ans, img_th, img_cnc;
	printf("Image: %s, Answer: %s\n", imageFileName.c_str(), answerFileName.c_str());
	img = imread(imageFileName, 0);
	ans = imread(answerFileName);

	// Threshold image
	if(invert)
		inRange(img, 0, 100, img_th);
	else
		inRange(img, 101, 255, img_th);

	// Find contours
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	Mat img_copy = img_th.clone();
	findContours(img_copy, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE); //The RETR_COMP will arrange all contours into a 2-level hierarchy.

	// Draw contours and plot center of mass
	int objects = 0;
	features_t FeaturesArray[20];

//	for(int i = 0; i < contours.size(); i++)
//		printf("hierarchy[%d][2] = %d\n", i, hierarchy[i][2]);

	for(int i = 0; i < contours.size(); i++)
	{
		if(hierarchy[i][3] == -1) // Only use objects, not holes
		{
			Scalar color = getColor(objects);
			features_t features = getContourFeatures(contours, hierarchy, ans, i);

			int centerX = HORIZONTAL_OUTER_SPACING + HORIZONTAL_INNER_SPACING * classCount[features.shapeNumber];
			int centerY = VERTICAL_OUTER_SPACING + VERTICAL_INNER_SPACING * features.shapeNumber;
			int offsetX = -features.center.x + centerX;
			int offsetY = -features.center.y + centerY;

			classCount[features.shapeNumber]++;

			drawContours(drawing, contours, i, (Scalar)features.shapeColor, 4, 8, hierarchy, 1, Point(offsetX, offsetY));
			//drawContours(img_th, contours, i, (Scalar)features.shapeColor, 4, 8, hierarchy, 1, Point());
			//printf("[%d] %f %f %f %f %f %d\n", i, features.eccentricity, features.rectDensity, features.scaledPerimeter, features.hullDensity, features.density, features.shapeNumber);

			FeaturesArray[objects] = features;

			//printf("Object %d Shape number %d Class count: %d\n", objects, features.shapeNumber, classCount[features.shapeNumber]);
//			circle(drawing, features.center, 2, color);

//			//Draw convex hull
//			vector<vector<Point> > hullArray = vector<vector<Point> >(1);
//			hullArray[0] = features.hull;
//	        drawContours(drawing, hullArray, 0, color, 1);
//
//	        // Draw rectangular bouding box
//	        RotatedRect rect = minAreaRect(contours[i]);
//	        Point2f pts[4];
//	        rect.points(pts);
//			for( int j = 0; j < 4; j++)
//				line(drawing, pts[j], pts[(j+1)%4], color, 1, 8);
			objects++;
		}
	}

	// Plot in matlab
	fprintf(file, "%s = [ ...\n", matlabLabel.c_str());
	for(int i = 0; i < objects; i++)
	{
		features_t features = FeaturesArray[i];
		fprintf(file, "%f %f %f %f %f %d; ...\n", features.eccentricity, features.rectDensity, features.scaledPerimeter, features.hullDensity, features.density, features.shapeNumber);
	}
	fprintf(file, "];\n\n");

	imshow("Thresholded", img_th);
}

int main( int argc, char** argv )
{
	Mat drawing = Mat::zeros(Size(ORGANIZATION_SPACE_WIDTH, ORGANIZATION_SPACE_HEIGHT), CV_8UC3);

	int classCount[MAX_OBJECT_CLASSES];
	for(int i = 0; i < MAX_OBJECT_CLASSES; i++)
		classCount[i] = 0; //Apparently you can't rely on C++ setting arrays to zero.

	FILE* file = fopen("plotFeatures.m", "w");
	processImage("shapes.pgm", "Ashapes.png", "shapes", true, file, classCount, drawing);
	processImage("testshapes.pgm", "Atestshapes.png", "testshapes", true, file, classCount, drawing);
	processImage("train1.pgm", "Atrain1.png", "train1", false, file, classCount, drawing);
	processImage("train2.pgm", "Atrain2.png", "train2", false, file, classCount, drawing);
	processImage("match1.pgm", "Amatch1.png", "match1", false, file, classCount, drawing);
	processImage("match2.pgm", "Amatch2.png", "match2", false, file, classCount, drawing);
	fclose(file);

	Mat resizedImage;
	resize(drawing, resizedImage, Size(drawing.cols / 3, drawing.rows / 3));
	imshow("Contours", resizedImage);
	imwrite("result.png", drawing);

	while(true)
	{
		char c = waitKey(0);
		if(c == ESC)
			break;
	}
	return 0;
}

