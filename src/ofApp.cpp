#include "ofApp.h"

using namespace ofxCv;
using namespace cv;


//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetFrameRate(60);
    
    image.loadImage("img2.jpg");
    image.resize(640, 480);
    
    //----
    // get our colors
    numColors = 4;
    curColor  = 0;
    colorQuantizer.setNumColors(numColors);
    colorQuantizer.quantize(image.getPixelsRef());
    
    //----
    //setup contourFinder
    contourFinder.setMinAreaRadius(100);
    contourFinder.setMaxAreaRadius(250);
    trackingColorMode = TRACK_COLOR_RGB;
    showDebug     = true;
    
    
    disThreshold = 50;
    thresholdIndex[0] = image.getWidth() * 0.25 - 30;
    thresholdIndex[1] = image.getWidth() * 0.35;
    thresholdIndex[2] = image.getWidth() * 0.5;
    thresholdIndex[3] = image.getWidth() * 0.65;
    imgThreshold =  thresholdIndex[0];
    //setting gui
    setUpGUI();
    
    shader.load("colorChange.vert", "colorChange.frag");
    setNumColors();

    //--------
    // set kinect
    // enable depth->video image calibration
    kinect.setRegistration(true);
    
    kinect.init();
    //kinect.init(true); // shows infrared instead of RGB video image
    //kinect.init(false, false); // disable video image (faster fps)
    
    kinect.open();		// opens first available kinect
    
    // print the intrinsic IR sensor values
    if(kinect.isConnected()) {
        ofLogNotice() << "sensor-emitter dist: " << kinect.getSensorEmitterDistance() << "cm";
        ofLogNotice() << "sensor-camera dist:  " << kinect.getSensorCameraDistance() << "cm";
        ofLogNotice() << "zero plane pixel size: " << kinect.getZeroPlanePixelSize() << "mm";
        ofLogNotice() << "zero plane dist: " << kinect.getZeroPlaneDistance() << "mm";
    }
    
    colorImg.allocate(kinect.width, kinect.height);
    grayImage.allocate(kinect.width, kinect.height);
    grayThreshNear.allocate(kinect.width, kinect.height);
    grayThreshFar.allocate(kinect.width, kinect.height);
    
    nearThreshold = 230;
    farThreshold = 70;
    bThreshWithOpenCV = true;
    
    // zero the tilt on startup
    angle = 0;
  
    kinect.setCameraTiltAngle(angle);
    tex.allocate(grayImage.getPixelsRef());
    tex.setRGToRGBASwizzles(true);
    tex1.allocate(colorImg.getPixelsRef());
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    ofBackground(100, 100, 100);
    
    kinect.update();
    
    // there is a new frame and we are connected
    if(kinect.isFrameNew()) {
        
        // load grayscale depth image from the kinect source
        grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
        
        // we do two thresholds - one for the far plane and one for the near plane
        // we then do a cvAnd to get the pixels which are a union of the two thresholds
        if(bThreshWithOpenCV) {
            grayThreshNear = grayImage;
            grayThreshFar = grayImage;
            grayThreshNear.threshold(nearThreshold, true);
            grayThreshFar.threshold(farThreshold);
            cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
        } else {
            
            // or we do it ourselves - show people how they can work with the pixels
            unsigned char * pix = grayImage.getPixels();
            
            int numPixels = grayImage.getWidth() * grayImage.getHeight();
            for(int i = 0; i < numPixels; i++) {
                if(pix[i] < nearThreshold && pix[i] > farThreshold) {
                    pix[i] = 255;
                } else {
                    pix[i] = 0;
                }
            }
        }
        
        // update the cv images
        grayImage.flagImageChanged();
        tex.loadData(grayImage.getPixelsRef());
        tex1.loadData(kinect.getDepthPixelsRef());
        
        // find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
        // also, find holes is set to true so we will get interior contours as well....
        contourFinderKinect.findContours(grayImage, 10, (kinect.width*kinect.height)/2, 20, false);
    }
}

