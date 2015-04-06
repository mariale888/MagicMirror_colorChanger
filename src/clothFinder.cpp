//
//  clothFinder.cpp
//  MagicMirror_colorChanger
//
//  Created by Mariale Montenegro on 3/31/15.
//
//

#include "clothFinder.h"

using namespace ofxCv;
using namespace cv;


//-------------------------------
// Constructors:

// Main class setUp
void clothFinder::setUp(int numColors_,ofPixels img)
{
    imgSet      = false;
    isFirstTime = true;
    isSelected  = false;
    selectedSet = false;
    
    numColors = numColors_;
    image.setFromPixels(img);
    
    //selectedImg.loadImage("img2.jpg");
    
    //----
    // get our colors
    curColor  = 0;
    colorQuantizer.setNumColors(numColors);
    colorQuantizer.quantize(image.getPixelsRef());
    
    //----
    //setup contourFinder
    contourFinder.setMinAreaRadius(10); //100
    contourFinder.setMaxAreaRadius(250); //250
    trackingColorMode = TRACK_COLOR_RGB;
    showDebug     = true;
    
    disThreshold = 50;
//    thresholdIndex[0] = image.getWidth() * 0.25;//5 - 10;
//    thresholdIndex[1] = image.getWidth() * 0.35;
//    thresholdIndex[2] = image.getWidth() * 0.5;
//    thresholdIndex[3] = image.getWidth() * 0.55;
    
    // every color could have a different threshold to find the contour
    for(int i=0;i<numColors; i++){
        colorThreshod.push_back(MIN_IMG_THRESH); // init to frist thresholdIndex
    }
    
    //imgThreshold =  thresholdIndex[0];
}

//--------------------------------------------------------------
// Function that determines if we need to reduce the amount
// of orignal colors we are looking for.
// If cur-color doesnt make a contour, reduce to more general
void clothFinder::setNumColors()
{
    cout<<"in setNumColors"<<endl;

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
        if(numColors < 3) numColors++;
        colorQuantizer.setNumColors(numColors);
        colorQuantizer.quantize(image.getPixelsRef());
        
        targetColor = colorQuantizer.getColors()[curColor];
        contourFinder.setTargetColor(targetColor, trackingColorMode);
    }
    //set img Threshold
    setImgThreshold();
}


