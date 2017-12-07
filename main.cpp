#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>
#include <time.h>

#define RAYON_FAST 5
#define SEUIL 50
#define TAILLE 16
#define SEUIL_SQUARE 2500
#define DELTA_SQUARE 5
#define SEUIL_DIFF 750
#define T 50
#define SEUIL_NORME 50
#define SEUIL_HOMO 0.01

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

/* Fonction pour récupérer une partie de l'image */
Mat getSample(Mat image, Point2i origine, int delta = 2) {
	Mat sample = Mat::zeros(Size(delta*2 + 1, delta*2 + 1), DataType<uchar>::type);
	for( int x = origine.x - delta, i = 0; x < image.rows && x <= origine.x + delta; x++, i++ ) { 
		for( int y = origine.y - delta, j = 0; y < image.cols && y <= origine.y + delta; y++, j++) { 
			sample.at<uchar>(i, j) = image.at<uchar>(x, y);
		}
	}
	return sample;
}

/* Fonction pour trouver le point qui match le mieux */
Point2i pointMatch(Mat imageIn1, Mat imageIn2, Point2i pointOriginal, vector<Point2i> vCompared) {
	// On récupère les voisins proche du points
	Mat squareOriginal = imageIn1( Rect(pointOriginal.x, pointOriginal.y, DELTA_SQUARE, DELTA_SQUARE) );
	// Mat squareOriginal = getSample(imageIn1, pointOriginal);
	// On récupère la moyenne de la matrice 
	int mu1 = averageMat(squareOriginal);

	Point2i pointMatch;
	int min = SEUIL_SQUARE;
	int lastMin = min;
	bool matchFound = false;
	//On parcourt les points de contours de l'image 2
	for(unsigned int j = 0; j < vCompared.size(); j++) {
		Mat squareCompared = imageIn2( Rect(vCompared[j].x, vCompared[j].y, DELTA_SQUARE, DELTA_SQUARE) );
		// Mat squareCompared = getSample(imageIn2, vCompared[j]);
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
			// On garde le lastMin pour vérifier qu'il y ait bien une forte différence 
			lastMin = min;
			min = sum;
			pointMatch = vCompared[j];
			matchFound = true;
		}
	}
	if (matchFound && lastMin - min > SEUIL_DIFF ) {
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

/* Fonction pour récupérer 4 matchs aléatoires */
vector<vector<Point2i> > getRandomMatches(vector<vector<Point2i> > matches, int nb = 4) {
	vector<vector<Point2i> > randomMatches;
	for (int i = 0; i < nb; i++) {
		int randIndex = rand() % matches.size();
		randomMatches.push_back(matches[randIndex]);
	}
	return randomMatches;
}

/* Fonction qui construit la matrice A à partier de 4 points d'apparairement */
Mat getA(vector<vector<Point2i> > matches) {
	Mat A = Mat::zeros(Size(9, matches.size()*2), DataType<double>::type);

	// On parcourt les lignes
	for( int x = 0, i = 0; x < A.rows; x+=2, i++ ) { 		
		// On construit la 1ère ligne
		A.at<double>(x, 0) = -1*matches[i][0].x;	// -xn
		A.at<double>(x, 1) = -matches[i][0].y;	// -yn
		A.at<double>(x, 2) = -1;	// -1
		A.at<double>(x, 3) = 0;	// 0
		A.at<double>(x, 4) = 0;	// 0
		A.at<double>(x, 5) = 0;	// 0
		A.at<double>(x, 6) = matches[i][1].x*matches[i][0].x;	// xn'xn
		A.at<double>(x, 7) = matches[i][1].x*matches[i][0].y;	// xn'yn
		A.at<double>(x, 8) = matches[i][1].x;	// xn'

		// On construit la 2ème ligne
		A.at<double>(x + 1, 0) = 0;	// 0
		A.at<double>(x + 1, 1) = 0;	// 0
		A.at<double>(x + 1, 2) = 0;	// 0
		A.at<double>(x + 1, 3) = -matches[i][0].x;	// -xn
		A.at<double>(x + 1, 4) = -matches[i][0].y;	// -yn
		A.at<double>(x + 1, 5) = -1;	// -1
		A.at<double>(x + 1, 6) = matches[i][1].y*matches[i][0].x;	// yn'xn
		A.at<double>(x + 1, 7) = matches[i][1].y*matches[i][0].y;	// yn'yn
		A.at<double>(x + 1, 8) = matches[i][1].y;	// yn'
	}
	return A;
}

/* Fonction qui construit la matrice H */
Mat constructH(Mat lastV) {
	double data[lastV.rows];
	for(int i = 0; i < lastV.rows; i++) {
		data[i] = lastV.at<double>(0, i);
		//cout << data[i] << endl;
	}
	Mat H(3,3, lastV.type());
	int rows = 3, cols = 3, cpt = 0;
	for( int x = 0; x < rows; x++ ) {
		for( int y = 0; y < cols; y++ ) {
			H.at<double>(x, y) = data[cpt];
			cpt++;
		}
	}
	//Mat H = Mat(3, 3, lastV.type(), data);
	return H;
}

/* Fonction qui compare 2 matrices avec un seuil */
bool isEqual(Mat A, Mat B, double seuil = SEUIL_HOMO) {	
	bool equal = true;
	for( int x = 0; x < A.rows && equal; x++ ) {
		for( int y = 0; y < A.cols && equal; y++ ) {
			if(!(A.at<double>(x, y) < B.at<double>(x, y) + seuil && A.at<double>(x, y) > B.at<double>(x, y) - seuil)) {
				equal = false;
			}
		}
	}
	return equal;
}

/* Fonction qui calcule la matrice homographique à partir de A (avec le SVD) */
Mat getHomography(Mat A) {
	Mat w, u, vt, V, lastC;
	
	SVD::compute(A, w, u, vt);
	V = vt.t();
	
	lastC = V.col(V.cols - 1 );
	return constructH(lastC);
}

/* Fonction qui récupère la meilleur homography */
Mat getBestHomography(vector<vector<Point2i> > matches, int limit = T) {
	vector<Mat> listH;
	vector<int> votes;
	for(int i = 0; i < limit; i++) {
		//On récupère 4 matches aléatoires
		vector<vector<Point2i> > randomMatches = getRandomMatches(matches);
		//On calcule la matrice homographique
		Mat hFound = getHomography(getA(randomMatches));
		
		bool equal = false;
		//On vérifie qu'il n'existe pas déjà une matrice H proche
		for(unsigned int j = 0; j < listH.size() && !equal; j++) {
			
			double data[3] = { randomMatches[0][0].x, randomMatches[0][0].y, 1 };
			double data1[3] = { randomMatches[0][1].x, randomMatches[0][1].y, 1 };
			
			Mat X = Mat(3, 1,  DataType<double>::type, data);
			Mat X2 = Mat(3, 1,  DataType<double>::type, data1);
			
			equal = isEqual(listH[j], hFound);
			
			if (norm(hFound*X - X2) < SEUIL_NORME && equal) {
				votes[j]++;
			}					
		}
		if(!equal) {
			listH.push_back(hFound);
			votes.push_back(1);
		}	
	}
	int indexMax, max = 0;
	for(unsigned int i = 0; i < votes.size(); i++) {
		if (votes[i] > max) {
			max = votes[i];
			indexMax = i;
		}
	}
	return listH[indexMax];
}

/* Fonction qui renvoie le point par l'interpolation bilinéaire */
Point2i getPointInterPol(double x, double y) {
	Point2f point;
	Point2f origine(x, y);
	Point2f points[] = {Point2f(floor(x), floor(y)), Point2f(floor(x), ceil(y)), Point2f(ceil(x), floor(y)), Point2f(ceil(x), ceil(y))};
	double min = 1;
	for(int i = 0; i < 4; i++) {
		if (norm(origine - points[i]) < min)  {
			point = Point2i(points[i].x, points[i].y);
			min = norm(origine - points[i]);
		}
	}
	return point;
}

bool isCoordInImage(Mat image, Point2i newPoint, int decalageTop = 0, int decalageWidth = 0) {
	return newPoint.x + decalageTop > 0 && newPoint.x + decalageTop  < image.rows && newPoint.y + decalageWidth > 0 && newPoint.y + decalageWidth < image.cols;
}

int main(int argc, char** argv){
	vector <Mat> images;
	for (int i = 1; i < argc; i++) {
		images.push_back(imread(argv[i], CV_LOAD_IMAGE_GRAYSCALE));
		if(!images[i].data ) {
			cout <<  "Could not open or find the image" << argv[i] << endl ;
			return -1;
		}
	}
	
	//Mat imageIn1 = imread( argv[1], CV_LOAD_IMAGE_GRAYSCALE );
	//Mat imageIn2 = imread( argv[2], CV_LOAD_IMAGE_GRAYSCALE );

	Mat imageCorners = imread( argv[1], CV_LOAD_IMAGE_COLOR );	//	TODO change

	int width = images[0].cols + images[1].cols;
	int decalage = images[0].cols;

	Mat imageOut = Mat::zeros(Size(width, images[0].rows), imageCorners.type());
	Mat imageOutMatches = Mat::zeros(Size(width, images[0].rows), imageCorners.type());
	
	Mat pano = Mat::zeros(Size(width+100, images[0].rows+100), images[0].type());

	srand(time(NULL));

	//On crée un image contenant les 2 images
	hconcat(images[0], images[1], imageOut);
	hconcat(images[0], images[1], imageOutMatches);
	cvtColor(imageOut, imageOut, CV_GRAY2RGB);
	cvtColor(imageOutMatches, imageOutMatches, CV_GRAY2RGB);

	// On récupère les points de corners des 2 images avec MY_FAST
	vector<Point2i> v1 = MY_FAST(images[0]);
	vector<Point2i> v2  = MY_FAST(images[1]);
	
	vector<vector<Point2i> > matches = getMatches(images[0], images[1], v1, v2);

	vector<Point2i> pts_src;
	vector<Point2i> pts_dst;
	int cpt = 0;
	for(unsigned int i = 0; i < matches.size() && cpt < matches.size(); i++, cpt++) {
		pts_src.push_back(Point2i(matches[i][0].y, matches[i][0].x));
		pts_dst.push_back(Point2i(matches[i][1].y, matches[i][1].x));
	}

	Mat h = findHomography( pts_dst, pts_src, CV_RANSAC );
	
	int decalageTop = 100;
	int decalageWidth = 100;
	
	// On affiche la 1ère image dans la pano
	for( int x = 0; x < images[0].rows; x++ ) {
		for( int y = 0; y < images[0].cols; y++ ) {
			int val = images[0].at<uchar>(x, y);
			pano.at<uchar>(x + decalageTop, y+decalageWidth) = val;
		}
	}
	
	// On affiche la 2ème image dans la pano
	for( int x = 0; x < images[1].rows; x++ ) {
		for( int y = 0; y < images[1].cols; y++ ) {			
			double data[3] = { x, y, 1 };
			Mat X = Mat(3, 1, h.type(), data);
			
			Mat X2 = h*X;
			double scale = X2.at<double>(0, 2);	
			
			int val = images[1].at<uchar>(x, y);
			
			Point2i newPoint = getPointInterPol(X2.at<double>(0, 0)/scale, X2.at<double>(0, 1)/scale);

			if (isCoordInImage(pano, newPoint, decalageTop, decalageWidth)) {
				pano.at<uchar>(newPoint.x+ decalageTop , newPoint.y+decalageWidth) = val;
			}
			
		}
	}
	
	showCorners(imageOut, v1);
	showCorners(imageOut, v2, decalage);
	showMatches(imageOutMatches, matches, decalage);

	//imshow( "Corners", imageOut );
	//imshow( "Matches", imageOutMatches );  
	imshow( "Pano", pano );  

	waitKey(0); 

}

