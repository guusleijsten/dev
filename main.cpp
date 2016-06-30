/*------------------------------------------------------------------------------------------*\
 Lane Detection

 General idea and some code modified from:
 chapter 7 of Computer Vision Programming using the OpenCV Library.
 by Robert Laganiere, Packt Publishing, 2011.

 This program is free software; permission is hereby granted to use, copy, modify,
 and distribute this source code, or portions thereof, for any purpose, without fee,
 subject to the restriction that the copyright notice may not be removed
 or altered from any source or altered source distribution.
 The software is released on an as-is basis and without any warranties of any kind.
 In particular, the software is not guaranteed to be fault-tolerant or free from failure.
 The author disclaims all warranties with regard to this software, any use,
 and any consequent failure, is purely the responsibility of the user.

 Copyright (C) 2013 Jason Dorweiler, www.transistor.io


 Notes:

 Add up number on lines that are found within a threshold of a given rho,theta and
 use that to determine a score.  Only lines with a good enough score are kept.

 Calculation for the distance of the car from the center.  This should also determine
 if the road in turning.  We might not want to be in the center of the road for a turn.

 Several other parameters can be played with: min vote on houghp, line distance and gap.  Some
 type of feed back loop might be good to self tune these parameters.

 We are still finding the Road, i.e. both left and right lanes.  we Need to set it up to find the
 yellow divider line in the middle.

 Added filter on theta angle to reduce horizontal and vertical lines.

 Added image ROI to reduce false lines from things like trees/powerlines
 \*------------------------------------------------------------------------------------------*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <raspicam/raspicam_cv.h>
#include <raspicam/raspicam_still_cv.h>
#include <raspicam/raspicam.h>
#include <raspicam/raspicamtypes.h>
#include <iostream>
#include <vector>
#include "serial.h"
#include "stop.h"
#include "uturn.h"
#include "blue.h"
#include "gray.h"

#define SKIP_FRAMES 15
#define DEBUG
#define DELAY 1000 //one second
#define TURN_DEGREE 15
#define ANGLE_TRESHOLD 5

using namespace raspicam;
using namespace cv;
using namespace std;

int stopFlag = 0, doneFlag = 1;

int prevTime = 0;
int getMilliCount();

int main(int argc, char* argv[]) {

    long frameCount = 0;

    RaspiCam_Cv cam;
    cam.set(CV_CAP_PROP_FORMAT, CV_8UC3);
    cam.set(CV_CAP_PROP_FRAME_WIDTH, 320);
    cam.set(CV_CAP_PROP_FRAME_HEIGHT, 240);

    if (!cam.open())
      return 1;

    printf("Starting program\n");
    int fd = setupSerial();
    int res;
    unsigned char bufRX[255];
    int toggle = 0;
    unsigned char msgTX[10];

    usleep(2000000);
    /*
    buildMessage(msgTX,CMD_STOP,90);
    sendMessage(msgTX,3,fd);*/
    prevTime = getMilliCount();

    Mat image;
    while (1)
    {
        cam.grab();
        cam.retrieve(image);
        frameCount++;


        if(doneFlag)
        {
            //float retangle = getangle();
            float dist = findLanes(image);
            if (hasLeft && !hasRight) {
                //turn right
                //printf("turn right\n");
                buildMessage(msgTX, CMD_RIGHT, TURN_DEGREE);
                sendMessage(msgTX, 3, fd);
                doneFlag=0;
            } else if (hasRight && ! hasLeft) {
                //turn left
                //printf("turn left\n");
                buildMessage(msgTX, CMD_LEFT, TURN_DEGREE);
                sendMessage(msgTX, 3, fd);
                doneFlag=0;
            } else if (hasLeft && hasRight) {
                if (abs(angle) < ANGLE_TRESHOLD) {
                    //printf("straight\n");
                    buildMessage(msgTX, CMD_STRAIGHT, (int)dist);
                    sendMessage(msgTX, 3, fd);
                    doneFlag=0;
                } else {
                    if (angle > 0) {
                        //printf("nt straight left\n");
                        buildMessage(msgTX, CMD_LEFT, abs(angle));
                        sendMessage(msgTX, 3, fd);
                        doneFlag=0;
                    } else {
                        //printf("nt straight right\n");
                        buildMessage(msgTX, CMD_RIGHT, abs(angle));
                        sendMessage(msgTX, 3, fd);
                        doneFlag=0;
                    }
                }
                //printf("ANGLE: %f\n", angle);
            } else {
                printf("NONE OF THE ABOVE\n");
            }
            //if (dist < 50.0) {
            //    doneFlag = 1;
            //    buildMessage(msgTX, CMD_STRAIGHT, (int)dist);
            //    sendMessage(msgTX, 3, fd);
            //}
        }

        /**
        //Do every DELAY ms.
        int curTime = getMilliCount();
        if (curTime - prevTime >= DELAY) {
            prevTime = curTime;
            
        }
        */
        
        
        int rx_length = receiveMessage(bufRX, fd);
        if (rx_length > 0)
          printf("Start of message\n");
        for (int i=0; i<rx_length; i++){
            //printByte(bufRX[i]);
            if (bufRX[i] == MSG_START) {
                if (bufRX[i+1] == VAL_DONE){
                    doneFlag = 1;
                    printf("STTOOP-");
                    usleep(2000000);
                }
            }
        }
        

        //Perform computations
        int ret = 0;
        if (frameCount % SKIP_FRAMES == 0)
        {
            int stopRet = stop(image);
            if (!stopFlag && stopRet) {
                //stopFlag = 1;
                printf("SEE STOP\n");
            }
            if (stopFlag && !stopRet) {
                //stopFlag = 0;
                printf("STOP!\n");
                buildMessage(msgTX, CMD_STOP, 0);
                sendMessage(msgTX, 3, fd);
            }
        }
        /*
        //Take action
        if (uturn(image)) {
            printf("FOUND UTURN!\n");
            //TURN 180 degree.
            buildMessage(msgTX, CMD_UTURN, 0);
            sendMessage(msgTX, 3, fd);
        }
        switch(blue(image)) {
            case 1:
                printf("FOUND LEFT\n");
                buildMessage(msgTX, CMD_LEFT, 45);
                sendMessage(msgTX, 3, fd);
                break;
            case 2:
                printf("FOUND RIGHT\n");
                buildMessage(msgTX, CMD_RIGHT, 45);
                sendMessage(msgTX, 3, fd);
                break;
            case 3:
                printf("FOUND STRAIGHT\n");
                buildMessage(msgTX, CMD_STRAIGHT, 30);
                sendMessage(msgTX, 3, fd);
                break;
            default:
                break;
        }
        */
        usleep(200);

    }
    cam.release();
    return 0;
}

int getMilliCount(){
    timeb tb;
    ftime(&tb);
    int nCount = tb.millitm + (tb.time & 0xfffff) * 1000;
    return nCount;
}

