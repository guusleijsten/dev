#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <raspicam/raspicam_cv.h>
#include <raspicam/raspicam_still_cv.h>
#include <raspicam/raspicam.h>
#include <raspicam/raspicamtypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <string>
#include <iostream>
#include <math.h>
#include "gray.h"

using namespace std;
using namespace raspicam;
using namespace cv;

#define BLUR_PAR 9
#define AREA_TRESHOLD 500.0
#define DEBUG 1
#define PI 3.14159265
float angle;
/// Global variables

#define TRESHOLD_TYPE 1
#define TRESHOLD_VALUE 45
#define ROI_VALUE 40
#define MAX_BINARY_VALUE 255
#define TRESHOLD_MID 15

Mat src, src_gray, src_tsh, dst;

int width = 320, height = 240*((double)ROI_VALUE/100.0);
bool hasRight = false, hasLeft = false;
int minX , maxX = 0;
int minY;

static string window_name = "Threshold Demo";
static string window_orig = "Original Image";

/**
 * @function Threshold_Demo
 */
float findLanes(cv::Mat image)
{
  //namedWindow( window_orig, CV_WINDOW_AUTOSIZE );
  //namedWindow( window_name, CV_WINDOW_AUTOSIZE );

  std::vector< std::vector <Point> > contours;
  std::vector<Vec4i> hierarchy;
  Moments moment, closest, leftMom, rightMom;

  cvtColor(image, src_gray, CV_BGR2GRAY);

  Mat src = clip(src_gray);
  equalizeHist( src, src );
  GaussianBlur( src, src, Size( BLUR_PAR, BLUR_PAR ), 0, 0 );
  //medianBlur(src, src, BLUR_PAR);

  threshold(src, src_tsh, TRESHOLD_VALUE, MAX_BINARY_VALUE, TRESHOLD_TYPE);

  findContours(src_tsh, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
  // iterate through all the top-level contours,
  // draw each connected component with its own random color
  int idx = 0;
  Mat dst = Mat::zeros(src_tsh.rows, src_tsh.cols, CV_8UC3);
  for( ; idx >= 0; idx = hierarchy[idx][0] )
  {
      Scalar color( rand()&255, rand()&255, rand()&255 );
      drawContours( dst, contours, idx, color, CV_FILLED, 8, hierarchy );
  }
  //imshow( window_name, dst );
  //imshow( window_orig, src );
  waitKey( 0 );

  if (DEBUG)
    printf("counters: %d\n", contours.size());
  int minX = src.cols, maxX = 0;
  int minY = src.rows;
  for (int i = 0; i < contours.size(); i++) {
    moment = moments((cv::Mat)contours[i]);
    float area = moment.m00;
    if (area > AREA_TRESHOLD)
    {
      int x = moment.m10/area;
      int y = moment.m01/area;
      y = src.rows - y;
      if (y < minY)
      {  
        minY = y;
        closest = moment;
      }
      if (x < minX) {
        minX = x;
        leftMom = moment;
      }
      if (x > maxX) {
        maxX = x;
        rightMom = moment;
      }
    }
  }
  //moment=
  float area = closest.m00;
  int x = closest.m10/area;
  int y = closest.m01/area;
  y = src.rows - y;
  float t = ((float)y-10);
  float dist = 0.01*t*t + 0.015*t + 23.0;
  
  if (DEBUG)
  {
    printf("Pos: (%d,%d)\n",x, y);
    printf("dist %f\n", dist);
  }

  hasLeft = false;
  hasRight = false;
  if (width/2 - TRESHOLD_MID < x && x < width/2 + TRESHOLD_MID) {

    return dist;
  }

  if (minX < width/2)
  {
    area = leftMom.m00;
    x = leftMom.m10/area;
    y = leftMom.m01/area;
    y = src.rows - y;

    if (y < height*0.9)
      hasLeft = true;
    if (DEBUG)
      printf("Left: (%d,%d)\n",x, y);
  }
  if (maxX > width/2)
  {
    area = rightMom.m00;
    x = rightMom.m10/area;
    y = rightMom.m01/area;
    y = src.rows - y;
    if (y < height*0.9)
      hasRight = true;
    if (DEBUG)
      printf("Right: (%d,%d)\n",x, y);
  }
  if (hasLeft && hasRight)
  {
    int dev = ((double)width/2.0) - (maxX + minX)/2.0;
    if (DEBUG)
      printf("Middle: %d\n", dev);
    angle = atan ((double)dev/(double)height) * 180.0 / PI;
    if (DEBUG)
      printf("Angle: %f\n", angle);
  } 
  return dist;
}

cv::Mat clip(cv::Mat src)
{
    Rect roi(0, (1-((double)ROI_VALUE/100.0))*src.rows, src.cols - 1, ((double)ROI_VALUE/100.0)*src.rows-1); // set the ROI for the image

    return src(roi);
}
/*float getangle()
{
  if (hasLeft && hasRight)
  {
    int dev = ((double)width/2.0) - (maxX + minX)/2.0;
    printf("Middle: %d\n", dev);
    float angle = atan ((double)dev/(double)height) * 180.0 / PI;
    printf("Angle: %f\n", angle);
    return angle;
  } 
  
}*/

