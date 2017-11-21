#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>

#define RAYON_FAST 5
#define SEUIL 70
#define TAILLE 16
#define SEUIL_SQUARE 1500
#define DELTA_SQUARE 5

using namespace std;
using namespace cv;

/* Fonction pour récupérer les points de corner */
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

/* Fonction pour calculer la moyenne d'une matrice */
float averageMat(Mat matrix) {
	int sum = 0;
	for( int x = 0; x < matrix.rows; x++ ) {
		for( int y = 0; y < matrix.cols; y++ ) {
			sum += matrix.at<uchar>(x, y);
		}
	}
	return sum/(matrix.rows*matrix.cols);
}

/* Fonction pour trouver le point qui match le mieux */
Point2i pointMatch(Mat imageIn1, Mat imageIn2, Point2i pointOriginal, vector<Point2i> vCompared) {
	// On récupère les voisins proche du points
	Mat squareOriginal = imageIn1( Rect(pointOriginal.x, pointOriginal.y, DELTA_SQUARE, DELTA_SQUARE) );
	// On récupère la moyenne de la matrice 
	int mu1 = averageMat(squareOriginal);

	Point2i pointMatch;
	int min = SEUIL_SQUARE;
	bool matchFound = false;
	//On parcourt les points de contours de l'image 2
	for(unsigned int j = 0; j < vCompared.size(); j++) {
		Mat squareCompared = imageIn2( Rect(vCompared[j].x, vCompared[j].y, DELTA_SQUARE, DELTA_SQUARE) );
		int mu2 = averageMat(squareCompared);
		int sum = 0;
		//On les compares
		for( int x = 0; x < squareOriginal.rows; x++ ) {
			for( int y = 0; y < squareOriginal.cols; y++ ) {
				sum += pow((squareOriginal.at<uchar>(x, y) - mu1) - (squareCompared.at<uchar>(x, y) - mu2), 2);
			}
		}
		// Si on trouve 
		if (sum < SEUIL_SQUARE && sum < min) {
			min = sum;
			pointMatch = vCompared[j];
			matchFound = true;
		}
	}
	if (matchFound) {
		// On retourne le point trouvé
		return pointMatch;
	}
	// On retourne un point NULL
	return Point2i(0, 0);
}

/* Fonction pour récupérer les couples qui matchent */
vector<vector<Point2i> > getMatches(Mat imageIn1, Mat imageIn2, vector<Point2i> v1, vector<Point2i> v2) {
	vector<vector<Point2i> > matches;
	for(unsigned int i = 0; i < v1.size(); i++) {
		// On cherche le point qui match
		Point2i point = pointMatch(imageIn1, imageIn2, v1[i], v2);
		if (point.x != 0 && point.y != 0) {
			// On vérifie que le point match bien en reverse
			Point2i pointReverse = pointMatch(imageIn2, imageIn1, point, v1);
			if (pointReverse == v1[i] && pointReverse.x != 0 && pointReverse.y != 0) {
				// On ajoute le match
				vector<Point2i> temp;
				temp.push_back(v1[i]);
				temp.push_back(point);
				matches.push_back(temp);
			}
		}			
	}
	return matches;
}

/* Fonction pour afficher les points de corner */
void showCorners(Mat &imageOut, vector<Point2i> corners, int decalage = 0) {
	for(unsigned int i = 0; i < corners.size(); i++) {
		circle(imageOut, Point2i(decalage + corners[i].x,  corners[i].y), 3, Scalar(0, 0, 255));
	}
}

/* Fonction pour afficher les matchs */
void showMatches(Mat &imageOut, vector<vector<Point2i> > matches, int decalage = 0) {
	for(unsigned int i = 0; i < matches.size(); i++) {
		arrowedLine(imageOut, matches[i][0] , Point2i(decalage + matches[i][1].x,  matches[i][1].y), Scalar(0, 0, 255), 1, 8, 0, 0);
	}
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
		cout <<  "Nb points left : " << v1.size() << endl;
		vector<Point2i> v2  = MY_FAST(imageIn2);
		cout <<  "Nb points right : " << v2.size() << endl;

		// vector<std::array<Point2i, 2>> matches;
		vector<vector<Point2i> > matches = getMatches(imageIn1, imageIn2, v1, v2);

		// for(unsigned int i = 0; i < v1.size(); i++) {

		// 	// On cherche le point qui match
		// 	Point2i point = pointMatch(imageIn1, imageIn2, v1[i], v2);
		// 	if (point.x != 0 && point.y != 0) {
		// 		// On vérifie que le point match bien en reverse
		// 		Point2i pointReverse = pointMatch(imageIn2, imageIn1, point, v1);
		// 		if (pointReverse == v1[i] && pointReverse.x != 0 && pointReverse.y != 0) {
		// 			// On illustre le match
		// 			// arrowedLine(imageOut, v1[i] , Point2i(decalage + point.x,  point.y), Scalar(0, 0, 255), 1, 8, 0, 0);
		// 		}
		// 	}			
		// }
		// showCorners(imageOut, v1);
		// showCorners(imageOut, v2, decalage);
		showMatches(imageOut, matches, decalage);

		imshow( "Corners", imageOut );  

		waitKey(0); 
	} 
	return 0;
}

