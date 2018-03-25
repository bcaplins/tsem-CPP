#define cimg_display 0
#include <CImg.h>

#include <alpbasic.h>

#include <string>
#include <iostream>
#include <exception>

using namespace cimg_library;

template<class T>
T max(T x, T y)
{
    return (x>y) ? x : y;
}

template<class T>
T min(T x, T y)
{
    return (x<y) ? x : y;
}

class MirrorException : public std::exception
{
public:
    MirrorException(std::string s)
    {
        std::cout << s;
    }
};


class DMD_Mirror
{
public:
    DMD_Mirror() : cleanup_required(false), image_data(NULL)
    {
        // The first thing to do is to allocate one ALP device.
        // The alpid serves for further requests to identify the device.
        if (0 > AlpbDevAlloc(0, &alpid))
        {
            throw MirrorException("Error: AlpbDevAlloc\n");
        }

        // Query serial number
        unsigned long serial;
        if (0 > AlpbDevInquire(alpid, ALPB_DEV_SERIAL, &serial))
        {
            throw MirrorException("Error: AlpbDevInquire (Serial number)\n");
        }
        printf("The allocated ALP has the serial number %i\n", serial);

        // Detect DMD type
        ALPB_DMDTYPES nDmdType;
        if (0 > AlpbDevInquire(alpid, ALPB_DEV_DMDTYPE, &nDmdType))
        {
            throw MirrorException("Error: AlpbDevInquire (DMD type)\n");
        }

        // Evaluate DMD type
        // Applications often depend on a particular DMD type. In this case just
        // inquire ALPB_DEV_DMDTYPE and reject all unsupported types.
        switch (nDmdType)
        {
        case ALPB_DMDTYPE_DISCONNECT:
            printf("DMD type: DMD disconnected or not recognized\nEmulate 1080p\n");
            nSizeX = 1920;
            nSizeY = 1080;
            break;
        case ALPB_DMDTYPE_1080P_095A:
            printf("DMD type: 1080p .95\" Type-A\n");
            nSizeX = 1920;
            nSizeY = 1080;
            break;
        case ALPB_DMDTYPE_WUXGA_096A:
            printf("DMD type: WUXGA .96\" Type-A\n");
            nSizeX = 1920;
            nSizeY = 1200;
            break;

        case ALPB_DMDTYPE_XGA:
            printf("DMD type: XGA\n");
            nSizeX = 1024;
            nSizeY = 768;
            break;
        case ALPB_DMDTYPE_XGA_055A:
            printf("DMD type: XGA .55\" Type-A\n");
            nSizeX = 1024;
            nSizeY = 768;
            break;
        case ALPB_DMDTYPE_XGA_055X:
            printf("DMD type: XGA .55\" Type-X\n");
            nSizeX = 1024;
            nSizeY = 768;
            break;
        case ALPB_DMDTYPE_XGA_07A:
            printf("DMD type: XGA .7\" Type-A\n");
            nSizeX = 1024;
            nSizeY = 768;
            break;

        default:
            throw MirrorException("DMD type: (unknown)\nError: DMD type not known\n");
        }

        std::cout << "Mirror successfully contacted.\n";
        std::cout << "Width  = " << nSizeX << " px\n";
        std::cout << "Height = " << nSizeY << " px\n";
        cleanup_required = true;
    }

    ~DMD_Mirror()
    {
        if (cleanup_required)
        {
            long bHalt = 1;
            AlpbDevControl(alpid, ALPB_DEV_HALT, &bHalt);	// actually only necessary in multithreading use
            AlpbDevFree(alpid);	// close device driver
        }

        delete image_data;
    }

    void write_image_to_mirror(std::string filename)
    {
        CImg<unsigned char> image(filename.c_str());
        std::cout << '\n' << filename << " loaded.\n"
                  << image.width() << " pixels wide\n"
                  << image.height() << " pixels tall\n"
                  << image.depth() << " slices deep\n"
                  << image.spectrum() << " color channels\n";

        delete image_data;
        image_data = new unsigned char[nSizeX*nSizeY];
        for(int x = 0; x < nSizeX; ++x)
        {
            for(int y = 0; y < nSizeY; ++y)
            {
                if((x >= image.width()) || (y >= image.height()))
                {
                    image_data[x+nSizeX*y] = (unsigned char)(0);
                }
                else
                {
                    image_data[x+nSizeX*y] = (unsigned char)((image(x,y) < 128) ? 0 : 128);
                }
            }
        }

        std::cout << "\nWriting images to mirror... (wait at least 10 seconds)\n";
        if(0 < AlpbDevLoadRows(alpid, image_data, 0, nSizeY-1))
        {
            throw MirrorException("Error writing image to mirror.");
        }
    }

private:
    ALPB_HDEVICE alpid;
    int nSizeX;
    int nSizeY;
    bool cleanup_required;
    unsigned char *image_data;
};





int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << argv[0] << " <image to display on DMD>\n";
        return 0;
    }

    try
    {
        DMD_Mirror mirror;
        mirror.write_image_to_mirror(argv[1]);
        std::cout << "Press any key to quit.\n";
        std::cin.get();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << '\n';
    }

    return 0;
}
