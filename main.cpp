#include "openCLWindow.h"
#include <QApplication>
#include <iostream>
#include <CL/cl.hpp>
#include <QDebug>

#pragma OPENCL EXTENSION cl_khr_fp64 : enable


const int ITERATION_STEP=1;
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
                    "void kernel simple_add(global double *offset, global double *limit, global double *inNumber, global double *rez){ \
                        double max = offset[get_global_id(0)] + limit[get_global_id(0)]; \
                        if(offset[get_global_id(0)] == 0){ \
                            offset[get_global_id(0)]=offset[get_global_id(0)]+1;} \
                        double squareNum =(offset[get_global_id(0)]-1)*(offset[get_global_id(0)]-1); \
                        ulong i; \
                        float j,rootNum; \
                        float z; \
                        double sequence = 1 + (offset[get_global_id(0)]-1)*2; \
                        for(i=offset[get_global_id(0)]; i < max; ++i){ \
                            squareNum=squareNum+sequence; \
                            sequence=sequence+2; \
                            if(squareNum <= inNumber[get_global_id(0)]){ \
                                j = inNumber[get_global_id(0)] - squareNum; \
                                if(j != 0.0){ \
                                    rootNum = sqrt(j); \
                                    if(modf(rootNum,&z) == 0){ \
                                        rez[get_global_id(0)*2] = rootNum; \
                                        rez[get_global_id(0)*2+1] = i; \
                                    } \
                                }else{ \
                                    rez[get_global_id(0)*2]=j; \
                                    rez[get_global_id(0)*2+1]=i; \
                                } \
                            }else{ \
                                break; \
                            } \
                        } \
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
            double m_inNumber = 2547483647;
            double max_right_value = sqrt(m_inNumber);
            double iterations = max_right_value/ITERATION_STEP;
            ulong sizeIter = iterations+1;

            std::cout<<"iterations found="<<iterations<<" \n";

            double offset[sizeIter];
            double inNumber[sizeIter];
            double iterationsLimit[sizeIter];

            std::cout<<"before FOR \n";
            for(ulong i=0; i <= iterations; i++)
            {
                offset[i] = i*ITERATION_STEP;
                inNumber[i] = m_inNumber;
                iterationsLimit[i] = ITERATION_STEP;
            }

            std::cout<<"for is done \n";

            cl::Buffer buffer_inNumber(context,CL_MEM_READ_WRITE,sizeof(double)*sizeIter);
            cl::Buffer buffer_offset(context,CL_MEM_READ_WRITE,sizeof(double)*sizeIter);
            cl::Buffer buffer_limit(context,CL_MEM_READ_WRITE,sizeof(double)*sizeIter);
            cl::Buffer buffer_rez(context,CL_MEM_READ_WRITE,sizeof(double)*sizeIter*2);

            std::cout<<"buffers created \n";
            //create queue to which we will push commands for the device.
            cl::CommandQueue queue(context,default_device);

            //write arrays A and B to the device
            queue.enqueueWriteBuffer(buffer_inNumber,CL_TRUE,0,sizeof(double)*sizeIter,inNumber);
            queue.enqueueWriteBuffer(buffer_offset,CL_TRUE,0,sizeof(double)*sizeIter,offset);
            queue.enqueueWriteBuffer(buffer_limit,CL_TRUE,0,sizeof(double)*sizeIter,iterationsLimit);
            std::cout<<"buffers in queue \n";

            //run the kernel
            cl::Kernel kernel_add=cl::Kernel(program,"simple_add");
            kernel_add.setArg(0,buffer_offset);
            kernel_add.setArg(1,buffer_limit);
            kernel_add.setArg(2,buffer_inNumber);
            kernel_add.setArg(3,buffer_rez);
            std::cout<<"buffers in kernel \n";

            //посмотреть, что делает.
            queue.enqueueNDRangeKernel(kernel_add,cl::NullRange,cl::NDRange(sizeIter),cl::NullRange);
            queue.finish();
            std::cout<<"kernels added \n";

            double rez[sizeIter*2];
            queue.enqueueReadBuffer(buffer_rez,CL_TRUE,0,sizeof(double)*sizeIter*2,rez);
            for(ulong i=0; i < sizeIter*2; i+=2)
            {
                if(rez[i] !=0 || rez[i+1] != 0)
                {
                    std::cout<<rez[i]<<" "<<rez[i+1]<<"\n";
                }
            }


            return 0;


    //return a.exec();
}
