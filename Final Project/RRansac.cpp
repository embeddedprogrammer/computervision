#include <cv.h>
#include <highgui.h>

using namespace cv;

#define ENTER 10
#define ESC 27

int main( int argc, char** argv )
{
	if ( argc != 3 )
	{
		printf("usage: RRansac <Image_Path> <Image_Path>\n");
		return -1;
	}

	Mat frame1 = imread(argv[1], 1);
	Mat frame2 = imread(argv[2], 1);
	Mat diff = 128 + frame2/2 - frame1/2;
	Mat th;
	int minVal = 128 - 2;
	int maxVal = 128 + 2;
	inRange(diff, Scalar(minVal, minVal, minVal), Scalar(maxVal, maxVal, maxVal), th);
	th = 255 - th;

	imshow("frame1", frame1);
	imshow("frame2", frame1);
	imshow("diff", diff);
	imshow("th", th);
	while(true)
	{
		char c = waitKey(0);
		if(c == ESC)
			break;
	}
	return 0;
}
