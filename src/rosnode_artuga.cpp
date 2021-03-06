/**

  @file rosnode_artuga.cpp
  @author Diogo Brandão Silva
  @brief Autonomous landing whit the fiducial marker Artuga
  @version 1.1
  @date 28-04-2022

  MIT License

  Copyright (c) 2022 Diogo Brandão

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

**/

/*#####################
  ##     Includes    ##
  #####################*/

#include <ros/ros.h>
#include <mavros_msgs/LandingTarget.h>
#include <string>

#include "camera/camera.h"
#include "artugaMarker/artugaMarker.h"

/*####################
  ##     Defines    ##
  ####################*/
#define MAX_ARUCOS 10

int main(int argc, char **argv){

  /*######################
    ##     Node Init    ##
    ######################*/

  ros::init(argc, argv, "rosnode_artuga");

  ros::NodeHandle  n; // Node
  ros::NodeHandle nh("~"); //Priv node

  ros::Rate rate(10.0);

  /*#######################
    ##     Variables     ##
    #######################*/

  ///Cameras
  std::string visualCameraImg, termalCameraImg, visualCamParam, termalCameraParam;
  nh.param<std::string>("visualCameraImg", visualCameraImg, "image_raw/compressed");
  nh.param<std::string>("termalCameraImg", termalCameraImg, "usb_cam/image_raw/compressed");
  nh.param<std::string>("visualCamParam", visualCamParam, "image_raw/camera_info");
  nh.param<std::string>("termalCameraParam", termalCameraParam, "usb_cam/image_raw/camera_info");
  ///LiDAR


  ///Markers
  int nMarkers;
  std::vector<int> idMarkers;
  std::vector<float> sizeMarkers;
  nh.param<int>("nMarkers", nMarkers, 1);
  nh.param<std::vector<int>>("idMarkers", idMarkers, {1});
  nh.param<std::vector<float>>("idMarkers", sizeMarkers, {0.2});


  ///Display
  bool displayBool;
  nh.param<bool>("displayBool", displayBool, false);
  /*#############################
    ##     Class instances     ##
    #############################*/

  camera *visualCamera = new camera();
  camera *termalCamera = new camera();

  artugaMarker *visualMarker[MAX_ARUCOS];
  artugaMarker *termalMarker[MAX_ARUCOS];
  artugaMarker *lidarMarker[MAX_ARUCOS];

  ///Config markers

  if(nMarkers>MAX_ARUCOS)
    nMarkers = MAX_ARUCOS;

  for (int i=0; i<nMarkers;i++) {
    visualMarker[i] = new  artugaMarker(idMarkers[i],sizeMarkers[i]);
    termalMarker[i] = new  artugaMarker(idMarkers[i],sizeMarkers[i]);
    lidarMarker[i] = new  artugaMarker(idMarkers[i],sizeMarkers[i]);
  }



  /*############################
    ##     Ros Subscriber     ##
    ############################*/
  ros::Subscriber visualCamera_par_sub = n.subscribe<sensor_msgs::CameraInfo>(visualCamParam, 10, &camera::cb_camPar,visualCamera);
  ros::Subscriber visualCamera_img_sub = n.subscribe<sensor_msgs::CompressedImage>(visualCameraImg, 10, &camera::cb_camImg,visualCamera);

  ros::Subscriber termalCamera_par_sub = n.subscribe<sensor_msgs::CameraInfo>(termalCameraParam, 10, &camera::cb_camPar,termalCamera);
  ros::Subscriber termalCamera_img_sub = n.subscribe<sensor_msgs::CompressedImage>(termalCameraImg, 10, &camera::cb_camImg,termalCamera);


  /*###########################
    ##     Ros Publisher     ##
    ###########################*/

  ros::Publisher landingTarget = n.advertise<mavros_msgs::LandingTarget >("mavros/landing_target/pose",10);  


  /*########################
    ##     Main cicle     ##
    ########################*/

  while(ros::ok()){
    int count = 0;


    if(termalCamera->getImageReady()==1  && termalCamera->getConfigReady() ){
      artugaMarker::detector(visualCamera->getImage()->image, visualMarker,nMarkers,visualCamera->getCameraMatrix(),visualCamera->getDistCoeffs(),"VisualDisplay",displayBool);
    }


    if(termalCamera->getImageReady()==1  && termalCamera->getConfigReady() ){
      cv::Mat termalImage;
      cv::bitwise_not(termalCamera->getImage()->image,termalImage);
      artugaMarker::detector(termalImage,termalMarker,nMarkers,termalCamera->getCameraMatrix(),termalCamera->getDistCoeffs(),"TermalDisplay",displayBool);
    }


  }
}
