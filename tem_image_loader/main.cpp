#include <string>
#include <iostream>
#include <exception>
#include <cctype>

#define cimg_display 0
#include "CImg.h"
using namespace cimg_library;

#include <alpbasic.h>


class MirrorException : public std::exception
{
public:
    MirrorException(std::string s) throw() : what_message(s) { }
    ~MirrorException() throw() { }

    const char* what() const throw()
    {
        return what_message.c_str();
    }

private:
    std::string what_message;
};


class DMD_Mirror
{
public:
    DMD_Mirror() : mirror_allocated(false), image_for_mirror(NULL)
    {
        try
        {
            // The first thing to do is to allocate one ALP device.
            // The alpid serves for further requests to identify the device.
            check_return_code(AlpbDevAlloc(0, &alpid), "Error in function AlpbDevAlloc");
            mirror_allocated = true;

            // Query serial number
            unsigned long serial;
            check_return_code(AlpbDevInquire(alpid, ALPB_DEV_SERIAL, &serial), "Error in function AlpbDevInquire (Serial number)");
            //std::cout << "The allocated ALP has the serial number " << serial << "\n";

            // Detect DMD type
            ALPB_DMDTYPES nDmdType;
            check_return_code(AlpbDevInquire(alpid, ALPB_DEV_DMDTYPE, &nDmdType), "Error in function AlpbDevInquire (DMD type)");

            // Evaluate DMD type
            // Applications often depend on a particular DMD type. In this case just
            // inquire ALPB_DEV_DMDTYPE and reject all unsupported types.
            switch(nDmdType)
            {
            case ALPB_DMDTYPE_DISCONNECT:
                //std::cout << "DMD type: DMD disconnected or not recognized\nEmulate 1080p\n";
                nSizeX = 1920;
                nSizeY = 1080;
                break;
            case ALPB_DMDTYPE_1080P_095A:
                //std::cout << "DMD type: 1080p .95\" Type-A\n";
                nSizeX = 1920;
                nSizeY = 1080;
                break;
            case ALPB_DMDTYPE_WUXGA_096A:
                //std::cout << "DMD type: WUXGA .96\" Type-A\n)";
                nSizeX = 1920;
                nSizeY = 1200;
                break;

            case ALPB_DMDTYPE_XGA:
                //std::cout << "DMD type: XGA\n";
                nSizeX = 1024;
                nSizeY = 768;
                break;
            case ALPB_DMDTYPE_XGA_055A:
                //std::cout << "DMD type: XGA .55\" Type-A\n";
                nSizeX = 1024;
                nSizeY = 768;
                break;
            case ALPB_DMDTYPE_XGA_055X:
                //std::cout << "DMD type: XGA .55\" Type-X\n";
                nSizeX = 1024;
                nSizeY = 768;
                break;
            case ALPB_DMDTYPE_XGA_07A:
                //std::cout << "DMD type: XGA .7\" Type-A\n";
                nSizeX = 1024;
                nSizeY = 768;
                break;

            default:
                throw MirrorException("DMD type: (unknown)\nError: DMD type not known");
            }

            //std::cout << "Mirror successfully contacted.\n";
            //std::cout << "Width  = " << nSizeX << " px\n";
            //std::cout << "Height = " << nSizeY << " px\n";

            image_for_mirror = new unsigned char[nSizeX*nSizeY];
        }
        catch(...)
        {
            cleanup();
            throw;
        }
    }

    ~DMD_Mirror()
    {
        cleanup();
    }

    void write_image_to_mirror(std::string filename)
    {
        // Pixel values
        const static unsigned char OFF = 0;
        const static unsigned char ON = 128;

        const static unsigned char THRESHOLD = 128;

        CImg<unsigned int> input_image;
        try
        {
            input_image.assign(filename.c_str());
        }
        catch(const CImgIOException& e)
        {
            // Error message printed by exception
            // constructor; no need to print it
            // here.  Just return and wait for the
            // next image.
            return;
        }

        // x and y are mirror pixel coordinates
        for(int x = 0; x < nSizeX; ++x)
        {
            for(int y = 0; y < nSizeY; ++y)
            {
                // expected input is binary black/white images, so just taking
                // the red channel should suffice.
                if( (x >= input_image.width()) || (y >= input_image.height()) )
                {
                    image_for_mirror[x+nSizeX*y] = OFF;
                }
                else
                {
                    image_for_mirror[x+nSizeX*y] = (input_image(x,y) < THRESHOLD) ? OFF : ON;
                }
            }
        }

        //std::cout << "\nWriting images to mirror... \n";
        check_return_code(AlpbDevLoadRows(alpid, image_for_mirror, 0, nSizeY-1),
                          std::string("Error in function AlpbDevLoadRows\nCould not write image (") + filename + ") to mirror.");
        check_return_code(AlpbDevReset(alpid, ALPB_RESET_GLOBAL, 0),
                          "Error in function AlpbDevReset");
    }

private:
    ALPB_HDEVICE alpid;
    int nSizeX;
    int nSizeY;
    bool mirror_allocated;
    unsigned char *image_for_mirror;

    void check_return_code(long return_code, std::string message)
    {
        if(return_code == ALPB_SUCC_PARTIAL)
        {
            std::cout << "Error message truncated.\n";
            return;
        }

        if(return_code < 0)
        {
            char strMsg[256];
            long nSize = sizeof(strMsg);
            long ret = AlpbDllGetResultText(return_code, &nSize, strMsg);
            check_return_code(ret, "Error in function AlpbDllGetResultText");
            if(ret < 0)
            {
                std::cout << "ALP basic API error code " << std::hex << return_code << std::dec << ", see alpbasic.h\n";
            }
            else
            {
                std::cout << "ALP basic API error (code " << std::hex << return_code << std::dec << ", see alpbasic.h):\n" << strMsg << "\n\n";
            }

            throw MirrorException(message);
        }
    }

    void cleanup()
    {
        if(mirror_allocated)
        {
            long bHalt = 1;
            AlpbDevControl(alpid, ALPB_DEV_HALT, &bHalt); // actually only necessary in multithreading use
            AlpbDevFree(alpid); // close device driver
        }

        delete [] image_for_mirror;
    }
};


int main(int argc, char* argv[])
{
    try
    {
        DMD_Mirror mirror;

        if(argc == 1)
        {
            std::string file_name;
            while(getline(std::cin, file_name))
            {
                if(file_name.empty())
                {
                    continue;
                }
                mirror.write_image_to_mirror(file_name);
            }
        }
        else
        {
            for(int i = 1; i < argc; ++i)
            {
                std::cout << "Image: " << argv[i] << '\n';
                mirror.write_image_to_mirror(argv[i]);
                std::cout << "Press enter to ";
                if(i < (argc - 1))
                {
                    std::cout << "go to next image.\n";
                }
                else
                {
                    std::cout << "quit.\n";
                }
                std::cin.get();
            }
        }
    }
    catch(const std::exception& e)
    {
        std::cout << e.what() << '\n';
    }

    return 0;
}
