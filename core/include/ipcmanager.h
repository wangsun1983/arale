#ifndef __IPCMANAGER_H__
#define __IPCMANAGER_H__

public void ipcmanager_init();
public int ipcmanager_regist(char *ipcname);
public int ipcmanager_transact(uint32_t command,addr_t buffer);

#endif
