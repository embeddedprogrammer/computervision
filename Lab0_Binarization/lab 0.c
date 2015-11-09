#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h> //Use https://www.eclipse.org/forums/index.php?t=msg&th=68204/ to link.

// **************************************** BASIC IMAGE LIBRARY ****************************************

typedef struct
{
	unsigned char r, g, b;
} RGB;

typedef struct
{
	int n;
	int w;
	int h;
	int maxVal;
	unsigned char* data;
} Image;

RGB getPixelRgb(Image i, int x, int y)
{
	return (RGB){
		i.data[(y * i.w + x) * i.n],
		i.data[(y * i.w + x) * i.n + 1],
		i.data[(y * i.w + x) * i.n + 2]};
}

unsigned char convertRgbToGrayscale(RGB val)
{
	return (unsigned char)(.299 * val.r + .587 * val.g + .114 * val.b);
}

unsigned char getPixel(Image i, int x, int y)
{
	if(i.n == 1)
	{
		return i.data[(y * i.w + x) * i.n];
	}
	else // if(i.n == 3)
	{
		return convertRgbToGrayscale(getPixelRgb(i, x, y));
	}
}

void setPixel(Image i, int x, int y, unsigned char val)
{
	i.data[(y * i.w + x) * i.n] = val;
}

void setPixelRgb(Image i, int x, int y, RGB val)
{
	if(i.n == 1)
	{
		i.data[(y * i.w + x) * i.n] = convertRgbToGrayscale(val);
	}
	else if(i.n == 3)
	{
		i.data[(y * i.w + x) * i.n] = val.r;
		i.data[(y * i.w + x) * i.n + 1] = val.g;
		i.data[(y * i.w + x) * i.n + 2] = val.b;
	}
}

unsigned char* allocImage(Image i)
{
	return (unsigned char*) malloc(i.w * i.h * i.n);
}

void freeImage(Image i)
{
	free(i.data);
}

int writeImage(Image i, const char* fileName)
{
	FILE *file = fopen(fileName, "wb");
	if (!file)
	{
		printf("Unable to open file!");
		return 1;
	}

	if(i.n == 1)
		fprintf(file, "P5\n"); //Grayscale
	else if (i.n == 3)
		fprintf(file, "P6\n"); //Raw
	fprintf(file, "%d %d\n", i.w, i.h); //Size
	fprintf(file, "%d\n", i.maxVal); // Max val

	fwrite(i.data, sizeof(unsigned char), i.w * i.h * i.n, file);

	fclose(file);
	return 0;
}

Image readImage(const char* fileName)
{
	Image i;
	FILE *file = fopen(fileName, "rb");
	if (!file)
	{
		printf("Unable to open file!");
		return i;
	}

	char line[200];

	int lineNum = 0;
	while(lineNum < 3)
	{
		fgets(line, 200, file);
		if(lineNum == 0)
		{
			if(strcmp(line, "P5\n") == 0)
				i.n = 1;
			else if(strcmp(line, "P6\n") == 0)
				i.n = 3;
			lineNum++;
		}
		else if(line[0] != '#')
		{
			if(lineNum == 1)
				sscanf(line, "%d %d", &i.w, &i.h);
			else if(lineNum == 2)
				sscanf(line, "%d", &i.maxVal);
			lineNum++;
		}
	}

	i.data = allocImage(i);

	fread(i.data, sizeof(unsigned char), i.w * i.h * i.n, file);
	fclose(file);
	return i;
}

Image newImage(int n, int w, int h)
{
	Image i = (Image){n, w, h, 255, NULL};
	i.data = allocImage(i);
	return i;
}

void clearImage(Image i)
{
	for (int y = 0; y < i.h; y++)
		for (int x = 0; x < i.w; x++)
			setPixel(i, x, y, 0);
}

void randomizeImage(Image i)
{
	for (int y = 0; y < i.h; y++)
		for (int x = 0; x < i.w; x++)
			setPixel(i, x, y, (x + y) % (i.maxVal + 1)); //rand() % 256); //
}

Image invertImage(Image i)
{
	Image newImage = i;
	allocImage(newImage);
	for (int y = 0; y < i.h; y++)
		for (int x = 0; x < i.w; x++)
			setPixel(newImage, x, y, i.maxVal - getPixel(i, x, y));
	return newImage;
}

Image convertToGrayscale(Image i)
{
	if(i.n == 3)
	{
		Image iGray = i;
		iGray.n = 1;
		iGray.data = allocImage(iGray);
		for (int y = 0; y < i.h; y++)
			for (int x = 0; x < i.w; x++)
				setPixel(iGray, x, y, convertRgbToGrayscale(getPixelRgb(i, x, y)));
		return iGray;
	}
	else
	{
		return i;
	}
}

