// =============================================================================
// BTDeviceIsNear
// =============================================================================
bool BTDeviceIsNear() {
	//Initialising winsock
	WSADATA data;
	int result;
	result = WSAStartup(MAKEWORD(2, 2), &data);
	if (result != 0){
		MsgBox("An error occured while initialising winsock, closing....");
		return false;
	}

	//Initialising query for device
	WSAQUERYSET queryset;
	memset(&queryset, 0, sizeof(WSAQUERYSET));
	queryset.dwSize = sizeof(WSAQUERYSET);
	queryset.dwNameSpace = NS_BTH;

	HANDLE hLookup;
	result = WSALookupServiceBegin(&queryset, LUP_CONTAINERS, &hLookup);
	if (result != 0){
		MsgBox("An error occured while initialising look for devices, closing....");
		return false;
	}

	//Initialisation succeed, start looking for devices
	BYTE buffer[4096];
	memset(buffer, 0, sizeof(buffer));
	DWORD bufferLength = sizeof(buffer);
	WSAQUERYSET *pResults = (WSAQUERYSET*)&buffer;
	while (result == 0) {
		result = WSALookupServiceNext(hLookup,
			LUP_RETURN_NAME | LUP_CONTAINERS | LUP_RETURN_ADDR | LUP_FLUSHCACHE |
			LUP_RETURN_TYPE | LUP_RETURN_BLOB | LUP_RES_SERVICE,
			&bufferLength, pResults);
		if (result == 0) { // A device found
			LPTSTR deviceFoundName = pResults->lpszServiceInstanceName;
			PSOCKADDR_BTH sa = PSOCKADDR_BTH(pResults->lpcsaBuffer->RemoteAddr.lpSockaddr);

			if (sa->addressFamily != AF_BTH)
			{
				// Address family is not AF_BTH  for bluetooth device discovered
				continue;
			}
			//the MAC address is available in sa->btAddr
			printf("[HookExe] Device found\n");
			if ((deviceFoundName != NULL) &&
				(0 == _wcsicmp(deviceFoundName, convertCharArrayToLPCWSTR("btdevice")))) {
				printf("[HookExe] Found the device!\n");
				printf("[HookExe] Device name is %S\t Device addr is 0x%0x\n", deviceFoundName, sa->btAddr);
				if (BluetoothIsConnectable(hLookup)) {
				//if (pResults->dwOutputFlags == BTHNS_RESULT_DEVICE_CONNECTED) {
					printf("[HookExe] Device is CONNECTED!!! 0x%0x \n", pResults->dwOutputFlags);
				}
				else {
					printf("[HookExe] Device is NOOOOOOOT CONNECTED!!! - 0x%0x\n", pResults->dwOutputFlags);
				}
				
				return true;
			} else {
				printf("[HookExe] Didn't find the device...\n");
				printf("[HookExe] Device name is %S\t Device addr is 0x%0x\n", deviceFoundName, sa->btAddr);
			}
		}
	}
	
	WSALookupServiceEnd(hLookup);
	return false;
}