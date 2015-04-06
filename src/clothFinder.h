//
//  clothFinder.h
//  MagicMirror_colorChanger
//
//  Created by Mariale Montenegro on 3/31/15.
//
//

#ifndef MagicMirror_colorChanger_clothFinder_h
#define MagicMirror_colorChanger_clothFinder_h
#include "ofxCv.h"
#include "ofxOpenCv.h"
#include "ofxColorQuantizer.h"

#define MAX_IMG_THRESH_ 4
#define MAX_IMG_THRESH 130
#define MIN_IMG_THRESH 30

class clothFinder
{
private:
    
    bool imgSet;
    bool isFirstTime;
    bool selectedSet;
    int numColors;
    int curColor;
    float disThreshold;
    //float imgThreshold;
    ofColor targetColor;
    //float thresholdIndex[MAX_IMG_THRESH_];
    ofxColorQuantizer colorQuantizer;
    vector<cv::RotatedRect> points;
    
    vector<int> colorThreshod;
   
    ofImage image;
    ofImage selectedImg;
    bool isSelected;
    
    void setNumColors();
    void setContourColor(bool isSet, int index);
    int removeSimColor(int i, int j);
    void setImgThreshold();

public:
    
    // Constructor
     void setUp(int numColors_,ofPixels im);
    
    // Functions
    void setImage(ofPixels pix);
    void selectCurrent();
    void setContourColor();
    float getImgThreshold();
    int getCurColor();
    bool getIsSet();
    ofImage getSelectedImg();
    ofImage getImg();
    
    void draw();
    void update(bool set);
    void mouseMoved(int x, int y );
    
    // VARS
    float threshold;
    bool showDebug;
    ofxCv::ContourFinder contourFinder;
    ofxCv::TrackingColorMode trackingColorMode;
    bool collisionCheckEllipses(cv::Point2f center1, float radius1X, float radius1Y, cv::Point2f center2, float radius2X, float radius2Y);
    cv::RotatedRect getEllipes(int index);
    
    
};
#endif
