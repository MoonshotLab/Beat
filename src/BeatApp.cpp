#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"

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
};

void BeatApp::prepareSettings( Settings *settings ){
    settings->setWindowSize( 1920, 1080 );
    settings->setFrameRate( 60.0f );
}

void BeatApp::setup()
{
    // Load the image and convert to cv Matrix
    ci::Surface8u surface( loadImage( loadAsset( "computer-drawn-outlined.jpg" ) ) );
	cv::Mat colorImage( toOcv( surface ) );
    
    // Convert the image to gray scale, threshold for strong black
    // and white image, then inverse so it's white on black
    cv::Mat bwImage;
    cv::cvtColor( colorImage, bwImage, CV_BGR2GRAY );
    cv::threshold( bwImage, bwImage, 150, 255, 0 );
    cv::bitwise_not( bwImage, bwImage );
    
    // Find the contours within an image
    cv::vector<cv::Vec4i> hierarchy;
    cv::findContours( bwImage, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE );

    // Transform output image into gl texture so we can write to screen
    mTexture = gl::Texture( fromOcv( bwImage ) );
}

void BeatApp::draw()
{
    gl::clear();
    gl::draw( mTexture );
    
    Color red       = Color( 255, 0, 0 );
    Color green     = Color( 0, 255, 0 );
    Color blue      = Color( 0, 0, 255 );
    Color yellow    = Color( 255, 255, 0);
    Color pink      = Color( 255, 0, 255 );
    Color teal      = Color( 0, 255, 0 );
    Color orange    = Color( 255, 100, 0 );
    Color white     = Color( 255, 255, 255 );
    

    // Loop over each contour and write it as a new image
    for( int i = 0; i < contours.size(); i++ )
    {
        switch(i)
        {
            case 0:
                gl::color( red );
                break;
            case 1:
                gl::color( green );
                break;
            case 2:
                gl::color( blue );
                break;
            case 3:
                gl::color( orange );
                break;
            case 4:
                gl::color( yellow );
                break;
            case 5:
                gl::color( pink );
                break;
            case 6:
                gl::color( teal );
                break;
            default:
                gl::color( white );
                break;
        }
        
        for( int j=0; j < contours[i].size(); j++ )
        {
            int x = contours[i][j].x;
            int y = contours[i][j].y;
            gl::drawSolidCircle( ci::Vec2i(x, y), 3 );
        }
    }
}


CINDER_APP_NATIVE( BeatApp, RendererGl )
