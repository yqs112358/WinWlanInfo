#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <iostream>
#include <Windows.h>
#include <wlanapi.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
using namespace std;

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wlanapi.lib")


void GetAdapterIP(char* adapterGuid)
{
	PIP_ADAPTER_ADDRESSES pAddresses = NULL;
	ULONG outBufLen = 0;
	DWORD dwRetVal = 0;

	// get buffer size
	GetAdaptersAddresses(AF_INET, 0, NULL, pAddresses, &outBufLen);
	pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(outBufLen);

	// get adapters data
	dwRetVal = GetAdaptersAddresses(AF_INET, 0, NULL, pAddresses, &outBufLen);
	if (dwRetVal == NO_ERROR) {
		// for each adapter
		PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses;
		while(pCurrAddresses) 
		{
			if (strcmp(pCurrAddresses->AdapterName, adapterGuid) == 0) {
				// adapter matched
				PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurrAddresses->FirstUnicastAddress;
				while(pUnicast)
				{
					char buff[100] = { 0 };
					LPSOCKADDR addr = pUnicast->Address.lpSockaddr;
					if (addr->sa_family == AF_INET6)
					{
						sockaddr_in6* sa_in6 = (sockaddr_in6*)addr;
						inet_ntop(AF_INET6, &(sa_in6->sin6_addr), buff, sizeof(buff));
					}
					else
					{
						sockaddr_in* sa_in = (sockaddr_in*)addr;
						inet_ntop(AF_INET, &(sa_in->sin_addr), buff, sizeof(buff));
					}
					cout << "IP Address: " << buff << endl;
					pUnicast = pUnicast->Next;
				}
			}
			pCurrAddresses = pCurrAddresses->Next;
		}
	}
	free(pAddresses);
}

int GetWlanInfo()
{
	PWLAN_INTERFACE_INFO_LIST ilist;
	PWLAN_AVAILABLE_NETWORK_LIST nlist;
	DWORD nv;
	HANDLE ClientHandle;
	bool found = false;

	if (WlanOpenHandle(1, 0, &nv, &ClientHandle) == 0)
	{
		if (WlanEnumInterfaces(ClientHandle, 0, &ilist) == 0)
		{
			for (DWORD i = 0; i < ilist->dwNumberOfItems; i++)
			{
				// wprintf(L"Interface: %s\n\n", ilist->InterfaceInfo[i].strInterfaceDescription);
				GUID guid = ilist->InterfaceInfo[i].InterfaceGuid;
				if (WlanGetAvailableNetworkList(ClientHandle, &guid, 0, 0, &nlist) == 0)
				{
					for (DWORD j = 0; j < nlist->dwNumberOfItems; j++)
					{
						WLAN_AVAILABLE_NETWORK wlanInfo = nlist->Network[j];
						if ((wlanInfo.dwFlags & WLAN_AVAILABLE_NETWORK_CONNECTED) != 0)
						{
							// wifi connected
							found = true;

							char ssid[256] = { 0 };
							memcpy(ssid, wlanInfo.dot11Ssid.ucSSID, wlanInfo.dot11Ssid.uSSIDLength);
							cout << "SSID: " << ssid << endl;

							// get wifi ip
							char buf[100] = { 0 };
							snprintf(buf, sizeof(buf), "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
								guid.Data1,
								guid.Data2,
								guid.Data3,
								guid.Data4[0], guid.Data4[1],
								guid.Data4[2], guid.Data4[3],
								guid.Data4[4], guid.Data4[5],
								guid.Data4[6], guid.Data4[7]);

							GetAdapterIP(buf);
						}
					}
					WlanFreeMemory(nlist);
				}
			}
			WlanFreeMemory(ilist);
		}
		WlanCloseHandle(ClientHandle, 0);
	}
	if (!found)
	{
		cout << "Wifi not connected." << endl;
	}
	return found;
}

int main(int argc, char** argv)
{
	system("chcp 65001 > nul");
	GetWlanInfo();

	return 0;
}