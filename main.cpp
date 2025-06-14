/*
#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <conio.h>
#include <vector>
#include "TeliCamApi.h"
#include "TeliCamUtl.h"

using namespace Teli;

#define DURATION_CAPTURE_MIN 1
#define INTERVAL_MIN 30
#define FRAME_RATE 20.0

std::atomic<bool> running(true);

void configureCamera(CAM_HANDLE hCam) {
    SetCamTriggerMode(hCam, false); // start in free-run to avoid blocking
    SetCamAcquisitionFrameRate(hCam, FRAME_RATE);
}

void enableSoftwareTrigger(CAM_HANDLE hCam) {
    SetCamTriggerMode(hCam, true);
    SetCamTriggerSource(hCam, CAM_TRIGGER_SOFTWARE);
}

void captureStereoImages(
    CAM_HANDLE camLeft, CAM_HANDLE camRight,
    CAM_STRM_HANDLE strmLeft, CAM_STRM_HANDLE strmRight,
    int& frameCounter)
{
    using namespace std::chrono;
    auto start = steady_clock::now();
    auto end = start + minutes(DURATION_CAPTURE_MIN);
    milliseconds interval(static_cast<int>(1000.0 / FRAME_RATE));

    uint32_t payloadSizeLeft = 0, payloadSizeRight = 0;
    uint32_t width = 0, height = 0;

    Strm_GetPayloadSize(strmLeft, &payloadSizeLeft);
    Strm_GetPayloadSize(strmRight, &payloadSizeRight);
    GetCamWidth(camLeft, &width);
    GetCamHeight(camLeft, &height);

    std::vector<uint8_t> bufferLeft(payloadSizeLeft);
    std::vector<uint8_t> bufferRight(payloadSizeRight);

    while (steady_clock::now() < end && running) {
        ExecuteCamSoftwareTrigger(camLeft);
        ExecuteCamSoftwareTrigger(camRight);

        if (Strm_ReadCurrentImage(strmLeft, bufferLeft.data(), &payloadSizeLeft, nullptr) == CAM_API_STS_SUCCESS &&
            Strm_ReadCurrentImage(strmRight, bufferRight.data(), &payloadSizeRight, nullptr) == CAM_API_STS_SUCCESS) {

            char fnameLeft[128], fnameRight[128];
            sprintf_s(fnameLeft, "C:\\StereoImages\\left_%06d.bmp", frameCounter);
            sprintf_s(fnameRight, "C:\\StereoImages\\right_%06d.bmp", frameCounter);

            SaveBmpMono(bufferLeft.data(), width, height, fnameLeft);
            SaveBmpMono(bufferRight.data(), width, height, fnameRight);
        }

        frameCounter++;
        std::this_thread::sleep_for(interval);

        if (_kbhit() && _getch() == 'q') {
            running = false;
            break;
        }
    }
}

int main() {
    std::cout << "Program started\n";
    CAM_STRM_HANDLE strmLeft = 0, strmRight = 0;
    SIGNAL_HANDLE signalLeft = nullptr, signalRight = nullptr;

    auto sdk_status = Sys_Initialize(CAM_TYPE_U3V | CAM_TYPE_GEV);
    if (sdk_status != CAM_API_STS_SUCCESS) {
        std::cout << "Failed to initialize TeliCamSDK. Error code: " << sdk_status << std::endl;
        return 1;
    }

    uint32_t numCams = 0;
    auto status = Sys_GetNumOfCameras(&numCams);
    if (status != CAM_API_STS_SUCCESS || numCams < 2) {
        std::cout << "Failed to find two cameras. Status: " << status << std::endl;
        Sys_Terminate();
        return 1;
    }

    CAM_HANDLE camA = 0, camB = 0;
    status = Cam_Open(0, &camA);
    status |= Cam_Open(1, &camB);
    if (status != CAM_API_STS_SUCCESS) {
        std::cout << "Failed to open both cameras. Status: " << status << std::endl;
        Sys_Terminate();
        return 1;
    }

    CAM_INFO_EX infoA = {}, infoB = {};
    Cam_GetInformationEx(camA, 0, &infoA);
    Cam_GetInformationEx(camB, 0, &infoB);

    CAM_HANDLE camLeft = (strcmp(infoA.szCamSerialNumber, "0106656") == 0) ? camA : camB;
    CAM_HANDLE camRight = (camLeft == camA) ? camB : camA;

    configureCamera(camLeft);
    configureCamera(camRight);

    status = Sys_CreateSignal(&signalLeft);
    status |= Sys_CreateSignal(&signalRight);
    if (status != CAM_API_STS_SUCCESS) {
        std::cout << "Failed to create signal handles. Status: " << status << std::endl;
        return 1;
    }

    uint32_t payloadSizeLeft = 0, payloadSizeRight = 0;
    status = Strm_OpenSimple(camLeft, &strmLeft, &payloadSizeLeft, signalLeft, 5, 9000);
    if (status != CAM_API_STS_SUCCESS) {
        std::cout << "Failed to open stream for left cam. Status: " << status << std::endl;
        return 1;
    }

    status = Strm_OpenSimple(camRight, &strmRight, &payloadSizeRight, signalRight, 5, 9000);
    if (status != CAM_API_STS_SUCCESS) {
        std::cout << "Failed to open stream for right cam. Status: " << status << std::endl;
        return 1;
    }

    Sleep(100);  // stabilize after stream open

    status = Strm_Start(strmLeft);
    status |= Strm_Start(strmRight);
    if (status != CAM_API_STS_SUCCESS) {
        std::cout << "Failed to start stream(s). Status: " << status << std::endl;
        return 1;
    }

    // Now enable software trigger after streams are live
    enableSoftwareTrigger(camLeft);
    enableSoftwareTrigger(camRight);

    int frameCounter = 0;
    std::cout << "Press 'q' to quit...\n";
    while (running) {
        captureStereoImages(camLeft, camRight, strmLeft, strmRight, frameCounter);
        if (!running) break;
        std::this_thread::sleep_for(std::chrono::minutes(INTERVAL_MIN - DURATION_CAPTURE_MIN));
    }

    Strm_Stop(strmLeft);
    Strm_Stop(strmRight);
    Cam_Close(camLeft);
    Cam_Close(camRight);
    Sys_Terminate();
    return 0;
}
*/