// Function to set img threshold for contours
// Once we know how many colors cur-image needs,
// look for similar colors that could be potential merges.
// If similar colors, check if contorur overlaps, if so combine.
void clothFinder::setImgThreshold()
{
    cout<<"in setImgThreshold"<<endl;
    //-------
    //Check if controur overlaps ONLY on sim colors:
    vector<ofVec2f> color_index;
    for(int i = 0;i < colorQuantizer.getColors().size(); i++)
    {
        ofColor tempC = colorQuantizer.getColors()[i];
        float r = tempC.r;
        float g = tempC.g;
        float b = tempC.b;
       
        if(r > 250 && g > 250 && b > 250) continue; // if white background ignore
        //if(r < 10 && g < 10 && b < 10) continue; // if white black ignore
       
        //cout<<"color: "<<r<<" "<<g<<" " <<b<<endl;

        for(int j=i+1; j < colorQuantizer.getColors().size();j++)
        {
            ofColor tempC_ = colorQuantizer.getColors()[j];
            if((float)tempC_.r > 250 && (float)tempC_.g > 250 && (float)tempC_.b > 250) continue; // if white background ignore
            //if((float)tempC_.r < 10 && (float)tempC_.g < 10 && (float)tempC_.b < 10) continue; // if white black ignore

            float distance = ofVec3f(tempC.r, tempC.g,tempC.b).distance(ofVec3f(tempC_.r, tempC_.g, tempC_.b) );
            if(distance < disThreshold)
                color_index.push_back(ofVec2f(i, j)); // save pair of similar colors
        }
    }
    
    //------
    // Now with the colors that are similar,
    // Check if contour overlaps, if yes reduce num and ++ img threshold
    
    for(int i = 0; i < color_index.size(); i++)
    {
        // color One:
        cv::RotatedRect ellipse_1 = getEllipes(color_index[i].x);
        cv::RotatedRect ellipse_2 = getEllipes(color_index[i].y);
        
        // if undefined continue
        if( (ellipse_1.center.x == 0 && ellipse_1.center.y == 0)  ||
           (ellipse_2.center.x == 0 && ellipse_2.center.y == 0)) continue;
        
        // Check if overlap
        if(collisionCheckEllipses(ellipse_1.center, ellipse_1.size.width/2, ellipse_1.size.height/2, ellipse_2.center, ellipse_2.size.width/2, ellipse_2.size.height/2 )) {
            points.clear();
            points.push_back(ellipse_1);
            points.push_back(ellipse_2);
            
            //reduce color by 1:
            numColors -= 1;

            /*
            //find what color we are currently looking at: then remove and re map
            int index_ = removeSimColor(color_index[i].x,color_index[i].y);
            
            targetColor = colorQuantizer.getColors()[curColor];
            //cout<<(float)targetColor.r << " " << (float)targetColor.g << " " <<(float)targetColor.b <<endl;
            if( (float)targetColor.r > 250 && (float)targetColor.g > 250 && (float)targetColor.b > 250) {
            //if( (float)targetColor.r < 10 && (float)targetColor.g < 10 && (float)targetColor.b < 10) {
                curColor ++;
                targetColor = colorQuantizer.getColors()[curColor];
            }
            
            contourFinder.setTargetColor(targetColor, trackingColorMode);
            
            // increase imgThreshold: until no contour or half of img.
            int index = 1;
            while(contourFinder.size() > 0 && index < 4){
                //imgThreshold = thresholdIndex[index];
                colorThreshod[curColor] = index;
                setContourColor(false, curColor);
                index ++;
            }
            if(contourFinder.size() == 0)
            {
                index -=1;
                //imgThreshold = thresholdIndex[index];
                colorThreshod[curColor] = index;
                setContourColor(false, curColor);
            }
            cout<<"true: "<< index<<" "<<curColor<<endl;
             */
        }
    }
    if(numColors < 3) numColors = 3;

    // regenerate colors
    colorQuantizer.setNumColors(numColors);
    colorQuantizer.quantize(image.getPixelsRef());

    // remove extra colros from theshold
    colorThreshod.clear();
    for(int i = 0;i < numColors; i++){
        colorThreshod.push_back(MIN_IMG_THRESH); // init to frist thresholdIndex
    }
    
    //--------
    // Check i f we can increase the initial imgColorThreshold for each color:
    //TODO:
      for(int j=0; j < numColors; j++)
      {
          targetColor = colorQuantizer.getColors()[j];
          
          if( (float)targetColor.r > 250 && (float)targetColor.g > 250 && (float)targetColor.b > 250) continue;
          //cout<<(float)targetColor.r <<" "<< (float)targetColor.g << " "<< (float)targetColor.b <<endl;
          contourFinder.setTargetColor(targetColor, trackingColorMode);
          
          // increase imgThreshold: until no contour or half of img.
        
          setContourColor(false, j);
          cout<<"h "<<colorThreshod[j] <<" "<<j<<" " <<contourFinder.size()<<endl;
          while(contourFinder.size() < 6 && colorThreshod[j] < MAX_IMG_THRESH){
              //imgThreshold = thresholdIndex[index];
              colorThreshod[j] += 5;
              cout<<"h "<<colorThreshod[j] <<" "<<j<<" " <<contourFinder.size()<<endl;
              setContourColor(false, j);
             
          }
          if(contourFinder.size() == 0)
          {
              colorThreshod[j] -=5;
              //imgThreshold = thresholdIndex[index];
              setContourColor(false, j);
          }
        cout<<"true: "<<colorThreshod[j]<<" "<<j<<endl;
      }

    setContourColor(true, 0);

}


