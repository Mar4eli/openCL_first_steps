#include "openCLWindow.h"
#include <QApplication>
#include <iostream>
#include <CL/cl.hpp>
#include <QDebug>

const long ITERATION_STEP=100000000;
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
            exit(1);
        }
        else
        {
            qDebug()<<QString::number(all_platforms.size());
        }

        cl::Platform default_platform=all_platforms[0];
        std::cout<< "Using platform: "<<default_platform.getInfo<CL_PLATFORM_NAME>()<<"\n";



        //get default device of the default platform
            std::vector<cl::Device> all_devices;
            default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
            if(all_devices.size()==0){
                std::cout<<" No devices found. Check OpenCL installation!\n";
                exit(1);
            }
            cl::Device default_device=all_devices[0];
            std::cout<< "Using device: "<<default_device.getInfo<CL_DEVICE_NAME>()<<"\n";


            cl::Context context({default_device});

            cl::Program::Sources sources;

            //примечание не удалось забиндить целочисленное значение в sqrt
            std::string kernel_code=
                    "void kernel simple_add(global const int* inNumber, global float *max){"
                    "max[get_global_id(0)] = [float4]sqrt(inNumber[get_global_id(0)]);}";
                    //"max[get_global_id(0)]=inNumber[get_global_id(0)]+100;}";


            sources.push_back({kernel_code.c_str(),kernel_code.length()});

            cl::Program program(context,sources);
            if(program.build({default_device})!=CL_SUCCESS){
                std::cout<<" Error building: "<<program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device)<<"\n";
                exit(1);
            }
            else
            {
                std::cout<<" Successfull build"<<"\n";
            }

            //Подготовка данных
           qint64 iterations,lastLimit;

            //Граничное условие. Дальше этого числа не стоит пытаться возвести его в квадрат, т.к. выпадешь с переполнением и задачу не решишь.
            //Из условия задачи следует, что максимальное значение квадрата одного из слагаемых меньше, либо равно заданному числу. Т.е. 20^2 = 20^2 + 0.
            //В таком случае, нам не надо исследовать числа большие, чем sqrt(заданное_число) c округлением в меньшую сторону.

            long m_inNumber = 400000000000000000;
            long max_right_value = sqrt(m_inNumber);
            iterations = max_right_value/ITERATION_STEP;
            lastLimit = max_right_value-(iterations*ITERATION_STEP);

            qint64 offset[iterations+1];
            qint64 inNumber[iterations+1];
            QList<QPair<qint64,qint64>> listOfPairs;
            for(qint64 i=0; i < iterations; ++i)
            {
                offset[i] = i*ITERATION_STEP;
                inNumber[i] = m_inNumber;
            }

            offset[iterations] = (iterations)*ITERATION_STEP;
            inNumber[iterations] = m_inNumber;

            // create buffers on the device
//            cl::Buffer buffer_A(context,CL_MEM_READ_WRITE,sizeof(int)*10);
//            cl::Buffer buffer_B(context,CL_MEM_READ_WRITE,sizeof(int)*10);
//            cl::Buffer buffer_C(context,CL_MEM_READ_WRITE,sizeof(int)*10);

//            int A[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
//            int B[] = {0, 1, 2, 0, 1, 2, 0, 1, 2, 0};

            cl::Buffer buffer_inNumber(context,CL_MEM_READ_WRITE,sizeof(long)*iterations);
            cl::Buffer buffer_max(context,CL_MEM_READ_WRITE,sizeof(long)*iterations);

            //create queue to which we will push commands for the device.
            cl::CommandQueue queue(context,default_device);

            //write arrays A and B to the device
             queue.enqueueWriteBuffer(buffer_inNumber,CL_TRUE,0,sizeof(int)*2,inNumber);

//            queue.enqueueWriteBuffer(buffer_A,CL_TRUE,0,sizeof(int)*10,A);
//            queue.enqueueWriteBuffer(buffer_B,CL_TRUE,0,sizeof(int)*10,B);

            //run the kernel
            cl::Kernel kernel_add=cl::Kernel(program,"simple_add");
            kernel_add.setArg(0,buffer_inNumber);
            kernel_add.setArg(1,buffer_max);
//            kernel_add.setArg(0,buffer_A);
//            kernel_add.setArg(1,buffer_B);
//            kernel_add.setArg(2,buffer_C);

            //посмотреть, что делает.
            queue.enqueueNDRangeKernel(kernel_add,cl::NullRange,cl::NDRange(2),cl::NullRange);
            queue.finish();

            //int C[10];
            //read result C from the device to array C
            //queue.enqueueReadBuffer(buffer_C,CL_TRUE,0,sizeof(int)*10,C);

            float max[2];
            queue.enqueueReadBuffer(buffer_max,CL_TRUE,0,sizeof(float)*2,max);
            std::cout<<" result: \n"<<max[0]<<"\n";
            for(int i=0; i< 2; i++)
            {
                std::cout<<max[i]<<" ";
            }
