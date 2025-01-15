
#include <stdio.h>
// A normal function with an int parameter
// and void return type
void DASH_FFT(int order)
{
    printf("DASH_FFT is called %d\n", order);
}

void DASH_GEMM(int order)
{
    printf("DASH_GEMM is called %d\n", order);
}

int main()
{
    //function pointer case
    void (*fun_ptr)(int) = &DASH_FFT;
    (*fun_ptr)(10);


    // will the preprocess or control DAG cause safety issue?
    // no : 1. loop unrolling is accurate, because it is tracked by loop entrance
    // 2. control redirection is safe for if-else local control scope because we split the basic block at the kernel boundry, 
    // and take data dependence into consideration, so all the if else control will be in its own scope  
    for (int i = 0; i < 5; i++)
    {
        if (i > 3)
        {
            DASH_FFT(1);
        }
        else if (i < 2)
        {
            DASH_GEMM(1);
        }
        else
        {
            i = i + 1;
        }
    }

    return 0;
}
