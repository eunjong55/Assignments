#include "cv.hpp"
#include <iostream>

using namespace std;
using namespace cv;

int main(){
    VideoCapture capture("background.mp4"); 

    Mat background, image, gray, result;
//    Mat foregroundMask, foregroundImg;
    vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
    Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));

    //일단 배경생성시키자??
    int cnt = 2; 
    
    Mat sum, avg;
    capture >> avg; 
    while (true) {
        if (!capture.read(background)) break; 
        add(background / cnt, avg*(cnt - 1) / cnt, avg);


        if((int)capture.get(CAP_PROP_POS_FRAMES)==10){
            break;
        }
    }
        
    cvtColor(background, background, CV_BGR2GRAY);
    while (true) {

        if (capture.grab() == 0) break; 
        capture.retrieve(image);
        cvtColor(image, gray, CV_BGR2GRAY);
        Mat image2 = gray.clone();

        MatIterator_<uchar> it, end;
        MatIterator_<uchar> it2, end2;

        for( it = image2.begin<uchar>(), it2 = background.begin<uchar>(), end = image2.end<uchar>(), end2 = background.end<uchar>(); it != end; ++it, ++it2) {
            if( (*it- *it2 ) >40 or (*it2- *it ) > 40){
                *it = 255;
            }
            else
                *it = 0;
        }

        erode(image2, image2, element);
        findContours(image2, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

        result = Mat(image.size(), CV_8UC3, Scalar(0, 0, 0));

        image.copyTo(result, image2);

        putText(result, format("number of extracted object : %d", (int)contours.size()), Point(50,50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0,255,0), 5);
        imshow("result", result);
        waitKey(33); }
    }
