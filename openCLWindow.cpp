#include "openCLWindow.h"
#include "ui_openCLWindow.h"

openCLWindow::openCLWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::openCLWindow)
{
    ui->setupUi(this);
}

openCLWindow::~openCLWindow()
{
    delete ui;
}

const int ITERATION_STEP=100;
const int ITERATION_SIZE=100;


bool asKernel(double *offset,  double *inNumber,  double *max,  ulong *rez, ulong *rez2, ulong zind)
{
    if(max[zind] > offset[zind])
    {
        ulong limit = ITERATION_STEP;
        ulong maxIt = offset[zind] + limit;
        //qDebug()<<"max="+QString::number(maxIt)+" offset="+QString::number(offset[zind]);
        ulong index = 0;
        ulong squareNum;
        ulong sequence;
        if(zind!=0)
        {
            squareNum =(offset[zind])*(offset[zind]);
            sequence = 1 + (offset[zind])*2;;
        }
        else
        {
            squareNum = 0;
            sequence = 1;
        }

        for(ulong i=offset[zind]; i <= maxIt; i++)
        {
            double rootNum;
            double z;
            double j;
            if(squareNum <= inNumber[zind])
            {
                j = inNumber[zind] - squareNum;
                if(j != 0.0){
                    rootNum = sqrt(j);
                    if(modf(rootNum,&z) == 0.0)
                    {
                        rez[zind*limit+index] = rootNum;
                        rez2[zind*limit+index] = i;
                    }
                }else{
                    rez[zind*limit+index]=j;
                    rez2[zind*limit+index]=i;
                }
            }
            else{
                break;
            }
            squareNum = squareNum + sequence;
            sequence = sequence + 2;
            index = index + 1;
        }
    }
}

bool openCLWindow::runTest()
{
    //Подготовка данных
    double m_inNumber = ui->lineEdit->text().toDouble();
    const double max_right_value = sqrt(m_inNumber);
    //std::cout<<max_right_value;
    ulong zind=0;

    QTime start = QTime::currentTime();

    double offset[ITERATION_SIZE];
    offset[0]=0;
    double inNumber[ITERATION_SIZE];
    double max[ITERATION_SIZE];
    ulong rez1[ITERATION_STEP*ITERATION_SIZE];
    ulong rez2[ITERATION_STEP*ITERATION_SIZE];
    ulong last = 0;
    ulong xz = 0;
    bool fContinue = true;
    while(offset[last] < max_right_value)
    {
        for(zind=0; zind < ITERATION_SIZE; zind++)
        {
            double tmp = zind*ITERATION_STEP+xz*ITERATION_SIZE*ITERATION_STEP;
            for(ulong j=0; j < ITERATION_STEP; j++)
            {
                rez1[zind*ITERATION_STEP+j] = 0;
                rez2[zind*ITERATION_STEP+j] = 0;
            }
            if(tmp <= max_right_value)
            {
                offset[zind] = zind*ITERATION_STEP+xz*ITERATION_SIZE*ITERATION_STEP;
                inNumber[zind] = m_inNumber;
                max[zind] = max_right_value;

                //qDebug()<<QString::number(z+zind);
                asKernel(offset,inNumber,max,rez1,rez2,zind);
                last = zind;
            }
        }
        for(ulong i=0; i < ITERATION_SIZE; i++)
        {
            for(ulong j=0; j < ITERATION_STEP; j++)
            {
                if(rez1[i*ITERATION_STEP+j] != 0 || rez2[i*ITERATION_STEP+j] != 0)
                {
                    qDebug()<<QString::number(rez1[i*ITERATION_STEP+j])+" "+QString::number(rez2[i*ITERATION_STEP+j]);
                    if(rez2[i*ITERATION_STEP+j] == 100)
                    {
                        qDebug()<<"i="+QString::number(i)+" i*ITERSTEP="+QString::number(i*ITERATION_STEP)+" j="+QString::number(j);
                    }
                }
            }
            //std::cout<<"\n";
        }
        xz++;
        //qDebug()<<"z="+QString::number(z);
        //std::cout<<"z="<<z<<"\n";
    }
    qDebug()<<"time="+QString::number(start.elapsed())<<"\n";
    return 0;
}

void openCLWindow::on_pushButton_clicked()
{
    runTest();
}

