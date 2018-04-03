/*

  No, I am your father. --Darth Vader 

*/

#include <stdio.h>
#include <winsock2.h>
#include <windows.h>

#include <shellapi.h>
#include <tlhelp32.h>
#include <Shlwapi.h>
#include <time.h>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "ws2_32.lib")

int skip=0;
int ishttp = 0;
int todel = 1;
int startingfrom = 0;
int cutme=10;
int verbose = 0;
int tosleep = 10;
int g_timeout = 20;

char progress[4]={'|','\\','-','/'};

int ieerror = 0;
char globalcname[1000] = {0};
char toload[255] = "tmp.htm";
char orgfile[255] = "tmp.htm";
char *logfile = "c:\\windows\\temp\\temp.log";

DWORD FindProcessIDByName(char *name)
{
	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(pe);
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	Process32First(hSnapshot, &pe);
	do
	{
		if(stricmp(pe.szExeFile, name) == 0)
		{
			CloseHandle(hSnapshot);
			return pe.th32ProcessID;
		}
	}
	while(Process32Next(hSnapshot, &pe));
	CloseHandle(hSnapshot);
	return 0;
}

void KillPid(DWORD Pid)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS,FALSE,Pid);
	if(hProcess)
	{
		TerminateProcess(hProcess,0);
		CloseHandle(hProcess);
	}
	return;
}

void KillAllByName(char *filename)
{
	int i=0;
	char cmd[255];
	sprintf(cmd, "cmd.exe /c taskkill /f /im %s", filename);
	while(FindProcessIDByName(filename) != 0)
	{
		DWORD pid = FindProcessIDByName(filename);
		KillPid(pid);
		i++;
//		printf(".%d",pid);
		if(i > 10)
		{
			WinExec(cmd, SW_SHOW);
			i=0;
		}
		Sleep(300);
	}
}

BOOL CALLBACK EnumChildWindows(HWND hwnd, LPARAM lParam)
{
	char str[1000]={0};
	GetWindowText(hwnd,str,sizeof(str));
	if(strstr(str, "Application Error") && strstr(str, "SysFader:"))
		ieerror = 1;
	if(strstr(str, "应用程序错误"))
		ieerror = 1;
	if(strstr(str, "Data Execution Prevention"))
		ieerror = 1;
	return TRUE;
}

BOOL CALLBACK lpEnumWindows(HWND hwnd, LPARAM lParam)
{
    
	char str[1000]={0};
	GetWindowText(hwnd,str,sizeof(str));
	if(strstr(str, "Application Error") && strstr(str, "SysFader:"))
	{
		SendMessage(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0);
		ieerror = 1;
	}
	if(strstr(str, "应用程序错误"))
	{
		SendMessage(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0);
		ieerror = 1;
	}
	if(strstr(str, "Data Execution Prevention"))
	{
		SendMessage(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0);
		ieerror = 1;
	}
	EnumChildWindows(hwnd,EnumChildWindows,NULL);
	return TRUE;
}

int FoundCrash()
{
	if(FindProcessIDByName("WerFault.exe")||FindProcessIDByName("dw20.exe")
			||FindProcessIDByName("dwwin.exe")||FindProcessIDByName("iedw.exe")
			||FindProcessIDByName("drwtsn32.exe")||FindProcessIDByName("cdb.exe")||PathFileExists(logfile))
		return 1;
	ieerror = 0;
	EnumWindows(lpEnumWindows,NULL);
	if(ieerror == 1)
	{
		//printf("\nEureka 95502\n");
		ieerror = 0;
		return 1;
	}
	return 0;
}

int CheckCrashRemoved()
{
	if(FindProcessIDByName("WerFault.exe")||FindProcessIDByName("dw20.exe")
			||FindProcessIDByName("dwwin.exe")||FindProcessIDByName("iedw.exe")
			||FindProcessIDByName("drwtsn32.exe")||FindProcessIDByName("cdb.exe")||PathFileExists(logfile))
		return 1;
	ieerror = 0;
	EnumWindows(lpEnumWindows,NULL);
	if(ieerror == 1)
	{
		//printf("\nEureka 95501\n");
		ieerror = 0;
		return 1;
	}
	return 0;
}

int RemoveCrash()
{
	KillAllByName("WerFault.exe");
	KillAllByName("cdb.exe");
	KillAllByName("MicrosoftEdgeCP.exe");
	KillAllByName("RuntimeBroker.exe");
	KillAllByName("browser_broker.exe");
//	KillAllByName("Windows.WARP.JITService.exe");
	KillAllByName("MicrosoftEdge.exe");
	DeleteFile(logfile);
	return 1;
}

void TryClean(char *path,int p=0)
{
	if(p==1)
		printf("cleaning %s\n",path);
	char szDir[1024];
	char file2del[2048];
	sprintf(szDir, "%s*.*", path);
	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(szDir, &ffd);
	do{
		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			sprintf(file2del, "%s%s", path,ffd.cFileName);
			DeleteFileA(file2del);
			Sleep(1);
		}
	} while (FindNextFile(hFind, &ffd) != 0);
	FindClose(hFind);
}

void GoClean(int p=0)
{
	char user[50];
	
	char tmp[1000];
	char edgecache[1000];

	DWORD temp=50;
	GetUserNameA(user, &temp);

	//crash case
	sprintf(tmp, "c:\\Users\\%s\\AppData\\Local\\Packages\\Microsoft.MicrosoftEdge_8wekyb3d8bbwe\\AC\\MicrosoftEdge\\User\\Default\\Recovery\\Active\\", user);
	TryClean(tmp,p);

	//edge cache
	sprintf(edgecache, "c:\\Users\\%s\\AppData\\Local\\Packages\\Microsoft.MicrosoftEdge_8wekyb3d8bbwe\\AC\\#!001\\MicrosoftEdge\\Cache\\*.*", user);
	WIN32_FIND_DATA ffd;
	
	HANDLE hFind = FindFirstFile(edgecache, &ffd);
	do{
		if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && ffd.cFileName[0]!='.')
		{
			sprintf(tmp, "c:\\Users\\%s\\AppData\\Local\\Packages\\Microsoft.MicrosoftEdge_8wekyb3d8bbwe\\AC\\#!001\\MicrosoftEdge\\Cache\\%s\\", user, ffd.cFileName);
			TryClean(tmp,p);
			Sleep(1);
		}
	} while (FindNextFile(hFind, &ffd) != 0);
	FindClose(hFind);
}

int maxstacklevel = 20;
char stackcallerendchar = ' ';
int nullifyhash = 1;
int useropemode = 0;
int nullifysize = 1;

