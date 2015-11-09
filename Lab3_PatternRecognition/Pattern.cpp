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
	Scalar c[] = {c0, c1, c2, c3, c4, c5, c6, c7, c8};
	if(i <= 8)
		return c[i];
	else
		return Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
}

int getColorNumber(Vec3b v)
{
	for(int i = 0; i < 8; i++)
	{
		Scalar c = getColor(i);
		if(c.val[0] == v[0] && c.val[1] == v[1] && c.val[2] == v[2])
			return i;
	}
	return -1;
}

char* getShape(int i)
{
	char* c[] = {"Rectangle", "Triangle", "Circle", "Millstone", "Oval"};
	if(i <= 4)
		return c[i];
	else
		return "";
}

typedef struct
{
	double scaledPerimeter;
	double eccentricity;
	Point center;
	char* shape;
} features_t;

features_t getContourFeatures(vector<vector<Point> > contours, vector<Vec4i> hierarchy, Mat ans, int i)
{
	Moments mm = moments((Mat)contours[i]);
	double m00 = mm.m00;
	double m10 = mm.m10;
	double m01 = mm.m01;
	double m20 = mm.m20;
	double m02 = mm.m02;
	if(hierarchy[i][2] != -1) //Subtract hole from mass.
	{
		mm = moments((Mat)contours[hierarchy[i][2]]);
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
	double eccentricity = sqrt((sigma_x > sigma_y) ? (sigma_x / sigma_y) : (sigma_y / sigma_x));
	double perimeter = arcLength((Mat)contours[i], true);
	features_t features;
	features.scaledPerimeter = perimeter / sqrt(mass);
	features.eccentricity = eccentricity;
	features.center = Point(centerX, centerY);
	features.shape = getShape(getColorNumber(ans.at<Vec3b>(features.center.y, features.center.x)));
	return features;
}

int main( int argc, char** argv )
{
	if (argc != 3)
	{
		printf("usage: Pattern <Image_Path> <Image ans>\n");
		return -1;
	}

	// Read image
	Mat img, ans, img_th, img_cnc;
	img = imread(argv[1], 0);
	ans = imread(argv[2]);

	// Threshold image
	inRange(img, 0, 100, img_th);

	// Find contours
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	Mat img_copy = img_th.clone();
	findContours(img_copy, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

	/// Draw contours and plot center of mass
	Mat drawing = Mat::zeros(img.size(), CV_8UC3);
	int objects = 0;
	features_t FeaturesArray[20];
	for(int i = 0; i< contours.size(); i++)
	{
		if(hierarchy[i][3] == -1) //hierarchy[idx][{0,1,2,3}]={next contour (same level), previous contour (same level), child contour, parent contour}
		{
			Scalar color = getColor(objects);
			drawContours(drawing, contours, i, color, 1, 8, hierarchy, 0, Point());
			features_t features = getContourFeatures(contours, hierarchy, ans, i);
			FeaturesArray[objects] = features;
			printf("Object %d Scaled perimeter %.3f Eccentricity %.2f Shape: %s\n", objects, features.scaledPerimeter, features.eccentricity, features.shape);
			circle(drawing, features.center, 2, color);
			objects++;
		}
	}

	imshow("Thresholded", img_th);
	imshow("Contours", drawing);
	while(true)
	{
		char c = waitKey(0);
		if(c == ESC)
			break;
	}
	return 0;
}