//--------------------------------------------------------------
void ofApp::draw(){

    ofBackground(ofColor::white);
    ofSetColor(255);
    
    // bind our texture. in our shader this will now be tex0 by default
    // so we can just go ahead and access it there.
    image.getTextureReference().bind();
    
    // start our shader, in our OpenGL3 shader this will automagically set
    // up a lot of matrices that we want for figuring out the texture matrix
    // and the modelView matrix
    shader.begin();
    
    // get mouse position relative to center of screen
   // float mousePosition = ofMap(mouseX, 0, ofGetWidth(), image.getWidth(), -image.getWidth(), true);
    
    
    //shader.setUniform1f("hueAdjust", 200);
    ofPushMatrix();
    image.draw(0,0);
    ofPopMatrix();
    
    shader.end();
    image.getTextureReference().unbind();
    
    colorQuantizer.draw(ofPoint(0, image.getHeight() + 5));
    
    
    
    //--------------------------
    //contour
    if(showDebug)
    {
        ofPushMatrix();
        ofSetLineWidth(3);
        ofSetColor(magentaPrint);
        contourFinder.draw();
        
        for(int p = 0; p<points.size();p++){
            ofCircle(points[p].center.x, points[p].center.y, 10);
        }
        
        ofNoFill();
        int n = contourFinder.size();
        for(int i = 0; i < n; i++) {
            // smallest rectangle that fits the contour
            ofSetColor(cyanPrint);
            ofPolyline minAreRect = toOf(contourFinder.getMinAreaRect(i));
            minAreRect.draw();
            
            // ellipse that best fits the contour
            ofSetColor(magentaPrint);
            cv::RotatedRect ellipse = contourFinder.getFitEllipse(i);
            ofPushMatrix();
            ofVec2f ellipseCenter = toOf(ellipse.center);
            ofVec2f ellipseSize = toOf(ellipse.size);
            ofTranslate(ellipseCenter.x, ellipseCenter.y);
            ofRotate(ellipse.angle);
            ofEllipse(0, 0, ellipseSize.x, ellipseSize.y);
            ofPopMatrix();
            
            // minimum area circle that encloses the contour
            ofSetColor(cyanPrint);
            float circleRadius;
            ofVec2f circleCenter = toOf(contourFinder.getMinEnclosingCircle(i, circleRadius));
            ofCircle(circleCenter, circleRadius);
            
            // convex hull of the contour
            ofSetColor(yellowPrint);
            ofPolyline convexHull = toOf(contourFinder.getConvexHull(i));
            convexHull.draw();
            
            // defects of the convex hull
            vector<cv::Vec4i> defects = contourFinder.getConvexityDefects(i);
            for(int j = 0; j < defects.size(); j++) {
                ofLine(defects[j][0], defects[j][1], defects[j][2], defects[j][3]);
            }
            
            // some different styles of contour centers
            ofVec2f centroid = toOf(contourFinder.getCentroid(i));
            ofVec2f average = toOf(contourFinder.getAverage(i));
            ofVec2f center = toOf(contourFinder.getCenter(i));
            ofSetColor(cyanPrint);
            ofCircle(centroid, 1);
            ofSetColor(magentaPrint);
            ofCircle(average, 1);
            ofSetColor(yellowPrint);
            ofCircle(center, 1);
            
            // you can also get the area and perimeter using ofPolyline:
            // ofPolyline::getArea() and ofPolyline::getPerimeter()
            double area = contourFinder.getContourArea(i);
            double length = contourFinder.getArcLength(i);
            
            // balance is useful for detecting when a shape has an "arm" sticking out
            // if balance.length() is small, the shape is more symmetric: like I, O, X...
            // if balance.length() is large, the shape is less symmetric: like L, P, F...
            ofVec2f balance = toOf(contourFinder.getBalance(i));
            ofPushMatrix();
            ofTranslate(centroid.x, centroid.y);
            ofScale(5, 5);
            ofLine(0, 0, balance.x, balance.y);
            ofPopMatrix();
        }
        
        ofSetColor(255);
        drawHighlightString(ofToString((int) ofGetFrameRate()) + " fps", 10, 10);
        drawHighlightString(ofToString((int) threshold) + " threshold", 10, 30);
        drawHighlightString(trackingColorMode == TRACK_COLOR_RGB ? "RGB tracking" : "hue tracking", 10, 50);
        
        ofTranslate(8, 75);
        ofFill();
        ofSetColor(0);
        ofRect(-3, -3, 64+6, 64+6);
        ofSetColor(targetColor);
        ofRect(0, 0, 64, 64);
    
        ofPopMatrix();
    }
    
    //----
    //kinect:
    ofSetColor(255, 255, 255);
    tex1.draw(10, 10, 400, 300);
    //kinect.drawDepth(10, 10, 400, 300);
    kinect.draw(420, 10, 400, 300);
    tex.draw(10, 320, 400, 300);
    //grayImage.draw(10, 320, 400, 300);
    ofPushMatrix();
    ofTranslate(10, 320);
    ofScale(grayImage.getWidth()/400.0 - 0.975, grayImage.getHeight() / 300 - 0.975);
    
    contourFinderKinect.draw(10, 320, 400, 300);
    ofPopMatrix();
}

