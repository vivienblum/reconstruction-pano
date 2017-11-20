#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>

#define RAYON_FAST 3
#define SEUIL 60
#define TAILLE 16

using namespace std;
using namespace cv;

void showCorners() {
	// return y - pente*x;
}

Mat FAST(Mat image, int rayon = 3) {
	Mat imageOut;
	for( int y = 0; y < image.rows; y++ ) {
		for( int x = 0; x < image.cols; x++ ) {
			vector <int>(16, 0);
		}
	}	
	return imageOut;
}

int main(int argc, char** argv){
	
	Mat imageIn = imread( argv[1], CV_LOAD_IMAGE_GRAYSCALE );
	//Mat imageOut = Mat::zeros(imageIn.size(), imageIn.type());
	Mat imageOut = imread( argv[1], CV_LOAD_IMAGE_COLOR );
	int parcourX [] = {-3,-3,-2,-1,0,1,2,3,3,3,2,1,0,-1,-2,-3};
    int parcourY [] = {0,1,2,3,3,3,2,1,0,-1,-2,-3,-3,-3,-2,-1};
	
	vector <Point2i> solution;
	if(! imageIn.data ) {
		cout <<  "Could not open or find the image" << endl ;
		return -1;
   	}
	else {
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
					circle(imageOut, solution.back(), 3, Scalar(0,0,255));

				}

				// 
			}
		}	

		cout <<  "nb points : " << solution.size() << endl ;

		imshow( "Corners", imageOut );       

		waitKey(0); 

	} 
	
	return 0;
}