void printPixel(Image i, int x, int y)
{
	if(i.n == 3)
	{
		return;
		RGB val = getPixelRgb(i, x, y);
		printf("<%d, %d, %d>", val.r, val.g, val.b);
	}
	else
	{
		printf("%d", getPixel(i, x, y));
	}
}

void printImage(Image i)
{
	if(i.w * i.h > 1000)
	{
		printf("Image size larger than maximum printing size\n");
		return;
	}
	for (int y = 0; y < i.h; y++)
	{
		for (int x = 0; x < i.w; x++)
		{
			printPixel(i, x, y);
			if(x < i.w - 1)
				printf(", ");
		}
		printf("\n");
	}
	printf("\n");
}

// **************************************** BINARIZATION ALGORITHMS ****************************************

Image thresholdImage(Image i, unsigned char th)
{
	Image newImage = i;
	newImage.data = allocImage(newImage);
	for (int y = 0; y < i.h; y++)
		for (int x = 0; x < i.w; x++)
			setPixel(newImage, x, y, (getPixel(i, x, y) >= th) ? 1 : 0);
	newImage.maxVal = 1;
	return newImage;
}

void computeHistogram(Image i, int* hist)
{
	for (int h = 0; h <= i.maxVal; h++)
		hist[h] = 0;
	for (int y = 0; y < i.h; y++)
		for (int x = 0; x < i.w; x++)
			hist[getPixel(i, x, y)]++;
}

int otsu(Image i)
{
	int hist[256];
	computeHistogram(i, hist);

	 // Set up initial threshold
	int threshold = 0;
	int count1 = 0;
	int count2 = i.w * i.h;

	int sum1 = 0;
	int sum2 = 0;

	for(int h = 0; h <= i.maxVal; h++)
		sum2 += h * hist[h];

	float maxVar = 0;
	int maxVarThreshold = 0;

	// Iterate to find maximum variance
	while(threshold <= i.maxVal)
	{
		float w1 = (float)count1 / (count1 + count2);
		float w2 = (float)count2 / (count1 + count2);

		float u1 = (count1 != 0) ? ((float)sum1 / count1) : 0;
		float u2 = (count2 != 0) ? ((float)sum2 / count2) : i.maxVal;

		float betweenClassVar = w1 * w2 * (u1 - u2) * (u1 - u2);

		if(betweenClassVar > maxVar)
		{
			maxVar = betweenClassVar;
			maxVarThreshold = threshold;
		}

		// Move threshold and compute new count and sum.
		count1 += hist[threshold];
		count2 -= hist[threshold];
		sum1 += threshold * hist[threshold];
		sum2 -= threshold * hist[threshold];
		threshold++;
	}
	return maxVarThreshold;
}


Image special(Image i, int winSize, double K, bool useSauvola, double R)
{
	Image newImage = i;
	newImage.data = allocImage(newImage);

	for (int y = 0; y < i.h; y++)
	{
		for (int x = 0; x < i.w; x++)
		{
//			printf("For pixel (%d, %d): ", x, y);
			int minj = (x - winSize > 0)       ? (x - winSize) : 0;
			int maxj = (x + winSize < i.w - 1) ? (x + winSize) : i.w - 1;
			int mink = (y - winSize > 0)       ? (y - winSize) : 0;
			int maxk = (y + winSize < i.h - 1) ? (y + winSize) : i.h - 1;
			int count = (maxj - minj + 1) * (maxk - mink + 1);
			int sum = 0;   // 1st moment (mean) of pixels in window.
			int sum_2 = 0; // 2nd moment (allows for more easy calculation of standard deviation).

			for (int k = mink; k <= maxk; k++)
				for (int j = minj; j <= maxj; j++)
				{
					int pixel = getPixel(i, j, k);
//					printf("%d ", pixel);

					sum += pixel;
					sum_2 += (pixel * pixel);
				}
			double mean = (double)sum / count;
			double mean_2 = (double)sum_2 / count;
			double sigma = sqrt(mean_2 - mean * mean);


			double th = 0;
			if(useSauvola)
				th = mean * ((double)1 + K * (sigma / R - 1)); //sauvola
			else
				th = mean + K * sigma; //niblack

			setPixel(newImage, x, y, (getPixel(i, x, y) >= th) ? 1 : 0);
//			printf("\nmean: %.2f mean^2: %.2f stdDev: %.2f Th: %.2f Px: %d Rs: %d\n", mean, mean_2, sigma, th, getPixel(i, x, y), getPixel(newImage, x, y));
		}
	}

	newImage.maxVal = 1;
	return newImage;
}