// Function that determines if we need to reduce the amount of orignal colors we are looking for
// If cur-color doenst make a contour, reduce to more genral one
void ofApp::setNumColors()
{
    int colorCount = 1; // white backround
    for(int i=0;i < colorQuantizer.getColors().size(); i++)
    {
        setContourColor(false, i);
        if(contourFinder.size() > 0)
            colorCount++;
    }
   
    if(colorCount < numColors)
    {
        numColors = colorCount;
        colorQuantizer.setNumColors(numColors);
        colorQuantizer.quantize(image.getPixelsRef());
        
        targetColor = colorQuantizer.getColors()[curColor];
        contourFinder.setTargetColor(targetColor, trackingColorMode);
    }
    //set img Threshold
    setImgThreshold();
}


// Function to set img threshold for contours
// Once we know how many colors cur image needs, loof for similar colors that could be potential merges
// if similar colors, check if contorur overlaps, if so combine.
void ofApp::setImgThreshold()
{
    //Check if controur overlaps ONLY on sim colors:
    vector<ofVec2f> color_index;
    for(int i = 0;i < colorQuantizer.getColors().size(); i++)
    {
        ofColor tempC = colorQuantizer.getColors()[i];
        float r = tempC.r;
        float g = tempC.g;
        float b = tempC.b;
        if(r > 250 && g > 250 && b > 250) continue; // if white background ignore

        //cout<<"color: "<<r<<" "<<g<<" " <<b<<endl;
      
        for(int j=i+1; j < colorQuantizer.getColors().size();j++)
        {
            ofColor tempC_ = colorQuantizer.getColors()[j];
            if((float)tempC_.r > 250 && (float)tempC_.g > 250 && (float)tempC_.b > 250) continue; // if white background ignore
            
            float distance = ofVec3f(tempC.r, tempC.g,tempC.b).distance(ofVec3f(tempC_.r, tempC_.g, tempC_.b) );
            if(distance < disThreshold)
                color_index.push_back(ofVec2f(i, j)); // save pair of similar colors
        }
    }
    
    //----
    // now with the colors that are similar, check if contour overlaps, if yes reduce num and  ++ img threshold
    for(int i =0; i < color_index.size(); i++)
    {
        
        // color One:
        cv::RotatedRect ellipse_1 = getEllipes(color_index[i].x);
        cv::RotatedRect ellipse_2 = getEllipes(color_index[i].y);
        
        // if undefined continue
        if( (ellipse_1.center.x == 0 && ellipse_1.center.y == 0)  ||
           (ellipse_2.center.x == 0 && ellipse_2.center.y == 0)) continue;
        
        //check if overlap
        if(collisionCheckEllipses(ellipse_1.center, ellipse_1.size.width/2, ellipse_1.size.height/2, ellipse_2.center, ellipse_2.size.width/2, ellipse_2.size.height/2 )) {
            points.clear();
            points.push_back(ellipse_1);
            points.push_back(ellipse_2);
           
            //reduce color by 1:
            numColors -= 1;
            colorQuantizer.setNumColors(numColors);
            colorQuantizer.quantize(image.getPixelsRef());
            targetColor = colorQuantizer.getColors()[curColor];
            //cout<<(float)targetColor.r << " " << (float)targetColor.g << " " <<(float)targetColor.b <<endl;
            if( (float)targetColor.r > 250 && (float)targetColor.g > 250 && (float)targetColor.b > 250){
                curColor ++;
                targetColor = colorQuantizer.getColors()[curColor];
            }
            
            contourFinder.setTargetColor(targetColor, trackingColorMode);
           
            // increase imgThreshold:
            // increase until no cpontour or half of img.
            int index = 1;
            while(contourFinder.size() > 0 && index < 4){
                imgThreshold = thresholdIndex[index];
                setContourColor(false, curColor);
                index ++;
            }
            if(contourFinder.size() == 0)
            {
                index -=1;
                imgThreshold = thresholdIndex[index];
                setContourColor(false, curColor);
            }
            cout<<"true: "<< index<<" "<<curColor<<endl;
        }
    }
}

