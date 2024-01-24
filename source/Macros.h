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


// ANSI escape codes for text color
#define RESET   "\x1b[0m"
#define RED     "\x1b[1;31m"        /* Red */
#define YELLOW  "\x1b[1;33m"      /* Yellow */
#define BLUE    "\x1b[34m"      /* Blue */

//Logging
#define LogWarning(message) std::cout << YELLOW << "Warning: " << (message) << RESET << std::endl;
#define LogError(message)   std::cout << RED << "Error: " << (message) << RESET << std::endl;
#define LogInfo(message)    std::cout << BLUE << "Info: " << (message) << RESET << std::endl;

#define LogWWaring(message) std::wcout << YELLOW << "Warning: " << (message) << RESET << std::endl;
#define LogWError(message)   std::wcout << RED << "Error: " << (message) << RESET << std::endl;
#define LogWInfo(message)    std::wcout << BLUE << "Info: " << (message) << RESET << std::endl;

//Hr result macro
#define ReturnOnFail(hr) if(FAILED(hr))\
{\
    LogWError(Utils::GetErrorDescription(hr));\
    return hr;\
} \

#define AssertOnFail(hr) if(FAILED(hr))\
{\
    LogWError(Utils::GetErrorDescription(hr));\
    assert(SUCCEEDED(hr) && "HRESULT failed!");\
}