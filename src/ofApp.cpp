#include "ofApp.h"

using namespace ofxCv;
using namespace cv;


//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetFrameRate(60);
   
    image.loadImage("img3.jpg");
   // image.resize(640, 480);
    useKinect = false;
    
    //----
    // get our colors
    imgProcessed = false;
    shader.load("colorChange.vert", "colorChange.frag");

    //setting gui
    setUpGUI();
    
    //-------
    // Setting Kinect OpenNI
    //openNIDevices[deviceID].setLogLevel(OF_LOG_VERBOSE); // ofxOpenNI defaults to ofLogLevel, but you can force to any level
    if(useKinect){
        openNIDevice.setup();
        openNIDevice.addDepthGenerator();
        openNIDevice.addImageGenerator();
        openNIDevice.addUserGenerator();
        openNIDevice.setUseBackgroundDepthSubtraction(true);
        openNIDevice.setRegister(true);
        openNIDevice.setMirror(true);
        openNIDevice.start();
        
        // NB: Only one device can have a user generator at a time - this is a known bug in NITE due to a singleton issue
        // so it's safe to assume that the fist device to ask (ie., deviceID == 0) will have the user generator...
        openNIDevice.setMaxNumUsers(1); // defualt is 4
        ofAddListener(openNIDevice.userEvent, this, &ofApp::userEvent);
        
        ofxOpenNIUser user;
        user.setUseMaskTexture(true);
        user.setUsePointCloud(true);
        user.setPointCloudDrawSize(2); // this is the size of the glPoint that will be drawn for the point cloud
        user.setPointCloudResolution(2); // this is the step size between points for the cloud -> eg., this sets it to every second point
        openNIDevice.setBaseUserClass(user); // this becomes the base class on which tracked users are created
        colorImg.allocate(openNIDevice.getImagePixels());
        grayImg.allocate(openNIDevice.getDepthPixels());
        grayImg.setRGToRGBASwizzles(true);
        
        //----
        // cloth finder - magic mirros
         mmirror.setUp(4, openNIDevice.getImagePixels());
    }
    else
        mmirror.setUp(6, image.getPixelsRef());

}

//--------------------------------------------------------------
void ofApp::update(){
   
    ofBackground(0, 0, 0);
    //OpenNI
    if(useKinect)
    {
        openNIDevice.update();
        int numUsers = openNIDevice.getNumTrackedUsers();
        if(numUsers > 0)
        {
            ofxOpenNIUser & user = openNIDevice.getTrackedUser(0);
           
            ofPixels depth =  user.getMaskPixels();
            grayImg.loadData(depth);
            ofPixels color;
            color.allocate(openNIDevice.getWidth(),openNIDevice.getHeight(),OF_PIXELS_RGB );
            for(int x = 0; x  < openNIDevice.getWidth() ;x++){
                for(int y = 0; y < openNIDevice.getHeight(); y++ )
                {
                    ofColor cc = depth.getColor(x, y);
                    if( (float)cc.r != 0 && (float)cc.g != 0 && (float)cc.b!= 0)
                    {
                        color.setColor(x, y,openNIDevice.getImagePixels().getColor(x, y));
                    }
                    else
                        color.setColor(x, y,depth.getColor(x,y));
                }
            }
            //setting color images with background removal
            colorImg.loadData(color);
            mmirror.setImage(color);
            imgProcessed = true;
        }
        else
        {
           // image.setFromPixels(openNIDevice.getImagePixels());
            imgProcessed = false;
        }
    }
    else
        imgProcessed = true;
    
    //-------
    // cloth finder - magic mirror
    mmirror.update(imgProcessed);
    
}

//--------------------------------------------------------------
void ofApp::draw(){

    ofBackground(ofColor::white);
    ofSetColor(255, 255, 255);
    
    //------
    // Kinect OpenNI
    if(useKinect)
    {
        ofPushMatrix();
        int numUsers = openNIDevice.getNumTrackedUsers();
        if( numUsers > 0)
        {
            colorImg.draw(640, 0, 640, 480);
            openNIDevice.drawSkeletons(640, 0, 640, 480);
            if(mmirror.getIsSet())
                drawShader();
            else
                mmirror.getImg().draw(0,0);
        }
        else
            openNIDevice.drawImage(640, 0, 640, 480);
        ofPopMatrix();
    }
    else{
        if(mmirror.getIsSet())
            drawShader();
        else
            mmirror.getImg().draw(0,0);
    }
    
    //--------------------------
    //cloth finder - magic mirror
    mmirror.draw();
}

//function to exectue shader in mirror img
void ofApp::drawShader()
{
    if(!mmirror.getSelectedImg().getTextureReference().isAllocated()) return;

   
    ofClear(255, 255, 255, 255);
    
  // mmirror.getImg().getTextureReference().bind();
   // mmirror.getSelectedImg().getTextureReference().bind();

    shader.begin();
    // set hue value
    shader.setUniform1f("hueAdjust", 500);
    shader.setUniformTexture("inputTexture", mmirror.getSelectedImg().getTextureReference(), 0);
    shader.setUniformTexture("mask", image.getTextureReference(), 1);
   // ofPushMatrix();
    mmirror.getSelectedImg().draw(0,0);
   // mmirror.getImg().draw(0,0);
    //ofPopMatrix();
    //image.draw(0,0);
    shader.end();
    
    //mmirror.getImg().getTextureReference().unbind();
   // mmirror.getSelectedImg().getTextureReference().unbind();
    
   // mmirror.getImg().draw(0,0);
}
//--------------------------------------------------------------
void ofApp::userEvent(ofxOpenNIUserEvent & event){
    cout << getUserStatusAsString(event.userStatus) << "for user" << event.id << "from device" << event.deviceID<<endl;
    
    ofLogNotice() << getUserStatusAsString(event.userStatus) << "for user" << event.id << "from device" << event.deviceID;
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
        mmirror.showDebug = !mmirror.showDebug;
    // next color
    if(key == 'c')
        mmirror.setContourColor();
    // select cur color to change
    if(key == 'a')
        mmirror.selectCurrent();
    if(key == 'p'){
         float threshold = ofMap(mouseX, 0, ofGetWidth(), 0, 255);
        cout<<"mm "<<threshold<<endl;
        cout<< mmirror.getImgThreshold()<<endl;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    mmirror.mouseMoved(x, y);
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
   // kinect.setCameraTiltAngle(0); // zero the tilt on exit
  //  kinect.close();
    openNIDevice.stop();


}
