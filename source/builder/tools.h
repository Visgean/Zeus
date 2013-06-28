/*
  Р Р°Р·Р»РёС‡РЅС‹Рµ РёРЅСЃС‚СЂСѓРјРµРЅРµРЅС‚С‹.
*/
#pragma once

/*
  Р’С‹РІРѕРґ СЃРѕРѕР±С€РµРЅРёСЏ РІ multilne edit control.

  IN hwnd   - С…СЌРЅРґР» edit-РѕРєРЅР°.
  IN format - С„РѕСЂРјР°С‚ СЃРѕРѕР±С€РµРЅРёРµ СЃРѕРіР»Р°СЃРЅРѕ sprintf.
  IN ...    - Р°СЂРіСѓРјРµРЅС‚С‹ СЃРѕРѕР±С‰РµРЅРёСЏ.
*/
void writeOutput(HWND hwnd, LPWSTR format, ...);

/*
  Р’С‹РІРѕРґ РѕС€РёР±РєРё РІ multilne edit control.

  IN hwnd   - С…СЌРЅРґР» edit-РѕРєРЅР°.
  IN format - С„РѕСЂРјР°С‚ СЃРѕРѕР±С€РµРЅРёРµ СЃРѕРіР»Р°СЃРЅРѕ sprintf.
  IN ...    - Р°СЂРіСѓРјРµРЅС‚С‹ СЃРѕРѕР±С‰РµРЅРёСЏ.
*/
void writeOutputError(HWND hwnd, LPWSTR format, ...);

/*Р’В Р’В Checks whether the stream ended.

Р’В Р’В IN OUT subThread - pointer to hegdl flow. If, returned with the handle == NULL, then the flow ended.*/
void closeThreadIfFinsinhed(HANDLE *subThread);

