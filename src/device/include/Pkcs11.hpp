#pragma once

// Platform macros the OASIS PKCS#11 headers require (Linux / Unix).
#define CK_PTR *
#define CK_DECLARE_FUNCTION(returnType, name) returnType name
#define CK_DECLARE_FUNCTION_POINTER(returnType, name) returnType(*name)
#define CK_CALLBACK_FUNCTION(returnType, name) returnType(*name)

#ifndef NULL_PTR
#define NULL_PTR nullptr
#endif

#include "pkcs11.h"