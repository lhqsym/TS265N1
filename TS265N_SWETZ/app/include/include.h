#ifndef _INCLUDE_H
#define _INCLUDE_H

#define A_CONTROL           1
#include "global.h"
#include "xcfg.h"
#include "config.h"
#include "io_def.h"

//API头文件
#include "api.h"

//bsp头文件
#include "bsp.h"
#include "3rd_party.h"
#include "system.h"
#include "modules.h"

//Functions头文件
#include "func.h"

//扩展
#include "plugin.h"

//资源文件
#include "res.h"

//资源文件
#include "res2.h"

#if TWS_LR
#include "app_lr.h"
#include "app_ringbuf.h"
#endif

#if SWETZ_SWITCH_BY_BAT
#include "app_role_switch.h"
#endif

#endif // _INCLUDE_H
