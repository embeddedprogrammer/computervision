#include <cv.h>
#include <highgui.h>

using namespace cv;

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

typedef struct
{
	double normalizedPerimeter;
	double eccentricity;
	double rectangularBoundingBoxDensity;
	double hullDensity;
	double density;
	double assymetry;
	Point center;
	int contourIndex;
} shapeInformation_t;

shapeInformation_t getShapeInformation(vector<vector<Point> > contours, vector<Vec4i> hierarchy, int i)
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
	shapeInformation_t shapeInformation;
	shapeInformation.normalizedPerimeter = perimeter / sqrt(outsideContourMass);
	shapeInformation.center = Point(centerX, centerY);
	shapeInformation.density = mass / outsideContourMass;

	// Find the convex hull object
	vector<Point> hull;
	convexHull(Mat(contours[i]), hull);
	shapeInformation.hullDensity = outsideContourMass / moments((Mat)hull).m00;

	// Find the rectangle bounding box
    RotatedRect rect = minAreaRect(contours[i]);
	shapeInformation.rectangularBoundingBoxDensity = moments((Mat)hull).m00 / rect.size.area();
	double eccentricity = (rect.size.width > rect.size.height) ? (rect.size.width / rect.size.height) : (rect.size.height / rect.size.width);
	shapeInformation.eccentricity = eccentricity;

	mm = moments((Mat)hull);
	Point2f hullCenter = Point2f(mm.m10 / mm.m00, mm.m01 / mm.m00);
	Point2f rectCenter = rect.center;
	float dx = hullCenter.x - rectCenter.x;
	float dy = hullCenter.y - rectCenter.y;
	shapeInformation.assymetry = sqrt(dx*dx + dy*dy) / sqrt(rect.size.width * rect.size.height);

	return shapeInformation;
}

