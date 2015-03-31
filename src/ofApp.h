#pragma once

#include "ofxOpenNI.h"
#include "ofMain.h"
#include "ofxUI.h"
#include "ofxCv.h"
#include "ofxOpenCv.h"
//#include "ofxKinect.h"

#include "clothFinder.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
        void drawShader();
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
    
    //-------
    bool useKinect;
    ofImage image;
    ofTexture colorImg;
    ofTexture grayImg;
    ofShader shader;
    bool imgProcessed;
    clothFinder mmirror;
          
    //ofxCvContourFinder contourFinderKinect;
    //Kinect OpenNI
    ofxOpenNI openNIDevice;
    ofTrueTypeFont verdana;
    void userEvent(ofxOpenNIUserEvent & event);

   
    
    // used for viewing the point cloud
    ofEasyCam easyCam;
    //GUI stuff
    ofxUISuperCanvas *gui0;
    void setUpGUI();
    void guiEvent(ofxUIEventArgs &e);

		
};
