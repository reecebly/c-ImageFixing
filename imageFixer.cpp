
#include <iostream>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <string>
#include <stdlib.h>

using namespace cv;
using namespace std;

Mat imgBlue, imgBlur, imgDetail, imgDetailNoScratch, imgJustScratch, imgCombine, imgRestore, img, hg, histImg;
int const MAX = 255; // Max Value
double alpha = 0.5; double beta;

Mat imHist(Mat hist, float scaleX = 1, float scaleY = 1) {
	double maxVal = 0;
	minMaxLoc(hist, 0, &maxVal, 0, 0);
	int rows = 64; //default height size
	int cols = hist.rows; //get the width size from the histogram
	Mat histImg = Mat::zeros(rows*scaleX, cols*scaleY, CV_8UC3);
	//for each bin
	for (int i = 0; i<cols - 1; i++) {
		float histValue = hist.at<float>(i, 0);
		float nextValue = hist.at<float>(i + 1, 0);
		Point pt1 = Point(i*scaleX, rows*scaleY);
		Point pt2 = Point(i*scaleX + scaleX, rows*scaleY);
		Point pt3 = Point(i*scaleX + scaleX, (rows - nextValue * rows / maxVal)*scaleY);
		Point pt4 = Point(i*scaleX, (rows - nextValue * rows / maxVal)*scaleY);

		int numPts = 5;
		Point pts[] = { pt1, pt2, pt3, pt4, pt1 };

		fillConvexPoly(histImg, pts, numPts, Scalar(255, 255, 255));
	}
	return histImg;
}

void mouseHandler(int event, int x, int y, int flags, void *param)
{
	Mat imgBlue;
	int thresh = (x / 3), black = 0, white = 0;

	switch (event) {

		// left button down 
		//-------------------------------------------------------------------
	case CV_EVENT_LBUTTONDOWN:

		// Clear screen
		system("CLS");

		// count black and white pixels
		for (int c = 0; c < hg.size().width; c++) {
			for (int r = 0; r < hg.size().height; r++) {

				uchar pixelValue = hg.at<uchar>(r, c);

				if (pixelValue == 255)
					white++;
				else if (pixelValue == 0)
					black++;
			}
		}

		//cout << endl;
		cout << "Black and White pixel values at Threshold: " << thresh << "\n\n";
		cout << "  Black pixels: " << black << endl;
		cout << "  White pixels: " << white << endl;
		break;

		// right button down
		//-------------------------------------------------------------------

	case CV_EVENT_RBUTTONDOWN:

		// Clear screen
		system("CLS");

		// Display cord values
		cout << "Cord value of Histogram (" << x << "," << y << ")" << endl;
		break;

		// mouse move
		//-------------------------------------------------------------------
	case CV_EVENT_MOUSEMOVE:

		imgBlue = histImg.clone(); // Clone histImg


								//system("CLS"); // Clear screen

								// draw a circle
		circle(imgBlue,
			cvPoint(x, y),
			15.0,
			Scalar(45, 245, 36),
			2,
			8);

		string text = "Threshold Value: " + to_string(x / 3); //Threshold Value
															  //cout << "Threshold Value: " << to_string(x / 3) << endl;

		int fontFace = CV_FONT_HERSHEY_SIMPLEX;  //FONT_HERSHEY_SCRIPT_SIMPLEX
		double fontScale = .5;
		int thickness = 2;

		int baseline = 0;
		Size textSize = getTextSize(text, fontFace,
			fontScale, thickness, &baseline);
		baseline += thickness;

		//Text location
		Point textOrg(5, textSize.height);

		// Inserting text
		putText(imgBlue, text, textOrg, fontFace, fontScale,
			Scalar(0, 0, 128), thickness, 8);

		imshow("Blue", imgBlue); // Display image blue

		threshold(img, hg, thresh, MAX, 0); // Apply threshold to img ang place in hg

		imshow("Threshold", hg); // Display image hg

		break;
	}

}

int main(int argc, char** argv) {

	// Variables
	int thresh = 0;		 // Threshold value
	int black = 0, white = 0;	// black and white pixel count

								// check for supplied argument
	if (argc < 2) {
		cout << "Usage: loadimg <filename>\n" << endl;
		return 1;
	}

	img = imread(argv[1], CV_LOAD_IMAGE_COLOR); // load the image
	//cvtColor(img, imgGray, CV_BGR2GRAY); // create grayscale image

										 // always check
	if (img.data == NULL) {
		cout << "Cannot load file " << argv[1] << endl;
		return 1;
	}

	MatND hist; //Hold the histogram

	int nbins = 256; // lets hold 256 levels
	int hsize[] = { nbins }; // just one dimension
	float range[] = { 0, 255 };
	const float *ranges[] = { range };
	int chnls[] = { 0 };

	// create colors channels
	vector<Mat> colors;
	split(img, colors);

	// compute for all colors
	calcHist(&colors[0], 1, chnls, Mat(), hist, 1, hsize, ranges);   // found the scratches in the blue channel with the optimal number at about 214
	histImg = imHist(hist, 3, 3);
	imshow("Blue", histImg);

	// show image
	imshow("Image", img);
	medianBlur(img, imgBlur, 5); //median filtering
	imshow("Blurred", imgBlur);  // now print median blurred image

	subtract(img, imgBlur, imgDetail); //image subtraction img3=img1-img2
	absdiff(img, imgBlur, imgDetail); //get absolute difference img3=img1-img2
	imshow("Detailed", imgDetail);    // print img with greater detail and primarily just the scratches

	threshold(imgDetail, imgJustScratch, 105, MAX, THRESH_BINARY);   // sweet spot to remove scratch and regain as much detail as possible
	imshow("Just Scratch", imgJustScratch);

	subtract(imgDetail, imgJustScratch, imgDetailNoScratch); //image subtraction img3=img1-img2
	imshow("DetailNoScratch", imgDetailNoScratch);
	imwrite("imgDetailNoScratch.png", imgDetailNoScratch);
	imwrite("imgBlur.png", imgBlur);

	beta = (1.0 - alpha);
	addWeighted(imgBlur, 1, imgDetailNoScratch, 1, 0.0, imgRestore);
	imshow("Image Restored", imgRestore);
	
	imwrite("imgRestore.png", imgRestore);


	int key;
	setMouseCallback("Blue", mouseHandler, NULL);

	waitKey(0); // wait until user press a key

				// Memory release 
	img.release();
	imgBlue.release();
	hg.release();
	histImg.release();
	hist.release();

	return 0;
}
