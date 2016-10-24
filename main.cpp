#include "openCLWindow.h"
#include <QApplication>
#include <iostream>
#include <CL/cl.hpp>
#include <QDebug>
#include <QTime>

const int ITERATION_STEP=1000;

void func(ulong z, double m_inNumber, double n_max, cl::Context context, cl::Device default_device, cl::Program program)
{
    double offset[ITERATION_STEP];
    double inNumber[ITERATION_STEP];
    double max[ITERATION_STEP];

//    std::cout<<"before FOR \n";
    for(ulong i=0; i < ITERATION_STEP; i++)
    {
        offset[i] = i+z;
        inNumber[i] = m_inNumber;
        max[i] = n_max;
    }

//    std::cout<<"for is done \n";

    cl::Buffer buffer_inNumber(context,CL_MEM_READ_WRITE,sizeof(double)*ITERATION_STEP);
    cl::Buffer buffer_offset(context,CL_MEM_READ_WRITE,sizeof(double)*ITERATION_STEP);
    cl::Buffer buffer_max(context,CL_MEM_READ_WRITE,sizeof(double)*ITERATION_STEP);
    cl::Buffer buffer_rez(context,CL_MEM_READ_WRITE,sizeof(double)*ITERATION_STEP);
    cl::Buffer buffer_rez2(context,CL_MEM_READ_WRITE,sizeof(double)*ITERATION_STEP);

//    std::cout<<"buffers created \n";
    //create queue to which we will push commands for the device.
    cl::CommandQueue queue(context,default_device);

    //write arrays A and B to the device
    queue.enqueueWriteBuffer(buffer_inNumber,CL_TRUE,0,sizeof(double)*ITERATION_STEP,inNumber);
    queue.enqueueWriteBuffer(buffer_offset,CL_TRUE,0,sizeof(double)*ITERATION_STEP,offset);
    queue.enqueueWriteBuffer(buffer_max,CL_TRUE,0,sizeof(double)*ITERATION_STEP,max);
//    std::cout<<"buffers in queue \n";

    //run the kernel
    cl::Kernel kernel_add=cl::Kernel(program,"simple_add");
    kernel_add.setArg(0,buffer_offset);
    kernel_add.setArg(1,buffer_inNumber);
    kernel_add.setArg(2,buffer_max);
    kernel_add.setArg(3,buffer_rez);
    kernel_add.setArg(4,buffer_rez2);
//    std::cout<<"buffers in kernel \n";

    //посмотреть, что делает.
    queue.enqueueNDRangeKernel(kernel_add,cl::NullRange,cl::NDRange(ITERATION_STEP),cl::NullRange);
    queue.finish();
//    std::cout<<"kernels added \n";

    double rez1[ITERATION_STEP];
    double rez2[ITERATION_STEP];
    queue.enqueueReadBuffer(buffer_rez,CL_TRUE,0,sizeof(double)*ITERATION_STEP,rez1);
    queue.enqueueReadBuffer(buffer_rez2,CL_TRUE,0,sizeof(double)*ITERATION_STEP,rez2);
    for(ulong i=0; i < ITERATION_STEP; i++)
    {
        if(rez1[i] !=0 || rez2[i] != 0)
        {
            std::cout<<rez1[i]<<" "<<rez2[i]<<"\n";
        }
    }
}

int main(int argc, char *argv[])
{
//    QApplication a(argc, argv);
//    openCLWindow w;
//    w.show();

    //get all platforms (drivers)
        std::vector<cl::Platform> all_platforms;
        cl::Platform::get(&all_platforms);
        if(all_platforms.size()==0){
            std::cout<<" No platforms found. Check OpenCL installation!\n";
            exit(2);
        }
        else
        {
            qDebug()<<QString::number(all_platforms.size());
        }

        cl::Platform default_platform=all_platforms[0];
        std::cout<< "Using platform: "<<default_platform.getInfo<CL_PLATFORM_NAME>()<<" v"<<default_platform.getInfo<CL_PLATFORM_VERSION>()<<"\n";



        //get default device of the default platform
            std::vector<cl::Device> all_devices;
            default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
            if(all_devices.size()==0){
                std::cout<<" No devices found. Check OpenCL installation!\n";
                exit(1);
            }
            cl::Device default_device=all_devices[0];
            std::cout<< "Using device: "<<default_device.getInfo<CL_DEVICE_NAME>()<<" "<<default_device.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE>()<<"\n";
            std::cout<< "Using device: "<<default_device.getInfo<CL_DEVICE_EXTENSIONS>()<<"\n";


            cl::Context context({default_device});

            cl::Program::Sources sources;
            std::cout<<"kernel binding \n";
            //примечание не удалось забиндить целочисленное значение в sqrt
            std::string kernel_code=
                    "#pragma OPENCL EXTENSION cl_khr_fp64 : enable \n"
                    "void kernel simple_add(global double *offset, global double *inNumber, global double *max, global double *rez, global double *rez2){ \
                        if(max[get_global_id(0)] > offset[get_global_id(0)]){\
                            double squareNum =(offset[get_global_id(0)])*(offset[get_global_id(0)]); \
                            double j,rootNum; \
                            double z; \
                            if(squareNum <= inNumber[get_global_id(0)]){ \
                                j = inNumber[get_global_id(0)] - squareNum; \
                                if(j != 0.0){ \
                                    rootNum = sqrt(j); \
                                    if(modf(rootNum,&z) == 0){ \
                                        rez[get_global_id(0)] = rootNum; \
                                        rez2[get_global_id(0)] = offset[get_global_id(0)]; \
                                    } \
                                }else{ \
                                    rez[get_global_id(0)]=j; \
                                    rez2[get_global_id(0)]=offset[get_global_id(0)]; \
                                } \
                            } \
                        }\
                    }";

            std::cout<<"kernel added \n";
            sources.push_back({kernel_code.c_str(),kernel_code.length()});

            std::cout<<"kernel pushed \n";
            cl::Program program(context,sources);
            if(program.build({default_device})!=CL_SUCCESS){
                std::cout<<" Error building: "<<program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device)<<"\n";
                exit(1);
            }
            else
            {
                std::cout<<" Successfull build"<<"\n";
            }


            std::cout<<"kernel good \n";

            //Граничное условие. Дальше этого числа не стоит пытаться возвести его в квадрат, т.к. выпадешь с переполнением и задачу не решишь.
            //Из условия задачи следует, что максимальное значение квадрата одного из слагаемых меньше, либо равно заданному числу. Т.е. 20^2 = 20^2 + 0.
            //В таком случае, нам не надо исследовать числа большие, чем sqrt(заданное_число) c округлением в меньшую сторону.

            //Подготовка данных
            double m_inNumber = 6400000000000000;
            const double max_right_value = sqrt(m_inNumber);
            //std::cout<<max_right_value;
            ulong sizeIter;
            if(max_right_value > ITERATION_STEP)
            {
                sizeIter = ITERATION_STEP;
            }
            else
            {
                sizeIter = max_right_value;
            }
            QTime start = QTime::currentTime();
            for(ulong z=0; z <= max_right_value; z=z+ITERATION_STEP)
            {
                func(z,m_inNumber,max_right_value,context,default_device,program);
            }
            std::cout<<"time="<<start.elapsed()<<"\n";
            return 0;


    //return a.exec();
}

