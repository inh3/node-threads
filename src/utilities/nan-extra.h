#ifndef _NAN_EXTRA_H_
#define _NAN_EXTRA_H_

#include <node_version.h>

#if NODE_VERSION_AT_LEAST(0, 11, 13)

#define IsolateGetData(isolate) isolate->GetData(0)
#define IsolateSetData(isolate, data) isolate->SetData(0, data)

#else

#define IsolateGetData(isolate) isolate->GetData()
#define IsolateSetData(isolate, data) isolate->SetData((void*)data)

#endif

#endif /* _NAN_EXTRA_H_ */