Image niblack(Image i, int winSize, double K)
{
	return special(i, winSize, K, false, 0);
}

Image sauvola(Image i, int winSize, double K, double R)
{
	return special(i, winSize, K, true, R);
}

// **************************************** MORPHOLOGICAL OPERATORS ****************************************

// Assumed to be binary images
Image dilate(Image a, Image b)
{
	Image newImage = a;
	newImage.w = a.w + b.w - 1;
	newImage.h = a.h + b.h - 1;
	newImage.data = allocImage(newImage);
	clearImage(newImage);

	for (int y = 0; y < a.h; y++)
		for (int x = 0; x < a.w; x++)
			if(getPixel(a, x, y))
				for (int k = 0; k < b.h; k++)
					for (int j = 0; j < b.w; j++)
						if(getPixel(b, j, k))
							setPixel(newImage, x + j, y + k, 1);
	return newImage;
}

// Assumed to be binary images
Image erode(Image a, Image b)
{
	Image newImage = a;
	newImage.w = a.w - b.w + 1;
	newImage.h = a.h - b.h + 1;
	newImage.data = allocImage(newImage);

	for (int y = 0; y < newImage.h; y++)
		for (int x = 0; x < newImage.w; x++)
		{
			unsigned char newPixel = 1;
			for (int k = 0; k < b.h && newPixel; k++)
				for (int j = 0; j < b.w && newPixel; j++)
					if(getPixel(b, j, k) && !getPixel(a, x + j, y + k))
						newPixel = 0; //fall out of loop
			setPixel(newImage, x, y, newPixel);
		}
	return newImage;
}

Image open(Image a, Image b)
{
	Image eroded = erode(a, b);
	Image newImage = dilate(eroded, b);
	freeImage(eroded);
	return newImage;
}

Image close(Image a, Image b)
{
	Image dilated = dilate(a, b);
	Image newImage = erode(dilated, b);
	freeImage(dilated);
	return newImage;
}

// **************************************** TESTING ****************************************

void testOtsu()
{
	Image i = readImage("Scan_half_crop_norm_009_small.pgm");
	Image o = thresholdImage(i, otsu(i));
	writeImage(o, "otsu.pgm");
}

void testNiblack() //FYI this takes about 5-10 minutes to run because it is not optimized.
{
	Image i = readImage("Scan_half_crop_norm_009_small.pgm");
	Image n5 = niblack(i, 5, -.2);
	Image n10 = niblack(i, 10, -.2);
//	Image n30 = niblack(i, 30, -.2);
	writeImage(n5, "niblack_5.pgm");
	writeImage(n10, "niblack_10.pgm");
//	writeImage(n30, "niblack_30.pgm");
	freeImage(i);
	freeImage(n5);
	freeImage(n10);
//	freeImage(n30);
}

void testSauvola() //FYI this takes about 5-10 minutes to run because it is not optimized.
{
	Image i = readImage("Scan_half_crop_norm_009_small.pgm");
	Image s3 = sauvola(i, 3, .5, 128);
	Image s5 = sauvola(i, 5, .5, 128);
	Image s10 = sauvola(i, 10, .5, 128);
//	Image s30 = sauvola(i, 30, .5, 128);
	writeImage(s3, "sauvola_3.pgm");
	writeImage(s5, "sauvola_5.pgm");
	writeImage(s10, "sauvola_10.pgm");
//	writeImage(s30, "sauvola_30.pgm");
	freeImage(s3);
	freeImage(s5);
	freeImage(s10);
//	freeImage(s30);
	freeImage(i);
}

void testMorph()
{
	Image i = readImage("Scan_half_crop_norm_009_small.pgm");
	Image a = thresholdImage(i, otsu(i));

	unsigned char data2[] = {
			0, 1, 0,
			1, 1, 1,
			0, 1, 0};
	Image b = (Image){1, 3, 3, 1, data2};

	Image dilateImage = dilate(a, b);
	Image erodeImage = erode(a, b);
	Image openImage = open(a, b);
	Image closeImage = close(a, b);

	writeImage(a, "threshold.pgm");
	writeImage(dilateImage, "dilate.pgm");
	writeImage(erodeImage, "erode.pgm");
	writeImage(openImage, "open.pgm");
	writeImage(closeImage, "close.pgm");

	freeImage(i);
	freeImage(a);
	freeImage(b);
	freeImage(dilateImage);
	freeImage(erodeImage);
	freeImage(openImage);
	freeImage(closeImage);
}

// **************************************** MAIN ****************************************

int main(void)
{
	testOtsu();
	testSauvola(); //Note: The Sauvola and Niblack functions take about 5-10 minutes to run.
	testNiblack();
	testMorph();
}
