#ifndef __INCLUDE_ARCH_X64_ACPI_H__
#define __INCLUDE_ARCH_X64_ACPI_H__

/* Advanced Configuration and Power Interface */

#include "limine.h"
extern struct limine_rsdp_request rsdp_req;

void acpi_init();

#endif /* __INCLUDE_ARCH_X64_ACPI_H__ */