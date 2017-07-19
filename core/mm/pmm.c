#include "pmm.h"
#include "mmzone.h"

void *pmm_kmalloc(size_t bytes)
{
    return zone_get_page(ZONE_NORMAL,bytes); 
}
