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
    
    cv::vector<cv::vector<cv::Point> > contours;
	gl::Texture	mTexture;
    double mApproxEps;
};

void BeatApp::prepareSettings( Settings *settings ){
    settings->setWindowSize( 1920, 1080 );
    settings->setFrameRate( 1.0f );
}

void BeatApp::setup()
{
    // Load the image and convert to cv Matrix
    ci::Surface8u surface( loadImage( loadAsset( "hand-drawn.jpg" ) ) );
	cv::Mat colorImage( toOcv( surface ) );
    
    // Convert the image to gray scale, threshold for strong black
    // and white image, then inverse so it's white on black
    cv::Mat bwImage;
    cv::cvtColor( colorImage, bwImage, CV_BGR2GRAY );
    cv::threshold( bwImage, bwImage, 150, 255, 0 );
    cv::bitwise_not( bwImage, bwImage );
    
    // Find the contours within an image
    cv::vector<cv::Vec4i> hierarchy;
    cv::findContours( bwImage, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE );

    // Transform output image into gl texture so we can write to screen
    mTexture = gl::Texture( fromOcv( bwImage ) );
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
    gl::draw( mTexture );

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