// Function to get ellipeses of specific color sent by the index
cv::RotatedRect ofApp::getEllipes(int index)
{
    cv::RotatedRect e;
    setContourColor(false, index);

    if(contourFinder.size() > 0)
        e = contourFinder.getFitEllipse(0);
    
    return e;
}

//set color to contour finder
void ofApp::setContourColor(bool isSet, int index)
{
    if(isSet){
        curColor = (curColor + 1)%colorQuantizer.getColors().size();
        index = curColor;
    }
    
    targetColor = colorQuantizer.getColors()[index];
    contourFinder.setTargetColor(targetColor, trackingColorMode);
    
    threshold = ofMap(imgThreshold, 0, ofGetWidth(), 0, 255);
    contourFinder.setThreshold(threshold);
    contourFinder.findContours(image);

}

// Function to check if ellipeses intersecting
bool ofApp::collisionCheckEllipses(Point2f center1, float radius1X, float radius1Y, Point2f center2, float radius2X, float radius2Y)
{
    //cout<<"center "<<center1.x<<" " <<center1.y<<" | "<<center2.x<<" " <<center2.y<<endl;
    float radiusSumX = radius1X + radius2X;
    float radiusSumY = radius1Y + radius2Y;

    double asquare = radiusSumX * radiusSumX;
    double bsquare = radiusSumY * radiusSumY;
    
    return ((center1.x - center2.x)*(center1.x - center2.x) * bsquare + (center1.y - center2.y) * (center1.y - center2.y) * asquare) < (asquare * bsquare);

    // float e2_x = e2.center.x + e2.size.width * cos(e2.angle) + e2.size.height * sin(e2.angle);
    // float e2_y = e2.center.y + e2.size.height * cos(e2.angle) + e2.size.width * sin(e2.angle);

}

//--------------------------------------------------------------
void ofApp::guiEvent(ofxUIEventArgs &e)
{
}

//--------------------------------------------------------------
void ofApp::setUpGUI()
{
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    if(key == 'd')
        showDebug = !showDebug;
    if(key == 'c')
        setContourColor(true, curColor);
    if(key == 'p'){
        cout<<"mm "<<mouseX<<endl;
        cout<< imgThreshold<<endl;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    threshold = ofMap(mouseX, 0, ofGetWidth(), 0, 255);
    contourFinder.setThreshold(threshold);
    contourFinder.findContours(image);
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
   // targetColor = image.getColor(x, y);
    //contourFinder.setTargetColor(targetColor, trackingColorMode);
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
//--------------------------------------------------------------
void ofApp::exit() {
    kinect.setCameraTiltAngle(0); // zero the tilt on exit
    kinect.close();
    

}
