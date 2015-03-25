#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <vector>
#include <algorithm>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

using namespace std;
using namespace cv;
int prevx, prevy;
void mouseTo(int x,int y){
	x = (x*1366)/600;
	y = (y*768)/400;
	if(sqrt( pow(x-prevx,2) + pow(y-prevy,2)) < 7){
		return;
	}
	prevx = x;
	prevy = y;
	Display *display = XOpenDisplay(0);
	Window root = DefaultRootWindow(display);
	XWarpPointer(display, None, root, 0, 0, 0, 0, x, y);
	XFlush(display);
	XCloseDisplay(display);
}

void mouseClick(){
	int button=Button1;
	Display *display = XOpenDisplay(NULL);
	XEvent event;
	if(display == NULL){
		cout<<"Error connecting to display"<<endl;
		exit(EXIT_FAILURE);
	}
	memset(&event, 0x00, sizeof(event));
	event.type = ButtonPress;
	event.xbutton.button = button;
	event.xbutton.same_screen = True;
	XQueryPointer(display, RootWindow(display, DefaultScreen(display)), &event.xbutton.root, &event.xbutton.window, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
	event.xbutton.subwindow = event.xbutton.window;
	while(event.xbutton.subwindow){
		event.xbutton.window = event.xbutton.subwindow;
		XQueryPointer(display, event.xbutton.window, &event.xbutton.root, &event.xbutton.subwindow, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
	}
	if(XSendEvent(display, PointerWindow, True, 0xfff, &event) == 0) cout<<"ERROR SENDING CLICK"<<endl;
	XFlush(display);
	XCloseDisplay(display);
}

//When called, it simulates a mouse release event at the current cursor location
void mouseRelease(){
	int button=Button1;
	Display *display = XOpenDisplay(NULL);
	XEvent event;
	if(display == NULL){
		cout<<"Error connecting to display"<<endl;
		exit(EXIT_FAILURE);
	}
	memset(&event, 0x00, sizeof(event));
	event.xbutton.button = button;
	event.xbutton.same_screen = True;
	XQueryPointer(display, RootWindow(display, DefaultScreen(display)), &event.xbutton.root, &event.xbutton.window, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
	event.xbutton.subwindow = event.xbutton.window;
	while(event.xbutton.subwindow){
		event.xbutton.window = event.xbutton.subwindow;
		XQueryPointer(display, event.xbutton.window, &event.xbutton.root, &event.xbutton.subwindow, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
	}
	event.type = ButtonRelease;
	event.xbutton.state = 0x100;
	if(XSendEvent(display, PointerWindow, True, 0xfff, &event) == 0) cout<<"ERROR RELEASING"<<endl;
	XFlush(display);
	XCloseDisplay(display);
}

bool getAngle(Point s, Point f, Point e){
	float l1 = sqrt(fabs( pow(s.x-f.x,2) + pow(s.y-f.y,2) )) ; 
	float l2 = sqrt(fabs( pow(f.x-e.x,2) + pow(f.y-e.y,2) )) ; 
	float dot=(s.x-f.x)*(e.x-f.x) + (s.y-f.y)*(e.y-f.y);
	float angle = acos(dot/(l1*l2));
	angle=angle*180/M_PI;
	return true;
}

int main(int argc, char** argv){
	VideoCapture cap(1);
	namedWindow("Control", CV_WINDOW_AUTOSIZE);

	int iLowH = 0;
	int iHighH = 39;

	int iLowS = 46; 
	int iHighS = 230;

	int iLowV = 0;
	int iHighV = 244;

	cvCreateTrackbar("LowH", "Control", &iLowH, 179); 
	cvCreateTrackbar("HighH", "Control", &iHighH, 179);

	cvCreateTrackbar("LowS", "Control", &iLowS, 255);
	cvCreateTrackbar("HighS", "Control", &iHighS, 255); 

	cvCreateTrackbar("LowV", "Control", &iLowV, 255);
	cvCreateTrackbar("HighV", "Control", &iHighV, 255);

	while(true){
		Mat imgOriginal;
		bool bSuccess = cap.read(imgOriginal);
		flip(imgOriginal, imgOriginal, 1);

		Mat imgHSV;

		cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV);

		Mat imgThresholded;
		Mat threshed;
		inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded);

		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
		dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 
	
		dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(6, 6)) ); 
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(6, 6)) );

		inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), threshed);

		erode(threshed, threshed, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
		dilate( threshed, threshed, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 
	
		dilate( threshed, threshed, getStructuringElement(MORPH_ELLIPSE, Size(6, 6)) ); 
		erode(threshed, threshed, getStructuringElement(MORPH_ELLIPSE, Size(6, 6)) );



		double largest_area = 0;
		int largest_contour_index = 0;
		Rect bounding_rect;
		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;

		findContours(imgThresholded, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
		for(int i = 0; i < contours.size(); i++){
			double a = contourArea(contours[i], false);
			if(a > largest_area){
				largest_area = a;
				largest_contour_index = i;
				bounding_rect = boundingRect(contours[i]);
			}
		}

		

		if(contours.size() > 0 && contours[largest_contour_index].size() > 3){
			approxPolyDP( Mat(contours[largest_contour_index]), contours[largest_contour_index], 20, true );

			drawContours(imgOriginal, contours,largest_contour_index, Scalar(0,255,255), 2);
			rectangle(imgOriginal, bounding_rect,  Scalar(0,255,0), 1, 8, 0);
			Moments mu = moments(contours[largest_contour_index], false);
			Point2f mc = Point2f(mu.m10/mu.m00, mu.m01/mu.m00);
			//mouseTo(mc.x, mc.y);
			circle( imgOriginal, mc, 4, Scalar(0,255,0), -1, 8, 0 );
			
			vector<Point> hull;
			vector<int> hulli;
			vector<Vec4i> defects;
			vector<Vec4i> fingers;
			convexHull(contours[largest_contour_index], hull, false);
			vector<vector<Point> > hulls(1, hull);
			drawContours(imgOriginal, hulls, -1, Scalar(255,0,0), 2);
			convexHull(contours[largest_contour_index], hulli, false);

			if(hulli.size() > 3){
				convexityDefects(contours[largest_contour_index], hulli, defects);
				for(int i = 0; i < defects.size(); i++){
					if(defects[i][3] > 10000){
						if(getAngle(contours[largest_contour_index][defects[i][0]], contours[largest_contour_index][defects[i][2]], contours[largest_contour_index][defects[i][1]])){
							fingers.push_back(defects[i]);
						}
					}
				}
				
				
				if(fingers.size() > 0){
					reverse(fingers.begin(), fingers.end());
					Point indexFinger = contours[largest_contour_index][fingers[0][0]];
					for(int i = 0; i < fingers.size(); i++){
						if(contours[largest_contour_index][fingers[i][0]].y < indexFinger.y){
							indexFinger = contours[largest_contour_index][fingers[i][0]];
						}
						if(contours[largest_contour_index][fingers[i][1]].y < indexFinger.y){
							indexFinger = contours[largest_contour_index][fingers[i][1]];
						}
						circle(imgOriginal, contours[largest_contour_index][fingers[i][0]],5,Scalar(0,255,0),-1);
						circle(imgOriginal, contours[largest_contour_index][fingers[i][1]],5,Scalar(0,255,0),-1);
						circle(imgOriginal, contours[largest_contour_index][fingers[i][2]],5,Scalar(0,255,0),-1);
					}
					circle(imgOriginal, indexFinger,5,Scalar(0,0,0),-1);
					
					/*cout << fingers.size() << endl;
					circle(imgOriginal, contours[largest_contour_index][fingers[0][0]],5,Scalar(0,255,0),-1);
					circle(imgOriginal, contours[largest_contour_index][fingers[0][1]],5,Scalar(0,255,0),-1);
					circle(imgOriginal, contours[largest_contour_index][fingers[0][2]],5,Scalar(0,255,0),-1);*/
					bool click = false;
					if(fingers.size() == 1){
						mouseTo(indexFinger.x, indexFinger.y);
						mouseRelease();
					}
					else if(fingers.size() == 2){
						mouseTo(indexFinger.x, indexFinger.y);
						mouseClick();
					}
				}
			}
		}

		imshow("Thresholded Image", threshed); //show the thresholded image
		imshow("Original", imgOriginal); //show the original image

		if(waitKey(30) == 27){
			cout << "esc key is pressed" << endl;
			break;
		}

	}

	return 0;
}
