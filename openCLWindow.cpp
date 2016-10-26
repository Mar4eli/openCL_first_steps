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

const int ITERATION_STEP=1000;
const int ITERATION_SIZE=100;


bool openCLWindow::asKernel(double *offset,  double *inNumber,  double *max,  ulong *rez, ulong *rez2, ulong zind)
{
        ulong limit = ITERATION_STEP;
        ulong maxIt = offset[zind] + limit;
        if (maxIt > max[zind])
        {
            maxIt = max[zind];
        }
        //qDebug()<<"max="+QString::number(maxIt)+" offset="+QString::number(offset[zind]);
        ulong index = 0;
        double squareNum;
        double sequence;
        squareNum =(offset[zind])*(offset[zind]);
        sequence = 1 + (offset[zind])*2; //offset стартовал с нуля, поэтому -1, + надо взять предыдущий -1.
        ulong i=offset[zind];
        while(i < maxIt)
        {
            double rootNum;
            double z;
            double j;

            if(squareNum <= inNumber[zind])
            {
                j = inNumber[zind] - squareNum;
                if(j != 0.0)
                {
                    rootNum = sqrt(j);
                    if(modf(rootNum,&z) == 0.0)
                    {
                        rez2[zind*limit+index] = rootNum;
                        rez[zind*limit+index] = i;
                    }
                }else{
                    rez2[zind*limit+index]=j;
                    rez[zind*limit+index]=i;
                }
            }
            else{
                break;
            }
            squareNum = squareNum + sequence;
            sequence = sequence + 2;
            index = index + 1;
            i++;
        }
    return true;
}

bool openCLWindow::runTest()
{
    try
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
        ulong rez1[ITERATION_SIZE*ITERATION_STEP];
        ulong rez2[ITERATION_SIZE*ITERATION_STEP];
        ulong last = 0;
        ulong xz = 0;
        ulong realIter = 0;
        while(last < max_right_value)
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
                    offset[zind] = tmp;
                    inNumber[zind] = m_inNumber;
                    max[zind] = max_right_value;
                    asKernel(offset,inNumber,max,rez1,rez2,zind);

                    last = offset[zind];
                    realIter = zind;
                }
                else
                {
                    last = max_right_value+1;
                    realIter = zind;
                    break;
                }
            }
            for(ulong i=0; i < realIter; i++)
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
            }

            xz++;
        }
        qDebug()<<"time="+QString::number(start.elapsed())<<"\n";
        return true;
    }
    catch(...)
    {
        qDebug()<<"popalsa";
        return false;
    }
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
    std::vector<cl::Device> device = {default_device};
    std::cout<< "Using device: "<<default_device.getInfo<CL_DEVICE_NAME>()<<" "<<default_device.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE>()<<"\n";
    std::cout<< "Using device: "<<default_device.getInfo<CL_DEVICE_EXTENSIONS>()<<"\n";


    cl::Context context(device);

    cl::Program::Sources sources;
    std::cout<<"kernel binding \n";
    //примечание не удалось забиндить целочисленное значение в sqrt

    std::string kernel_code=
            "#pragma OPENCL EXTENSION cl_khr_fp64 : enable \n"
            "void kernel simple_add(global double *offset, global double *inNumber, global double *max, global ulong *rez, global ulong *rez2, global int *zindK){ \n \
                ulong limit = 1000; \n \
                ulong curGlobalId = get_global_id(0); \n \
                int curZindK = zindK[curGlobalId]; \n \
                for(ulong j=0; j < limit; j++) \n \
                {\n \
                    rez[curZindK*limit+j] = 0; \n \
                    rez2[curZindK*limit+j] = 0; \n \
                }\n \
                double curOffset = offset[curGlobalId]; \n \
                double curInNumber = inNumber[curGlobalId]; \n \
                double previous1 = 0; \
                ulong previous2 = 0; \
                ulong maxIt = curOffset + limit; \n \
                if(maxIt > max[curGlobalId]) \n \
                { \n \
                    maxIt = max[curGlobalId]; \n \
                } \n \
                ulong index = 0; \n \
                double squareNum = curOffset*curOffset;; \n \
                double sequence= 1 + curOffset*2;; \n \
                ulong i = curOffset; \n \
                while (i < maxIt) \n \
                { \n \
                    double z,rootNum; \n \
                    double j;\n \
                    if(squareNum <= curInNumber){ \n \
                        j = curInNumber - squareNum; \n \
                        if(j != 0.0){ \n \
                            rootNum = sqrt(j); \n \
                            if(modf(rootNum,&z) == 0.0){ \n \
                                if(!(previous1 == (ulong)rootNum && previous2 == i))\n \
                                {\n \
                                    rez2[curGlobalId*limit+index] = rootNum; \n \
                                    rez[curGlobalId*limit+index] = i; \n \
                                    previous1 = rootNum; \n \
                                    previous2 = i; \n \
                                }\n \
                            } \n \
                        }else{ \n \
                            rez2[curGlobalId*limit+index]=j; \n \
                            rez[curGlobalId*limit+index]=i; \n \
                        } \n \
                    } \n \
                    squareNum = squareNum + sequence; \n \
                    sequence = sequence + 2;\n \
                    index = index + 1; \n \
                    i=i+1; \n \
                }\
            }";

