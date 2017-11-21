#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>

#define RAYON_FAST 3
#define SEUIL 50
#define TAILLE 16
#define SEUIL_SQUARE 1000
#define DELTA_SQUARE 3

using namespace std;
using namespace cv;

vector<Point2i> MY_FAST(Mat imageIn) {
	int parcourX [] = {-3,-3,-2,-1,0,1,2,3,3,3,2,1,0,-1,-2,-3};
    int parcourY [] = {0,1,2,3,3,3,2,1,0,-1,-2,-3,-3,-3,-2,-1};

    vector <Point2i> solution;

    for( int x = RAYON_FAST; x < imageIn.rows - RAYON_FAST; x++ ) {
		for( int y = RAYON_FAST; y < imageIn.cols - RAYON_FAST; y++ ) {

			int cercles[TAILLE];
			for( int i = 0; i < TAILLE; i++ ) {
				int posX = x + parcourX[i];
				int posY = y + parcourY[i];

				
				int val = imageIn.at<uchar>(x, y) - imageIn.at<uchar>(posX, posY);
				if (val <= SEUIL && val >= -SEUIL) {
					cercles[i] = 0;
				}
				else if(val > SEUIL) {
					cercles[i] = 1;
				}
				else {
					cercles[i] = -1;
				}
			}
			int valPixM1 = 0;
			int valPix1 = 0;
			bool cornerFound = false;
			for( int i = 0; i < TAILLE + 10 && !cornerFound ; i++ ) {
				switch (cercles[i%TAILLE]) {
					case -1:
						valPixM1++;
						valPix1 = 0;
						break;
					case 1:
						valPix1++;
						valPixM1 = 0;
						break;
					default :
						valPixM1 = 0;
						valPix1 = 0;
						break;
				}
				if (valPixM1 >= 11 || valPix1 >= 11 ) {
					cornerFound = true;
				}
			}
			if (cornerFound) {
				solution.push_back(Point2i(y,x));

			}
		}
	}
	return solution;
}

Point2i pointMatch(Mat imageIn1, Mat imageIn2, Point2i pointOriginal, vector<Point2i> vCompared) {
	Mat squareOriginal = imageIn1( Rect(pointOriginal.x, pointOriginal.y, DELTA_SQUARE, DELTA_SQUARE) );
	Point2i pointMatch;
	int min = SEUIL_SQUARE;
	bool matchFound = false;
	//On parcourt les points de contours de l'image 2
	for(unsigned int j = 0; j < vCompared.size(); j++) {
		Mat squareCompared = imageIn2( Rect(vCompared[j].x, vCompared[j].y, DELTA_SQUARE, DELTA_SQUARE) );
		int sum = 0;
		//On les compares
		for( int x = 0; x < squareOriginal.rows; x++ ) {
			for( int y = 0; y < squareOriginal.cols; y++ ) {
				// sum += abs((int)squareOriginal.at<uchar>(x, y) - (int) squareCompared.at<uchar>(x, y));
				sum += pow(squareOriginal.at<uchar>(x, y) - squareCompared.at<uchar>(x, y), 2);
			}
		}
		if (sum < SEUIL_SQUARE) {
			if (sum < min) {
				min = sum;
				pointMatch = vCompared[j];
				matchFound = true;
			}
		}
	}
	if (matchFound) {
		return pointMatch;
	}
	return Point2i(0, 0);
}

int main(int argc, char** argv){
	
	Mat imageIn1 = imread( argv[1], CV_LOAD_IMAGE_GRAYSCALE );
	Mat imageIn2 = imread( argv[2], CV_LOAD_IMAGE_GRAYSCALE );

	Mat imageCorners = imread( argv[1], CV_LOAD_IMAGE_COLOR );

	int width = imageIn1.cols + imageIn2.cols;
	int decalage = imageIn1.cols;

	Mat imageOut = Mat::zeros(Size(width, imageIn1.rows), imageCorners.type());

	if(! imageIn1.data ) {
		cout <<  "Could not open or find the image" << endl ;
		return -1;
   	}
	else {

		//On crée un image contenant les 2 images
		hconcat(imageIn1, imageIn2, imageOut);
		cvtColor(imageOut, imageOut, CV_GRAY2RGB);

		vector<Point2i> v1 = MY_FAST(imageIn1);
		// vector<Point2i> v1 = FAST(imageIn1, v1, 1, false);
		cout <<  "Nb points : " << v1.size() << endl;
		vector<Point2i> v2  = MY_FAST(imageIn2);
		// vector<Point2i> v2 = FAST(imageIn2, v2,1,false);

		//On parcourt les points de contours de l'image 1
		for(unsigned int i = 0; i < v1.size(); i++) {
			circle(imageOut, v1[i], 3, Scalar(0, 0, 255));

			Point2i point = pointMatch(imageIn1, imageIn2, v1[i], v2);
			if (point.x != 0 && point.y != 0) {
				// cout <<  "Match: " << v1[i] << point  <<endl;
				arrowedLine(imageOut, v1[i] , Point2i(decalage + point.x,  point.y), Scalar(0, 255, 0), 1, 8, 0, 0.1);
			}
			


			// Mat squareOriginal = imageIn1( Rect(v1[i].x, v1[i].y, DELTA_SQUARE, DELTA_SQUARE) );
			// Point2i pointMatch;
			// int min = SEUIL_SQUARE;
			// bool matchFound = false;
			// //On parcourt les points de contours de l'image 2
			// for(unsigned int j = 0; j < v2.size(); j++) {
			// 	Mat squareCompared = imageIn2( Rect(v2[j].x, v2[j].y, DELTA_SQUARE, DELTA_SQUARE) );
			// 	int sum = 0;
			// 	//On les compares
			// 	for( int x = 0; x < squareOriginal.rows; x++ ) {
			// 		for( int y = 0; y < squareOriginal.cols; y++ ) {
			// 			// sum += abs((int)squareOriginal.at<uchar>(x, y) - (int) squareCompared.at<uchar>(x, y));
			// 			sum += pow(squareOriginal.at<uchar>(x, y) - squareCompared.at<uchar>(x, y), 2);
			// 		}
			// 	}
			// 	if (sum < SEUIL_SQUARE) {
			// 		if (sum < min) {
			// 			min = sum;
			// 			pointMatch = v2[j];
			// 			matchFound = true;
			// 		}
			// 	}
			// }
			// if (matchFound) {
			// 	cout <<  "Match: " << v1[i] << pointMatch << " Sum " << min <<endl;
			// 	arrowedLine(imageOut, v1[i] , Point2i(decalage + pointMatch.x,  pointMatch.y), Scalar(0, 255, 0), 1, 8, 0, 0.1);
			// }

			
		}

		for(unsigned int i = 0; i < v2.size(); i++) {
			circle(imageOut, Point2i(decalage + v2[i].x,  v2[i].y), 3, Scalar(0, 0, 255));
		}

		imshow( "Corners", imageOut );       
		// imshow( "Corners", roi );       

		waitKey(0); 

	} 
	
	return 0;
}

