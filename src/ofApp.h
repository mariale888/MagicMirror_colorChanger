#pragma once

#include "ofxOpenNI.h"
#include "ofMain.h"
#include "ofxUI.h"
#include "ofxCv.h"
#include "ofxOpenCv.h"
#include "ofxColorQuantizer.h"
//#include "ofxKinect.h"

#define MAX_DEVICES 2

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
        void exit();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
   
    
    ofImage image;
    ofShader shader;
    
    // Contour Stuff
    float threshold;
    bool showDebug;
    ofColor targetColor;
    float imgThreshold;
    float thresholdIndex[4];
    ofxCv::ContourFinder contourFinder;
    ofxCv::TrackingColorMode trackingColorMode;
    
    // Color Generator
    int numColors;
    int curColor;
    float disThreshold;
    ofxColorQuantizer colorQuantizer;
    vector<cv::RotatedRect> points;
    void setNumColors();
    void setContourColor(bool isSet, int index);
    void setImgThreshold();
    bool collisionCheckEllipses(cv::Point2f center1, float radius1X, float radius1Y, cv::Point2f center2, float radius2X, float radius2Y);
    cv::RotatedRect getEllipes(int index);
    
    //kinect stuff
   /* ofxKinect kinect;
    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage grayImage; // grayscale depth image
    ofxCvGrayscaleImage grayThreshNear; // the near thresholded image
    ofxCvGrayscaleImage grayThreshFar; // the far thresholded image
    ofTexture tex;
    ofTexture tex1;*/
    ofxCvContourFinder contourFinderKinect;
  
    
    //Kinect OpenNI
    int numDevices;
    ofxOpenNI openNIDevices[MAX_DEVICES];
    ofTrueTypeFont verdana;
    void userEvent(ofxOpenNIUserEvent & event);

    //-------
    bool bThreshWithOpenCV;
    bool bDrawPointCloud;
    
    int nearThreshold;
    int farThreshold;
    
    int angle;
    
    // used for viewing the point cloud
    ofEasyCam easyCam;
    //GUI stuff
    ofxUISuperCanvas *gui0;
    void setUpGUI();
    void guiEvent(ofxUIEventArgs &e);

		
};