int clothFinder::removeSimColor(int i, int j)
{
    int index = 0;
    // save old colors
    ofColor sim1 =colorQuantizer.getColors()[i];
    ofColor sim2 =colorQuantizer.getColors()[j];
    // regenerate colors
    colorQuantizer.setNumColors(numColors);
    colorQuantizer.quantize(image.getPixelsRef());

    //find sim color
    for(int j=0; j < colorQuantizer.getColors().size();j++)
    {
        ofColor tempC = colorQuantizer.getColors()[j];
        if((float)tempC.r > 250 && (float)tempC.g > 250 && (float)tempC.b > 250) continue; // if white background ignore
        //if((float)tempC.r < 10 && (float)tempC.g < 10 && (float)tempC.b < 10) continue; // if white black ignore
        
        float distance1 = ofVec3f(tempC.r, tempC.g,tempC.b).distance(ofVec3f(sim1.r, sim1.g, sim1.b) );
        float distance2 = ofVec3f(tempC.r, tempC.g,tempC.b).distance(ofVec3f(sim2.r, sim2.g, sim2.b) );

        if(distance1 < disThreshold || distance2 < disThreshold){
            index = j;
            break;
        }
    }
    
    int th = colorThreshod[i];
    int th2 = colorThreshod[j];
    colorThreshod[index] = th;
    if(th < th2) colorThreshod[index] = th2;
    
    return index;
}

//---------------------------------
// Main color Setters
//set color to contour finder
void clothFinder::setContourColor(bool isSet, int index)
{
    if(isSet){
        curColor = (curColor + 1)%colorQuantizer.getColors().size();
        index = curColor;
        cout<<"my thresh " <<index<<" "<<colorThreshod[index]<<endl;
    }
    
    targetColor = colorQuantizer.getColors()[index];
    contourFinder.setTargetColor(targetColor, trackingColorMode);
    
    //int cindex = colorThreshod[index];
    float cc = colorThreshod[index];
    threshold = ofMap(cc, 0, ofGetWidth(), 0, 255);  //thresholdIndex[cindex]
    contourFinder.setThreshold(threshold);
    contourFinder.findContours(image);
}
void clothFinder::setContourColor()
{
    setContourColor(true, curColor);
}
void clothFinder::setImage(ofPixels pix)
{
    image.setFromPixels(pix);
}

//-----------------------------------------------------
// Function to get ellipeses of specific color sent by the index
cv::RotatedRect clothFinder::getEllipes(int index)
{
    cv::RotatedRect e;
    setContourColor(false, index);
    if(contourFinder.size() > 0)
        e = contourFinder.getFitEllipse(0);
    
    return e;
}
// Function to check if ellipeses intersecting
bool clothFinder::collisionCheckEllipses(Point2f center1, float radius1X, float radius1Y, Point2f center2, float radius2X, float radius2Y)
{
    float radiusSumX = radius1X + radius2X;
    float radiusSumY = radius1Y + radius2Y;
    
    double asquare = radiusSumX * radiusSumX;
    double bsquare = radiusSumY * radiusSumY;
    
    return ((center1.x - center2.x)*(center1.x - center2.x) * bsquare + (center1.y - center2.y) * (center1.y - center2.y) * asquare) < (asquare * bsquare);
}


//--------------------------------------
//Functions to construct img that goint to shader
// construnct img of ONLY what is inside of cur contour
void clothFinder::selectCurrent()
{
    isSelected = true;
}

