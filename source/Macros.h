#pragma once
#include <type_traits>

#define SafeRelease(pInterface)\
if(pInterface != nullptr)\
{\
pInterface->Release();\
pInterface = nullptr;\
}


#define SafeDelete(pObject)\
static_assert(std::is_pointer<decltype(pObject)>::value, "SafeDelete requires a pointer");\
if(pObject != nullptr)\
{\
delete pObject;\
pObject = nullptr;\
}


//Hr result macro
#define ReturnOnFail(hr) if(FAILED(hr)) return hr;
#define ReturnAndAssertOnFail(hr) if(FAILED(hr)) { assert(false && "HRESULT failed!"); return; }
#define AssertOnFail(hr) assert(SUCCEEDED(hr) && "HRESULT failed!");