//    for(ulong j=0; j < limit; j++) \n \
//    { \n \
//        rez[get_global_id(0)*limit+j] = 0; \n \
//        rez2[get_global_id(0)*limit+j] = 0; \n \
//    }
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
    double max_right_value = sqrt(m_inNumber);
    //std::cout<<max_right_value;
    ulong z,zind=0;
    z=0;
    QTime start = QTime::currentTime();

    double offset[ITERATION_SIZE];
    offset[0] = 0;
    double inNumber = m_inNumber;
    double max = max_right_value;
    ulong rez1[ITERATION_STEP*ITERATION_SIZE];
    ulong rez2[ITERATION_STEP*ITERATION_SIZE];
    int zindK[ITERATION_SIZE];

    cl::Buffer buffer_inNumber(context,CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR,sizeof(double),&inNumber);
    cl::Buffer buffer_offset(context,CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR,sizeof(double)*ITERATION_SIZE,offset);
    cl::Buffer buffer_max(context,CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR,sizeof(double),&max);
    cl::Buffer buffer_rez(context,CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR,sizeof(ulong)*ITERATION_SIZE*ITERATION_STEP,rez1);
    cl::Buffer buffer_rez2(context,CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR,sizeof(ulong)*ITERATION_SIZE*ITERATION_STEP,rez2);
    cl::Buffer buffer_zindK(context,CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR,sizeof(ulong)*ITERATION_SIZE,zindK);
    int xz = 0;
    double last=0;
    while(last < max_right_value)
    {
        for(zind=0; zind < ITERATION_SIZE; zind++)
        {
            double tmp = zind*ITERATION_STEP+xz*ITERATION_SIZE*ITERATION_STEP;
            if(tmp <= max_right_value)
            {
                last = tmp+ITERATION_STEP-1;
            }
            else
            {
                last = max_right_value+1;
            }
            offset[zind] = tmp;
        }

        //create queue to which we will push commands for the device.
        cl::CommandQueue queue(context,default_device);

        //write arrays A and B to the device
        queue.enqueueWriteBuffer(buffer_inNumber,CL_FALSE,0,sizeof(double),&inNumber);
        queue.enqueueWriteBuffer(buffer_offset,CL_FALSE,0,sizeof(double)*ITERATION_SIZE,offset);
        queue.enqueueWriteBuffer(buffer_zindK,CL_FALSE,0,sizeof(double)*ITERATION_SIZE,zindK);
        queue.enqueueWriteBuffer(buffer_max,CL_FALSE,0,sizeof(double),&max);
    //    std::cout<<"buffers in queue \n";

        //run the kernel
        cl::Kernel kernel_add=cl::Kernel(program,"simple_add");
        kernel_add.setArg(0,buffer_offset);
        kernel_add.setArg(1,buffer_inNumber);
        kernel_add.setArg(2,buffer_max);
        kernel_add.setArg(3,buffer_rez);
        kernel_add.setArg(4,buffer_rez2);
        kernel_add.setArg(5,buffer_zindK);

        //посмотреть, что делает.
        queue.enqueueNDRangeKernel(kernel_add,cl::NullRange,cl::NDRange(100),cl::NullRange);
        queue.finish();

        queue.enqueueReadBuffer(buffer_rez,CL_FALSE,0,sizeof(ulong)*ITERATION_SIZE*ITERATION_STEP,rez1);
        queue.enqueueReadBuffer(buffer_rez2,CL_FALSE,0,sizeof(ulong)*ITERATION_SIZE*ITERATION_STEP,rez2);
//        for(ulong i=0; i < ITERATION_SIZE; i++)
//        {
//            for(ulong j=0; j < ITERATION_STEP; j++)
//            {
//                if(rez1[i*ITERATION_STEP+j] !=0 || rez2[i*ITERATION_STEP+j] !=0)
//                {
//                    qDebug()<<QString::number(rez1[i*ITERATION_STEP+j])+" "+QString::number(rez2[i*ITERATION_STEP+j]);
//                }
//            }
//        }
        xz++;
    }
    qDebug()<<"time="+ QString::number(start.elapsed())+" ms";
}