int getAllShapesInformation(string imageFileName, Mat &thresholdedImage, vector<vector<Point> > &contours, vector<Vec4i> &hierarchy, shapeInformation_t shapeInformationArray[], bool invert)
{
	// Read image
	Mat img;
	img = imread(imageFileName, 0);

	// Threshold image
	if(invert)
		inRange(img, 0, 100, thresholdedImage);
	else
		inRange(img, 101, 255, thresholdedImage);

	// Find contours
	Mat img_copy = thresholdedImage.clone();
	findContours(img_copy, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

	// Draw contours and plot center of mass
	int objects = 0;
	for(int i = 0; i < contours.size(); i++)
	{
		if(hierarchy[i][3] == -1) // Only use objects, not holes
		{
			shapeInformationArray[objects] = getShapeInformation(contours, hierarchy, i);
			shapeInformationArray[objects].contourIndex = i;
			objects++;
		}
	}
	return objects;
}


#define NUM_FEATURES 3

void getFeatures(shapeInformation_t shapeInformationArray[], double featureArray[][NUM_FEATURES], int numObjects)
{
	for(int i = 0; i < numObjects; i++)
	{
//		featureArray[i][0] = shapeInformationArray[i].normalizedPerimeter;
//		featureArray[i][1] = shapeInformationArray[i].density;
		featureArray[i][0] = shapeInformationArray[i].rectangularBoundingBoxDensity;
		featureArray[i][1] = shapeInformationArray[i].hullDensity;
		featureArray[i][2] = shapeInformationArray[i].density;
	}
}

void calcW(double featureArray[][NUM_FEATURES], double W[], int numObjects)
{
	double minVal[NUM_FEATURES];
	double maxVal[NUM_FEATURES];
	for(int i = 0; i < numObjects; i++)
		for(int j = 0; j < NUM_FEATURES; j++)
		{
			double val = featureArray[i][j];
			if(val < minVal[j] || i == 0)
				minVal[j] = val;
			if(val > maxVal[j] || i == 0)
				maxVal[j] = val;
		}
	for(int j = 0; j < NUM_FEATURES; j++)
	{
		W[j] = 1 / (maxVal[j] - minVal[j]);
	}
}

#define CLUSTER_DIM_WIDTH (NUM_FEATURES + 1)

void printCluster(double clusters[20][CLUSTER_DIM_WIDTH], int numClusters)
{
	for(int i = 0; i < numClusters; i++)
	{
		printf("Cluster %2d: ", i);
		for(int j = 0; j < CLUSTER_DIM_WIDTH; j++)
		{
			printf("%10f", clusters[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

void findClusters(double featureArray[][NUM_FEATURES], double W[NUM_FEATURES], double clusters[20][CLUSTER_DIM_WIDTH], int numObjects, int maxNumClusters)
{
	//Initialize clusters
	int numClusters = numObjects;
	for(int i = 0; i < numObjects; i++)
	{
		for(int j = 0; j < NUM_FEATURES; j++)
			clusters[i][j] = featureArray[i][j];
		clusters[i][NUM_FEATURES] = 1; //mass of cluster
	}

	while(numClusters > maxNumClusters) //repeat until only 5 clusters remain.
	{
		// find minimum distance between any two clusters
		double minDist = -1;
		int cluster1;
		int cluster2;
		for(int i = 0; i < numClusters; i++)
		{
			for(int j = i + 1; j < numClusters; j++)
			{
				double sum = 0;
				for(int k = 0; k < NUM_FEATURES; k++)
				{
					double diff = (clusters[i][k] - clusters[j][k]) * W[k];
					sum += diff*diff;
				}
				double dist = sqrt(sum);
				if(dist < minDist || minDist == -1)
				{
					minDist = dist;
					cluster1 = i;
					cluster2 = j;
				}
			}
		}

		// Merge clusters into a new cluster and replace the first cluster with it.
		double mass1 = clusters[cluster1][NUM_FEATURES];
		double mass2 = clusters[cluster2][NUM_FEATURES];
		for(int k = 0; k < NUM_FEATURES; k++)
		{
			//Combine features, weighting them by the mass of each.
			clusters[cluster1][k] = (clusters[cluster1][k]*mass1 + clusters[cluster2][k]*mass2) / (mass1 + mass2);
		}
		clusters[cluster1][NUM_FEATURES] = mass1 + mass2;

		// Remove the second cluster by replacing the second cluster with the last cluster.
		for(int k = 0; k < CLUSTER_DIM_WIDTH; k++)
			clusters[cluster2][k] = clusters[numClusters - 1][k];
		numClusters--;
	}
}

void minimumDistanceClassifier(double featureArray[][NUM_FEATURES], double W[NUM_FEATURES], double clusters[][CLUSTER_DIM_WIDTH], int classAssignment[], int numObjects, int numClusters)
{
	for(int i = 0; i < numObjects; i++)
	{
		double maxLinearDiscriminent;
		for(int j = 0; j < numClusters; j++)
		{
			double linearDiscriminent = 0;
			for(int k = 0; k < NUM_FEATURES; k++)
				linearDiscriminent += (featureArray[i][k]*clusters[j][k] - clusters[j][k]*clusters[j][k]/2)*W[k];
			if(linearDiscriminent > maxLinearDiscriminent || j == 0)
			{
				maxLinearDiscriminent = linearDiscriminent;
				classAssignment[i] = j;
			}
		}
	}
}

void drawResults(Mat &drawing, vector<vector<Point> > contours, vector<Vec4i> hierarchy, shapeInformation_t shapeInformationArray[], int classAssignment[], int numObjects)
{
	for(int i = 0; i < numObjects; i++)
	{
		int contourIndex = shapeInformationArray[i].contourIndex;
		drawContours(drawing, contours, contourIndex, getColor(classAssignment[i]), CV_FILLED, 8, hierarchy, 1, Point());
	}
}

#define MAX_SHAPES 20

void trainAndMatch(string trainingImageFilename, string matchImageFilename, bool invert)
{
//Train
	//Extract features from image
	vector<Vec4i> hierarchy;
	vector<vector<Point> > contours;
	shapeInformation_t shapeInformationArray[20];
	double featureArray[MAX_SHAPES][NUM_FEATURES];
	Mat thresholdedImage;
	int numObjects = getAllShapesInformation(trainingImageFilename, thresholdedImage, contours, hierarchy, shapeInformationArray, invert);

	getFeatures(shapeInformationArray, featureArray, numObjects);

	//Find clusters
	double W[NUM_FEATURES];
	calcW(featureArray, W, numObjects);
	double clusters[MAX_SHAPES][CLUSTER_DIM_WIDTH];
	int maxNumClusters = 5;
	findClusters(featureArray, W, clusters, numObjects, maxNumClusters);

	// Classify using minimum distance classifier
	int classes[MAX_SHAPES];
	minimumDistanceClassifier(featureArray, W, clusters, classes, numObjects, maxNumClusters);

	// Show results
	Mat result = Mat::zeros(thresholdedImage.size(), CV_8UC3);
	drawResults(result, contours, hierarchy, shapeInformationArray, classes, numObjects);
	imshow("Training Image", result);
	imwrite("train.png", result);

//Match
	// Extract features from image
	numObjects = getAllShapesInformation(matchImageFilename, thresholdedImage, contours, hierarchy, shapeInformationArray, invert);
	getFeatures(shapeInformationArray, featureArray, numObjects);

	// Classify using minimum distance classifier
	minimumDistanceClassifier(featureArray, W, clusters, classes, numObjects, maxNumClusters);

	// Show results
	result = Mat::zeros(thresholdedImage.size(), CV_8UC3);
	drawResults(result, contours, hierarchy, shapeInformationArray, classes, numObjects);
	imshow("Match image", result);
	imwrite("match.png", result);
}

int main( int argc, char** argv )
{
//	trainAndMatch("shapes.pgm", "testshapes.pgm", true);
	trainAndMatch("train1.pgm", "match1.pgm", false);
//	trainAndMatch("train2.pgm", "match2.pgm", false);

	waitKey(0);
	return 0;
}

