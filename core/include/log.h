#ifndef __LOG_H__
#define __LOG_H__

#include "klibc.h"

#define LOG_DEBUG
#define LOG_INFO

#ifdef LOG_DEBUG
#define LOGD(fmt,...)  \
({ \
  kprintf(__FUNCTION__);  \
  kprintf(" Debug:"); \
  kprintf(fmt,##__VA_ARGS__); \
})
#else
#define LOGD(tag,fmt,...)  \
({ \
})
#endif

#ifdef LOG_INFO
#define LOGI(fmt,...)  \
({ \
  kprintf(fmt,##__VA_ARGS__); \
})
#else
#define LOGI(tag,fmt,...)  \
({ \
})
#endif

#define LOGE(fmt,...)  \
({ \
  kprintf(__FUNCTION__); \
  kprintf(" Error:"); \
  kprintf(fmt,##__VA_ARGS__); \
})

#endif
