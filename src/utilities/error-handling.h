#ifndef _ERROR_HANDLING_H_
#define _ERROR_HANDLING_H_

// node
#include <v8.h>
using namespace v8;

#include "nan.h"

class ErrorHandling
{
    public:

        static Handle<Object>   HandleException(TryCatch* tryCatch);
};

#endif /* _ERROR_HANDLING_H_ */