void openCLWindow::on_pushButton_2_clicked()
{
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
            "void kernel simple_add(global double *offset, global double *inNumber, global double *max, global ulong *rez, global ulong *rez2){ \n \
                //size_t get_global_id(0) = get_global_id(0); \n \
                if(max[get_global_id(0)] > offset[get_global_id(0)]){ \n \
                    ulong limit = 100; \n \
                    ulong maxIt = offset[get_global_id(0)] + limit-1; \n \
                    ulong index = 0; \n \
                    ulong squareNum; \n \
                    ulong sequence; \n \
                    if(get_global_id(0) != 0)\n \
                    {\n \
                        squareNum =(offset[get_global_id(0)])*(offset[get_global_id(0)]); \n \
                        sequence = 1 + (offset[get_global_id(0)])*2; \n \
                    }\n \
                    else\n \
                    {\n \
                        squareNum = 0;\n \
                        sequence = 1;\n \
                    }\n \
                    for(ulong i=offset[get_global_id(0)]; i <= maxIt; i++) \n \
                    { \n \
                        double z,rootNum; \n \
                        double j;\n \
                        if(squareNum <= inNumber[get_global_id(0)]){ \n \
                            j = inNumber[get_global_id(0)] - squareNum; \n \
                            if(j != 0.0){ \n \
                                rootNum = sqrt(j); \n \
                                if(modf(rootNum,&z) == 0.0){ \n \
                                    rez[get_global_id(0)*limit+index] = rootNum; \n \
                                    rez2[get_global_id(0)*limit+index] = i; \n \
                                } \n \
                            }else{ \n \
                                rez[get_global_id(0)*limit+index]=j; \n \
                                rez2[get_global_id(0)*limit+index]=i; \n \
                            } \n \
                        } \n \
                        else{\n \
                            break;\n \
                        }\n \
                        squareNum = squareNum + sequence; \n \
                        sequence = sequence + 2;\n \
                        index = index + 1; \n \
                    }\
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

    //Подготовка данных
    double m_inNumber = ui->lineEdit->text().toDouble();
    const double max_right_value = sqrt(m_inNumber);
    //std::cout<<max_right_value;
    ulong z,zind=0;
    z=0;
    QTime start = QTime::currentTime();

    double offset[ITERATION_SIZE];
    offset[0] = 0;
    double inNumber[ITERATION_SIZE];
    double max[ITERATION_SIZE];
    ulong rez1[ITERATION_STEP*ITERATION_SIZE];
    ulong rez2[ITERATION_STEP*ITERATION_SIZE];

    cl::Buffer buffer_inNumber(context,CL_MEM_READ_WRITE,sizeof(double)*ITERATION_SIZE);
    cl::Buffer buffer_offset(context,CL_MEM_READ_WRITE,sizeof(double)*ITERATION_SIZE);
    cl::Buffer buffer_max(context,CL_MEM_READ_WRITE,sizeof(double)*ITERATION_SIZE);
    cl::Buffer buffer_rez(context,CL_MEM_READ_WRITE,sizeof(ulong)*ITERATION_SIZE*ITERATION_STEP);
    cl::Buffer buffer_rez2(context,CL_MEM_READ_WRITE,sizeof(ulong)*ITERATION_SIZE*ITERATION_STEP);

    int xz = 0;
    while(offset[zind] < max_right_value)
    {
        for(zind=0; zind < ITERATION_SIZE; zind++)
        {
            offset[zind] = zind*ITERATION_STEP+xz*ITERATION_SIZE*ITERATION_STEP;
            inNumber[zind] = m_inNumber;
            max[zind] = max_right_value;
            for(ulong j=0; j < ITERATION_STEP; j++)
            {
                rez1[zind*ITERATION_STEP+j] = 0;
                rez2[zind*ITERATION_STEP+j] = 0;
            }
        }

        //create queue to which we will push commands for the device.
        cl::CommandQueue queue(context,default_device);

        //write arrays A and B to the device
        queue.enqueueWriteBuffer(buffer_inNumber,CL_TRUE,0,sizeof(double)*ITERATION_SIZE,inNumber);
        queue.enqueueWriteBuffer(buffer_offset,CL_TRUE,0,sizeof(double)*ITERATION_SIZE,offset);
        queue.enqueueWriteBuffer(buffer_max,CL_TRUE,0,sizeof(double)*ITERATION_SIZE,max);
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
        queue.enqueueNDRangeKernel(kernel_add,cl::NullRange,cl::NDRange(ITERATION_SIZE),cl::NullRange);
        queue.finish();

        queue.enqueueReadBuffer(buffer_rez,CL_TRUE,0,sizeof(ulong)*ITERATION_SIZE*ITERATION_STEP,rez1);
        queue.enqueueReadBuffer(buffer_rez2,CL_TRUE,0,sizeof(ulong)*ITERATION_SIZE*ITERATION_STEP,rez2);
        for(ulong i=0; i < ITERATION_SIZE; i++)
        {
            for(ulong j=0; j < ITERATION_STEP; j++)
            {
                if(rez1[i*ITERATION_STEP+j] != 0 || rez2[i*ITERATION_STEP+j] != 0)
                {
                    qDebug()<<QString::number(rez1[i*ITERATION_STEP+j])+" "+QString::number(rez2[i*ITERATION_STEP+j]);
                }
            }
        }
        xz++;
        //std::cout<<"z="<<z<<"\n";
    }
    qDebug()<<"time="+ QString::number(start.elapsed())+" ms";
}

    //std::vector<cl::Platform> all_platforms;
    //cl::Platform::get(&all_platforms);
    //if(all_platforms.size()==0){
    //    std::cout<<" No platforms found. Check OpenCL installation!\n";
    //    exit(2);
    //}
    //else
    //{
    //    qDebug()<<QString::number(all_platforms.size());
    //}

    //cl::Platform default_platform=all_platforms[0];
    //std::cout<< "Using platform: "<<default_platform.getInfo<CL_PLATFORM_NAME>()<<" v"<<default_platform.getInfo<CL_PLATFORM_VERSION>()<<"\n";

    ////get default device of the default platform
    //std::vector<cl::Device> all_devices;
    //default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
    //if(all_devices.size()==0){
    //    std::cout<<" No devices found. Check OpenCL installation!\n";
    //    exit(1);
    //}
    //cl::Device default_device=all_devices[0];
    //std::cout<< "Using device: "<<default_device.getInfo<CL_DEVICE_NAME>()<<" "<<default_device.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE>()<<"\n";
    //std::cout<< "Using device: "<<default_device.getInfo<CL_DEVICE_EXTENSIONS>()<<"\n";


    //cl::Context context({default_device});

    //cl::Program::Sources sources;
    //std::cout<<"kernel binding \n";
    ////примечание не удалось забиндить целочисленное значение в sqrt
    ////    std::string kernel_code=
    ////            "#pragma OPENCL EXTENSION cl_khr_fp64 : enable \n"
    ////            "void kernel simple_add(global double *offset, global double *inNumber, global double *max, global double *rez, global double *rez2){ \
    ////                if(max[get_global_id(0)] > offset[get_global_id(0)]){\
    ////                    int limit = 10; \
    ////                    ulong maxIt = offset[get_global_id(0)] + limit-1; \
    ////                    int index = 0; \
    ////                    double squareNum =(offset[get_global_id(0)]-1)*(offset[get_global_id(0)-1]); \
    ////                    double sequence = 1 + (offset[get_global_id(0)]-1)*2;; \
    ////                    for(ulong i=offset[get_global_id(0)]; i <= maxIt; i++) \
    ////                    { \
    ////                        squareNum = squareNum + sequence; \
    ////                        sequence = sequence + 2;\
    ////                        double j,rootNum; \
    ////                        double z; \
    ////                        if(squareNum <= inNumber[get_global_id(0)]){ \
    ////                            j = inNumber[get_global_id(0)] - squareNum; \
    ////                            if(j != 0){ \
    ////                                rootNum = sqrt(j); \
    ////                                if(modf(rootNum,&z) == 0){ \
    ////                                    rez[get_global_id(0)*limit+index] = rootNum; \
    ////                                    rez2[get_global_id(0)*limit+index] = i; \
    ////                                } \
    ////                            }else{ \
    ////                                rez[get_global_id(0)*limit+index]=j; \
    ////                                rez2[get_global_id(0)*limit+index]=i; \
    ////                            } \
    ////                        } \
    ////                        else{\
    ////                            break;\
    ////                        }\
    ////                        index = index + 1; \
    ////                    }\
    ////                }\
    ////            }";


    ////    std::cout<<"kernel added \n";
    ////    sources.push_back({kernel_code.c_str(),kernel_code.length()});

    ////    std::cout<<"kernel pushed \n";
    ////    cl::Program program(context,sources);
    ////    if(program.build({default_device})!=CL_SUCCESS){
    ////        std::cout<<" Error building: "<<program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device)<<"\n";
    ////        exit(1);
    ////    }
    ////    else
    ////    {
    ////        std::cout<<" Successfull build"<<"\n";
    ////    }


    ////    std::cout<<"kernel good \n";

    ////Подготовка данных
    //double m_inNumber = 40000;
    //const double max_right_value = sqrt(m_inNumber);
    ////std::cout<<max_right_value;
    //ulong z,zind;
    //z=0;
    //QTime start = QTime::currentTime();

    //double offset[ITERATION_SIZE];
    //double inNumber[ITERATION_SIZE];
    //double max[ITERATION_SIZE];

    ////    cl::Buffer buffer_inNumber(context,CL_MEM_READ_WRITE,sizeof(double)*ITERATION_SIZE);
    ////    cl::Buffer buffer_offset(context,CL_MEM_READ_WRITE,sizeof(double)*ITERATION_SIZE);
    ////    cl::Buffer buffer_max(context,CL_MEM_READ_WRITE,sizeof(double)*ITERATION_SIZE);
    ////    cl::Buffer buffer_rez(context,CL_MEM_READ_WRITE,sizeof(double)*ITERATION_SIZE*ITERATION_STEP);
    ////    cl::Buffer buffer_rez2(context,CL_MEM_READ_WRITE,sizeof(double)*ITERATION_SIZE*ITERATION_STEP);

    //while(z+ITERATION_SIZE<max_right_value)
    //{
    //    for(zind=0; zind < ITERATION_SIZE; zind++)
    //    {
    //        offset[zind] = z*ITERATION_STEP+zind;
    //        inNumber[zind] = m_inNumber;
    //        max[zind] = max_right_value;
    //    }

    ////        //create queue to which we will push commands for the device.
    ////        cl::CommandQueue queue(context,default_device);

    ////        //write arrays A and B to the device
    ////        queue.enqueueWriteBuffer(buffer_inNumber,CL_TRUE,0,sizeof(double)*ITERATION_SIZE,inNumber);
    ////        queue.enqueueWriteBuffer(buffer_offset,CL_TRUE,0,sizeof(double)*ITERATION_SIZE,offset);
    ////        queue.enqueueWriteBuffer(buffer_max,CL_TRUE,0,sizeof(double)*ITERATION_SIZE,max);
    ////    //    std::cout<<"buffers in queue \n";

    ////        //run the kernel
    ////        cl::Kernel kernel_add=cl::Kernel(program,"simple_add");
    ////        kernel_add.setArg(0,buffer_offset);
    ////        kernel_add.setArg(1,buffer_inNumber);
    ////        kernel_add.setArg(2,buffer_max);
    ////        kernel_add.setArg(3,buffer_rez);
    ////        kernel_add.setArg(4,buffer_rez2);
    ////    //    std::cout<<"buffers in kernel \n";

    ////        //посмотреть, что делает.
    ////        queue.enqueueNDRangeKernel(kernel_add,cl::NullRange,cl::NDRange(ITERATION_SIZE),cl::NullRange);
    ////        queue.finish();
    ////    std::cout<<"kernels added \n";


    //    double rez1[ITERATION_STEP*ITERATION_SIZE];
    //    double rez2[ITERATION_STEP*ITERATION_SIZE];

    //    asKernel(offet,inNumber,max,rez1,rez2);
    //    //queue.enqueueReadBuffer(buffer_rez,CL_TRUE,0,sizeof(double)*ITERATION_SIZE*ITERATION_STEP,rez1);
    //    //queue.enqueueReadBuffer(buffer_rez2,CL_TRUE,0,sizeof(double)*ITERATION_SIZE*ITERATION_STEP,rez2);
    //    for(ulong i=0; i < ITERATION_SIZE; i++)
    //    {
    //        for(ulong j=0; j < ITERATION_STEP; j++)
    //        {
    //            //if(rez1[i+j] !=0 || rez2[i+j] != 0)
    //            //{
    //                std::cout<<rez1[i+j]<<" "<<rez2[i+j]<<" ";
    //            //}
    //        }
    //        std::cout<<"\n";
    //    }
    //    z = z + zind;
    //    //std::cout<<"z="<<z<<"\n";
    //}
    //std::cout<<"time="<<start.elapsed()<<"\n";
    //return 0;
