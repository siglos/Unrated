/*

  If you ever get cold, just stand in a corner for a bit,
  They're usually around 90 degrees.

                                         -- <<????????>>
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

char ptt1[] = "\"c:\\Program Files\\Internet Explorer\\iexplore.exe\"";
char ptt2[] = "\"c:\\Program Files\\Internet Explorer (x86)\\iexplore.exe\"";
char* ptt = ptt1;

char progress[4]={'|','\\','-','/'};

void initLoader()
{
	FILE*fp=fopen(ptt2, "rb");
	if(fp)
	{
		ptt = ptt2;
		fclose(fp);
	}
	ptt = ptt1;
}

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

int ieerror = 0;

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
			||FindProcessIDByName("drwtsn32.exe")
			||0==FindProcessIDByName("iexplore.exe"))
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
			||FindProcessIDByName("drwtsn32.exe"))
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

}

void GoClean(int p=0)
{
	char user[50];
	
	char tmp1[1000];
	char tmp2[1000];
	char tmp3[1000];
	char tmp4[1000];
	char tmp5[1000];
	char tmp6[1000];
	char iecache[1000];

	DWORD temp=50;
	GetUserNameA(user, &temp);

	sprintf(tmp1, "C:\\Documents and Settings\\%s\\Local Settings\\Temp\\", user);
	sprintf(tmp2, "c:\\Users\\%s\\Local Settings\\Temp\\", user);
	sprintf(tmp3, "c:\\Documents and Settings\\%s\\Local Settings\\Application Data\\Microsoft\\Internet Explorer\\Recovery\\Active\\", user);
	sprintf(tmp4, "c:\\Users\\%s\\AppData\\Local\\Microsoft\\Internet Explorer\\Recovery\\Active\\", user);
	sprintf(tmp5, "c:\\Users\\%s\\AppData\\Local\\Microsoft\\Internet Explorer\\Recovery\\High\\Active\\", user);
	TryClean(tmp1,p);
	TryClean(tmp2,p);
	TryClean(tmp3,p);
	TryClean(tmp4,p);
	TryClean(tmp5,p);

	//xp
	sprintf(iecache, "C:\\Documents and Settings\\%s\\Local Settings\\Temporary Internet Files\\Content.IE5\\*.*", user);
	WIN32_FIND_DATA ffd;
	
	HANDLE hFind = FindFirstFile(iecache, &ffd);
	do{
		if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && ffd.cFileName[0]!='.')
		{
			sprintf(tmp6, "C:\\Documents and Settings\\%s\\Local Settings\\Temporary Internet Files\\Content.IE5\\%s\\", user, ffd.cFileName);
			TryClean(tmp6,p);
			Sleep(1);
		}
	} while (FindNextFile(hFind, &ffd) != 0);
	
	//vista 7
	sprintf(iecache, "C:\\Users\\%s\\AppData\\Local\\Microsoft\\Windows\\Temporary Internet Files\\Content.IE5\\*.*", user);
	hFind = FindFirstFile(iecache, &ffd);
	do{
		if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && ffd.cFileName[0]!='.')
		{
			sprintf(tmp6, "C:\\Documents and Settings\\%s\\Local Settings\\Temporary Internet Files\\Content.IE5\\%s\\", user, ffd.cFileName);
			TryClean(tmp6,p);
			Sleep(1);
		}
	} while (FindNextFile(hFind, &ffd) != 0);
}

int counter=0;

int RunAndTest(char *content, int itime)
{
	GoClean();
	SHDeleteKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Internet Explorer\\Recovery\\Active");
	initLoader();
	char dir[256];
	GetCurrentDirectory(255, dir);
//	sprintf(dir, "%s\\tmp%d.htm", dir, counter++);
	strcat(dir, "\\tmp.htm");
	DeleteFile(dir);
	FILE*fp = fopen(dir, "wb");
	fwrite(content,strlen(content),1,fp);
	fclose(fp);

	char cmd[1024];
	if(ishttp == 0)
		sprintf(cmd, "%s \"%s\"", ptt, dir);
	else
		sprintf(cmd, "%s http://127.0.0.1/tmp.htm", ptt, dir);
	WinExec(cmd, SW_SHOW);

	for(int i=0;i<itime;i++)
	{
		Sleep(1000);
		if(FoundCrash()==1)
		{
			KillAllByName("WerFault.exe");
			KillAllByName("dwwin.exe");
			KillAllByName("iedw.exe");
			KillAllByName("iexplore.exe");
			KillAllByName("dw20.exe");
			KillAllByName("drwtsn32.exe");
			Sleep(500);//so flush
			return 1;
		}
	}
	KillAllByName("WerFault.exe");
	KillAllByName("dwwin.exe");
	KillAllByName("iedw.exe");
	KillAllByName("iexplore.exe");
	KillAllByName("dw20.exe");
	KillAllByName("drwtsn32.exe");
	KillAllByName("iexplore.exe");

	Sleep(500);
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
		return;
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

	for(int i=0;i<len;i++)
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

	for(int i=0;i<len;i++)
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
		return;
	fwrite(tmp, 1, strlen(tmp), fp);
	fclose(fp);
	printf("\rdone! %d lines were processed.", i);
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

void memelilevel2(char*buf, char*tmp, int maxlen)
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

void memelilevel3(char*buf, char*tmp, int maxlen, char*li="//")
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
		return;
	fwrite(tmp, 1, strlen(tmp), fp);
	fclose(fp);
}

void DoFull(char *in, char*out, int level = 3, int itime=6, char*li="//")
{
	char mid[255], mid2[255];
	sprintf(mid, "_w1%s", out);
	sprintf(mid2, "_w2%s", out);
	eli2(in, mid2, level, li);
	DoQuick(mid2, mid, itime);
	DeleteFile(mid2);
	sprintf(mid2, "_w4%s", out);
	eli2(mid, mid2, level, li);
	DeleteFile(mid);
	sprintf(mid, "_w5%s", out);
	Do(mid2, mid, itime);
	DeleteFile(mid2);
	eli2(mid, out, level, li);
	//DeleteFile(mid2);
}

void DoRedux(char *in, char*out, int itime=6)
{
	char mid[255];
	sprintf(mid, "_w1%s", out);
	DoQuick(in, mid, itime);
	Do(mid, out, itime);
	DeleteFile(mid);
}

#define BUFSIZE 1000000

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
	char file[256];
	char togo[1000];

	char t0[80],t1[80];
	time_t rawtime;
	struct tm * timeinfo;
	char *buf=NULL;
	while(1)
	{
		ret = recv(s, ok, 2000, 0);
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
//		printf("8080 request: %s\n", p+5);

		sprintf(togo, "%s\\%s", at, file);
		FILE*fp=fopen(togo, "rb");
		if(fp == NULL)
		{
			//404
			sprintf(ok, cdbhttp404, t0, strlen(cdbhttp404c), cdbhttp404c);
			send(s, ok, strlen(ok), 0);
			break;
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
	printf("                    Bo Qu (Swan@0x557) v2014.05.21\n");
	printf("===================================================\n");
	printf("\n");
	printf("\n");
	printf(" ");
	printf("%s <-i input> [-o output] [-l level] [-s timeout] [-t|-r|-a|-x|-z] [-h]\n",_);
	printf("\n");
	printf("  -t: shrink file size only\n");
	printf("  -r: minimize file but keep commemts\n");
	printf("  -a: do full minimization\n");
	printf("  -x: legacy mode\n");
	printf("  -z: deal with HTML string only\n");
	printf("  -i: input file\n");
	printf("  -o: output file\n");
	printf("  -s: timeout for crash\n");
	printf("  -h: using http other than local file\n");
	printf("  -l: 0 = remove empty line; \n");
	printf("      1 = 0 + empty block;\n");
	printf("      2 = 1 + empty function; \n");
	printf("      3 = 2 + comments\n");
}

int main(int argc, char**argv)
{
	if(argc < 2)
	{
		ShowHelp(argv[0]);
		return 0;
	}

	int level = 3;
	char trim[30] = {"//"};
	int timeout = 6;
	char*outfile="out.htm";
	char*infile=NULL;

	SHDeleteValueA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AeDebug", "Debugger");
	if(argc == 2)
	{
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
		else if(strcmp(argv[i], "-h")==0)
		{
			ishttp = 1;
		}
		else if(strcmp(argv[i], "-o")==0)
		{
			i++;
			if(i < argc)
				outfile = argv[i];
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
		else if(strcmp(argv[i], "-s")==0)
		{
			i++;
			if(i < argc)
				timeout = atoi(argv[i]);
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
				infile = argv[i];
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

	if(ishttp == 1)
	{
		DWORD id;
		HANDLE h = CreateThread(NULL, 0, ListenThread, NULL, 0, &id);
		CloseHandle(h);
	}

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
			eli(infile, outfile, trim);
		case 4:
			DoHTMLString(infile, outfile, timeout);
			break;
	}
	return 1;
}