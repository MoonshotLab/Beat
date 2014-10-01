#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "cinder/Capture.h"
#include <numeric>

#include "CinderOpenCv.h"

using namespace ci;
using namespace ci::app;

class BeatApp : public AppNative {
  public:
    void prepareSettings( Settings *settings );
	void setup();
    void update();
	void draw();
    void createMemberContoursFromFrame();

    int xRes = 640;
    int yRes = 480;

    Capture mCapture;
    Color getColor( cv::Mat image );
    
    gl::Texture mDisplayTexture;
    cv::Mat mHsvImage;
    cv::vector<cv::vector<cv::Point> > mContours;
};

void BeatApp::prepareSettings( Settings *settings ){
    settings->setWindowSize( xRes, yRes );
    settings->setFrameRate( 30.0f );
}

void BeatApp::setup()
{
    // Select the right camera, start recording
    try{
        ci::Capture::DeviceRef externalCam = Capture::findDeviceByName("Logitech Camera");
        if(externalCam)
            mCapture = Capture(xRes, yRes, externalCam);
        else
            mCapture = Capture(xRes, yRes);

        mCapture.start();
    } catch(...){
        mCapture.reset();
    }
}

void BeatApp::update()
{
    if( mCapture && mCapture.checkNewFrame() ) {
        createMemberContoursFromFrame();
    }
}

void BeatApp::createMemberContoursFromFrame(){
    // Load the most recent frame and convert to cv Matrix
    Surface surface = mCapture.getSurface();
    mDisplayTexture = gl::Texture( surface );
    cv::Mat rgbImage = cv::Mat( toOcv( surface ) );
    
    // Convert to hsv for color detection
    cv::cvtColor( rgbImage, mHsvImage, CV_BGR2HSV_FULL );

    // Convert the image to gray scale, threshold for strong black
    // and white image, then inverse so it's white on black
    cv::Mat bwImage;
    cv::cvtColor( rgbImage, bwImage, CV_BGR2GRAY );
    cv::threshold( bwImage, bwImage, 100, 255, 0 );
    cv::bitwise_not( bwImage, bwImage );

    // Find the contours within an image
    cv::vector<cv::Vec4i> hierarchy;
    cv::findContours( bwImage, mContours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE );
}

Color BeatApp::getColor( cv::Mat image ){
    Color color = Color( 0, 0, 0 );
    
    // Set up arrays for color checking: Red, Green, Blue
    Color colors[3] = { Color(255,0,0), ci::Color(0,255,0), ci::Color(0,0,255) };
    int huesLo[3]   = { 0,  50,  100 };
    int huesHi[3]   = { 60, 100, 180 };
    cv::Mat hsvImages[3] = { *new cv::Mat, *new cv::Mat, *new cv::Mat };
    
    // Loop over the ranges, building black and white images
    // for each color sequence in a range
    for( int i=0; i<3; i++ ){
        cv::inRange(image, cv::Scalar(huesLo[i], 100, 100), cv::Scalar(huesHi[i], 256, 256), hsvImages[i]);
    }
    
    // Loop over the count, setting the final color
    int maxCount = 0;
    for( int i=0; i<3; i++ ){
        int count = cv::countNonZero(hsvImages[i]);
        if(count > maxCount)
            color = colors[i];
    }
    
    return color;
}

void drawTriangle( std::vector<cv::Point> points, Color color ){
    gl::color( color );
    
    gl::begin( GL_TRIANGLES );
    for( int i=0; i<points.size(); i++ ){
        gl::vertex( points[i].x, points[i].y );
    }
    gl::end();
}

void drawRectangle( std::vector<cv::Point> points, Color color ){
    gl::color( color );
    
    gl::begin( GL_QUADS );
    for( int i=0; i<points.size(); i++ ){
        gl::vertex( points[i].x, points[i].y );
    }
    gl::end();
}

void drawCircle( std::vector<cv::Point> points, Color color ){
    gl::color( color );
    
    // Find the center point
    cv::Point2i zero(0.0f, 0.0f);
    cv::Point2i sum = std::accumulate(points.begin(), points.end(), zero);
    cv::Point2i centerPoint(sum.x / points.size(), sum.y / points.size());
    
    // Find the radius
    int radius = 0;
    for( int i=0; i<points.size(); i++ ){
        int dist = points[i].x - centerPoint.x;
        if(dist > radius)
            radius = dist;
    }
    
    gl::drawSolidCircle( ci::Vec2i(centerPoint.x, centerPoint.y), radius );
}

void BeatApp::draw()
{
    gl::clear( Color( 0, 0, 0 ), true );
    
    if( mDisplayTexture )
        gl::draw( mDisplayTexture );
    
    // Loop over all contours in memory and draw shapes
    for( int i = 0; i < mContours.size(); i++ )
    {
        // Take the contours and build polygon points to help
        // simulate a shape
        std::vector<cv::Point> polygonPoints;
        cv::approxPolyDP(cv::Mat(mContours[i]), polygonPoints,
                         cv::arcLength(cv::Mat(mContours[i]), true)*0.02, true);
        
        // Isolate the shape by cropping around it's bounding box
        cv::Rect boundingBox = cv::boundingRect(polygonPoints);
        cv::Mat cropped(mHsvImage.size(), CV_8UC1);
        cv::Mat mask(mHsvImage.size(), CV_8UC1, cv::Scalar::all(0));
        mask(boundingBox).setTo(cv::Scalar::all(255));
        mHsvImage.copyTo(cropped, mask);
        
        // Discover it's color
        Color color = getColor(cropped);
        if(boundingBox.width < xRes/2 && boundingBox.height < yRes/2){
            if(polygonPoints.size() > 2){
                switch( polygonPoints.size() ){
                    case 3:
                        drawTriangle( polygonPoints, color );
                        break;
                    case 4:
                        drawRectangle( polygonPoints, color );
                        break;
                    default:
                        drawCircle( polygonPoints, color );
                        break;
                }
            }
        }
    }
}


CINDER_APP_NATIVE( BeatApp, RendererGl )