unsigned int hashstr(char *str)
{
    unsigned int hash = 5381;
    int c;

    while (c = *str++)
	{
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

int checkcrash(char *buf, int len, char *cname, DWORD*objlen=NULL,DWORD*stackhash=NULL)
{
	int i,j,k;
	int ihash = 0;

	//check if register values are found
	//but sometimes there is no register!!!!
	for(i=len-5;i>0;i--)
	{
		if(*(DWORD*)(buf+i) == '=lfe')
			break;
	}
	if(i!=0)
		i+=4;
	else
	{
		//This exception may be expected and handled
		for(i=len-16;i>0;i--)
		{
			if(*(DWORD*)(buf+i) == 'epxe'&&*(DWORD*)(buf+i+4) == 'detc'&&*(DWORD*)(buf+i+8) == 'dna '&&*(DWORD*)(buf+i+12) == 'nah ')
				break;
		}
		if(i==0)
			return 0;
		i+=20;
	}
	for(;i<len;i++)
	{
		if((buf[i] >= 'a' && buf[i] <= 'z')||(buf[i] >= 'A' && buf[i] <= 'Z'))
			break;
	}
	if(i==len)
		return 0;
	//usually we meet the dll name and symbol, but sometime it is 
	//"*** ERROR: Symbol file could not be found."   or
	//"*** WARNING: xxoo"
	//so we skip this line

	if((*(DWORD*)(buf+i) == 'ORRE' && buf[i-3] == '*')||(*(DWORD*)(buf+i) == 'NRAW' && buf[i-3] == '*'))
	{
		for(;i<len;i++)
		{
			if(buf[i] == '\r' || buf[i] == '\n')
				break;
		}
		if(i==len)
			return 0;

		for(;i<len;i++)
		{
			if((buf[i] >= 'a' && buf[i] <= 'z')||(buf[i] >= 'A' && buf[i] <= 'Z'))
				break;
		}
		if(i==len)
			return 0;
	}


	for(j=i;j<len;j++)
	{
		if(buf[j]==':' && buf[j-1]!=':' && buf[j+1]!=':')
			break;
	}
	if(j==len)
		return 0;
	//deal with stack overflow

	if(strstr(buf, "Stack overflow - code c00000fd"))
	{
		cname[0]='!';
		cname[1]='!';
		k=2;
	}
	else if(strstr(buf, "=??") == 0)
	{
		cname[0]='@';
		cname[1]='@';
		k=2;
	}
	else
		k=0;
	///////////////////////////////////////////

	

	for(;i<j;i++)
	{
		if((buf[i] >= 'a' && buf[i] <= 'z')||(buf[i] >= 'A' && buf[i] <= 'Z')||(buf[i] >= '0' && buf[i] <= '9'))
			cname[k]=buf[i];
		else
			cname[k]='!';
		k++;
		if(k>200)
			break;
	}
	//printf("%s\n",cname);

	//added by Swarovski 2015.07.06
	if(cname[0] == '!' || cname[0] == '@')
		return 1;

	if(useropemode==1)
		return 1;
	//continue parsing
	char *index = strstr(buf, "=??");
	if(index == NULL)
		return 1; //garbage code
	
	int last3;
	sscanf(index - 8, "%x", &last3);
	if(last3 > 0x1000)
	{
		last3 = last3 % 0x1000+0xffff0000;
	}
	index = strstr(buf, "Args to Child");
	if(index == NULL)
		return 1;

	index = strchr(index, '\n');
	while(index && !(index[1] <= '9' && index[1] >= '0'))
	{
		index = strchr(index + 1, '\n');
	}
	if(index == NULL)
		return 1;
	index ++;
	//parse stack and ignor error message
//	printf("%x\n%s", 0x1000-last3, index);

	char stack[500];
	char caller[500];

	int nowstack = 0;
	while(1)
	{
		if(*index == 0 || !((*index <= '9' && *index >= '0') || *index == '*') || nowstack > maxstacklevel)
			break;
		if(*index == '*')
		{
			index = strchr(index, '\n');
			if(index == NULL)
				return 1;
			index ++;
			continue;
		}
		i = 0;
		while(index[i] != '\n' && index[i] != '\r' && index[i] != 0)
		{
			stack[i] = index[i];
			i++;
			if(i >= 400)
				break;
		}
		stack[i] = 0;
//		printf("%s\n", stack);

		char*tmp = stack;
		tmp = strchr(tmp, '+');
		if(tmp == NULL)
		{
			tmp = stack;
			tmp = strchr(tmp, '!');
			if(tmp == NULL)
				goto nextstackline;
		}
		while(*tmp != ' ') tmp --; //buggy here?
		tmp ++;
		i = 0;
		while(tmp[i] != stackcallerendchar && tmp[i] != 0 && tmp[i] != '\r' && tmp[i] != '\n')
			caller[i] = tmp[i], i++;

		caller[i] = 0;
//		printf("%s\n", caller);
		ihash += hashstr(caller);

		nowstack++;
nextstackline:
		index = strchr(index, '\n');
		if(index == NULL)
			return 1;
		index ++;
	}

	if(nullifyhash == 1)
		ihash = 0;

	if(nullifysize == 1)
		last3 &= 0xfffff000;

	char padding[50];
	sprintf(padding, ".%0.8x.%0.8x",last3,ihash);
	strcat(cname, padding);
	if(objlen) *objlen = last3;
	if(stackhash) *stackhash = ihash;
	return 1;
}

int ParseCrashLog(char*filename, char*cname)
{
	if(NULL == cname)
		return 0;
	FILE*fp = fopen(filename, "rb");
	if(NULL == fp)
		return 0;
	fseek(fp, 0, SEEK_END);
	int len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *buf = (char*)malloc(len+1);
	if(NULL == buf)
	{
		fclose(fp);
		return 0;
	}
	memset(buf, 0, len+1);
	fread(buf, len, 1, fp);
	fclose(fp);
	checkcrash(buf, len, cname);
	free(buf);
	return 1;
}

int cdbcheck(char*cmd, char*localname, char*unused)
{
	DeleteFile(logfile);
	ShellExecute(NULL, "open", cmd, NULL, NULL, SW_SHOW);
	while(FindProcessIDByName("MicrosoftEdgeCP.exe") == 0)
		Sleep(50);
	int crashfound = 0;
	int i = 0;
	for(i=0;i<g_timeout*10;i++)
	{
		if(FoundCrash())
		{
			crashfound = 1;
			break;
		}
		Sleep(100);
	}
	if(crashfound == 0)
	{
		//clear all edge processes...
		KillAllByName("MicrosoftEdge.exe");
		KillAllByName("MicrosoftEdgeCP.exe");
		KillAllByName("RuntimeBroker.exe");
		KillAllByName("browser_broker.exe");
//		KillAllByName("Windows.WARP.JITService.exe");
		Sleep(1000);
		return 0;
	}

	i = 0;
	//Sleep for a while
	while(i < 10 && !PathFileExists(logfile))
	{
		Sleep(200);
	}
	//please, cdb does not hang
	while(FindProcessIDByName("cdb.exe") == 0)
		Sleep(50);

	ParseCrashLog(logfile, localname);

	RemoveCrash();
	while(i < 10 && CheckCrashRemoved()==1)
	{
		Sleep(50);
		i++;
		RemoveCrash();
	}
	
	return 2;
}

void iSetCrashLevel(int n=0)
{
	switch(n)
	{
		case 0:
			useropemode = 1;
			break;
		case 1:
			useropemode = 0;
			nullifyhash = 1;
			break;
		case 2:
			maxstacklevel = 15;
			stackcallerendchar = '+';
			nullifyhash = 0;
			useropemode = 0;
			break;
		case 3:
			maxstacklevel = 22;
			stackcallerendchar = ' ';
			nullifyhash = 0;
			useropemode = 0;
			break;
		case 4:
			maxstacklevel = 15;
			stackcallerendchar = '+';
			nullifyhash = 0;
			useropemode = 0;
			nullifysize = 1;
			break;
	}
}

int counter=0;

int RunAndTest(char *content, int itime)
{
	GoClean();
	SHDeleteKey(HKEY_CURRENT_USER, "Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\CurrentVersion\\AppContainer\\Storage\\microsoft.microsoftedge_8wekyb3d8bbwe\\MicrosoftEdge\\Recovery");
	//initLoader();
	char dir[256];
	char *pp;
	GetCurrentDirectory(255, dir);
//	sprintf(dir, "%s\\tmp%d.htm", dir, counter++);
	strcat(dir, "\\");
	strcat(dir, orgfile);
	DeleteFile(dir);
	FILE*fp = fopen(dir, "wb");
	fwrite(content,strlen(content),1,fp);
	fclose(fp);

	char cmd[1024]={0};
	char localcname[1024]={0};
	if(ishttp == 0)
		sprintf(cmd, "\"%s\"", toload);
	else
	{
		sprintf(cmd, "http://127.0.0.1/%s", toload);
		pp = strstr(cmd, "\\");
		while(pp)
		{
			*pp = '/';
			pp = strstr(cmd, "\\");
		}
	}
	int ret = cdbcheck(cmd, localcname, "tmp");

	if(verbose)
		printf("local cname=%s\n",localcname);
	//printf("global cname=%s\n",globalcname);
	int len=strlen(localcname);
	if(ret == 2 && strncmp(globalcname,localcname,len-1)==0)
	{
		if(verbose)
			printf("\n[match!] len-1\n");
		else
			printf("\b+");
		return 1;
	}
	if(strstr(globalcname,localcname) || strstr(localcname,globalcname))
	{
		if(ret == 2)
		{
			if(verbose)
				printf("\n[match!] substring\n");
			else
				printf("\b+");
			return 1;
		}
	}
	printf("\b-");
	//printf("Crash is %s", cname);
	return 0;
}

void Do(char *in, char *out, int itime=6)
{
	int ret;
	FILE* fp = fopen(in, "rb");
	if(fp == NULL)
		return;
	fseek(fp, 0, SEEK_END);
	int len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* buf = new char[len+1];
	memset(buf, 0, len+1);
	fread(buf, len, 1, fp);
	fclose(fp);

	char *tmp = new char[len*2];
	char *final = new char[len*2];
	memset(tmp, 0, len*2);
	memset(final, 0, len*2);
	char *l0, *l1;
	l0 = buf;
	char *next;

	l1 = strchr(l0, '\n');
	if(l1 == NULL)
	{
		delete buf;
		delete tmp;
		delete final;
		return;
	}
	memcpy(final, l0, l1-l0);
	while(1)
	{
		l1 = strchr(l0, '\n');
		if(l1 == NULL)
			break;

		//ignore html element, yeah..
		//but for those grinder kids I don't think you really need it
		if(l1[1] == '<' || l1[1] == '{' || l1[1] == '}' || (l1[1] == '/'&&l1[2] == '/') || skip > 0)
		{
			skip--;
			next = strchr(l1+1,'\n');
			if(next == NULL)
				next = buf + strlen(buf);
			memcpy(final+strlen(final), l1, next-l1);
			l0 = l1+1;
			continue;
		}

		strcpy(tmp, final);
		strcat(tmp, "\n//");
		strcat(tmp, l1+1);

		//free version
		ret = RunAndTest(tmp, itime);

		if(ret == 1)
			printf(".");
		
		next = strchr(l1+1,'\n');
		if(next == NULL)
			next = buf + strlen(buf) - 1;

		if(ret == 1)
		{
			strcat(final, "\n//");
			if(next-l1>1)
				memcpy(final+strlen(final), l1+1, next-l1-1);
		}
		else
		{
			if(next-l1>0)
				memcpy(final+strlen(final), l1, next-l1);
		}

		l0 = l1+1;
	}

	fp = fopen(out, "wb");
	fwrite(final, strlen(final), 1, fp);
	fclose(fp);

	delete buf;
	delete tmp;
	delete final;
}

char *lastchar(char*start, char*head, char what)
{
	if(start == NULL || head == NULL)
		return NULL;
	if(start < head)
		return NULL;
	while(start >= head)
	{
		if(*start == what)
			return start;
		start --;
	}
	return NULL;
}

void Od(char *in, char *out, int itime=6)
{
	int ret;
	FILE* fp = fopen(in, "rb");
	if(fp == NULL)
		return;
	fseek(fp, 0, SEEK_END);
	int len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* buf = new char[len+1];
	memset(buf, 0, len+1);
	fread(buf, len, 1, fp);
	fclose(fp);

	char *tmp = new char[len*2];
	char *final = new char[len*2];
	memset(tmp, 0, len*2);
	memset(final, 0, len*2);
	strcpy(final, buf);
	strcpy(tmp, buf);
	char *l0, *l1;
	l0 = tmp+len-1;
//	char *next;

	l1 = lastchar(l0, tmp, '\n');
	if(l1 == NULL)  //hmm, interesting
	{
		delete buf;
		delete tmp;
		delete final;
		printf("less than 2 lines");
		return;
	}
	
	while(1)
	{
		l1 = lastchar(l0, tmp, '\n');
		if(l1 == NULL)
			break;

		//ignore html element, yeah..
		//but for those grinder kids I don't think you really need it
		if(l1[1] == '<' || l1[1] == '{' || l1[1] == '}' || (l1[1] == '/'&&l1[2] == '/'))
		{
			l0 = l1 - 1;
			continue;
		}

		while(*(l0+1)=='\n')
			l0++;
		strcpy(l1+1, l0+1);

		ret = RunAndTest(tmp, itime);

		if(ret == 1)
			strcpy(final, tmp);
		else
			strcpy(tmp, final);

		l0 = l1 - 1;
	}

	fp = fopen(out, "wb");
	fwrite(final, strlen(final), 1, fp);
	fclose(fp);

	delete buf;
	delete tmp;
	delete final;
}

char*findmatch(char*content, char pp='{')
{
	if(content == NULL)
		return NULL;
	char *p = content;
	char ok[2];
	if(pp == '{')
		ok[0] = '{', ok[1] = '}';
	else if(pp == '(')
		ok[0] = '(', ok[1] = ')';
	else if(pp == '[')
		ok[0] = '[', ok[1] = ']';
	else if(pp == '<')
		ok[0] = '<', ok[1] = '>';
	else if(pp == ' ')
		ok[0] = ' ', ok[1] = ' ';

//	printf("{%c %c}", ok[0], ok[1]);
	int quote=0;
	int bracket=1;
	while(*p)
	{
		if(*p == '\"')
		{
			p ++;
			while(*p)
			{
				if(*p == '\\')
					p ++;
				else if(*p == '\"')
					break;
				p ++;
			}
			if(*p == 0)
				return NULL;
		}
		else if(*p == ok[1])
		{
			bracket --;
			if(bracket == 0)
				return p;
		}
		else if(*p == ok[0])
		{
			bracket ++;
		}
		p ++;
	}
	return NULL;
}

char*findmatch2(char*content, char pp='\'')
{
	if(content == NULL)
		return NULL;
	char *p = content;
	char ok[2];
	if(pp == '\'')
		ok[0] = '\'', ok[1] = '\'';
	else if(pp == '\"')
		ok[0] = '\"', ok[1] = '\"';
	
	int quote=0;
	int bracket=1;
	while(*p)
	{
		if(*p == ok[1] && *(p-1) != '\\')
		{
			return p;
		}
		p ++;
	}
	return NULL;
}

void DoQuick(char *in, char *out, int itime=6)
{
	if(in == NULL)
	{
		printf("[error] wrong file name\n");
		exit(0);
	}
	FILE* fp = fopen(in, "rb");
	if(fp == NULL)
		return;
	fseek(fp, 0, SEEK_END);
	int len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* buf = (char*)malloc(len+1);
	
	if(buf == NULL)
	{
		fclose(fp);
		printf("[error] failed allocating buffer %d\n",GetLastError());
		exit(0);
	}
	memset(buf, 0, len+1);
	fread(buf, len, 1, fp);
	fclose(fp);
	char *p;

	char* w = (char*)malloc(len+1);
	if(buf == NULL)
	{
		free(buf);
		printf("[error] failed allocating temp buffer\n");
		exit(0);
	}
	memcpy(w, buf, len+1);
	int ret;

	for(int i=startingfrom;i<len;i++)
	{
		if(buf[i] == '{')
		{
			p = findmatch(buf + i + 1);
			if(p)
			{
				if(p - buf - i - 1 > 2)
				{
					memcpy(w, buf, len+1);
					memset(w + i + 1, 0x20, p - buf - i - 1);
					ret = RunAndTest(w, itime);
					if(ret == 1)
					{
						printf(".");
						strcpy(buf + i + 1, p);
						len = strlen(buf);
					}
				}
			}
		}
	}

	fp = fopen(out, "wb");
	fwrite(buf, len, 1, fp);
	fclose(fp);

	free(buf);
	free(w);
}

void DoHTMLString(char *in, char *out, int itime=6)
{
	if(in == NULL)
	{
		printf("[error] wrong file name\n");
		exit(0);
	}
	FILE* fp = fopen(in, "rb");
	if(fp == NULL)
		return;
	fseek(fp, 0, SEEK_END);
	int len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* buf = (char*)malloc(len+1);
	
	if(buf == NULL)
	{
		fclose(fp);
		printf("[error] failed allocating buffer %d\n",GetLastError());
		exit(0);
	}
	memset(buf, 0, len+1);
	fread(buf, len, 1, fp);
	fclose(fp);
	char *p;

	char* w = (char*)malloc(len+1);
	if(buf == NULL)
	{
		free(buf);
		printf("[error] failed allocating temp buffer\n");
		exit(0);
	}
	memcpy(w, buf, len+1);
	int ret;

	for(int i=startingfrom;i<len;i++)
	{
		if(buf[i] == '<')
		{
			//printf("[offset:%d]",i);
			p = findmatch(buf + i + 1, '<');
			//printf("[match:%d]",p - buf - i);
			if(p)
			{
				if(p - buf - i > 1)
				{
					memcpy(w, buf, len+1);
					memset(w + i, 0x20, p - buf - i + 1);
					ret = RunAndTest(w, itime);
					if(ret == 1)
					{
						printf(".");
						strcpy(buf + i , p + 1);
						len = strlen(buf);
						i--;
					}
				}
			}
		}
	}

	fp = fopen(out, "wb");
	fwrite(buf, len, 1, fp);
	fclose(fp);

	free(buf);
	free(w);
}

void DoHTMLAttribute(char *in, char *out, int itime=6)
{
	if(in == NULL)
	{
		printf("[error] wrong file name\n");
		exit(0);
	}
	FILE* fp = fopen(in, "rb");
	if(fp == NULL)
		return;
	fseek(fp, 0, SEEK_END);
	int len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* buf = (char*)malloc(len+1);
	
	if(buf == NULL)
	{
		fclose(fp);
		printf("[error] failed allocating buffer %d\n",GetLastError());
		exit(0);
	}
	memset(buf, 0, len+1);
	fread(buf, len, 1, fp);
	fclose(fp);
	char *p;

	char* w = (char*)malloc(len+1);
	if(buf == NULL)
	{
		free(buf);
		printf("[error] failed allocating temp buffer\n");
		exit(0);
	}
	memcpy(w, buf, len+1);
	int ret;

	for(int i=startingfrom;i<len;i++)
	{
		if(buf[i] == ' ')
		{
			printf("[offset:%d]",i);
			p = findmatch(buf + i + 1, ' ');
			printf("[match:%d]",p - buf - i);
			if(p)
			{
				if(p - buf - i > 1)
				{
					memcpy(w, buf, len+1);
					memset(w + i + 1, 0x09, p - buf - i - 1);
					ret = RunAndTest(w, itime);
					if(ret == 1)
					{
						if(verbose)
							printf("bingo.");
						strcpy(w + i + 1, p);
						if(RunAndTest(w, itime)==1)
						{
							strcpy(buf + i + 1, p);
							len = strlen(buf);
							i--;
						}
						else
						{
							memset(buf + i + 1, 0x09, p - buf - i - 1);
						}
					}
				}
			}
		}
	}

	fp = fopen(out, "wb");
	fwrite(buf, len, 1, fp);
	fclose(fp);

	free(buf);
	free(w);
}

void DoHTMLAttribute2(char *in, char *out, int itime=6)
{
	if(in == NULL)
	{
		printf("[error] wrong file name\n");
		exit(0);
	}
	FILE* fp = fopen(in, "rb");
	if(fp == NULL)
		return;
	fseek(fp, 0, SEEK_END);
	int len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* buf = (char*)malloc(len+1);
	
	if(buf == NULL)
	{
		fclose(fp);
		printf("[error] failed allocating buffer %d\n",GetLastError());
		exit(0);
	}
	memset(buf, 0, len+1);
	fread(buf, len, 1, fp);
	fclose(fp);
	char *p;

	char* w = (char*)malloc(len+1);
	if(buf == NULL)
	{
		free(buf);
		printf("[error] failed allocating temp buffer\n");
		exit(0);
	}
	memcpy(w, buf, len+1);
	int ret;

	for(int i=startingfrom;i<len;i++)
	{
		if(buf[i] == '\"')
		{
			if(verbose)
				printf("[offset:%d]",i);
			p = findmatch2(buf + i + 1, '\"');
			if(verbose)
				printf("[match:%d]",p - buf - i);
			if(p)
			{
				if(p - buf - i > 1)
				{
					memcpy(w, buf, len+1);
					memset(w + i + 1, 0x09, p - buf - i - 1);
					ret = RunAndTest(w, itime);
					if(ret == 1)
					{
						if(verbose)
							printf("bingo.");
						strcpy(w + i + 1, p);
						if(RunAndTest(w, itime)==1)
						{
							strcpy(buf + i + 1, p);
							len = strlen(buf);
							i--;
						}
						else
						{
							memset(buf + i + 1, 0x09, p - buf - i - 1);
						}
					}
				}
			}
		}
	}

	fp = fopen(out, "wb");
	fwrite(buf, len, 1, fp);
	fclose(fp);

	free(buf);
	free(w);
}

void DoHTMLAttribute3(char *in, char *out, int itime=6)
{
	if(in == NULL)
	{
		printf("[error] wrong file name\n");
		exit(0);
	}
	FILE* fp = fopen(in, "rb");
	if(fp == NULL)
		return;
	fseek(fp, 0, SEEK_END);
	int len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* buf = (char*)malloc(len+1);
	
	if(buf == NULL)
	{
		fclose(fp);
		printf("[error] failed allocating buffer %d\n",GetLastError());
		exit(0);
	}
	memset(buf, 0, len+1);
	fread(buf, len, 1, fp);
	fclose(fp);
	char *p;

	char* w = (char*)malloc(len+1);
	if(buf == NULL)
	{
		free(buf);
		printf("[error] failed allocating temp buffer\n");
		exit(0);
	}
	memcpy(w, buf, len+1);
	int ret;

	for(int i=startingfrom;i<len;i++)
	{
		if(buf[i] == '\'')
		{
			if(verbose)
				printf("[offset:%d]",i);
			p = findmatch2(buf + i + 1, '\'');
			if(verbose)
				printf("[match:%d]",p - buf - i);
			if(p)
			{
				if(p - buf - i > 1)
				{
					memcpy(w, buf, len+1);
					memset(w + i + 1, 0x09, p - buf - i - 1);
					ret = RunAndTest(w, itime);
					if(ret == 1)
					{
						if(verbose)
							printf("bingo.");
						strcpy(w + i + 1, p);
						if(RunAndTest(w, itime)==1)
						{
							strcpy(buf + i + 1, p);
							len = strlen(buf);
							i--;
						}
						else
						{
							memset(buf + i + 1, 0x09, p - buf - i - 1);
						}
					}
				}
			}
		}
	}

	fp = fopen(out, "wb");
	fwrite(buf, len, 1, fp);
	fclose(fp);

	free(buf);
	free(w);
}

void eli(char *in, char*out, char*li="//")
{
	int i=0;
	int llen = strlen(li);
	FILE* fp = fopen(in, "rb");
	if(fp == NULL)
		return;
	fseek(fp, 0, SEEK_END);
	int len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* buf = new char[len+1+llen];
	memset(buf, 0, len+1+llen);
	fread(buf, len, 1, fp);
	fclose(fp);

	char *tmp = new char[len+1+llen];
	memset(tmp, 0, len+1+llen);
	char*p, *q;
	p = buf;
	q = strstr(p, "\n");

	while(1)
	{
		if(q)
		{
			if(*(q-1) == '\r')
				*(q-1) = 0;
			else
				*q = 0;
		}
		if(strncmp(p, li, llen) != 0)
		{
			i++;
			printf("\b%c", progress[i%4]);
			strcat(tmp, p);
		}
		if(q)
			p = q + 1;
		else
			break;
		strcat(tmp, "\r\n");
		//p = q + 1;
		q = strstr(p, "\n");
	}
	fp = fopen(out, "wb");
	if(fp == NULL)
	{
		delete tmp;
		delete buf;
		return;
	}
	fwrite(tmp, 1, strlen(tmp), fp);
	fclose(fp);
	printf("\rdone! %d lines were processed.", i);
	delete tmp;
	delete buf;
}

char*skipchar(char*in)
{
	if(in == NULL)
		return NULL;
	char*p = in;
	while(*p && (*p==' '||*p=='\r'||*p=='\n'||*p=='\t')) p++;
	if(*p)
		return p;
	return NULL;
}

void memelilevel0(char*buf, char*tmp, int maxlen)
{
	if(buf == NULL)
		return;
	if(tmp == NULL)
		return;
	memset(tmp, 0, maxlen);
	int len = strlen(buf);
	int j = 0;
	for(int i=0;i<len;i++)
	{
		tmp[j]=buf[i];
		j++;
		if(buf[i] == '\n')
		{
			while(buf[i+1] =='\r' || buf[i+1] =='\n')
				i++;
		}
	}
}

void memelilevel1(char*buf, char*tmp, int maxlen)
{
	if(buf == NULL)
		return;
	if(tmp == NULL)
		return;
	memset(tmp, 0, maxlen);
	char*p, *q;
	char*bq, *qb;
	p = buf;
	while(1)
	{
		p = skipchar(p);
		if(p == NULL)
			break;
		if(strncmp(p, "switch", 6) == 0)
		{
			bq=strchr(p, ')');
			{
				if(bq)
				{
					bq = skipchar(bq+1);
					if(*bq == '{')
					{
						bq = skipchar(bq+1);
						if(*bq == '}')
						{
							p = bq + 1;
						}
					}
				}
			}
		}
		else if(strncmp(p, "try", 3) == 0)
		{
			bq=strchr(p, '{');
			{
				if(bq)
				{
					bq = skipchar(bq+1);
					if(*bq == '}')
					{
						bq = strstr(bq, "catch");
						if(bq)
						{
							bq=strchr(bq, '{');
							if(bq)
							{
								bq = skipchar(bq+1);
								if(*bq == '}')
								{
									p = bq + 1;
								}
							}
						}
					}
				}
			}
		}else if(strncmp(p, "for", 3) == 0)
		{
			bq=p+3;
			if(bq)
			{
				bq = skipchar(bq+1);
				if(bq && (*bq == '('))
				{
					//printf("[%d,",bq-buf);
					bq = findmatch(bq+1, '(');
					if(bq)
					{
						//printf("%d,",bq-buf);
						bq=strchr(bq, '{');
						if(bq)
						{
							//printf("%d,",bq-buf);
							bq = skipchar(bq+1);
							if(*bq == '}')
							{
								//printf("%d]\n",bq-buf);
								p = bq + 1;
								//printf("hooray!");
							}
						}
					}
				}
			}
			//printf("\n");
		}
		else if(strncmp(p, "if", 2) == 0)
		{
			bq=p+2;
			if(bq)
			{
				bq = skipchar(bq+1);
				if(bq && (*bq == '('))
				{
					//printf("[%d,",bq-buf);
					bq = findmatch(bq+1, '(');
					if(bq)
					{
						//printf("%d,",bq-buf);
						bq=strchr(bq, '{');
						if(bq)
						{
							//printf("%d,",bq-buf);
							bq = skipchar(bq+1);
							if(*bq == '}')
							{
								qb = skipchar(bq + 1);
								if(strncmp(qb, "else", 4) != 0)
								{
									//printf("%d]\n",bq-buf);
									p = bq + 1;
									//printf("hooray!");
								}
								else
								{
									if(qb)
									{
										qb = skipchar(qb+4);
										if(*qb == '{')
										{
											qb = skipchar(qb+1);
											if(*qb == '}')
											{
												//printf("endif [%d]\n",qb-buf);
												p = qb + 1;
												//printf("hooray!");
											}
										}
									}
								}
							}
						}
					}
				}
			}
			//printf("\n");
		}
		q = strstr(p, "\n");
		if(q)
		{
			if(*(q-1) == '\r')
				*(q-1) = 0;
			else
				*q = 0;
		}
		strcat(tmp, p);
		if(*p != 0)
			strcat(tmp, "\r\n");
//		printf("\n======\n%s", p);
		if(q)
			p = q + 1;
		else
			break;
	}
}

void memelilevel3(char*buf, char*tmp, int maxlen)
{
	if(buf == NULL)
		return;
	if(tmp == NULL)
		return;
	memset(tmp, 0, maxlen);
	char*p, *q;
	char*bq;
	p = buf;
	while(1)
	{
		p = skipchar(p);
		if(p == NULL)
			break;
		if(strncmp(p, "function", 8) == 0)
		{
			bq=strchr(p, ')');
			{
				if(bq)
				{
					bq = skipchar(bq+1);
					if(*bq == '{')
					{
						bq = skipchar(bq+1);
						if(*bq == '}')
						{
							p = bq + 1;
						}
					}
				}
			}
		}
		q = strstr(p, "\n");
		if(q)
		{
			if(*(q-1) == '\r')
				*(q-1) = 0;
			else
				*q = 0;
		}
		strcat(tmp, p);
		if(*p != 0)
			strcat(tmp, "\r\n");
		if(q)
			p = q + 1;
		else
			break;
	}
}

void memelilevel2(char*buf, char*tmp, int maxlen, char*li="//")
{
	if(buf == NULL)
		return;
	if(tmp == NULL)
		return;
	int llen = strlen(li);
	memset(tmp, 0, maxlen);
	char*p, *q;
	p = buf;
	q = strstr(p, "\n");

	while(1)
	{
		if(q)
		{
			if(*(q-1) == '\r')
				*(q-1) = 0;
			else
				*q = 0;
		}
		if(strncmp(p, li, llen) != 0)
		{
			strcat(tmp, p);
		}
		if(q)
			p = q + 1;
		else
			break;
		strcat(tmp, "\r\n");
		//p = q + 1;
		q = strstr(p, "\n");
	}
}

void elimem(char*buf, char*tmp, int maxlen, int level=0, char*li="//")
{
	int len;
	do
	{
		len = strlen(buf);
		memelilevel0(buf, tmp, maxlen);
		strcpy(buf, tmp);
		if(level > 0)
		{
			memelilevel1(buf, tmp, maxlen);
			strcpy(buf, tmp);
		}
		if(level > 1)
		{
			memelilevel2(buf, tmp, maxlen);
			strcpy(buf, tmp);
		}
		if(level > 2)
		{
			memelilevel3(buf, tmp, maxlen);
			strcpy(buf, tmp);
		}
	}while(len != (int)(strlen(buf)));
}

void eli2(char *in, char*out, int level = 3, char*li="//")
{
	FILE* fp = fopen(in, "rb");
	if(fp == NULL)
		return;
	fseek(fp, 0, SEEK_END);
	int len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* buf = new char[len*2];
	memset(buf, 0, len*2);
	fread(buf, len, 1, fp);
	fclose(fp);

	int llen = strlen(li);

	char *tmp = new char[len*2+llen];
	memset(tmp, 0, len*2+llen);

	elimem(buf, tmp, len*2+llen, level, li);

	fp = fopen(out, "wb");
	if(fp == NULL)
	{
		delete tmp;
		delete buf;
		return;
	}
	fwrite(tmp, 1, strlen(tmp), fp);
	fclose(fp);
	delete tmp;
	delete buf;
}

void DoAttributeFull(char *in, char*out, int level = 3, int itime=6, char*li="//")
{
	char mid[255], mid2[255];
	sprintf(mid, "_w1%s", out);
	sprintf(mid2, "_w2%s", out);
	eli2(in, mid2, level, li);
	DoHTMLAttribute(mid2, mid, itime);
	if(todel==1) DeleteFile(mid2);
	sprintf(mid2, "_w4%s", out);
	eli2(mid, mid2, level, li);
	if(todel==1) DeleteFile(mid);
	sprintf(mid, "_w5%s", out);
	printf("checking for quote\n");
	DoHTMLAttribute2(mid2, mid, itime);
	if(todel==1) DeleteFile(mid2);
	//eli2(mid, out, level, li);
	
	sprintf(mid2, "_w6%s", out);
	eli2(mid, mid2, level, li);
	if(todel==1) DeleteFile(mid);
	sprintf(mid, "_w7%s", out);
	printf("checking for single quote\n");
	DoHTMLAttribute3(mid2, out, itime);
	if(todel==1) DeleteFile(mid2);
	if(todel==1) DeleteFile("tmp.htm");
	if(todel==1) DeleteFile("tmp.log");
}

void DoRedux(char *in, char*out, int itime=6)
{
	char mid[255];
	sprintf(mid, "_w1%s", out);
	DoQuick(in, mid, itime);
	Do(mid, out, itime);
	DeleteFile(mid);
}

void DoFull(char *in, char*out, int level = 3, int itime=6, char*li="//")
{
	char mid[255], mid2[255];
	sprintf(mid, "_w1%s", out);
	sprintf(mid2, "_w2%s", out);
	eli2(in, mid2, level, li);
	DoQuick(mid2, mid, itime);
	if(todel==1) DeleteFile(mid2);
	sprintf(mid2, "_w4%s", out);
	eli2(mid, mid2, level, li);
	if(todel==1) DeleteFile(mid);
	sprintf(mid, "_w5%s", out);
	Do(mid2, mid, itime);
	if(todel==1) DeleteFile(mid2);
	//eli2(mid, out, level, li);
	
	sprintf(mid2, "_w6%s", out);
	eli2(mid, mid2, level, li);
	if(todel==1) DeleteFile(mid);
	sprintf(mid, "_w7%s", out);
	DoHTMLString(mid2, mid, itime);
	if(todel==1) DeleteFile(mid2);
//	sprintf(mid2, "_w8%s", out);
	DoAttributeFull(mid, out, itime);
	if(todel==1) DeleteFile(mid);
	if(todel==1) DeleteFile("tmp.htm");
	if(todel==1) DeleteFile("tmp.log");
}

//#define BUFSIZE 5000000

char*cdbhttpresp = "HTTP/1.1 200 OK\r\n"
"Date: %s\r\n"
"Server: SST Web/3.14159 (like a boss)\r\n"
"Last-Modified: %s\r\n"
"Accept-Ranges: bytes\r\n"
"Content-Length: %d\r\n"
"Vary: Accept-Encoding\r\n"
"Content-Type: text/html\r\n"
"X-Pad: Yes we scan\r\n\r\n";

char*cdbhttp404="HTTP/1.1 404 Not Found\r\n"
"Date: %s\r\n"
"Server: Apache/2.2.14 (Ubuntu)\r\n"
"Vary: Accept-Encoding\r\n"
"Content-Length: %d\r\n"
"Content-Type: text/html; charset=iso-8859-1\r\n\r\n%s";

char*cdbhttp404c=
"<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n"
"<html><head>\r\n"
"<title>404 Not Found</title>\r\n"
"</head><body>\r\n"
"<h1>Not Found</h1>\r\n"
"<p>The requested URL /asdf was not found on this server.</p>\r\n"
"<hr>\r\n"
"<address>aaaaa ooooo eeeee iiiii uuuuu ?????</address>\r\n"
"</body></html>";

DWORD WINAPI CDBCheckSocketThread(PVOID para)
{
	char ok[10000]={0};
	int ret;
	SOCKET s = (SOCKET)para;

	char at[256];
	GetCurrentDirectoryA(256, at);
	char file[2560];
	char file2[2560];
	char togo[1000];

	char t0[80],t1[80];
	time_t rawtime;
	struct tm * timeinfo;
	char *buf=NULL;
	while(1)
	{
		ret = recv(s, ok, 6000, 0);
		if(ret <= 0)
			break; //disconnect or crash?

		time (&rawtime);
		timeinfo = localtime (&rawtime);
		strftime (t0,80,"%a, %d %b %Y %X %Z",timeinfo);
		rawtime -=80;
		timeinfo = localtime (&rawtime);
		strftime (t1,80,"%a, %d %b %Y %X %Z",timeinfo);



		char *p = strstr(ok, "GET /");
		char *q = strstr(ok, " HTTP/1.1");
		if(!p || !q)
		{
			//quit if not http request
			break;
		}
		*q=0;
		//if file name is longer than 256, ooops
		strcpy(file, p+5);
		char *zero=strstr(file, "?");
		if(zero) zero[0]=0;
		zero=strstr(file, "%3f");
		if(zero) zero[0]=0;
		zero=strstr(file, "%3F");
		if(zero) zero[0]=0;

		strcpy(file2, p+5);
		zero=strstr(file2, "?");
		if(zero) zero[0]='_',zero[cutme]=0;
		zero=strstr(file2, "%3f");
		if(zero) zero[0]='_',zero[1]='_',zero[2]='_',zero[cutme]=0;
		zero=strstr(file2, "%3F");
		if(zero) zero[0]='_',zero[1]='_',zero[2]='_',zero[cutme]=0;
//		printf("8080 request: %s\n", p+5);

		sprintf(togo, "%s\\%s", at, file);
		FILE*fp=fopen(togo, "rb");
		if(fp == NULL)
		{
			//404
			sprintf(togo, "%s\\%s", at, file2);
			printf("try %s\n", togo);
			fp=fopen(togo, "rb");
			if(fp == NULL)
			{
				sprintf(ok, cdbhttp404, t0, strlen(cdbhttp404c), cdbhttp404c);
				send(s, ok, strlen(ok), 0);
				break;
			}
		}
		fseek(fp, 0, SEEK_END);
		int flen = ftell(fp);
		buf = new char[flen];
		fseek(fp, 0, SEEK_SET);
		fread(buf, flen, 1, fp);
		fclose(fp);


		sprintf(ok, cdbhttpresp, t0, t1, flen);
		send(s, ok, strlen(ok), 0);
		send(s, buf, flen, 0);
		delete buf;
		buf = NULL;
	}
	if(buf)
		delete buf;
	closesocket(s);
	return 1;
}

DWORD WINAPI ListenThread(PVOID para)
{
	WSADATA   WsaData;
	SOCKET   swait, s;
	SOCKADDR_IN Addr;
	SOCKADDR_IN inAddr;
	if(WSAStartup(MAKEWORD(2,2),&WsaData))
		return false;
	swait = WSASocket(AF_INET, SOCK_STREAM, NULL, NULL, NULL, NULL);
	if(swait == INVALID_SOCKET)
		return false;
	Addr.sin_family = AF_INET;
	Addr.sin_addr.S_un.S_addr = ADDR_ANY;
	Addr.sin_port = htons(80);
	if(bind(swait, (SOCKADDR *)&Addr, sizeof(Addr)))
	{
		closesocket(swait);
		WSACleanup();
		return false;
	}
	if(listen(swait, 40))
	{
		closesocket(swait);
		WSACleanup();
		return false;
	}
	
	while(1)
	{
		int inAddrSize = sizeof(inAddr);
		s = WSAAccept(swait, (SOCKADDR *)&inAddr, &inAddrSize, NULL, NULL);
		if(s != INVALID_SOCKET)
		{
			DWORD id;
			HANDLE h = CreateThread(NULL, 0, CDBCheckSocketThread, (PVOID)(s), 0, &id);
			CloseHandle(h);
		}
		else
		{
			closesocket(swait);
			WSACleanup();
			return false;
		}
	}
}

void ShowHelp(char*_)
{
	printf("===================================================\n");
	printf("     A 0x557 tool designing to minimize the PoC to\n");
	printf(" Save Microsoft And Researcher's Time, a.k.a SMART\n");
	printf("                    Bo Qu (Swan@0x557) v2017.05.21\n");
	printf("===================================================\n");
	printf("\n");
	printf(" ");
	printf("%s <-i file> [-o file] [-e file] [-l level] [-f offset] [-t|r|a|x|z|q] [-h]\n",_);
	printf("\n");
	printf("  -t: shrink file size only\n");
	printf("  -r: minimize file but keep commemts\n");
	printf("  -a: do full minimization\n");
	printf("  -x: legacy mode by commenting each line\n");
	printf("  -z: deal with HTML lables only\n");
	printf("  -q: process strings and attributes only\n");
	printf("  -i: input file\n");
	printf("  -o: output file\n");
	printf("  -e: entry file, this overwrites output file\n");
	printf("  -f: starting offset for string and attribute\n");
	printf("  -h: using http mode to load file\n");
	printf("  -l: 0 = remove empty line\n");
	printf("      1 = 0 + empty block\n");
	printf("      2 = 1 + comments\n");
	printf("      3 = 2 + empty function");
}

void DoAll(int level, int timeout, char*trim)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError=0;
	hFind = FindFirstFile("*.htm*", &ffd);
	int index = 0;
	char out[255] = {0};
	char cmd[1024];
	char dir[256];

	DWORD id;
	HANDLE h = CreateThread(NULL, 0, ListenThread, NULL, 0, &id);
	CloseHandle(h);

	if(hFind == INVALID_HANDLE_VALUE)
		return;
	do
	{
		if(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			//lol
		}
		else
		{
			GetCurrentDirectory(255, dir);
			iSetCrashLevel();	
			sprintf(cmd, "http://127.0.0.1/%s", ffd.cFileName);
			int ret = cdbcheck(cmd, globalcname, "tmp");
			if(ret == 2)
			{
				printf("Crash is %s\n", globalcname);
				sprintf(out, "%0.4d.out", index);
				DoFull(ffd.cFileName, out, level, timeout, trim);
				sprintf(out, "%0.4d.old", index);
				MoveFile(ffd.cFileName, out);
				sprintf(out, "%0.4d.out", index);
				CopyFile(out, ffd.cFileName, false);
				index++;
			}
		}
	}
	while(FindNextFile(hFind, &ffd) != 0);
	FindClose(hFind);
}

int main(int argc, char**argv)
{
	if(argc < 2)
	{
		ShowHelp(argv[0]);
		return 0;
	}

	int hasentry = 0;
	int level = 1;
	char trim[30] = {"//"};
	int timeout = 6;
	char outfile[255]="out.htm";
	char infile[255];
	char cmd[1024];
	char dir[256];
//	initLoader();

	SHDeleteValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AeDebug", "Debugger");
	if(argc == 2)
	{
		ishttp = 1;
		if(argv[1][0] == '*')
			DoAll(level, timeout, trim);
		else
			DoFull(argv[1], "out.htm", level, timeout, trim);
		return 0;
	}
	int mode = 0; //0: trim only, 1: reduce only, 2: reduce and trim
	for(int i=1; i<argc; i++)
	{
		if(strcmp(argv[i], "-t")==0)
		{
			mode = 0;
		}
		else if(strcmp(argv[i], "-r")==0)
		{
			mode = 1;
		}
		else if(strcmp(argv[i], "-a")==0)
		{
			mode = 2;
		}
		else if(strcmp(argv[i], "-x")==0)
		{
			mode = 3;
		}
		else if(strcmp(argv[i], "-z")==0)
		{
			mode = 4;
		}
		else if(strcmp(argv[i], "-q")==0)
		{
			mode = 5;
		}
		else if(strcmp(argv[i], "-h")==0)
		{
			ishttp = 1;
		}
		else if(strcmp(argv[i], "-k")==0)
		{
			todel = 0;
		}
		else if(strcmp(argv[i], "-X")==0)
		{
			mode = 6;
		}
		else if(strcmp(argv[i], "-v")==0)
		{
			verbose = 1;
		}
		else if(strcmp(argv[i], "-o")==0)
		{
			i++;
			if(i < argc)
				strcpy(outfile, argv[i]);
			else
			{
				ShowHelp(argv[0]);
				return 0;
			}
		}
		else if(strcmp(argv[i], "-l")==0)
		{
			i++;
			if(i < argc)
				level = atoi(argv[i]);
			else
			{
				ShowHelp(argv[0]);
				return 0;
			}
		}
		else if(strcmp(argv[i], "-f")==0)
		{
			i++;
			if(i < argc)
				startingfrom = atoi(argv[i]);
			else
			{
				ShowHelp(argv[0]);
				return 0;
			}
		}
		else if(strcmp(argv[i], "-s")==0)
		{
			i++;
			if(i < argc)
			{
				timeout = atoi(argv[i]);
				tosleep = timeout*100;
			}
			else
			{
				ShowHelp(argv[0]);
				return 0;
			}
		}
		else if(strcmp(argv[i], "-m")==0)
		{
			i++;
			if(i < argc)
				strcpy(trim, argv[i]); //backdoor here.
			else
			{
				ShowHelp(argv[0]);
				return 0;
			}
		}
		else if(strcmp(argv[i], "-i")==0)
		{
			i++;
			if(i < argc)
				strcpy(infile, argv[i]);
			else
			{
				ShowHelp(argv[0]);
				return 0;
			}
		}
		else if(strcmp(argv[i], "-e")==0)
		{
			i++;
			if(i < argc)
			{
				hasentry = 1;
				strcpy(toload, argv[i]);
			}
			else
			{
				ShowHelp(argv[0]);
				return 0;
			}
		}
	}

	if(infile == NULL)
	{
		ShowHelp(argv[0]);
		return 0;
	}
	if(hasentry == 1)
	{
		strcpy(orgfile, infile);
		sprintf(infile, "%s.bak", orgfile);
		CopyFile(orgfile, infile, false);
		strcpy(outfile, orgfile);
	}
	else
	{
		strcpy(toload, infile);
	}
	if(ishttp == 1)
	{
		DWORD id;
		HANDLE h = CreateThread(NULL, 0, ListenThread, NULL, 0, &id);
		CloseHandle(h);
	}
	
	//init cname
	
	GetCurrentDirectory(255, dir);

	iSetCrashLevel();	

	if(ishttp == 0)
		sprintf(cmd, "\"%s\\%s\"", dir, toload);
	else
		sprintf(cmd, "http://127.0.0.1/%s", toload);
	int ret = cdbcheck(cmd, globalcname, "tmp");
	if(ret != 2)
	{
		printf("no crash, dude!");
		return 0;
	}
	printf("Crash is %s\n", globalcname);
	//return 1;
	printf("offset has been set to %d\n", startingfrom);

	switch(mode)
	{
		case 0:
			eli2(infile, outfile, level, trim);
			break;
		case 1:
			DoRedux(infile, outfile, timeout);
			break;
		case 2:
			DoFull(infile, outfile, level, timeout, trim);
			break;
		case 3:
			Do(infile, outfile, timeout);
			break;
		case 4:
			DoHTMLString(infile, outfile, timeout);
			break;
		case 5:
			DoAttributeFull(infile, outfile, timeout);
			break;
		case 6:
			Od(infile, outfile, timeout);
			break;
	}
	return 1;
}