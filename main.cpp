#include "openCLWindow.h"
#include <QApplication>

const ulong ITERATION_STEP=10;
const ulong ITERATION_SIZE=100;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    openCLWindow w;
    w.show();

//    std::vector<cl::Platform> all_platforms;
//    cl::Platform::get(&all_platforms);
//    if(all_platforms.size()==0){
//        std::cout<<" No platforms found. Check OpenCL installation!\n";
//        exit(2);
//    }
//    else
//    {
//        qDebug()<<QString::number(all_platforms.size());
//    }

//    cl::Platform default_platform=all_platforms[0];
//    std::cout<< "Using platform: "<<default_platform.getInfo<CL_PLATFORM_NAME>()<<" v"<<default_platform.getInfo<CL_PLATFORM_VERSION>()<<"\n";

//    //get default device of the default platform
//    std::vector<cl::Device> all_devices;
//    default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
//    if(all_devices.size()==0){
//        std::cout<<" No devices found. Check OpenCL installation!\n";
//        exit(1);
//    }
//    cl::Device default_device=all_devices[0];
//    std::vector<cl::Device> device = {default_device};
//    std::cout<< "Using device: "<<default_device.getInfo<CL_DEVICE_NAME>()<<" "<<default_device.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE>()<<"\n";
//    std::cout<< "Using device: "<<default_device.getInfo<CL_DEVICE_EXTENSIONS>()<<"\n";
//    std::cout<<default_device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();;
    return a.exec();
}

