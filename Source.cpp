//Name:Kedamawi Mulualem
//Date:5/31/2017
//UDC Robotics lab 105A
//facial recognition and face detection 
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include <sstream>
#include <string>
#include <iostream>
#include <opencv\highgui.h>
#include <opencv\cv.h>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"


#include <stdio.h>



using namespace std;
using namespace cv;

//our sensitivity value to be used in the threshold() function
const static int SENSITIVITY_VALUE = 10;
//size of blur used to smooth the image to remove possible noise and
//increase the size of the object we are trying to track. (Much like dilate and erode)
const static int BLUR_SIZE = 20;
//we'll have just one object to search for
//and keep track of its position.
int Object[2] = { 0,0 };
//bounding rectangle of the object, we will use the center of this as its position.
Rect objectBoundingRectangle = Rect(0, 0, 0, 0);
Mat Back_ground;
void draw_dot(Point a, Mat frame);
void crop_frame(Mat image, Point left_point, Point right_point);
//int to string helper function
string intToString(int number) {

	//this function has a number input and string output
	std::stringstream ss;
	ss << number;
	return ss.str();
}

void searchForMovement(Mat thresholdImage, Mat &cameraFeed) {
	//notice how we use the '&' operator for the cameraFeed. This is because we wish
	//to take the values passed into the function and manipulate them, rather than just working with a copy.
	//eg. we draw to the cameraFeed in this function which is then displayed in the main() function.
	bool objectDetected = false;
	Mat temp;
	thresholdImage.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	//findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );// retrieves all contours
	findContours(temp, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);// retrieves external contours

																					  //if contours vector is not empty, we have found some objects
	if (contours.size()>0)objectDetected = true;
	else objectDetected = false;

	if (objectDetected) {
		//the largest contour is found at the end of the contours vector
		//we will simply assume that the biggest contour is the object we are looking for.
		vector< vector<Point> > largestContourVec;
		largestContourVec.push_back(contours.at(contours.size() - 1));
		//make a bounding rectangle around the largest contour then find its centroid
		//this will be the object's final estimated position.
		objectBoundingRectangle = boundingRect(largestContourVec.at(0));
		int xpos = objectBoundingRectangle.x + objectBoundingRectangle.width / 2;
		int ypos = objectBoundingRectangle.y + objectBoundingRectangle.height / 2;

		//update the objects positions by changing the 'theObject' array values
		Object[0] = xpos, Object[1] = ypos;
	}
	//make some temp x and y variables so we dont have to type out so much
	int x = Object[0];
	int y = Object[1];
	//draw some crosshairs on the object
	Point left_point(objectBoundingRectangle.x + objectBoundingRectangle.width*0.001 + objectBoundingRectangle.width*0.16, objectBoundingRectangle.y + objectBoundingRectangle.height*0.001 + objectBoundingRectangle.height*0.16);
	Point right_point(objectBoundingRectangle.x + objectBoundingRectangle.width, objectBoundingRectangle.y + objectBoundingRectangle.height);
	Rect box(right_point,left_point);
	rectangle(cameraFeed, box, Scalar(200, 200, 255), 1,8,0);
	crop_frame(cameraFeed, left_point, right_point);
	//putText(cameraFeed, "+", Point(x+19, y+19), 1, 1, Scalar(255, 0, 0), 2);
	//putText(cameraFeed, "+", Point(x -19, y - 19), 1, 1, Scalar(255, 0, 0), 2);


}
void draw_dot(Point a, Mat frame) {


	putText(frame, ".", a, 1.1, 2, Scalar(255, 255, 0), 1, 1, false);



}

