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

double getPente(int x, int y, int angle) {
	double angleRad = angle*PI/180;
	return sin(angleRad)/cos(angleRad);
}

double getOrdOrigine(int x, int y, double pente) {
	return y - pente*x;
}

int getDistance(int x1, int y1, int x2, int y2) {
	return (int) sqrt((x2 - x1)*(x2 - x1) + (y2 -y1)*(y2 -y1));
}

bool belongsToDoite(double pente, double ord, int i, int j) {
	return (j <= pente*i + ord + DELTA_DROITE && j >= pente*i + ord - DELTA_DROITE);
}

bool belongsToCircle(int x, int y, int r, int i, int j) {
	double equationCercle = (x - i)*(x - i) + (y - j)*(y - j);
	return (r*r > equationCercle - DELTA_CERCLE && r*r < equationCercle + DELTA_CERCLE);
}

int main(int argc, char** argv){
	
	Mat imageIn = imread( argv[1], CV_LOAD_IMAGE_GRAYSCALE );
	
	if(! imageIn.data ) {
		cout <<  "Could not open or find the image" << std::endl ;
		return -1;
   	}
	else {
		Mat imageOutX, imageOutY, absImageOutX, absImageOutY, imageOut, cercles = Mat::zeros(imageIn.size(), imageIn.type());
		
		int height = imageIn.rows;
		int width = imageIn.cols;
		int depth = min(imageIn.rows, imageIn.cols);
		
		vector<vector<vector<int> > > votesCercles (height,vector<vector<int> >(width,vector <int>(depth, 0)));
		int nbCercles = 0;
		int nbPixelContour = 0;
		
		/*Detection de contour*/
		//Application d'un flou gaussien pour limiter le bruit
		GaussianBlur( imageIn, imageIn, Size(3,3), 0, 0, BORDER_DEFAULT );
		
		//Application d'un seuil de gris pour lisser
		threshold(imageIn, imageIn, SEUIL, 255, THRESH_BINARY);

		//Apply the laplcaian operator
		//Laplacian( imageIn, imageOut, CV_16S, 3, 1, 0, BORDER_DEFAULT );
		//convertScaleAbs( imageOut, imageOut );
		
		//Filtre de Sobel sur X
		Sobel(imageIn, imageOutX, CV_32F, 1 , 0 , 3, 1, 0, BORDER_DEFAULT); 
		convertScaleAbs( imageOutX, absImageOutX );

		//Filtre de Sobel sur Y
		Sobel(imageIn, imageOutY, CV_32F, 0 , 1 , 3, 1, 0, BORDER_DEFAULT);
		convertScaleAbs( imageOutY, absImageOutY );

		//Addition des 2 matrices pour calculer la magnitude du gradient
		addWeighted( absImageOutX, 0.5, absImageOutY, 0.5, 0, imageOut ); 
		

		//Gradient
		Mat mag, angle; 
		cartToPolar(imageOutX, imageOutY, mag, angle, 1); 

		/*Parcours des pixels de contour*/
		for( int y = 0; y < angle.rows; y++ ) {
			for( int x = 0; x < angle.cols; x++ ) {
				//Si c'est un point de contour
				if (mag.at<uchar>(y,x) > SEUIL_MAG) {
					int angleVal = angle.at<uchar>(y,x);
					double pente = getPente(x, y, angleVal);
					double ord = getOrdOrigine(x, y, pente);
					nbPixelContour++;
				
					for( int j = 0; j < imageOut.rows; j++ ) {
						for( int i = 0; i < imageOut.cols; i++ ) {
							//Pour chaque point de la droite
							if (belongsToDoite(pente, ord, i, j)) {
								int rayon = getDistance(x, y, i, j);
								if (rayon > SEUIL_RAYON && rayon < MAX_RAYON) {
									int xCercle = i;
									int yCercle = j;
									int rayonCercle = rayon;
									int max = votesCercles[j][i][rayon];
									//On vérifie s'il n'existe pas déjà un cercle
									for(int unsigned l = j - DELTA_CENTRE; l < j + DELTA_CENTRE && l < votesCercles.size() && l >= 0; l++){
										for(int unsigned k = i - DELTA_CENTRE; k < i + DELTA_CENTRE && k < votesCercles[l].size() && k >= 0; k++){
											for(int unsigned r = rayon - DELTA_RAYON; r < rayon + DELTA_RAYON && r < votesCercles[l][k].size() && r >= 0; r++){
												if (votesCercles[l][k][r] > max) {
													xCercle = k;
													yCercle = l;
													rayonCercle = r;
													max = votesCercles[l][k][r];
												}
											}
										}
									}
									votesCercles[yCercle][xCercle][rayonCercle]++;
								}
							}
						}
					}
				}
			}
		}
		//On regarde les votes
		for(int unsigned y = 0; y < votesCercles.size(); y++){
			for(int unsigned x = 0; x < votesCercles[y].size(); x++){
				for(int unsigned r = 0; r < votesCercles[y][x].size(); r++){
					if (votesCercles[y][x][r] >= SEUIL_VOTE) {
						nbCercles++;
						//On trace les cercles
						for( int j = 0; j < cercles.rows; j++ ) {
							for( int i = 0; i < cercles.cols; i++ ) {
								if (belongsToCircle(x, y, r, i, j)) {
									cercles.at<uchar>(j,i) = 255;
								}
							}
						}
					}
				} 
			} 
		}
		//printf("Pixels de contour : %d\n", nbPixelContour);
		printf("Cercles : %d\n", nbCercles);		
		
		//Affichage de l'image
		//imshow( "out", imageOut );       
		imshow( "Cercles", cercles );       

		waitKey(0); 
	} 
	
	return 0;
}

