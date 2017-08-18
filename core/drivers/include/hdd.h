#ifndef __HDD_H__
#define __HDD_H__

#include "list.h"
#include "ctype.h"

#define HDD_PARTION_PRIME_NUM 4
#define HDD_PARTION_LOGIC_NUM 8

#define DIV_ROUND_UP(X, STEP) ((X + STEP - 1) / (STEP))

enum DISK_STATUS {
    DISK_IDLE = 0,
    DISK_WORKING
};

/* 分区结构 */
typedef struct partition {
   uint32_t start_lba;		 // 起始扇区
   uint32_t sec_cnt;		 // 扇区数
   struct disk* my_disk;	 // 分区所属的硬盘
   struct list_head ll;	         // 用于队列中的标记
   char name[8];		 // 分区名称
   struct super_block* sb;	 // 本分区的超级块
   //struct bitmap block_bitmap;	 // 块位图
   //struct bitmap inode_bitmap;	 // i结点位图
   struct list_head open_inodes;	 // 本分区打开的i结点队列
}partition;

/* 硬盘结构 */
typedef struct disk {
   char name[8];                                           // 本硬盘的名称，如sda等
   struct ide_channel* my_channel;	                       // 此块硬盘归属于哪个ide通道
   uint8_t dev_no;                                         // 本硬盘是主0还是从1
   struct partition prim_parts[HDD_PARTION_PRIME_NUM];     // 主分区顶多是4个
   struct partition logic_parts[HDD_PARTION_LOGIC_NUM];	   // 逻辑分区数量无限,但总得有个支持的上限,那就支持8个
   uint8_t status;
}disk;

/* ata通道结构 */
typedef struct ide_channel {
   char name[8];		 // 本ata通道名称, 如ata0,也被叫做ide0. 可以参考bochs配置文件中关于硬盘的配置。
   uint16_t port_base;		 // 本通道的起始端口号
   uint8_t irq_no;		 // 本通道所用的中断号
   //struct lock lock;
   bool expecting_intr;		 // 向硬盘发完命令后等待来自硬盘的中断
   //struct semaphore disk_done;	 // 硬盘处理完成.线程用这个信号量来阻塞自己，由硬盘完成后产生的中断将线程唤醒
   struct disk devices[2];	 // 一个通道上连接两个硬盘，一主一从
}ide_channel;

void x86_id0_do_handler();
void x86_id1_do_handler();
void x86_id_do_handler();
//void intr_hd_handler(uint8_t irq_no);
void hdd_init();
void hdd_write(disk* hd, uint32_t lba, void* buf, uint32_t sec_cnt);
void hdd_read(struct disk* hd, uint32_t lba, void* buf, uint32_t sec_cnt);

#endif
