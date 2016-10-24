void kernel simple_add(global int *offset, global int *limit, global int *inNumber, global int *rez)
{
    int max = *offset[get_global_id(0)] + *limit[get_global_id(0)];
    if(*offset[get_global_id(0)] == 0)
    {
        *offset[get_global_id(0)]=*offset[get_global_id(0)]+1;
    }

    int squareNum =(*offset[get_global_id(0)]-1)*(*offset[get_global_id(0)]-1);
    int i,j,rootNum,z;
    int sequence = 1 + (*offset[get_global_id(0)]-1)*2;
    for(i=*offset[get_global_id(0)]; i <= max; ++i)
    {
        squareNum=squareNum+sequence;
        sequence=sequence+2;
        if(squareNum <= inNumber[get_global_id(0)])
        {
            j = inNumber - squareNum;
            if(j != 0)
            {
                rootNum= sqrt([float]j);
                if(modf([float]rootNum,[float]&z) == 0)
                {
                    *rez[get_global_id(0)]=*rez[get_global_id(0)]+1;
                }
            }
            else
            {
                *rez[get_global_id(0)]=*rez[get_global_id(0)]+1;
            }
        }
        else
        {
            break;
        }
    }
};
