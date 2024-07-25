//=============================================================================
// Copyright © 2008 Point Grey Research, Inc. All Rights Reserved.
//
// This software is the confidential and proprietary information of Point
// Grey Research, Inc. ("Confidential Information").  You shall not
// disclose such Confidential Information and shall use it only in
// accordance with the terms of the license agreement you entered into
// with PGR.
//
// PGR MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
// SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE, OR NON-INFRINGEMENT. PGR SHALL NOT BE LIABLE FOR ANY DAMAGES
// SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
// THIS SOFTWARE OR ITS DERIVATIVES.
//=============================================================================
//=============================================================================
// $Id: MultipleCameraEx.cpp,v 1.17 2010-02-26 01:00:50 soowei Exp $
//=============================================================================

#include "stdafx.h"

#include "FlyCapture2.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <Windows.h>
#include <chrono>
#include <future>
#include <fstream>
#include <thread>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <cuda_runtime.h>
#include <string>

using namespace FlyCapture2;
using namespace std;

Error error;

Error CheckCamBuffer(Camera* Cam, Image& Im)
{
    Error Er;
    Cam->RetrieveBuffer(&Im);
    return Er;
}

void PrintError(Error error)
{
    error.PrintErrorTrace();
}

int setProperty(
    Camera&		camera,
    Property&		property,
    PropertyType	type,
    const float		f
)
{
    property.type = type;
    error = camera.GetProperty(&property);
    if (error != PGRERROR_OK)
    {
	PrintError(error);
	return -1;
    }

    property.absControl = true;
    property.onePush = false;
    property.onOff = true;
    property.autoManualMode = false;

    property.absValue = f;

    error = camera.SetProperty(&property);
    if (error != PGRERROR_OK)
    {
	PrintError(error);
	return -1;
    }

    return 0;
}

int get_current_second() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    return st.wSecond;
}

string get_time()
{
    //Function to get the current timestamp and turn it into a string
    SYSTEMTIME st;
    GetLocalTime(&st);
    string stime;
    unsigned int year = st.wYear;
    unsigned int month = st.wMonth;
    unsigned int hours = st.wHour;
    unsigned int minutes = st.wMinute;
    unsigned int seconds = st.wSecond;
    unsigned int milliseconds = st.wMilliseconds;
    unsigned int day = st.wDay;
    //Concatenate the time stamp string
    stime += to_string(year);
    stime += "-";
    stime += to_string(month);
    stime += "-";
    stime += to_string(day);
    stime += "-";
    stime += to_string(hours);
    stime += "-";
    stime += to_string(minutes);
    stime += "-";
    stime += to_string(seconds);
    stime += "-";
    stime += to_string(milliseconds);
    return stime;
}

// returns a string of int i of length length
// for example make_fixed_length(2, 4) will return a string with "0002"
string make_fixed_length(const int i, const int length) {
    ostringstream ostr;
    if (i < 0)
	ostr << '-';
    ostr << setfill('0') << setw(length) << (i < 0 ? -1 : i);
    return ostr.str();
}

// returns a string for YYYYMMDDHHMMSS
string get_time_2() {
    //Function to get the current timestamp and turn it into a string
    SYSTEMTIME st;
    GetLocalTime(&st);
    string stime;
    unsigned int year = st.wYear;
    unsigned int month = st.wMonth;
    unsigned int hours = st.wHour;
    unsigned int minutes = st.wMinute;
    unsigned int seconds = st.wSecond;
    unsigned int milliseconds = st.wMilliseconds;
    unsigned int day = st.wDay;
    //Concatenate the time stamp string
    stime += make_fixed_length(year, 4);
    stime += make_fixed_length(month, 2);
    stime += make_fixed_length(day, 2);
    stime += make_fixed_length(hours, 2);
    stime += make_fixed_length(minutes, 2);
    stime += make_fixed_length(seconds, 2);
    return stime;
}

void loginfo(string message, string logfileName) {
    string time = get_time();
    ofstream o;
    o.open(logfileName, std::ios_base::app);
    //o << "str";
    string p = __FILE__;
    time.append(" ");
    time.append(p);
    time.append(" ");
    string finalLog = time.append(message);
    //o.open(logfileName);
    o << finalLog << endl;
    o.close();
}

int timetohour(string str, string deli = " ")
{
    vector<string> v;
    int start = 0;
    int end = str.find(deli);
    while (end != -1) {
	//cout << str.substr(start, end - start) << endl;
	string temp = str.substr(start, end - start);
	v.push_back(temp);
	start = end + deli.size();
	end = str.find(deli, start);
    }
    //cout << str.substr(start, end - start);
    string hour = v[3];
    int currh = atoi(hour.c_str());
    return currh;
}