int main() {
	long int kd = 99999999999999999;
	//some boolean variables for added functionality
	bool objectDetected = false;
	//these two can be toggled by pressing 'd' or 't'
	bool debugMode = false;
	bool trackingEnabled = false;
	//pause and resume code
	bool pause = false;
	//set up the matrices that we will need
	//the two frames we will be comparing
	Mat frame1, frame2, frame0;
	//their grayscale images (needed for absdiff() function)
	Mat grayImage1, grayImage2;
	//resulting difference image
	Mat differenceImage;
	//thresholded difference image (for use in findContours() function)
	Mat thresholdImage;
	//video capture object.
	VideoCapture capture;
	capture.open(0);
	bool tryi = true;
	capture.read(frame0);
	while (1) {

		//we can loop the video by re-opening the capture every time the video reaches its last frame

		capture.open("KDD.mp4");
		if (!capture.isOpened()) {
			cout << "ERROR ACQUIRING VIDEO FEED\n";
			getchar();
			return -1;
		}
		if (tryi) {
			frame1 = frame0;
			tryi = false;
		}
		//read first frame
		cv::cvtColor(frame1, grayImage1, COLOR_BGR2GRAY);

		//check if the video has reach its last frame.
		//we add '-1' because we are reading two frames from the video at a time.
		//if this is not included, we get a memory error!
		while (capture.get(CV_CAP_PROP_POS_FRAMES)<capture.get(CV_CAP_PROP_FRAME_COUNT) - 1) {
			

			capture.read(frame1);
			//read first frame
			cv::cvtColor(frame1, grayImage1, COLOR_BGR2GRAY);

			//convert frame1 to gray scale for frame differencing
			  
			//copy second frame
			capture.read(frame2);
			//convert frame2 to gray scale for frame differencing
			cvtColor(frame2, grayImage2, COLOR_BGR2GRAY);

			//perform frame differencing with the sequential images. This will output an "intensity image"
			//do not confuse this with a threshold image, we will need to perform thresholding afterwards.
			absdiff(grayImage1, grayImage2, differenceImage);
			//threshold intensity image at a given sensitivity value
			threshold(differenceImage, thresholdImage, SENSITIVITY_VALUE, 255, cv::THRESH_BINARY);
			searchForMovement(thresholdImage, frame1);
			if (debugMode == true) {
				//show the difference image and threshold image
				imshow("deffrence image", differenceImage);
				imshow("treshold image", thresholdImage);

			}
			else {
				//if not in debug mode, destroy the windows so we don't see them anymore
				destroyWindow("deffrence image");
				destroyWindow("treshold image");
			}
			//use blur() to smooth the image, remove possible noise and
			//increase the size of the object we are trying to track. (Much like dilate and erode)

			//threshold again to obtain binary image from blur output

			if (debugMode == true) {
				//show the threshold image after it's been "blurred"

			}
			else {
				//if not in debug mode, destroy the windows so we don't see them anymore

			}

			//if tracking enabled, search for contours in our thresholded image

			//show our captured frame
			imshow("Frame1", frame1);
			//check to see if a button has been pressed.
			//this 10ms delay is necessary for proper operation of this program
			//if removed, frames will not have enough time to referesh and a blank 
			//image will appear.
			switch (waitKey(10)) {

			case 27: //'esc' key has been pressed, exit program.
				return 0;
			case 116: //'t' has been pressed. this will toggle tracking
				trackingEnabled = !trackingEnabled;
				if (trackingEnabled == false) cout << "Tracking disabled." << endl;
				else cout << "Tracking enabled." << endl;
				break;
			case 100: //'d' has been pressed. this will debug mode
				debugMode = !debugMode;
				if (debugMode == false) cout << "Debug mode disabled." << endl;
				else cout << "Debug mode enabled." << endl;
				break;
			case 112: //'p' has been pressed. this will pause/resume the code.
				pause = !pause;
				if (pause == true) {
					cout << "Code paused, press 'p' again to resume" << endl;
					while (pause == true) {
						//stay in this loop until 
						switch (waitKey()) {
							//a switch statement inside a switch statement? Mind blown.
						case 112:
							//change pause back to false
							pause = false;
							cout << "Code resumed." << endl;
							break;
						}
					}
				}


			}


		}
		//release the capture before re-opening and looping again.
		capture.release();
	}

	return 0;

}
int count_frames = 0;
void crop_frame(Mat image, Point left_point, Point right_point) {

	//Mat ROI(image, Rect(left_point.x, left_point.y, right_point.x, right_point.y));

	count_frames++;
	
	Mat frame_crop;
	rectangle(image, left_point, right_point, Scalar(255, 255, 0), 1, 8, 0);
	image(Rect(left_point.x, left_point.y, right_point.x - left_point.x, right_point.y - left_point.y)).copyTo(frame_crop);
	resize(frame_crop, frame_crop, Size(200, 200), 0, 0, CV_INTER_LINEAR);
	//imwrite(to_string(count_frames) + "frame_" + to_string(count_frames) + "kd.jpg", frame_crop);
	imwrite(to_string(count_frames) + "frame_" + to_string(count_frames) + "kd.jpg", image);
	count_frames++;

}