#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <conio.h>
#include <vector>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "TeliCamApi.h"
#include "TeliCamUtl.h"

using namespace Teli;

#define FRAME_RATE 20.0
#define BURST_DURATION_MIN 5
#define BURST_INTERVAL_MIN 30
#define BURST_FRAMES (FRAME_RATE * 60 * BURST_DURATION_MIN)

std::atomic<bool> running(true);

void configureCamera(CAM_HANDLE hCam) {
    SetCamTriggerMode(hCam, true);
    SetCamTriggerSource(hCam, CAM_TRIGGER_SOFTWARE);
    SetCamTriggerActivation(hCam, CAM_TRIGGER_RISING_EDGE);
    SetCamTriggerSequence(hCam, CAM_TRIGGER_SEQUENCE0);
    SetCamAcquisitionFrameRate(hCam, FRAME_RATE);
}

bool captureFrame(CAM_HANDLE hCam, CAM_STRM_HANDLE hStrm, SIGNAL_HANDLE hSignal, std::vector<uint8_t>& buffer, uint32_t& payloadSize) {
    ExecuteCamSoftwareTrigger(hCam);

    CAM_API_STATUS status = Sys_WaitForSignal(hSignal, 1000);
    if (status != CAM_API_STS_SUCCESS) return false;

    status = Strm_ReadCurrentImage(hStrm, buffer.data(), &payloadSize, nullptr);
    return status == CAM_API_STS_SUCCESS;
}

