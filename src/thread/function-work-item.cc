#define _FUNCTION_WORK_ITEM_CC_

#include "function-work-item.h"

// C
#include <stdio.h>

FunctionWorkItem::FunctionWorkItem()
{
}

FunctionWorkItem::~FunctionWorkItem()
{
}

void* FunctionWorkItem::InstanceWorkFunction()
{
    printf("FunctionWorkItem::InstanceWorkFunction\n");
    return NULL;
}

void FunctionWorkItem::InstanceWorkCallback()
{
    printf("FunctionWorkItem::InstanceWorkCallback\n");
}