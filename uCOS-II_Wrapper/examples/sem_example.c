#include <ucos_ii.h>

static OS_EVENT *pSem;

void sem_example (void)
{
    INT8U err;
    
    pSem = OSSemCreate(0);
    
    OSSemDel(pSem, OS_DEL_ALWAYS, &err);
}
