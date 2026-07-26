//
//
#include <lvm/common.h>

LVM_API const lvm_version_t* lvm_version(void) {
    static const lvm_version_t version = {
        .major = LVM_VERSION_MAJOR,
        .minor = LVM_VERSION_MINOR,
        .patch = LVM_VERSION_PATCH
    };
    return &version;
}
