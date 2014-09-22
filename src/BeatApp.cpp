#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include <numeric>

#include "CinderOpenCv.h"

using namespace ci;
using namespace ci::app;

class BeatApp : public AppNative {
  public:
    void prepareSettings( Settings *settings );
	void setup();
	void draw();

    Color getColor( std::vector<cv::Point> points );
    cv::Mat mHsvImage;
    cv::vector<cv::vector<cv::Point> > contours;
};

void BeatApp::prepareSettings( Settings *settings ){
    settings->setWindowSize( 1920, 1080 );
    settings->setFrameRate( 1.0f );
}

void BeatApp::setup()
{
    // Load the image and convert to cv Matrix
    ci::Surface8u surface( loadImage( loadAsset( "hand-drawn.jpg" ) ) );
	cv::Mat rgbImage( toOcv( surface ) );
    
    // Conver to hsv for color detection
    cv::cvtColor(rgbImage, mHsvImage, CV_BGR2HSV_FULL);

    // Convert the image to gray scale, threshold for strong black
    // and white image, then inverse so it's white on black
    cv::Mat bwImage;
    cv::cvtColor( rgbImage, bwImage, CV_BGR2GRAY );
    cv::threshold( bwImage, bwImage, 150, 255, 0 );
    cv::bitwise_not( bwImage, bwImage );
    
    // Find the contours within an image
    cv::vector<cv::Vec4i> hierarchy;
    cv::findContours( bwImage, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE );
}

Color BeatApp::getColor(std::vector<cv::Point> points){
    Color color = Color( 0, 0, 0 );
    
    // Todo: Isolate the shape

    // Set up arrays for color checking: Red, Green, Blue
    Color colors[3] = { Color(255,0,0), ci::Color(0,255,0), ci::Color(0,0,255) };
    int huesLo[3]   = { 0,  50,  100 };
    int huesHi[3]   = { 60, 100, 180 };
    cv::Mat hsvImages[3] = { *new cv::Mat, *new cv::Mat, *new cv::Mat };

    // Loop over the ranges, building black and white images
    // for each color sequence in a range
    for( int i=0; i<3; i++ ){
        cv::inRange(mHsvImage, cv::Scalar(huesLo[i], 100, 100), cv::Scalar(huesHi[i], 256, 256), hsvImages[i]);
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

void drawTriangle( std::vector<cv::Point> points ){
    gl::color( Color( 255,0,0 ) );

    gl::begin( GL_TRIANGLES );
    for( int i=0; i<points.size(); i++ ){
        gl::vertex( points[i].x, points[i].y );
    }
    gl::end();
}

void drawRectangle( std::vector<cv::Point> points ){
    gl::color( Color(0,255,0 ) );

    gl::begin( GL_QUADS );
    for( int i=0; i<points.size(); i++ ){
        gl::vertex( points[i].x, points[i].y );
    }
    gl::end();
}

void drawCircle( std::vector<cv::Point> points ){
    gl::color( Color( 0,0,255 ) );
    
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
    gl::clear();

    // Loop over all contours in memory and draw shapes
    for( int i = 0; i < contours.size(); i++ )
    {
        std::vector<cv::Point> polygonPoints;
        cv::approxPolyDP(cv::Mat(contours[i]), polygonPoints,
                         cv::arcLength(cv::Mat(contours[i]), true)*0.02, true);
        
        switch( polygonPoints.size() ){
            case 3:
                drawTriangle( polygonPoints );
                break;
            case 4:
                drawRectangle( polygonPoints );
                break;
            default:
                drawCircle( polygonPoints );
                break;
        }
    }
}


CINDER_APP_NATIVE( BeatApp, RendererGl )
