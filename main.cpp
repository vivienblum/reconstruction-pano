#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>

#define SEUIL 50    
#define SEUIL_VOTE 17
#define SEUIL_MAG 10
#define SEUIL_RAYON 10
#define PI 3.14159265
#define DELTA_DROITE 0.2
#define DELTA_CERCLE 50
#define DELTA_CENTRE 3
#define DELTA_RAYON 1
#define MAX_RAYON 300

using namespace std;
using namespace cv;

void showCorners() {
	// return y - pente*x;
}

Mat FAST(InputArray image, int rayon = 3) {
	Mat imageOut;
	for( int y = 0; y < image.rows; y++ ) {
		for( int x = 0; x < image.cols; x++ ) {
			vector <int>(16, 0);
		}
	}	
	return imageOut;
}

int main(int argc, char** argv){
	
	Mat imageLeft = imread( argv[1], CV_LOAD_IMAGE_GRAYSCALE );
	Mat imageRight = imread( argv[2], CV_LOAD_IMAGE_GRAYSCALE );
	
	if(! imageLeft.data || ! imageRight.data ) {
		cout <<  "Could not open or find the image" << std::endl ;
		return -1;
   	}
	else {
		Mat imageOut;
		
		// int height = imageIn.rows;
		// int width = imageIn.cols;
		// int depth = min(imageIn.rows, imageIn.cols);
		
		// vector<vector<vector<int> > > votesCercles (height,vector<vector<int> >(width,vector <int>(depth, 0)));
		// int nbCercles = 0;
		// int nbPixelContour = 0;
		
		// /*Detection de contour*/
		// //Application d'un flou gaussien pour limiter le bruit
		// GaussianBlur( imageIn, imageIn, Size(3,3), 0, 0, BORDER_DEFAULT );
		
		// //Application d'un seuil de gris pour lisser
		// threshold(imageIn, imageIn, SEUIL, 255, THRESH_BINARY);

		// //Apply the laplcaian operator
		// //Laplacian( imageIn, imageOut, CV_16S, 3, 1, 0, BORDER_DEFAULT );
		// //convertScaleAbs( imageOut, imageOut );
		
		// //Filtre de Sobel sur X
		// Sobel(imageIn, imageOutX, CV_32F, 1 , 0 , 3, 1, 0, BORDER_DEFAULT); 
		// convertScaleAbs( imageOutX, absImageOutX );

		// //Filtre de Sobel sur Y
		// Sobel(imageIn, imageOutY, CV_32F, 0 , 1 , 3, 1, 0, BORDER_DEFAULT);
		// convertScaleAbs( imageOutY, absImageOutY );

		// //Addition des 2 matrices pour calculer la magnitude du gradient
		// addWeighted( absImageOutX, 0.5, absImageOutY, 0.5, 0, imageOut ); 
		
		
		
		//Affichage de l'image
		//imshow( "out", imageOut );       
		// imshow( "Cercles", imageOut );       

		waitKey(0); 
	} 
	
	return 0;
}

