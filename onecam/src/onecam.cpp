// 2024-07-21  William A. Hudson
//
// Single Camera test program.
//    Find, configure, and capture one image from one camera.
//    Demonstrate the essential data flow.
//--------------------------------------------------------------------------

#include "FlyCapture2.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <stdexcept>

//#include <Windows.h>
//#include <chrono>
//#include <future>
//#include <fstream>
//#include <thread>

//#include <opencv2/core.hpp>
//#include <opencv2/imgcodecs.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/imgproc.hpp>
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/core/core.hpp>
//#include <opencv2/cudaimgproc.hpp>
//#include <opencv2/cudaarithm.hpp>
//#include <opencv2/imgcodecs/imgcodecs.hpp>

//#include <cuda_runtime.h>

//using namespace FlyCapture2;
using namespace std;

// Use Fly:: to identify FlyCapture2 names and avoid poluting our namespace.

namespace Fly = FlyCapture2;		// shorter alias for FlyCapture2

//--------------------------------------------------------------------------

/*
* Handle a FlyCapture2 Error.
*    Customize to suit.
*/
void
flyErr( Fly::Error  err )
{
    if ( err != Fly::PGRERROR_OK ) {

	cerr << "Error:  FlyCapture2 trace:" <<endl;
	err.PrintErrorTrace();		// formatted log trace to stderr

	throw std::runtime_error ( err.GetDescription() );
    }
}

void
print_cam_info( const Fly::CameraInfo* pinfo )
{
    cout << "   Serial number       = " << pinfo->serialNumber      <<endl;
    cout << "   Camera model        = " << pinfo->modelName         <<endl;
    cout << "   Camera vendor       = " << pinfo->vendorName        <<endl;
    cout << "   Sensor              = " << pinfo->sensorInfo        <<endl;
    cout << "   Resolution          = " << pinfo->sensorResolution  <<endl;
    cout << "   Firmware version    = " << pinfo->firmwareVersion   <<endl;
    cout << "   Firmware build time = " << pinfo->firmwareBuildTime <<endl;
}

void
print_cam_config( const Fly::FC2Config* config )
{
    cout << "   numBuffers          = " << config->numBuffers        <<endl;
    cout << "   grabMode            = " << config->grabMode          <<endl;
    cout << "   grabTimeout         = " << config->grabTimeout       <<endl;
    cout << "   highPerfRetBuffer   = " << config->highPerformanceRetrieveBuffer        <<endl;
}

//--------------------------------------------------------------------------

int
main( int	argc,
      char	*argv[]
) {
  try {

    Fly::Error		ferror;

    Fly::BusManager	busMgr;
    unsigned		numCameras;

    flyErr( busMgr.GetNumOfCameras( &numCameras ) );

    cout << "numCameras = " << numCameras <<endl;

    Fly::PGRGuid	guid;		// Camera ID
    Fly::Camera		camX;

  // List all cameras
    for ( unsigned ii=0;  ii < numCameras;  ii++)	// camera index
    {
	Fly::Camera		camer;		// Camera object
	Fly::CameraInfo		camInfo;	// Camera info struct

	flyErr( busMgr.GetCameraFromIndex( ii, &guid ) );

	// Connect to a camera  (CameraBase.h)
	flyErr( camer.Connect( &guid ) );

	// Get the camera information
	flyErr( camer.GetCameraInfo( &camInfo ) );

	cout <<endl << "Info for Camera:  " << ii <<endl;
	print_cam_info( &camInfo );

	flyErr( camer.Disconnect() );
    }

  // Connect one camera by serial number

    unsigned		one_sn = 15444696;

    flyErr( busMgr.GetCameraFromSerialNumber( one_sn, &guid ) );
    flyErr( camX.Connect( &guid ) );

    cout << "Connected to:  s/n " << one_sn <<endl;

  // Configure camera  (FlyCapture2Defs.h)  (CameraBase.h)

    Fly::FC2Config       one_config;

    flyErr( camX.GetConfiguration( &one_config ) );

    cout <<endl << "Default Config:" <<endl;
    print_cam_config( &one_config );

    one_config.numBuffers                    = 50;
    one_config.grabMode                      = Fly::BUFFER_FRAMES;
    one_config.grabTimeout                   = 60000;	// milli-seconds
    one_config.highPerformanceRetrieveBuffer = true;

    flyErr( camX.SetConfiguration( &one_config ) );

    flyErr( camX.GetConfiguration( &one_config ) );

    cout << "Set Config:" <<endl;
    print_cam_config( &one_config );

  // Trigger Mode settings  (CameraBase.h)

    Fly::TriggerMode	one_trigg;

    flyErr( camX.GetTriggerMode( &one_trigg ) );

    one_trigg.onOff     = true;
    one_trigg.mode      = 0;
    one_trigg.parameter = 0;
    one_trigg.polarity  = 0;
    one_trigg.source    = 0;

    flyErr( camX.SetTriggerMode( &one_trigg ) );

  // Shutter settings
    {
	Fly::Property	prop_shutter  ( Fly::SHUTTER );

	prop_shutter.absValue       = 0.035;
	prop_shutter.absControl     = true;
	prop_shutter.onePush        = false;
	prop_shutter.onOff          = true;
	prop_shutter.autoManualMode = false;

	flyErr( camX.SetProperty( &prop_shutter ) );
    }

  // Gain settings
    {
	Fly::Property	prop_gain  ( Fly::GAIN );

	prop_gain.absValue       = 30.0;
	prop_gain.absControl     = true;
	prop_gain.onePush        = false;
	prop_gain.onOff          = true;
	prop_gain.autoManualMode = false;

	flyErr( camX.SetProperty( &prop_gain ) );
    }

  // Embedded Image info
    {
	Fly::EmbeddedImageInfo		emInfo;

	flyErr( camX.GetEmbeddedImageInfo( &emInfo ) );

	if ( emInfo.timestamp.available == true ) {
	     emInfo.timestamp.onOff = true;
	    cout << "+ Enabled camera timestamp." <<endl;
	}

	if ( emInfo.frameCounter.available == true ) {
	     emInfo.frameCounter.onOff = true;
	    cout << "+ Enabled camera frameCounter." <<endl;
	}

	flyErr( camX.SetEmbeddedImageInfo( &emInfo ) );
    }

  // Start Capture

    cout << "+ StartCapture()" <<endl;
    flyErr( camX.StartCapture() );

  // Reset camera diagnostic infomation
	// This may not be effective after StartCapture()?
	// See FlyCapture2Defs.h highPerformanceRetrieveBuffer = true
	// any interaction other then grabbing the image is disabled.

    cout << "+ ResetStats()" <<endl;
    flyErr( camX.ResetStats() );

  // Retrieve Image
    {
	Fly::Image		imx;

	// Wait at most Fly::FC2Config.grabTimeout ms for an image.
	cout << "+ RetrieveBuffer()" <<endl;
	flyErr( camX.RetrieveBuffer( &imx ) );

	cout << "+ Save image" <<endl;
	flyErr( imx.Save( "foo.bmp" ) );
    }

  // Finish and exit

    cout << "+ StopCapture()" <<endl;
    flyErr( camX.StopCapture() );
    flyErr( camX.Disconnect() );

    return  0;
  }
  catch ( std::exception& e ) {
    cerr << "Error:  exception caught:  " << e.what() <<endl;
  }
  catch (...) {
    cerr << "Error:  unexpected exception" <<endl;
  }

  return  1;
}

