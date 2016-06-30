#ifndef GRAY_H
#define GRAY_H

#include "opencv2/highgui/highgui.hpp"
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>

float findLanes(cv::Mat image);
float getangle();
cv::Mat clip(cv::Mat src);
extern bool hasLeft, hasRight;
extern float angle;

#endif

