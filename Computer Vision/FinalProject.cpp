#include "cv.hpp"
#include <iostream>
#include <fstream>
#include <opencv2/dnn.hpp>

using namespace std;
using namespace cv;
using namespace cv::dnn;

struct CallbackParam
{
	Mat frame;
	Point pt1, pt2;
	Rect roi;
	bool updated;
	//추가한 파라미터
	int area;
	int lastArea;
	float center;
	bool stop;
};

int main(int argc, const char * argv[]) {
	Mat frame;
	Mat left_ROI_area, right_ROI_area, left_ROI_area_gray, right_ROI_area_gray;
	Mat canny_left, canny_right;
	Rect left_ROI, right_ROI;
	Point Left_LT, Left_BR, Right_LT, Right_BR, p1, p2;
	vector<cv::Vec2f> left_lines, left_lines_filtered;
	vector<cv::Vec2f> right_lines,right_lines_filtered;

	int current_time;
	double fps;
	float rho, theta, rho_left_avg, rho_right_avg, theta_left_avg, theta_right_avg, a, b, x0, y0;

    ///새로추가한 변수 영역

	//linedeparture감지를 위한 변수
    int noLineFrameCount = 0;
    bool lineDeparture = false;
    int lineFrameCount = 0;
    bool currentFrame = false;

	//차량인식을 위한 변수
    Mat Middle_ROI_area;
    Rect Middle_ROI;
    Point Middle_BR;
    Point Middle_LT;
	Mat Middle_ROI_area_gray;

	int frameCount = 15;

	//yolo 사용을 위한 변수들
	String modelConfiguration = "deep/cfg/yolov2.cfg"; 
	String modelBinary = "deep/cfg/yolov2.weights";
	Net net = readNetFromDarknet(modelConfiguration, modelBinary);
	vector<String> classNamesVec;
	ifstream classNamesFile("deep/coco.names");
	if (classNamesFile.is_open()) {
		string className = "";
		while (std::getline(classNamesFile, className)) classNamesVec.push_back(className);
	}
	CallbackParam param;
	bool tracking = false;
	param.frame = frame;
    param.updated = false;
	/*
	//tracking을 위한 변수들
	MatND m_model3d;
	Rect m_rc;
    
    Mat m_backproj, hsv;
	float hrange[] = { 0,180 };
    float vrange[] = { 0,255 };
    const float* ranges[] = { hrange, vrange, vrange };	// hue, saturation, brightness
    int channels[] = { 0, 1, 2 };
    int hist_sizes[] = { 16, 16, 16 };

	*/

	//go, stop, okay감지하기 위한 변수들
	bool increase;
	bool decrease;
	bool stop = false;
	bool go = false;
	bool okay = true;
	param.area=0;
	bool dark= false;
	Mat background;
	int count_white = 0;
	int count_red=0;
	Mat stopImg1;
	Mat stopImg2;
	int stop_count;

	//

	//Initialization
	VideoCapture cap("6.mp4");
	cap>>stopImg1;
	cvtColor(stopImg1, stopImg1, CV_BGR2GRAY);
	//낮인지 밤인지 확인
	int count=0;
    cap>>background;
    cvtColor(background, background, CV_BGR2GRAY);
    
    MatIterator_<uchar> it, end;

    for( it = background.begin<uchar>(), end = background.end<uchar>(); it != end; ++it) {
            if(*it<128){
                count++;
            }
    }
	if(count>300000){
		dark = true;
	}
	//

	fps = cap.get(CV_CAP_PROP_FPS);
/*
	namedWindow("Left");
	namedWindow("Left canny");
	namedWindow("Right");
	namedWindow("Right canny");
    
	moveWindow("Left", 0, 0);
	moveWindow("Left canny", 200, 0);
	moveWindow("Right", 0, 150);
	moveWindow("Right canny",200, 150);
*/
	while (1) {
		cap >> frame;
		//Check end condition
		current_time = cap.get(CAP_PROP_POS_MSEC);
		if (frame.empty()) break;

		//Set ROI
		Left_LT = Point(300, 295);
		Left_BR = Point(400, 400);

		Right_LT = Point(500, 295);
		Right_BR = Point(600, 400);
		
		left_ROI = Rect(Left_LT,  Left_BR);
		right_ROI= Rect(Right_LT, Right_BR);

		left_ROI_area =  frame(left_ROI);
		right_ROI_area = frame(right_ROI);	
		///2
        Middle_LT = Point(300, 200);
        Middle_BR = Point(550, 400);
        Middle_ROI = Rect(Middle_LT,  Middle_BR);
		Middle_ROI_area = frame(Middle_ROI);

		Point night_LT = Point(400, 250);
        Point night_BR = Point(480, 300);
        Rect night_ROI = Rect(night_LT,  night_BR);
		Mat night_ROI_area = frame(night_ROI);

		Point stop_LT = Point(200, 100);
        Point stop_BR = Point(600, 200);
        Rect stopROI = Rect(stop_LT,  stop_BR);
		Mat stop_ROI_area = frame(stopROI);
        
		frameCount++;

		///낮 차량 검출
		if(frameCount > 15&& dark == false){
			frameCount=0;
			if (Middle_ROI_area.channels() == 4)cvtColor(Middle_ROI_area, Middle_ROI_area, COLOR_BGRA2BGR);
			Mat inputBlob = blobFromImage(Middle_ROI_area, 1 / 255.F, Size(416, 416), Scalar(), true, false); 
			net.setInput(inputBlob, "data"); //set the network input
			Mat detectionMat = net.forward("detection_out"); //compute output 
			vector<double> layersTimings;
			float confidenceThreshold = 0.24; //by default
			for (int i = 0; i < detectionMat.rows; i++) {
				const int probability_index = 5;
				const int probability_size = detectionMat.cols - probability_index;
				float *prob_array_ptr = &detectionMat.at<float>(i, probability_index);
				size_t objectClass = max_element(prob_array_ptr, prob_array_ptr + probability_size) - prob_array_ptr; 
				//특정한 물체가 detection된 확률
				float confidence = detectionMat.at<float>(i, (int)objectClass + probability_index);
				//For drawing
				if (confidence > confidenceThreshold && objectClass == 2) {
					float x_center = detectionMat.at<float>(i, 0) * Middle_ROI_area.cols; 
					float y_center = detectionMat.at<float>(i, 1) * Middle_ROI_area.rows; 
					float width = detectionMat.at<float>(i, 2) * Middle_ROI_area.cols; 
					float height = detectionMat.at<float>(i, 3) * Middle_ROI_area.rows;
					Point p1(cvRound(x_center - width / 2)+Middle_LT.x, cvRound(y_center - height / 2)+Middle_LT.y); 
					Point p2(cvRound(x_center + width / 2)+Middle_LT.x, cvRound(y_center + height / 2)+Middle_LT.y); 
					Rect object(p1, p2);
					Scalar object_roi_color(0, 255, 0);
					Scalar ob(255,0,0);
					//rectangle(frame, object, ob);
					//가운데 차선의 차를 인식하는 방법 - 인식된 물체의 중심의 x값이 내가 설정한 값 이내인지 확인한다.
					if(100.0 <x_center && x_center< 150.0){
						//rectangle(frame, object, object_roi_color);
						param.center = x_center;
						param.roi.x = cvRound(x_center - width / 2)+Middle_LT.x;
						param.roi.y = cvRound(y_center - height / 2)+Middle_LT.y;
						param.roi.width =width;
						param.roi.height =height;
						param.updated = true;
						param.lastArea = param.area;
						param.area = width*height;

					}
					else{
            			tracking = false;
        			}
				}
				else{
            			tracking = false;
        		}

					
			}
			
		}

		//밤에 차량 검출
		if(dark == true){
			//RoI영역에 일정량 이상의 빨간빛과 횐색빛이 감지될 경우 차량이라고 봄
			Mat YCrCb;
			cvtColor(night_ROI_area, YCrCb, CV_BGR2YCrCb);

			//YCrCb 이미지를 각 채널별로 분리
			count_red=0;
			vector<Mat> planes;

			split(YCrCb, planes);

			int nr= YCrCb.rows;

			int nc= YCrCb.cols;

			

			// 170 < Cr <230, 70 <Cb < 130인 영역만 255로 표시해서 mask 만들기

			for(int i=0; i<nr; i++){

				uchar* Cr=planes[1].ptr<uchar>(i);

				uchar* Cb=planes[2].ptr<uchar>(i);

				for(int j=0; j<nc;j++){

					if((170<Cr[j] && Cr[j] <230) && (70<Cb[j] && Cb[j]<130) )

					count_red ++;

				}

			}

			count_white = 0;
			
			cvtColor(night_ROI_area, Middle_ROI_area, CV_BGR2GRAY);
		
			MatIterator_<uchar> it, end;

			for( it = Middle_ROI_area.begin<uchar>(), end = Middle_ROI_area.end<uchar>(); it != end; ++it) {
				if(*it>240){
					count_white++;
					*it = 255;
				}
				else
					*it =0;
			}

		}
		if (param.updated) {
			param.updated = false;
			tracking = true;
		}

		//stop 조건
		stopImg2 = stop_ROI_area.clone();
		cvtColor(stopImg2, stopImg2, CV_BGR2GRAY);
		MatIterator_<uchar> it, end;
        MatIterator_<uchar> it2, end2;
		stop_count = 0;
        for( it = stopImg1.begin<uchar>(), it2 = stopImg2.begin<uchar>(), end = stopImg1.end<uchar>(), end2 = stopImg2.end<uchar>(); it != end; ++it, ++it2) {
            if( (*it- *it2 ) >20 or (*it2- *it ) > 20){
                stop_count++;
            }
        }
		stopImg1 = stopImg2.clone();
		if(stop_count<2500){
			param.stop = true;
		}
		else{
			param.stop = false;
		}

		//putText(frame, format("area : %d ", stop_count), Point(350,350), FONT_HERSHEY_SIMPLEX, 1, Scalar(0,255,0), 5);





		if(dark == false){
			/*
			go조건 차량감지가 true, stop이 true, 앞 차 크기가 감소, 앞 차 크기가 일정 범위 이하
			stop조건 차량감지가 true, 앞 차 크기가 증가, 앞 차 크기가 일정 범위 이상
			okay조건 go가 true 차량감지가 true, 앞 차 크기가 일정 범위 이하
			*/
			if(param.area-param.lastArea>0){
			increase = true;
			decrease = false;
			}
			if(param.area-param.lastArea<0){
				increase = false;
				decrease = true;
			}

			if(tracking == true && increase == true && param.area>8000){
				stop = true;
				okay = false;
				go = false;
			}	

			//go 조건 앞에 차가 있어서 멈춰 있었거나
			if(((tracking == false || param.area<8000 )&& stop ==true &&param.stop == true)  ){
				stop = false;
				okay = false;
				go = true;
			}
			//okay 조건
			if( (tracking == false || param.area < 8000) && param.stop == false ){
				stop = false;
				okay = true;
				go = false;
			}
			
		}

		if(dark == true){
			//putText(frame, format("area : %d, %d, %f", count_white, count_red, fps), Point(250,250), FONT_HERSHEY_SIMPLEX, 1, Scalar(0,255,0), 5); 

			if(count_white>1000 && count_red>1500){
				stop = true;
				go = false;
				okay = false;
			}
			
			if(param.stop == false && (count_white<100 || count_red<100) ){
				stop = false;
				go = false;
				okay = true;
			}
			if(stop == true && param.stop == true && (count_white<500 || count_red<500)){
				stop = false;
				go = true;
				okay = false;
			}
			
		}
		if(stop == true){
				putText(frame, format("Stop"), Point(700,50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0,0,255), 5);
			}
			if(go == true){
				putText(frame, format("Go"), Point(700,50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0,255,0), 5);
			}
			if(okay == true){
				putText(frame, format("Okay"), Point(700,50), FONT_HERSHEY_SIMPLEX, 1, Scalar(255,0,0), 5);
			}
		//Convert image to gray
		cvtColor(left_ROI_area, left_ROI_area_gray, CV_BGR2GRAY);
		cvtColor(right_ROI_area, right_ROI_area_gray, CV_BGR2GRAY);

		//Gaussian blurring
		GaussianBlur(left_ROI_area_gray, left_ROI_area_gray, Size(5, 5), 1.5);
		GaussianBlur(right_ROI_area_gray, right_ROI_area_gray, Size(5, 5), 1.5);

		//Canny edge detection
		Canny(left_ROI_area_gray,  canny_left,  10,60);
		Canny(right_ROI_area_gray, canny_right, 10,60);

	imshow("Left cdanny", stop_ROI_area);
	//	imshow("Right canny", canny_right);

		//Line detection
		HoughLines(canny_left,  left_lines,  1, CV_PI / 180, 25);
		HoughLines(canny_right, right_lines, 1, CV_PI / 180, 25);

		//Line filtering
		left_lines_filtered.resize(0);
		right_lines_filtered.resize(0);

		for (int i = 0; i < left_lines.size(); i++) {
			theta = left_lines[i][1];
			//cout << "left lines:" << theta << "   "<< CV_PI<< endl;
			if (theta > CV_PI/6.0f && theta <2*CV_PI/6.0f)
				left_lines_filtered.push_back(left_lines[i]);
		}
		for (int i = 0; i < right_lines.size(); i++) {
			theta = right_lines[i][1];
			//cout << "right lines:" << theta <<"   "<<CV_PI<< endl;
			if (theta > 4*CV_PI /6.0f && theta <5 * CV_PI /6.0f)
				right_lines_filtered.push_back(right_lines[i]);
		}
	
		//Line merging
		rho_left_avg = rho_right_avg = 0.0f;
		theta_left_avg = theta_right_avg = 0.0f;

		if (left_lines_filtered.size() > 0) {
			for (int i = 0; i < left_lines_filtered.size(); i++) {
				rho   = left_lines_filtered.at(i)[0];
				theta = left_lines_filtered.at(i)[1];
				rho_left_avg += rho;
				theta_left_avg += theta;
			}
			rho_left_avg /= (double)(left_lines_filtered.size());
			theta_left_avg /= (double)(left_lines_filtered.size());
		}

		if (right_lines_filtered.size() > 0) {
			for (int i = 0; i < right_lines_filtered.size(); i++) {
				rho = right_lines_filtered.at(i)[0];
				theta = right_lines_filtered.at(i)[1];
				rho_right_avg += rho;
				theta_right_avg += theta;
			}
			rho_right_avg /= (double)(right_lines_filtered.size());
			theta_right_avg /= (double)(right_lines_filtered.size());
		}

		/////Drawdeparture 감지
        if (lineDeparture == false && left_lines_filtered.size() == 0 && right_lines_filtered.size() == 0 ){
            noLineFrameCount ++;
            if(noLineFrameCount>10){
                noLineFrameCount = 0;
                lineDeparture = true;
            }
        }

        if(lineDeparture == false && left_lines_filtered.size() > 0 && right_lines_filtered.size() > 0){
            noLineFrameCount = 0;
        }

        if(lineDeparture == true){
            putText(frame, format("Warning:Lane departure"), Point(50,50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0,0,255), 5);

        }
        
        if(lineDeparture == true && (left_lines_filtered.size() > 0 || right_lines_filtered.size() > 0)){
            lineFrameCount++;
            if(lineFrameCount>10){
                lineFrameCount = 0;
                lineDeparture = false;
            }
        }

        if(lineDeparture == true && left_lines_filtered.size() == 0 && right_lines_filtered.size() == 0){
            noLineFrameCount = 0;
        }
		
		
		if (left_lines_filtered.size() > 0) {
			a = cos(theta_left_avg);
			b = sin(theta_left_avg);
			x0 = a*rho_left_avg + Left_LT.x;
			y0 = b*rho_left_avg + Left_LT.y;
			p1 = Point(cvRound(x0 + 1000 * (-b)), cvRound(y0 + 1000 * a));
			p2 = Point(cvRound(x0 - 1000 * (-b)), cvRound(y0 - 1000 * a));
			line(frame, p1, p2, Scalar(0, 0, 255), 3, 8);
		}
		if (right_lines_filtered.size() > 0) {
			a = cos(theta_right_avg);
			b = sin(theta_right_avg);
			x0 = a*rho_right_avg + Right_LT.x;
			y0 = b*rho_right_avg + Right_LT.y;
			p1 = Point(cvRound(x0 + 1000 * (-b)), cvRound(y0 + 1000 * a));
			p2 = Point(cvRound(x0 - 1000 * (-b)), cvRound(y0 - 1000 * a));
			line(frame, p1, p2, Scalar(0, 0, 255), 3, 8);
		}
		imshow("Frame", frame);

		waitKey(500/fps);

        //
        char ch = waitKey(33);
        if (ch == 27) break;	// ESC Key (exit)
		else if (ch == 32) {	// SPACE Key (pause)
			while ((ch = waitKey(33)) != 32 && ch != 27);
			if (ch == 27) break;
		}
	}
	return 0;
}