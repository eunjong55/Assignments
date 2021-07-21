#include "cv.hpp"
#include <iostream>

using namespace std;
using namespace cv;

int main(){
    VideoCapture cap("Road_Image.mp4");
    Mat original;
    Mat filter;
    Mat black;
    Mat result;
    Mat laplacian, abs_laplacian, sharpening;
    int threshold1 = 190;
    int threshold2 = 168;
    
     while(1){
        //영상 읽기
        if(cap.grab()==0)
            break;

        cap.retrieve(original);

        filter = Mat(original.size(), CV_8U, Scalar(0, 0, 0));

        cvtColor(original, black, CV_BGR2GRAY);
//sharpening
        GaussianBlur(black, black, Size(3, 3), 0, 0, BORDER_DEFAULT); 
        Laplacian(black, laplacian, CV_16S, 1, 1, 0, BORDER_DEFAULT); 
        convertScaleAbs(laplacian, abs_laplacian);
        sharpening = abs_laplacian + black;
//roi 설정
        Rect rect1(450,280, original.cols/4, original.rows/4);
        Rect rect2(300,280, original.cols/6, original.rows/4);
//thresholding
        threshold(sharpening(rect1), filter(rect1), threshold1, 255, THRESH_BINARY);
        threshold(sharpening(rect2), filter(rect2), threshold2, 255, THRESH_BINARY);
//검은 배경에 필터화면에 해당하는 이미지만 출력
        result = Mat(original.size(), CV_8UC3, Scalar(0, 0, 0));
        original.copyTo(result, filter);
//동영상 출력
        imshow("result", result);
        waitKey(33);
//종료조건
        if((int)(cap.get(CAP_PROP_FPS))*20 == (int)cap.get(CAP_PROP_POS_FRAMES)){
            break;
        }
//터널 진입시 한계값 변경
        if((int)(cap.get(CAP_PROP_FPS))*17 == (int)cap.get(CAP_PROP_POS_FRAMES)){
            threshold1 = 130;
            threshold2 = 110;
        }
    }
}
