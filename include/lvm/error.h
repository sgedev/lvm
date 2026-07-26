//
//
#ifndef LVM_ERROR_H
#define LVM_ERROR_H

#include <lvm/common.h>

#define LVM_OK          0
#define LVM_EPARAM      1
#define LVM_ESTATE      2
#define LVM_EUNKNOWN    255

LVM_BEGIN_DECLS

LVM_API const char* lvm_error_message(int err);

LVM_END_DECLS

#endif // LVM_ERROR_H