//            for(int i=0;i<10;i++){
//                std::cout<<C[i]<<" ";
//            }


            return 0;


    //return a.exec();
}
///**
// * @brief findSquareSumConcur - потоко безопасный метод поиска слагаемых.
// * @details Данный метод работает с изолированным набором данных, использует мало памяти и при массовом запуске в потоках может занять весь процессор.
// *
// * В теле описан один твик производительности, позволивший увеличить её на 2,5% на тестовой машине.
// * @param n_offsetInNumberPair - парный объект, содержащий в себе: offset - начальное натуральное число для поиска квадратов, и inNumber - число, которое мы должны получить в сумме.
// * @return QHash<qint64,qint64>, содержащий все найденные пары значений. Практика показала, что длина такого хеша редко превышает 0 элементов, поэтому передача по указателю не дала бы заметного прироста производительности.
// */
//QHash<qint64,qint64> findSquareSumConcur(QPair<qint64,qint64> n_offsetInNumberPair)
//{
//    //меньше памяти, больше вычислений процессора
//    QHash<qint64,qint64> squareSumsHash;
//    qint64 offset = n_offsetInNumberPair.first;
//    qint64 inNumber = n_offsetInNumberPair.second;

//    qint64 max = offset + ITERATION_STEP;

//    if(offset == 0)
//    {
//        offset++;
//    }
//    qint64 i,j;
//    qreal z,secondSquareRoot;
//    //находим квадрат предыдущего числа из последовательности. Можно было сделать -1 до, но оставил для наглядности.
//    qint64 x=(offset-1)*(offset-1);

//    //квадрат любого натурального числа N может быть представлен,
//    //как сумма N элементов последовательности натуральных чисел, начинающейся с 1, и с шагом 2.
//    //1,3,5,7...
//    qint64 sequence = 1 + (offset-1)*2;
//    for(i = offset; i <= max; ++i)
//    {
//        //используя последовательность и сложение вместо возведения в степень, мы экономим такты.
//        //так как процессор разбивает операцию умножения, на операции сложения.
//        //5*5 = 5 + 5 + 5 + 5 + 5
//        //на моём домашнем компьютере int64_max выполняется примерно 15100 мс при умножение
//        // и 14700-14800 при сложении. экономия около 2,5%
//        x+=sequence;
//        sequence+=2;

//        //проверяем, меньше ли полученное число искомого числа.
//        if(x <= inNumber)
//        {
//            //находим второе слагаемое
//            j = inNumber - x;
//            if(j != 0)
//            {
//                //проверяем, является ли оно квадратом. т.е. пустой ли остаток от взятия корня.
//                secondSquareRoot = qSqrt(j);
//                if(modf(secondSquareRoot,&z) == 0)
//                {
//                    squareSumsHash.insert(i,secondSquareRoot);
//                }
//            }
//            else
//            {
//                //0*0=0 - тоже квадрат.
//                squareSumsHash.insert(i,j);
//            }
//        }
//        else
//        {
//            break;
//        }
//    }

//    return squareSumsHash;
//}


///**
// * @brief findSquareSum - запускает многопоточный вариант поиска сумм квадратов.
// * @details Выделен в отдельный класс и поток, чтобы не замораживать интерфейс.
// * Рассчитывает, сколько потоков, с какими диапазонами нужно создать и запускает многопоточный поиск с использованием API высокого уровня QtConcurrent.
// * Алгоритм нетребователен к памяти, но процессор может загрузить полностью.
// */
//void findSquareSumWorker::findSquareSum()
//{
//    qDebug()<<"hi";
//    QTime start = QTime::currentTime();
//    qint64 iterations,lastLimit;

//    //Граничное условие. Дальше этого числа не стоит пытаться возвести его в квадрат, т.к. выпадешь с переполнением и задачу не решишь.
//    //Из условия задачи следует, что максимальное значение квадрата одного из слагаемых меньше, либо равно заданному числу. Т.е. 20^2 = 20^2 + 0.
//    //В таком случае, нам не надо исследовать числа большие, чем sqrt(заданное_число) c округлением в меньшую сторону.
//    qint64 max_right_value = qSqrt(m_inNumber);
//    iterations = max_right_value/ITERATION_STEP;
//    lastLimit = max_right_value-(iterations*ITERATION_STEP);

//    QList<QPair<qint64,qint64>> listOfPairs;
//    for(qint64 i=0; i <= iterations; ++i)
//    {
//        listOfPairs.append(QPair<qint64,qint64>(i*ITERATION_STEP,m_inNumber));
//    }
//    listOfPairs.append(QPair<qint64,qint64>((iterations+1)*ITERATION_STEP,m_inNumber));

//    QHash<qint64,qint64> total = QtConcurrent::mappedReduced(listOfPairs, findSquareSumConcur, reduce);
//    //qDebug()<<"jiraf" + QString::number(total.size())+" " + QString::number(start.elapsed())+" ms";
//    QHash<qint64, qint64>::const_iterator iter = total.constBegin();
//    QHash<qint64,qint64>::const_iterator stop = total.constEnd();

//    QStringList results;
//    while (iter != stop) {
//        results.append(QString::number(iter.key())+";"+QString::number(iter.value()));
//        ++iter;
//    }
//    emit resultReady(start.elapsed(),results);
//}
