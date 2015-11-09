/*
 ============================================================================
 Name        : CVLab1.c
 Author      : Jacob White
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "ImageLibrary.h"
#include "math.h"

void blocks()
{
	Image i = readImage("blocks.pgm");
	GradientAndEdges result = findGradientAndEdges(i, 3, 1, 1, 7);
}


typedef struct
{
	int x, y;
} Point;

Point findBrightestPoint(Image i)
{
	int maxPixelValue = 0;
	Point maxPixelLocation = (Point){0, 0};
	for (int y = 0; y < i.h; y++)
		for (int x = 0; x < i.w; x++)
			if(getPixel(i, x, y) > maxPixelValue && getPixel(i, x, y) != 255)
			{
				maxPixelValue = getPixel(i, x, y);
				maxPixelLocation = (Point){x, y};
			}
	return maxPixelLocation;
}

void eraseSection(Image i, Image eraser, int locationX, int locationY)
{
	int xCenter = eraser.w / 2; //0,0 assumed to be in center
	int yCenter = eraser.h / 2;
	for (int y = 0; y < eraser.h; y++)
		for (int x = 0; x < eraser.w; x++)
			if(getPixel(eraser, x, y))
				setPixel(i, x - xCenter + locationX, y - yCenter + locationY, 0);
}

void findCircles(GradientAndEdges gradientAndEdges, Image result, int radius, int count, int bufferSize, int eraserSize, int degreeSweep)
{
	Image accumulatorArray = houghTransform(gradientAndEdges.edges, gradientAndEdges.grad.dir, bufferSize, radius, degreeSweep);

	printf("Circles of radius %d: ", radius);

	for(int n = 0; n < count; n++)
	{
		// Find the brightest point
		Point location = findBrightestPoint(accumulatorArray);

		// Since there may be nearby bright points, erase them so we don't pick the same circle more than once
		eraseSection(accumulatorArray, diamondStructuringElement(eraserSize), location.x, location.y);

		// Mark the center of the circle
		setPixel(accumulatorArray, location.x, location.y, 255);

		// Draw the circle
		drawCircle(result, location.x, location.y, radius, 255);

		printf("(%d, %d), ", location.x - bufferSize, location.y - bufferSize);
	}
	printf("\n");

	writeImage(accumulatorArray, "brightest.pgm");
}


void circles()
{
	// Find edges
	Image i = readImage("circles.ppm");
	GradientAndEdges gradientAndEdges = findGradientAndEdges(i, 5, 5, 3, 20);

	// Create buffer
	int bufferSize = 50;
	Image result = newImage(1, i.w + bufferSize * 2, i.h + bufferSize * 2);
	clearImage(result);

	// Find circles
	findCircles(gradientAndEdges, result, 32, 12, bufferSize, 15, 10);
	findCircles(gradientAndEdges, result, 16, 5, bufferSize, 15, 10);
	findCircles(gradientAndEdges, result, 48, 3, bufferSize, 51, 20);

	// Show result
	writeImage(result, "result.pgm");
	return;
}

void square()
{
	Image i = readImage("2D_White_Box.pgm");
	GradientAndEdges result = findGradientAndEdges(i, 5, 5, 3, 20);

	//Calculate Hough transform
	houghTransform(result.edges, result.grad.dir, 20, 32, 10);
}

int main(void)
{
	//square();
	//blocks();
	circles();

	freeImages();
	return 0;
}
