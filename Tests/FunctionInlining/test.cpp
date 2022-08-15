
#include <stdio.h>
// A normal function with an int parameter
// and void return type
void DASH_FFT(int a)
{
    printf("DASH_FFT is called %d\n", a);
}

int main()
{

    void (*fun_ptr)(int) = &DASH_FFT;
    (*fun_ptr)(10);
    return 0;
}
