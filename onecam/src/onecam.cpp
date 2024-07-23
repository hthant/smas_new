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


//--------------------------------------------------------------------------

int
main( int	argc,
      char	*argv[]
) {
  try {

    Fly::Error		ferror;

    Fly::BusManager	busMgr;
    unsigned int	numCameras;

    ferror = busMgr.GetNumOfCameras( &numCameras );
    flyErr( ferror );

    flyErr( busMgr.GetNumOfCameras( &numCameras ) );

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

