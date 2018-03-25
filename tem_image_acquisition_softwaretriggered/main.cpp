#include <iostream>
#include <string>
#include <Windows.h>
#include "uEye.h"

void detailed_error_message(const HIDS& hCam, bool quiet)
{
    INT errNum;
    IS_CHAR* errMessage;
    int errReturnValue = is_GetError(hCam, &errNum, &errMessage);
    if(errNum != IS_SUCCESS || errReturnValue != IS_SUCCESS)
    {
        if(errReturnValue != IS_SUCCESS)
        {
            std::cout << "GetError() return value " << errReturnValue << std::endl;
        }

        if(errNum != IS_SUCCESS)
        {
            std::cout << "Error number " << errNum << std::endl;
            std::wcout << "Error Message " << errMessage << std::endl;
            exit(1);
        }
    }
    else
    {
        if( ! quiet) { std::cout << "Success!" << std::endl; }
    }
}

int main(int argc, char **argv)
{
    std::string image_save_file_name;
    bool interactiveFilenames = 1;
    double exposure_time = 0;
    int gain_setting = 0;
    int blacklvl_setting = 0;
    bool quiet = false;
    for(int i = 1; i < argc; i += 2)
    {
        if(std::string(argv[i]) == "--exposure") { exposure_time = atof(argv[i+1]);  }
        if(std::string(argv[i]) == "--blacklvl")     { blacklvl_setting = atoi(argv[i+1]);   }
        if(std::string(argv[i]) == "--gain")     { gain_setting = atoi(argv[i+1]);   }
        if(std::string(argv[i]) == "--filename") { image_save_file_name = argv[i+1]; interactiveFilenames = 0;}
        if(std::string(argv[i]) == "--quiet")    { quiet = true; --i; }
    }

    bool errors = false;
    //if(image_save_file_name.empty())
    //{
     //   std::cerr << "Image file name must be supplied with --filename <name>" << std::endl;
      //  errors = true;
    //}

    if(exposure_time <= 0)
    {
        std::cerr << "Exposure time must be set to a positive number with --exposure <milliseconds>" << std::endl;
        errors = true;
    }

    if(gain_setting < 0 || gain_setting > 100)
    {
        std::cerr << "Gain setting must be set to a number between 0 and 100 with --gain <number>" << std::endl;
        errors = true;
    }

    if(blacklvl_setting < 0 || blacklvl_setting > 255)
    {
        std::cerr << "Black level setting must be set to a number between 0 and 255 with --blacklvl <number>" << std::endl;
        errors = true;
    }
    if(errors)
    {
        return 1;
    }

    SYSTEMTIME time;
    GetSystemTime(&time);
    LONG prev_time_ms = (time.wSecond*1000) + time.wMilliseconds;
    LONG delta_time;



    HIDS m_hCam = 1;
    if( ! quiet) { std::cout << "Initializing camera ..." << std::endl; }
    is_InitCamera (&m_hCam, NULL);
    detailed_error_message(m_hCam, quiet);


    if( ! quiet) { std::cout << "Getting sensor info ..." << std::endl; }
    SENSORINFO pInfo;
    is_GetSensorInfo (m_hCam, &pInfo);
    detailed_error_message(m_hCam, quiet);


    int width = pInfo.nMaxWidth;
    int height = pInfo.nMaxHeight;
    int bit_depth = 24;
    if( ! quiet) { std::cout << "Camera ID: " << m_hCam << std::endl; }
    if( ! quiet) { std::cout << "Sensor dimensions: " << width << " x " << height << " (assuming bit-depth per pixel of " << bit_depth << ")" << std::endl; }


    if( ! quiet) { std::cout << "Allocating memory for images ..." << std::endl; }
    char* image_memory;
    INT memory_ID;
    is_AllocImageMem(m_hCam, width, height, bit_depth, &image_memory, &memory_ID);
    detailed_error_message(m_hCam, quiet);


    if( ! quiet) { std::cout << "Setting image memory location ..." << std::endl; }
    is_SetImageMem(m_hCam, image_memory, memory_ID);
    detailed_error_message(m_hCam, quiet);

// Was EXP, FRAME RATE, PIXEL CLOCK





    if( ! quiet) { std::cout << "Getting current frame rate ..." << std::endl; }
    double new_frame_rate;
    is_SetFrameRate(m_hCam, IS_GET_FRAMERATE, &new_frame_rate);
    detailed_error_message(m_hCam, quiet);
    if( ! quiet) { std::cout << "Current frame rate " << new_frame_rate << std::endl << std::endl; }


    if( ! quiet) { std::cout << "Getting current exposure ..." << std::endl; }
    double exposure_ms;
    is_Exposure(m_hCam, IS_EXPOSURE_CMD_GET_EXPOSURE, &exposure_ms, sizeof(exposure_ms));
    detailed_error_message(m_hCam, quiet);
    if( ! quiet) { std::cout << "Current exposure: " << exposure_ms << " ms" << std::endl; }

   // SET current pixel clock
    int nPixelClock = 10;
    is_PixelClock(m_hCam, IS_PIXELCLOCK_CMD_SET, (void*)&nPixelClock, sizeof(nPixelClock));
    detailed_error_message(m_hCam, quiet);
    std::cout << "Just attempted setting pixel clock ..." << std::endl;

    // Set long exposure enable
    UINT expmode = 1;
    is_Exposure (m_hCam, IS_EXPOSURE_CMD_SET_LONG_EXPOSURE_ENABLE, (void*)&expmode, sizeof(expmode));
    detailed_error_message(m_hCam, quiet);

    // SET LOG MODE OFF
    UINT logmode = IS_LOG_MODE_OFF;
    is_DeviceFeature(m_hCam, IS_DEVICE_FEATURE_CMD_SET_LOG_MODE, (void*)&logmode, sizeof(logmode));
    detailed_error_message(m_hCam, quiet);

    // Set rolling shutter
    UINT shuttermode = IS_DEVICE_FEATURE_CAP_SHUTTER_MODE_ROLLING;
    is_DeviceFeature(m_hCam, IS_DEVICE_FEATURE_CMD_SET_SHUTTER_MODE, (void*)&shuttermode, sizeof(shuttermode));
    detailed_error_message(m_hCam, quiet);





    if( ! quiet) { std::cout << "Getting current gain ..." << std::endl; }
    if( ! quiet) { std::cout << "Current gain setting is: " << is_SetHardwareGain(m_hCam, IS_GET_MASTER_GAIN, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER) << std::endl; }
    detailed_error_message(m_hCam, quiet);
    if( ! quiet) { std::cout << "Setting gain to " << gain_setting << " ..." << std::endl; }
    is_SetHardwareGain(m_hCam, gain_setting, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER);
    detailed_error_message(m_hCam, quiet);
    if( ! quiet) { std::cout << "Getting current gain ..." << std::endl; }
    if( ! quiet) { std::cout << "Current gain setting is: " << is_SetHardwareGain(m_hCam, IS_GET_MASTER_GAIN, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER, IS_IGNORE_PARAMETER) << std::endl << std::endl; }
    detailed_error_message(m_hCam, quiet);

    int current_blacklevel;
    if( ! quiet) { std::cout << "Getting current blacklevel ..." << std::endl; }
    is_Blacklevel(m_hCam, IS_BLACKLEVEL_CMD_GET_OFFSET, (void*)&current_blacklevel, sizeof(current_blacklevel));
    detailed_error_message(m_hCam, quiet);
    if( ! quiet) { std::cout << "Current blacklevel: " << current_blacklevel << std::endl; }

    if( ! quiet) { std::cout << "Setting blacklevel to " << blacklvl_setting << "..." << std::endl; }
    is_Blacklevel(m_hCam, IS_BLACKLEVEL_CMD_SET_OFFSET, (void*)&blacklvl_setting, sizeof(blacklvl_setting));
    detailed_error_message(m_hCam, quiet);

    if( ! quiet) { std::cout << "Getting current blacklevel ..." << std::endl; }
    is_Blacklevel(m_hCam, IS_BLACKLEVEL_CMD_GET_OFFSET, (void*)&current_blacklevel, sizeof(current_blacklevel));
    detailed_error_message(m_hCam, quiet);
    if( ! quiet) { std::cout << "Current blacklevel: " << current_blacklevel << std::endl; }

//    new_frame_rate = 0.1;
 //   double frame_rate_readback;
  //  if( ! quiet) { std::cout << "Setting frame rate to " << new_frame_rate << " fps ..." << std::endl; }
   // is_SetFrameRate(m_hCam, new_frame_rate, &frame_rate_readback);
    //detailed_error_message(m_hCam, quiet);
    //if( ! quiet) { std::cout << "Frame rate set to " << frame_rate_readback << " fps ..." << std::endl << std::endl; }


    if( ! quiet) { std::cout << "Getting valid exposure range ..." << std::endl; }
    double exposure_range[3];
    is_Exposure(m_hCam, IS_EXPOSURE_CMD_GET_EXPOSURE_RANGE, exposure_range, sizeof(exposure_range));
    detailed_error_message(m_hCam, quiet);
    if( ! quiet) { std::cout << "Min: " << exposure_range[0] << " ms\nMax: " << exposure_range[1] << " ms\nInc: " << exposure_range[2] << " ms" << std::endl; }



    if( ! quiet) { std::cout << "Setting new exposure to: " << exposure_time << " ms ..." << std::endl; }
    is_Exposure(m_hCam, IS_EXPOSURE_CMD_SET_EXPOSURE, &exposure_time, sizeof(exposure_ms));
    detailed_error_message(m_hCam, quiet);
    if( ! quiet) { std::cout << "Current exposure now " << exposure_time << " ms" << std::endl << std::endl; }



    is_SetExternalTrigger(m_hCam, IS_SET_TRIGGER_SOFTWARE);
    detailed_error_message(m_hCam, quiet);



    if( ! quiet) { std::cout << "\nFreezing video ..." << std::endl; }
    is_FreezeVideo(m_hCam, IS_WAIT);
    detailed_error_message(m_hCam, quiet);



    GetSystemTime(&time);
    delta_time = (time.wSecond*1000) + time.wMilliseconds-prev_time_ms;
    std::cout << delta_time << std::endl;
    prev_time_ms = (time.wSecond*1000) + time.wMilliseconds;

    if(interactiveFilenames==1){
        while(getline(std::cin, image_save_file_name))
        {
            if(image_save_file_name.empty())
            {
                continue;
            }

            std::cout << image_save_file_name << std::endl;

            if( ! quiet) { std::cout << "\nFreezing video ..." << std::endl; }
            is_FreezeVideo(m_hCam, IS_WAIT);
            detailed_error_message(m_hCam, quiet);

            wchar_t *wchar_file_name = new wchar_t[image_save_file_name.size() + 1];
            std::wstring w_image_name(image_save_file_name.begin(), image_save_file_name.end());
            w_image_name.copy(wchar_file_name, image_save_file_name.size());
            wchar_file_name[image_save_file_name.size()] = '\0';
            IMAGE_FILE_PARAMS ImageFileParams;
            if( ! quiet) { std::cout << "\nSaving image ..." << std::endl; }
            ImageFileParams.pwchFileName = wchar_file_name;
            ImageFileParams.nFileType = IS_IMG_PNG;
            ImageFileParams.pnImageID = NULL;
            ImageFileParams.ppcImageMem = NULL;
            ImageFileParams.nQuality = 100;
            if( ! quiet) { std::wcout << "\nSaving image to " << ImageFileParams.pwchFileName << " ..." << std::endl; }
            is_ImageFile(m_hCam, IS_IMAGE_FILE_CMD_SAVE, &ImageFileParams, sizeof(ImageFileParams));
            detailed_error_message(m_hCam, quiet);

        }
    }else{
        if( ! quiet) { std::cout << "\nFreezing video ..." << std::endl; }
            is_FreezeVideo(m_hCam, IS_WAIT);
            detailed_error_message(m_hCam, quiet);

            wchar_t *wchar_file_name = new wchar_t[image_save_file_name.size() + 1];
            std::wstring w_image_name(image_save_file_name.begin(), image_save_file_name.end());
            w_image_name.copy(wchar_file_name, image_save_file_name.size());
            wchar_file_name[image_save_file_name.size()] = '\0';
            IMAGE_FILE_PARAMS ImageFileParams;
            if( ! quiet) { std::cout << "\nSaving image ..." << std::endl; }
            ImageFileParams.pwchFileName = wchar_file_name;
            ImageFileParams.nFileType = IS_IMG_PNG;
            ImageFileParams.pnImageID = NULL;
            ImageFileParams.ppcImageMem = NULL;
            ImageFileParams.nQuality = 100;
            if( ! quiet) { std::wcout << "\nSaving image to " << ImageFileParams.pwchFileName << " ..." << std::endl; }
            is_ImageFile(m_hCam, IS_IMAGE_FILE_CMD_SAVE, &ImageFileParams, sizeof(ImageFileParams));
            detailed_error_message(m_hCam, quiet);
    }



    if( ! quiet) { std::cout << "\nShutting down camera ..." << std::endl; }
    is_ExitCamera(m_hCam);



    return 0;
}