std::string getTimestampString() {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    std::time_t t = system_clock::to_time_t(now);
    std::tm tm;
    localtime_s(&tm, &t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d_%H%M%S") << '_' << std::setw(3) << std::setfill('0') << ms.count();
    return oss.str();
}

int main() {
    Sys_Initialize(CAM_TYPE_U3V | CAM_TYPE_GEV);

    uint32_t numCams = 0;
    Sys_GetNumOfCameras(&numCams);
    if (numCams < 2) return -1;

    CAM_HANDLE cam0, cam1;
    Cam_Open(0, &cam0);
    Cam_Open(1, &cam1);

    CAM_INFO_EX info0 = {}, info1 = {};
    Cam_GetInformationEx(cam0, 0, &info0);
    Cam_GetInformationEx(cam1, 0, &info1);

    CAM_HANDLE camLeft = (strcmp(info0.szCamSerialNumber, "0106656") == 0) ? cam0 : cam1; //Change serial number depending on which camera is used
    CAM_HANDLE camRight = (camLeft == cam0) ? cam1 : cam0;

    configureCamera(camLeft);
    configureCamera(camRight);

    SIGNAL_HANDLE sigLeft, sigRight;
    Sys_CreateSignal(&sigLeft);
    Sys_CreateSignal(&sigRight);

    CAM_STRM_HANDLE strmLeft, strmRight;
    uint32_t payloadSizeLeft, payloadSizeRight;
    Strm_OpenSimple(camLeft, &strmLeft, &payloadSizeLeft, sigLeft);
    Strm_OpenSimple(camRight, &strmRight, &payloadSizeRight, sigRight);

    std::vector<uint8_t> bufferLeft(payloadSizeLeft);
    std::vector<uint8_t> bufferRight(payloadSizeRight);

    uint32_t width = 0, height = 0;
    GetCamWidth(camLeft, &width);
    GetCamHeight(camLeft, &height);

    Strm_Start(strmLeft);
    Strm_Start(strmRight);

    int frameCounter = 0;
    std::cout << "Press 'q' to quit...\n";

    while (running) {
        auto burstStart = std::chrono::steady_clock::now();
        auto nextFrameTime = std::chrono::steady_clock::now();

        for (int i = 0; i < BURST_FRAMES && running; ++i) {
            nextFrameTime += std::chrono::milliseconds(static_cast<int>(1000.0 / FRAME_RATE));
            std::this_thread::sleep_until(nextFrameTime);

            auto triggerTime = getTimestampString();  // ?? Captured just before trigger

            bool okLeft = captureFrame(camLeft, strmLeft, sigLeft, bufferLeft, payloadSizeLeft);
            bool okRight = captureFrame(camRight, strmRight, sigRight, bufferRight, payloadSizeRight);

            if (okLeft && okRight) {
                char fnameLeft[128], fnameRight[128];
                sprintf_s(fnameLeft, "C:\\\\StereoImages\\\\left_%s.bmp", triggerTime.c_str()); //Change path for where it's saved
                sprintf_s(fnameRight, "C:\\\\StereoImages\\\\right_%s.bmp", triggerTime.c_str()); //Change path for where it's saved
                 
                SaveBmpMono(bufferLeft.data(), width, height, fnameLeft);
                SaveBmpMono(bufferRight.data(), width, height, fnameRight);
            }
            else {
                std::cerr << "Frame " << frameCounter << " capture failed.\n";
            }

            frameCounter++;

            if (_kbhit() && _getch() == 'q') {
                running = false;
                break;
            }
        }

        if (!running) break;

        std::cout << "Burst complete. Waiting for next burst...\n";
        std::this_thread::sleep_until(burstStart + std::chrono::minutes(BURST_INTERVAL_MIN));
    }

    Strm_Stop(strmLeft);
    Strm_Stop(strmRight);
    Cam_Close(camLeft);
    Cam_Close(camRight);
    Sys_Terminate();

    return 0;
}