//-------------------------------------
// Main loop - draw/update
void clothFinder::draw()
{
    
    colorQuantizer.draw(ofPoint(0, image.getHeight() + 5));
    if(showDebug)
    {
        ofPushMatrix();
        ofSetLineWidth(3);
        ofSetColor(magentaPrint);
        contourFinder.draw();
        
        for(int p = 0; p<points.size();p++)
            ofCircle(points[p].center.x, points[p].center.y, 10);
        
        
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
    
    
    if(isSelected)
    {
         ofSetColor(255);
        int size = contourFinder.size();
        
        for (int i=0; i<size; i++)
        {
            
            cv::Rect boundRect = contourFinder.getBoundingRect(i);
            ofPixels color;
            color.allocate(image.getWidth(),image.getHeight(), OF_PIXELS_RGB );
            for(int x = 0; x < image.getWidth(); x++)
            {
                //if(x < boundRect.x) continue;
               // if(x > (boundRect.x + boundRect.)
                 for(int y= 0; y < image.getHeight(); y++)
                 {
                     if(boundRect.contains(cv::Point(x, y) ))
                     {
                          color.setColor(x, y,image.getColor(x, y));
                     }
                     else
                         color.setColor(x, y,image.getColor(0, 0));
                    
                            
                    
                 }
            }
          
            selectedImg.setFromPixels(color);
            selectedSet = true;
           /* cv::Rect img_Rec (0,238,image.getWidth(), image.getHeight());
            
            //create new binary Mat that has the blob's contours as a white image
            cv::Mat contourMat = Mat::zeros(boundRect.height,boundRect.width,CV_8UC3);
            Scalar color(255,255,255);
            cv::Point offset(-contourFinder.getBoundingRect(i).x,-contourFinder.getBoundingRect(i).y);
            drawContours(contourMat, contourFinder.getContours(), i, color, CV_FILLED, 8, noArray(), 0, offset);
            //drawMat(contourMat, boundRect.width + 20, 0);
            
            cv::Mat rgbMat = ofxCv::toCv(image.getPixelsRef());
            //create rgb Mat with the content of the blob's bounding rect
             cv::Mat croppedMat = Mat(boundRect.height,boundRect.width,CV_8UC3);
             Mat croppedRgbMat(rgbMat,boundRect);
             resize(croppedRgbMat, croppedMat);
             //drawMat(croppedMat, 0, 0);
             
             //create RGB Mat that only contains blob's RGB pixles
            cv::Mat maskedMat = Mat::zeros(boundRect.height,boundRect.width, CV_8UC3);//boundRect.height,boundRect.width,CV_8UC3);
             croppedMat.copyTo(maskedMat,contourMat);
             //drawMat(maskedMat, 0, boundRect.height + 20);
            
            ofxCv::toOf(maskedMat, selectedImg);
            selectedSet = true;
            */
             /*
             //create RGBA Mat that has the contourMat as it's 4th channel (is not yet alpha)
             cv::Mat maskedRgbaMat(boundRect.height,boundRect.width,CV_8UC4); //,Scalar(1,2,3,4));
             Mat in[] = { maskedMat, contourMat };
             // rgb[0] -> rgba[0], rgb[1] -> rgba[1], rgb[2] -> rgba[2], alpha[0] -> rgba[3]
             int from_to[] = { 0,0, 1,1, 2,2,3,3 };
             mixChannels( in, 2, &maskedRgbaMat, 1, from_to, 4 );
             //drawMat(maskedRgbaMat, boundRect.width + 20, boundRect.height + 20);
             
             //create ofImage to make 4th channel the alpha; i.e. transparents
             ofImage maskedRgbaImg;
             ofxCv::toOf(maskedRgbaMat, maskedRgbaImg);
             maskedRgbaImg.setImageType(OF_IMAGE_COLOR_ALPHA);
             
             ofEnableAlphaBlending();
             maskedRgbaImg.draw((boundRect.width + 20)*2, boundRect.height + 20);
             ofDisableAlphaBlending();
             
             
             //   blur(right_roi, right_roi, edgeBlur);
             
             //create combine Mat that has rgb blob and contour blob info beside one an other
             cv::Rect targetRect = cv::Rect(0,0,boundRect.width,boundRect.height);
             cv::Rect targetRect2 = cv::Rect(boundRect.width,0,boundRect.width,boundRect.height);
             Mat combine = Mat::zeros(boundRect.height,boundRect.width *2,CV_8UC3);
             Mat left_roi(combine, targetRect);
             maskedMat.copyTo(left_roi);
             Mat right_roi(combine,targetRect2);
             contourMat.copyTo(right_roi);*/
             //drawMat(combine,0, (boundRect.height+20)*2);
            
        }
    }
}

void clothFinder::update(bool set)
{
    imgSet = set;
   
    // We have a new usefull img
    if(imgSet)
    {
        if(isFirstTime && image.bAllocated())
        {
           isFirstTime = false;
            setNumColors();
        }
        else if(image.bAllocated())
            contourFinder.findContours(image);
    }
}

void clothFinder::mouseMoved(int x, int y )
{
   // threshold = ofMap(x, 0, ofGetWidth(), 0, 255);
    //contourFinder.setThreshold(threshold);
    //contourFinder.findContours(image);
}

//---------------
// main getters
bool clothFinder::getIsSet()
{
    return isSelected;
}
ofImage clothFinder::getImg()
{

    return image;
}
ofImage clothFinder::getSelectedImg()
{
    return selectedImg;
}
float clothFinder::getImgThreshold()
{
    cout<<curColor<<endl;
    float index = colorThreshod[curColor];
    return index;//thresholdIndex[index];
}
int clothFinder::getCurColor()
{
    return curColor;
}