string dateandhour(string str, string deli = " ")
{
    vector<string> v;
    int start = 0;
    int end = str.find(deli);
    while (end != -1) {
	//cout << str.substr(start, end - start) << endl;
	string temp = str.substr(start, end - start);
	v.push_back(temp);
	start = end + deli.size();
	end = str.find(deli, start);
    }
    //cout << str.substr(start, end - start);
    string hour = v[3];
    string retStr = v[0] + "-" + v[1]+ "-" + v[2] + "-" + v[3];
    return retStr;
}

static const void  saveImage( Image& im,
    int		count,
    int		picNum,
    string	id,
    string	root,
    string	logFilePath,
    int		secondcount
)
{
    //creating a folder for each hour of capturing
    std::cout << "saveImage called, Camera ID: " << id
	<< " Count: " << count << endl;

    //string logStr = "Camera ID " + id + " Saved " + time_stamped_folder;
    //loginfo(logStr,logFilePath);

    SYSTEMTIME st;
    GetLocalTime(&st);
    string time_stamped_folder = "";
    unsigned int year = st.wYear;
    unsigned int month = st.wMonth;
    unsigned int day = st.wDay;
    unsigned int hours = st.wHour;

    std::cout << "IN SAVEIMAGE\n";
    
    //concatenate string to create folder path
    time_stamped_folder = to_string(year)+ "_" +to_string(month) + "_" + to_string(day) + "__" + to_string(hours);
    string time_stamped_path = root + "/" + time_stamped_folder;
    string logStr = "Camera ID " + id + " Saved " + time_stamped_path;
    loginfo(logStr, logFilePath);
    if (CreateDirectory(time_stamped_path.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError()) {
	//Directory created
    }

    std::cout << "directory created: " << time_stamped_path << "\n";

    //cout << "saving. ID: " << id << "\n";
    string t = get_time();
    string t2 = get_time_2();

    // Create formatted string so that images are sorted as they get put into
    // the folder
    char fileName[100];
    char* c_time_stamped_path = new char[time_stamped_path.length() + 1];
    strcpy_s( c_time_stamped_path, time_stamped_path.length() + 1,
	time_stamped_path.c_str());
    // c_time_stamped_path is correct now

    stringstream ss;
    ss.str(id);
    int newid = stoi(id);
    int testvar = id.length();

    char* c_id = new char[id.length() + 1];
    strcpy_s(c_id, id.length() + 1, id.c_str());

    char* c_t = new char[t.length() + 1];
    strcpy_s(c_t, t.length() + 1, t.c_str());

    char* c_t2 = new char[t2.length() + 1];
    strcpy_s(c_t2, t2.length() + 1, t2.c_str());

    string secondcount_str = make_fixed_length(secondcount, 3);
    char* c_secondcount = new char[secondcount_str.length() + 1];
    strcpy_s(c_secondcount, secondcount_str.length() + 1, secondcount_str.c_str());

    //cout << c_time_stamped_path << " and " << c_id << " and " << c_t << "\n";
    //sprintf_s(fileName, "%s/Flake%.6u_Cam%s_%u_%s.bmp", c_time_stamped_path, count, c_id, picNum, c_t); //old image file name saving convention
    //cout << "  OG: " << fileName << "\n";
    sprintf_s(fileName, "%s/%s%sCAM%s_%u.bmp", c_time_stamped_path, c_t2, c_secondcount, c_id, picNum); // new image file name saving convention: path/YYYYMMDDHHMMSS{secondcount}Cam#_#
    //cout << "  new: " << fileName << "\n";
    //cout << "  global count: " << secondcount << "\n";
    //printf("%s", fileName);

    string file = time_stamped_path + "/Flake" + to_string(count) + "_Cam" + id + "_" + to_string(picNum) + "_" + t + ".bmp";
    //im.Save(file.c_str())

    FlyCapture2::Error error;
    cout << id << "\n";
    //cout << "ID: "<< id << im.Image.receivedDataSize << "\n";

    error = im.Save(fileName);
    if (error != FlyCapture2::PGRERROR_OK) {
	cout << "error";
	error.PrintErrorTrace();
    }

    std::terminate;
}

class SSS_Camera
{
public:
    unsigned int id;
    Camera* cam;
    Image Im;
    Image Im1;
    Image Im2, Im3, Im4, Im5, Im6;
    string path;
    string msg;
    string filename;
    string logFilePath;
    string baselogFilePath;
    int lasthour;
    int lastsecond;
    int secondcount;

    SSS_Camera()
    {
	//Default constructor...
    }

    //Constructor method...
    SSS_Camera(unsigned int id, Camera* cam, string path)
    {
	cout << "\nCamera Object Created\n";
	Image im;
	Image im1; //im2;, im3, im4, im5, im6;
	//Image im1;
	this->id = id;
	this->cam = cam;
	this->Im = im;
	this->path = path;
	this->msg = "Successfully started image capture for camera " + to_string(this->id);
	this->logFilePath = "C:\\Users\\Z440\\oneDrive - Colostate\\Desktop\\Logs\\Logs.txt";
	this->baselogFilePath= "C:\\Users\\Z440\\oneDrive - Colostate\\Desktop\\Logs\\";
	this->lasthour = -1;
	this->secondcount = -1;
	//if (this->id == 1)
	//{
	    //When high speed camera is being run
	    //this->Im1 = im1;
	    //this->Im2 = im2;
	    //this->Im3 = im3;
	    //this->Im4 = im4;
	    //this->Im5 = im5;
	    //this->Im6 = im6;
	//}   // This section commented out to try to get camera 1 to only take 1 image
    }
    //The method to be run within the separate threads
    void run_Cam(std::atomic<bool>& program_is_running, std::atomic<int>& imagesCapturedSinceSave, std::atomic<bool>& allowSave, std::atomic<int>& globalFlakeCount)
    {
	Error error;
	bool hasPicture0 = false;
	bool hasPicture1 = false;
	//bool hasPicture2 = false;
	//bool hasPicture3 = false;
	//bool hasPicture4 = false;
	//bool hasPicture5 = false;
	//bool hasPicture6 = false;
	//cout << this->msg << endl;
	string id = to_string(this->id);
	string root = this->path;
	unsigned int count = 0;
	unsigned int rows = 0;
	unsigned int cols = 0;
	int return_val = 0;


	while (program_is_running)
	{
	    string t = get_time();
	    int currhour = timetohour(t, "-");
	    if (currhour != lasthour) {
		this->lasthour = currhour;
		string dwithhour = dateandhour(t, "-");
		string finalLogPath = this->baselogFilePath + dwithhour + "logs.txt";
		this->logFilePath = finalLogPath;
	    }

	    int current_second = get_current_second();
	    if (current_second != lastsecond) {
		this->lastsecond = current_second;
		secondcount = 0;
	    }
	    if(false){
	    //if (this->id == 1) {
	    //if (false){
		Error error0;
		Error error1;



		if (hasPicture0 == false) {
		    error0 = this->cam->RetrieveBuffer(&this->Im); // checks if image was captured, returns PGERROR_OK is true
		    string errorDescription0 = error0.GetDescription();
		    if (errorDescription0 != "No buffer arrived within the specified timeout.") {
			cout << "Error returned by RetrieveBuffer (cam1,image0): " << error0.GetDescription() << id << "\n";
			cout << "error0: ";
			PrintError(error0);

		    }
		}
		if (hasPicture0 == true && hasPicture1 == false) {
		    error1 = this->cam->RetrieveBuffer(&this->Im1);
		    string errorDescription1 = error1.GetDescription();
		    if (errorDescription1 != "No buffer arrived within the specified timeout.") {
			cout << "Error returned by RetrieveBuffer (cam1,image1): " << error1.GetDescription() << id << "\n";
		    }
		}
		// if retrieving from the buffer was successful, set has picture0 to true




		// if only one image has been captured so far
		// update the shutter speed and gain for the second image
		if (error0 == PGRERROR_OK) {
		    //cout << "high speed captured first frame\n";
		    //std::cout << "PGERROR_OK 0" << endl;
		    hasPicture0 = true;

		    // Set the shutter property of the camera
		    Property shutterProp;
		    float HS_Shutter = 30.0f;//0.011f;//0.032f;//500.0f;
		    float HS_Gain = 15.0f; //20
		    cout << "HEEEEELPPPPPPP!!!!!!!!!!";
		    if (setProperty(*cam, shutterProp, SHUTTER, HS_Shutter) != 0)
		    {
			std::cout << "  SMAS: could not set shutter property." << endl;
		    }
		    else
		    {
			std::cout << "  Successfully set shutter time for camera " << fixed << setprecision(2) << HS_Shutter << " ms" << endl;
		    }

		    // Set the gain property of the camera
		    Property gainProp;



		    if (setProperty(*cam, gainProp, GAIN, HS_Gain) != 0)
		    {
			std::cout << "  SMAS: could not set gain property." << endl;
		    }
		    else
		    {
			std::cout << "  Successfully set gain to " << HS_Gain << " dB" << endl;
		    }

		}
		// If both images have been captured, do the following:
		//PGRERROR_OK indicates image sucessfully captured
		//Try only incrementing imagesCapturedSinceSave if both images have been captured
		if (error1 == PGRERROR_OK)
		{
		    //cout << "Incrementing counter for Cam " << id << "Value before incrementing: " << imagesCapturedSinceSave << "\n";
		    // Temporarily try only incrementing this if it is 0, otherwise pause for a moment and then output its value
		    imagesCapturedSinceSave++;



		    // Then set shutter speed and gain back to where they were before
		    //cout << "high speed captured second frame\n";
		    //std::cout << "PGERROR_OK 1" << endl;
		    hasPicture1 = true;

		    Property shutterProp;
		    float HS_Shutter = 18.5f;//0.011f;//0.032f;//3.0f;//500.0f;
		    float HS_Gain = 20.0f;//140.0f;
		    cout << "HEEEEELPPPPPPPPPP";
		    if (setProperty(*cam, shutterProp, SHUTTER, HS_Shutter) != 0)
		    {
			std::cout << "  SMAS: could not set shutter property." << endl;
		    }
		    else
		    {
			std::cout << "  Successfully set shutter time for camera " << fixed << setprecision(2) << HS_Shutter << " ms" << endl;
		    }

		    // Set the gain property of the camera
		    Property gainProp;

		    if (setProperty(*cam, gainProp, GAIN, HS_Gain) != 0)
		    {
			std::cout << "  SMAS: could not set gain property." << endl;
		    }
		    else
		    {
			std::cout << "  Successfully set gain to " << HS_Gain << " dB" << endl;
		    }
		}
		bool t = true;
		if (allowSave.compare_exchange_strong(t, false)) {
		    imagesCapturedSinceSave = 0;
		    thread saveThread(saveImage, Im, (int)globalFlakeCount, 1, id, root, logFilePath, secondcount);
		    thread saveThread2(saveImage, Im1, (int)globalFlakeCount, 2, id, root, logFilePath, secondcount);
		    secondcount++;
		    saveThread.detach();
		    saveThread2.detach();
		    //NOW WE HAVE TO RESET THE HAS PICTURE VARIABLES
		    hasPicture0 = false;
		    hasPicture1 = false;
		}
	    }
	    else {
		error = this->cam->RetrieveBuffer(&this->Im);
		string errorDescription = error.GetDescription();
		if (errorDescription != "No buffer arrived within the specified timeout." && errorDescription != "Ok.") {
		    cout << "Here is the error returned by RetrieveBuffer: " << error.GetDescription() << id << "\n";
		}
		//PGRERROR_OK indicates image sucessfully captured
		if (error == PGRERROR_OK)
		{
		    cout << "PGRERROR Camera ID: " << id << "\n";
		    //cout << "TESTTINSETINA;SKLNGA;LKSDJF;AJ" << "image captured by " << id << "\n";
		    //cout << "Incrementing counter for Cam " << id << "Value before incrementing: " << imagesCapturedSinceSave << "\n";
		    // Temporarily try only incrementing this if it is 0, otherwise pause for a moment and then output its value
		    imagesCapturedSinceSave++;

		}
		bool t = true;
		if (allowSave.compare_exchange_strong(t, false)) {
		    imagesCapturedSinceSave = 0;
		    thread saveThread(saveImage, Im, (int)globalFlakeCount, 1, id, root, logFilePath, secondcount);
		    secondcount++;
		    saveThread.detach();
		}
	    }

	    //Sleep(DWORD(2));
	}
	//return return_val;
    }
};


void PrintBuildInfo()
{
    FC2Version fc2Version;
    Utilities::GetLibraryVersion(&fc2Version);

    ostringstream version;
    version << "FlyCapture2 library version: " << fc2Version.major << "." << fc2Version.minor << "." << fc2Version.type << "." << fc2Version.build;
    std::cout << version.str() << endl;

    ostringstream timeStamp;
    timeStamp << "Application build date: " << __DATE__ << " " << __TIME__;
    std::cout << timeStamp.str() << endl << endl;
}


void PrintCameraInfo(CameraInfo* pCamInfo)
{
    std::cout << endl;
    std::cout << "*** CAMERA INFORMATION ***" << endl;
    std::cout << "Serial number -" << pCamInfo->serialNumber << endl;
    std::cout << "Camera model - " << pCamInfo->modelName << endl;
    std::cout << "Camera vendor - " << pCamInfo->vendorName << endl;
    std::cout << "Sensor - " << pCamInfo->sensorInfo << endl;
    std::cout << "Resolution - " << pCamInfo->sensorResolution << endl;
    std::cout << "Firmware version - " << pCamInfo->firmwareVersion << endl;
    std::cout << "Firmware build time - " << pCamInfo->firmwareBuildTime << endl << endl;


}


void static updateAtomics(int numThreads, std::atomic<bool>& program_is_running, std::atomic<int>* imagesCapturedSinceSave, std::atomic<bool>* allowSave, std::atomic<int>& globalFlakeCount)
{

    bool droppedFrame = false;
    bool readyToSave;
    while (program_is_running) {
	readyToSave = true;
	int frameDropCam = -1;
	for (int i = 0; i < numThreads; i++) { // numthreads = 7 = number of cameras
	    //std::cout << "imagesCapturedSinceSave[i]: " <<imagesCapturedSinceSave[i] << "; thread number: " << i << endl;
	    //cout << imagesCapturedSinceSave[i];
	    if (i != 1) {
		switch (imagesCapturedSinceSave[i]) {
		case 2: // a frame was dropped, if imagesCapturedSinceSave[i] == 2
		    cout << "FRAME WAS DROPPED, READY TO SAVE VALUE: " << readyToSave << "\n";
		    droppedFrame = true;
		    readyToSave = false;
		    frameDropCam = i;
		    break;
		case 1: // an image was captured correctly
		    cout << "IMAGE CAPTURED SUCCESSFULLY" << imagesCapturedSinceSave[i] << "\n";
		    readyToSave = true;
		    cout << "READY TO SAVE? :" << readyToSave << "Dropped frame? :" << droppedFrame << "\n";
			imagesCapturedSinceSave[i] = 0;
		    break;
		case 0: // still waiting for capture
		    readyToSave = false;
		    break;
		default:
		    cout << imagesCapturedSinceSave[i] << "\n";
		}
	    }
	    else {
		switch (imagesCapturedSinceSave[i]) {
		case 2: // a frame was dropped, if imagesCapturedSinceSave[i] == 2
		    cout << "FRAME WAS DROPPED, READY TO SAVE VALUE: " << readyToSave << "\n";
		    droppedFrame = true;
		    readyToSave = false;
		    frameDropCam = i;
		    imagesCapturedSinceSave[i] = 0;
		    break;
		case 1: // an image was captured correctly
		    cout << "IMAGE CAPTURED SUCCESSFULLY" << imagesCapturedSinceSave[i] << "\n";
		    readyToSave = true;
		    cout << "READY TO SAVE? :" << readyToSave << "Dropped frame? :" << droppedFrame << "\n";
		    break;
		case 0: // still waiting for capture
		    readyToSave = false;
		    break;
		default:
		    cout << imagesCapturedSinceSave[i] << "\n";
		}
	    }
	    if (droppedFrame) {
		break;
	    }
	}
	// the following handles when a save is skipped
	if (droppedFrame) {
	    cout << "FRAME COUNT MANAGER: skipped save: " << frameDropCam << "\n";
	    for (int i = 0; i < numThreads; i++) {
		switch (imagesCapturedSinceSave[i]) {
		case 1:
		    imagesCapturedSinceSave[i] = 0;
		    std::cout << "HERE IS CASE 1!" << "imagesCapturedSinceSave[i]: " << imagesCapturedSinceSave[i] << "; thread number: " << i << endl;
		    break;
		case 2:
		    imagesCapturedSinceSave[i] = 1; //EDIT THIS MAYBE?
		    cout << "Is anything happening here?" << endl;
		    break;
		default:
		    cout << imagesCapturedSinceSave[i] << "\n";
		}
	    }
	    droppedFrame = false;
	}
	//cout << "Ready to Save pt. 2? :" << readyToSave << "\n";
	if (readyToSave) {
	    ++globalFlakeCount;
	    // set allow save flags to true to let runCam thread save captures
	    cout << "FRAME COUNT MANAGER: allowing saves. " << globalFlakeCount << "\n";
	    //cout << "Num Threads: " << numThreads << "\n" << endl;
	    for (int i = 0; i < numThreads; i++) {
		allowSave[i] = true;
	    }
	}
	readyToSave = false;
	Sleep(DWORD(2));
    }
}


//**************************************************************************

int main(int /*argc*/, char** /*argv*/)
{
    //Parameters (Values that can be changed)
    //string p = "C:/Test_07_21_21"; // CHANGE THIS STRING TO CHANGE THE FOLDER WHERE IMAGES ARE SAVED. FOLDER MUST ALREADY EXIST.
    string logFilePath = R"(C:\Users\Z440\oneDrive - Colostate\Desktop\Logs\Logs.txt)";
    string p = "D:/UCONN_2023_2024";
    loginfo("Starting program", logFilePath);
    cout << "it is here\n\n";
    

    bool calibration = true;        //True uses calShutter and calGain instead of default value
				    //False runs normal settings (Use for actual recording)

    bool highSpeedTrail = true;     //True uses HS_Shutter and HS_Gain for the high speed camera 
				    //False uses same values as the other cameras (No camera streak/trail)

				    //NOTE: Calibration overrides highSpeedTrail (if calibration == true, higSpeedTrail ignored)
    //Manual Values for shutter and gain
    // calShutter and calGain are for all cameras except 1 (change these)
    float calShutter = 0.02f;//0.011f;//0.5f; // Changed from 2 to 1 1/3/22, Changed from 1 to 0.7 1/18/22, Changed from 0.2 to 0.025 on 11/15/22
    float calGain = 30.0f;//140.0f; // Changed from 15.0 to 18.0 1/18/22. changed from 35 to 50 on 11/27/22 //was 140f
    float HS_Shutter = 20.0f; // 0.011f;//500.0f; Changed from 0.2 to 0.029 on 11/15/22 changed from 0.032 to .025 1-22-23
    float HS_Gain = 18.5f;//140.0f;

    float cam0_Gain = 30.0f;
    float cam2_Gain = 25.0f;
    float cam3_Gain = 25.0f;
    float cam4_Gain = 25.0f;
    float cam5_Gain = 30.0f;
    float cam6_Gain = 30.0f; // Changed from 5 to 6 1/18/22
    //cam1Gain and cam1Shutter are gain and shutter speed for camera (change test)
    float cam1Gain = 15.0f; //140
    float cam1Shutter = 30.0f; //ms  orig value .5f

    float cam5Shutter = 0.035f;
    float cam3Shutter = 0.035f;
    float cam4Shutter = 0.040f;
    float cam6Shutter = 0.040f;


    
    //**********************************************************************
    // Commands to start running the background program
    fstream	logFile;
    string	path = "C:/LogFiles/log_" + get_time() + ".txt";
    logFile.open(path, fstream::out);
    logFile << "Starting Time: " << get_time() << "\n";

    PrintBuildInfo();
    EmbeddedImageInfo   EmbeddedInfo;

    BusManager		busMgr;
    unsigned int	numCameras;

    error = busMgr.GetNumOfCameras(&numCameras);
    if (error != PGRERROR_OK)
    {
	PrintError(error);
	return -1;
    }

    std::cout << "Number of cameras detected: " << numCameras << endl;
    logFile   << "Number of cameras detected: " << numCameras << "\n";

    if (numCameras < 1)
    {
	std::cout << "Insufficient number of cameras... press Enter to exit." << endl;
	logFile << "INSUFFICIENT NUMBER OF CAMERAS DETECTED. THIS ERROR IS FATAL. ";
	cin.ignore();
	return -1;
    }

    Camera**	ppCameras = new Camera * [numCameras];

    // Connect to all detected cameras and attempt to set them to
    // a common video mode and frame rate
    for (unsigned int i = 0; i < numCameras; i++)
    {
	ppCameras[i] = new Camera();

	PGRGuid guid;
	error = busMgr.GetCameraFromIndex(i, &guid);
	if (error != PGRERROR_OK)
	{
	    PrintError(error);
	    return -1;
	}

	// Connect to a camera
	error = ppCameras[i]->Connect(&guid);
	if (error != PGRERROR_OK)
	{
	    PrintError(error);
	    return -1;
	}

	// Get the camera information
	CameraInfo camInfo;
	error = ppCameras[i]->GetCameraInfo(&camInfo);
	if (error != PGRERROR_OK)
	{
	    PrintError(error);
	    return -1;
	}
	PrintCameraInfo(&camInfo);
    }

    Camera** ppCamerasSorted = new Camera * [numCameras];

    for (unsigned int i = 0; i < numCameras; i++)
    {
	PGRGuid guid;
	error = busMgr.GetCameraFromIndex(i, &guid);
	if (error != PGRERROR_OK)
	{
	    PrintError(error);
	    return -1;
	}
	CameraInfo camInfo;
	error = ppCameras[i]->GetCameraInfo(&camInfo);
	if (error != PGRERROR_OK)
	{
	    PrintError(error);
	    return -1;
	}
	//Initializing the camera serial numbers and sorting the array of
	// camera objects
	if (camInfo.serialNumber == 15444692) {
	    ppCamerasSorted[0] = ppCameras[i]; //15444692 = camera 0
	    std::cout << "Camera 0 is declared" << "\n";
	}

	if (camInfo.serialNumber == 13510226) {
		//SPEED CAMERA!!! 13510226 = camera 1
	    ppCamerasSorted[1] = ppCameras[i];
	    std::cout << "Camera 1 is declared" << "\n";
	}

	if (camInfo.serialNumber == 15444697) {
	    ppCamerasSorted[2] = ppCameras[i]; //15444697 = camera 2
	    std::cout << "Camera 2 is declared" << "\n";
	}
	if (camInfo.serialNumber == 15444696) {
	    ppCamerasSorted[3] = ppCameras[i]; //15444696 = camera 3
	    std::cout << "Camera 3 is declared" << "\n";
	}
	if (camInfo.serialNumber == 15405697) {
	    ppCamerasSorted[4] = ppCameras[i]; //15405697 = camera 4
	    std::cout << "Camera 4 is declared" << "\n";
	}
	if (camInfo.serialNumber == 15444687) {
	    ppCamerasSorted[5] = ppCameras[i]; //15444687 = camera 5
	    std::cout << "Camera 5 is declared" << "\n";
	}
	if (camInfo.serialNumber == 15444691) {
	    ppCamerasSorted[6] = ppCameras[i]; //15444691 = camera 6
	    std::cout << "Camera 6 is declared" << "\n";
	}
    }

    //cout << "The whole array: " << ppCamerasSorted << endl;
    //cout << "The 0th instance: " << ppCamerasSorted[1] << endl;

    for (unsigned int i = 0; i < numCameras; i++)
    {
	FC2Config	Config;

	// Set buffered modes
	error = ppCamerasSorted[i]->GetConfiguration(&Config);
			//error happen here
	if (error != PGRERROR_OK)
	{
	    PrintError(error);
	    return -1;
	}

	Config.numBuffers                    = 50;
	Config.grabMode                      = BUFFER_FRAMES;
	Config.highPerformanceRetrieveBuffer = true;
	Config.grabTimeout                   = 0;
		// was originally 0, set to 100000 for testing on 9/10/21 -Peter
	//Config.registerTimeoutRetries = 5;
	//Config.registerTimeout = 10000;
	//Config.asyncBusSpeed = BUSSPEED_S_FASTEST;

	ppCamerasSorted[i]->SetConfiguration(&Config);

	error = ppCamerasSorted[i]->GetConfiguration(&Config);

	std::cout << "Buffer Mode: "       << Config.grabMode    << endl;
	std::cout << "Number of Buffers: " << Config.numBuffers  << endl;
	std::cout << "Grab Timeout: "      << Config.grabTimeout << endl;
	if (error != PGRERROR_OK)
	{
	    PrintError(error);
	    return -1;
	}

	PGRGuid guid;

	// Get current trigger settings
	TriggerMode	triggerMode;
	error = ppCamerasSorted[i]->GetTriggerMode(&triggerMode);
	if (error != PGRERROR_OK)
	{
	    PrintError(error);
	    return -1;
	}

	// Set camera to trigger mode 0 (standard ext. trigger)
	//   Set GPIO to receive input from pin 0

	triggerMode.onOff     = true;
	triggerMode.mode      = 0;
	triggerMode.parameter = 0;
	triggerMode.polarity  = 0;
	triggerMode.source    = 0;

	error = ppCamerasSorted[i]->SetTriggerMode(&triggerMode);
	if (error != PGRERROR_OK)
	{
	    PrintError(error);
	    return -1;
	}

	std::cout << "  Successfully set to external trigger, low polarity, source GPIO 0." << endl;

	// Set the shutter property of the camera
	Property	shutterProp;
	float k_shutterVal = 0.02f;//0.011f;        //Default Value

	if (calibration) {
	    if (i == 1) {
		k_shutterVal = cam1Shutter;
	    }
	    else {
		k_shutterVal = calShutter;
	    }
	}
	else if (i == 1) { //&& highSpeedTrail) {
	    k_shutterVal = HS_Shutter;
	}
	
	if (i == 3) {
	    k_shutterVal = cam3Shutter;
	}

	if (i == 4) {
	    k_shutterVal = cam4Shutter;
	}

	if (i == 5) {
	    k_shutterVal = cam5Shutter;
	}

	if (i == 6) {
	    k_shutterVal = cam6Shutter;
	}


	if (setProperty(*ppCamerasSorted[i], shutterProp, SHUTTER, k_shutterVal) != 0)
	{
	    std::cout << "  SMAS: could not set shutter property." << endl;
	}
	else
	{
	    std::cout << "  Successfully set shutter time for camera " << i << " to " << fixed << setprecision(2) << k_shutterVal << " ms" << endl;
	}

	// Set the gain property of the camera
	Property	gainProp;
	float		k_gainVal = 30.0f;

	if (calibration) {
	    if (i == 1) {
		k_gainVal = cam1Gain;
	    }
	    else {
		k_gainVal = calGain;
	    }
	}
	else if (i == 1) { // && highSpeedTrail) {
	    k_gainVal = HS_Gain;
	}
	if (i == 0) {
	    k_gainVal = cam0_Gain;
	}

	if (i == 2) {
	    k_gainVal = cam2_Gain;
	}

	if (i == 3){
	    k_gainVal = cam3_Gain;
	}
	
	if (i == 4) {
	    k_gainVal = cam4_Gain;
	}

	if (i == 5){
	    k_gainVal = cam5_Gain;
	}

	if (i == 6) { // Specifically adjusting cam6 gain}
	    k_gainVal = cam6_Gain;
	    cout << "Successfully set Cam6 Gain" << endl;
	}

	if ( setProperty( *ppCamerasSorted[i], gainProp, GAIN, k_gainVal ) != 0)
	{
	    std::cout << "  SMAS: could not set gain property." << endl;
	}
	else
	{
	    std::cout << "  Successfully set gain to "
		<< fixed << setprecision(2) << k_gainVal << " dB" << endl;
	}

	// Get current embedded image info
	error = ppCamerasSorted[i]->GetEmbeddedImageInfo( &EmbeddedInfo );
	if (error != PGRERROR_OK)
	{
	    PrintError(error);
	    return -1;
	}

	// If camera supports timestamping, set to true
	if (EmbeddedInfo.timestamp.available == true)
	{
	    EmbeddedInfo.timestamp.onOff = true;
	    std::cout << "  Successfully enabled timestamping." << endl;
	}
	else
	{
	    std::cout << "Timestamp is not available!" << endl;
	}

	// If camera supports frame counting, set to true
	if (EmbeddedInfo.frameCounter.available == true)
	{
	    EmbeddedInfo.frameCounter.onOff = true;
	    std::cout << "  Successfully enabled frame counting." << endl;
	}
	else
	{
	    std::cout << "Framecounter is not avalable!" << endl;
	}

	// Sets embedded info
	error = ppCamerasSorted[i]->SetEmbeddedImageInfo(&EmbeddedInfo);
	if (error != PGRERROR_OK)
	{
	    PrintError(error);
	    return -1;
	}

	// Initializing camera capture capability
	error = ppCamerasSorted[i]->StartCapture();
	if (error != PGRERROR_OK)
	{
	    PrintError(error);
	    std::cout << "Error starting to capture images. Error from camera "
		<< i << endl
		<< "Press Enter to exit." << endl;
	    cin.ignore();
	    return -1;
	}

    }

    std::cout << "\nBeginning capture. Waiting for signal..." << endl;

    logFile << "The sequential setup has been completed. All cameras detected have been declared. \n";

    SSS_Camera* CamList = new SSS_Camera[numCameras];

    // TODO: ideally the above should be inputted as an argument when
    // starting execution of the application

    bool exitC = false;
    for (unsigned int i = 0; i < numCameras; i++)
    {
	//Clear all camera data first
	error = ppCamerasSorted[i]->ResetStats();
	CamList[i] = SSS_Camera(i, ppCamerasSorted[i], p);
    }
    logFile << "All objects for the parallel threads have been declared. \n";

    // This is the portion of the code that starts to perform the parallel setup
    int nThreads = numCameras;
    thread* threadList = new thread[nThreads];

    std::atomic<bool> program_is_running{ true };

    std::atomic<int>* imagesCapturedSinceSave = new std::atomic<int>[nThreads]; // number of images caputured by each thread since last save
    std::atomic<bool>* allowSave = new std::atomic<bool>[nThreads]; // 0: don't save, 1: save

    std::atomic<int> globalFlakeCount = -1;

    for (int i = 0; i < nThreads; i++) {
	imagesCapturedSinceSave[i] = 0;
	allowSave[i] = false;
    }

    // thread to coordinate saving images
    thread updateAtomicsThread(updateAtomics, nThreads, std::ref(program_is_running), imagesCapturedSinceSave, allowSave, std::ref(globalFlakeCount));
    std::cout << "updateAtomics called: " << endl;
    for (int i = 0; i < nThreads; i++) {
	std::cout << "imagesCapturedSinceSave: " << imagesCapturedSinceSave[i] << "; thread number: " << i << "\n" << endl;
    }

    for (unsigned int i = 0; i < nThreads; i++)
    {
	threadList[i] = thread(&SSS_Camera::run_Cam, CamList[i], std::ref(program_is_running), std::ref(imagesCapturedSinceSave[i]), std::ref(allowSave[i]), std::ref(globalFlakeCount));

    }
    logFile << "Threads are running. Time is: " << get_time() << "\n";
    logFile.close();
    string command;
    std::cout << "There are now " + to_string(nThreads) + " threads running!" << endl;
    std::cout << "Enter 'exit' to just stop the camera capture and exit the application. " << endl << endl;
    while (true)
    {
	cin >> command;
	if (command == "exit")
	{
	    std::cout << "Exiting...\n";
	    break;
	}
	std::cout << "You typed: " << command << " which is not a valid command. ";
    }
    exitC = true;
    if (command == "exit")
    {
	program_is_running = false;
	for (unsigned int i = 0; i < numCameras; i++)
	{
	    ppCameras[i]->StopCapture();
	    ppCameras[i]->Disconnect();
	    delete ppCameras[i];
	}
	std::cout << "Cameras disconnected.\n";
	delete[] ppCameras;
	delete[] ppCamerasSorted;
	delete[] imagesCapturedSinceSave;
	delete[] allowSave;
	return 0;
    }
    for (unsigned int i = 0; i < nThreads; i++)
    {
	threadList[i].join(); // Wait for the threads to finish...
	updateAtomicsThread.join();
    }
    return 0;
}
