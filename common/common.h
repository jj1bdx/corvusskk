﻿
#ifndef COMMON_H
#define COMMON_H

#include "version.h"

#define MAX_KRNLOBJNAME		256
#define PIPEBUFSIZE			0x2000	//named pipe, 16KB with wchar_t
#define READBUFSIZE			0x800	//read skk dictionary, 4KB with wchar_t/char
#define RECVBUFSIZE			0x800	//receive from skk server, 2KB with char

#define MAX_SKKSERVER_HOST	(255 + 1)	//SKKサーバー ホスト 読み込みバッファ
#define MAX_SKKSERVER_PORT	(5 + 1)		//SKKサーバー ポート 読み込みバッファ
#define COMPMULTIDISP_NUM	3		//複数補完/複数動的補完の表示数
#define MAX_SELKEY_C		9		//候補一覧選択キー最大数
#define MAX_SELKEY			7		//候補一覧選択キー数
#define MAX_WIDTH_DEFAULT	800		//候補一覧の最大幅
#define PRESERVEDKEY_NUM	2		//キー設定ON/OFF
#define MAX_PRESERVEDKEY	8		//キー設定ON/OFF最大数
#define MAX_KEYRE			256		//キー１/２ 読み込みバッファ
#define MAX_CONV_POINT		256		//変換位置指定最大数

//request
#define REQ_SEARCH			L'1'	//辞書検索
#define REQ_COMPLEMENT		L'4'	//補完
#define REQ_CONVERTKEY		L'5'	//見出し語変換
#define REQ_CONVERTCND		L'6'	//候補変換
#define REQ_USER_ADD_0		L'A'	//ユーザー辞書追加(送りあり、補完なし)
#define REQ_USER_ADD_1		L'B'	//ユーザー辞書追加(送りなし、補完あり)
#define REQ_USER_DEL_0		L'C'	//ユーザー辞書削除(送りあり、補完なし)
#define REQ_USER_DEL_1		L'D'	//ユーザー辞書削除(送りなし、補完あり)
#define REQ_USER_SAVE		L'S'	//ユーザー辞書保存
#define REQ_EXEC_CNF		L'P'	//設定ダイアログ起動
#define REQ_CAPS_LOCK		L'I'	//Caps Lock
#define REQ_KANA_LOCK		L'J'	//Kana Lock
//reply
#define REP_OK				L'T'	//hit
#define REP_FALSE			L'F'	//nothing

#define IMCRVMGREXE			L"imcrvmgr.exe"
#define IMCRVCNFEXE			L"imcrvcnf.exe"
#ifndef _DEBUG
#define IMCRVKRNLOBJ		L"corvusskk-"
#else
#define IMCRVKRNLOBJ		L"corvusskk-debug-"
#endif
#define IMCRVMGRMUTEX		IMCRVKRNLOBJ L"mgr-"
#define IMCRVCNFMUTEX		IMCRVKRNLOBJ L"cnf-"
#define IMCRVMGRPIPE		L"\\\\.\\pipe\\" IMCRVKRNLOBJ

#define BOM L'\uFEFF'

extern LPCWSTR RccsUTF16;
extern LPCWSTR WccsUTF16;
extern LPCWSTR RccsUTF8;
extern LPCWSTR WccsUTF8;
extern LPCWSTR RB;
extern LPCWSTR WB;

extern LPCWSTR fnconfigxml;	//設定
extern LPCWSTR fnuserdic;	//ユーザー辞書
extern LPCWSTR fnuserbak;	//ユーザー辞書バックアッププレフィックス
extern LPCWSTR fnskkdic;	//取込SKK辞書
extern LPCWSTR fninitlua;	//init.lua

#define DISPLAY_LIST_COLOR_NUM	8
#define DISPLAY_MODE_COLOR_NUM	8
#define DISPLAYATTRIBUTE_INFO_NUM	7

extern const TF_DISPLAYATTRIBUTE c_daDisplayAttributeInputMark;
extern const TF_DISPLAYATTRIBUTE c_daDisplayAttributeInputText;
extern const TF_DISPLAYATTRIBUTE c_daDisplayAttributeInputOkuri;
extern const TF_DISPLAYATTRIBUTE c_daDisplayAttributeConvMark;
extern const TF_DISPLAYATTRIBUTE c_daDisplayAttributeConvText;
extern const TF_DISPLAYATTRIBUTE c_daDisplayAttributeConvOkuri;
extern const TF_DISPLAYATTRIBUTE c_daDisplayAttributeConvAnnot;
extern const BOOL c_daDisplayAttributeSeries[DISPLAYATTRIBUTE_INFO_NUM];

BOOL IsWindowsVersionOrLater(DWORD dwMajorVersion, DWORD dwMinorVersion, DWORD dwBuildNumber);
#define IsWindowsVersion62OrLater() IsWindowsVersionOrLater(6, 2, 0) // Windows 8
#define IsWindowsVersion63OrLater() IsWindowsVersionOrLater(6, 3, 0) // Windows 8.1
#define IsWindowsVersion100OrLater() IsWindowsVersionOrLater(10, 0, 0) // Windows 10
#define IsWindowsVersion100TH1OrLater() IsWindowsVersionOrLater(10, 0, 10240) // Windows 10 ver.1507
#define IsWindowsVersion100TH2OrLater() IsWindowsVersionOrLater(10, 0, 10586) // Windows 10 ver.1511
#define IsWindowsVersion100RS1OrLater() IsWindowsVersionOrLater(10, 0, 14393) // Windows 10 ver.1607
#define IsWindowsVersion100RS2OrLater() IsWindowsVersionOrLater(10, 0, 15063) // Windows 10 ver.1703
#define IsWindowsVersion100RS3OrLater() IsWindowsVersionOrLater(10, 0, 16299) // Windows 10 ver.1709

BOOL GetUserUUID(LPWSTR *ppszUUID);
BOOL GetUserSid(LPWSTR *ppszUserSid);

BOOL StartProcess(HMODULE hCurrentModule, LPCWSTR lpFileName);

#define FORWARD_ITERATION_I(iterator, container) \
	for(auto (iterator) = (container).begin(); (iterator) != (container).end(); ++(iterator))
#define FORWARD_ITERATION(iterator, container) \
	for(auto (iterator) = (container).begin(); (iterator) != (container).end(); )
#define REVERSE_ITERATION_I(reverse_iterator, container) \
	for(auto (reverse_iterator) = (container).rbegin(); (reverse_iterator) != (container).rend(); ++(reverse_iterator))
#define REVERSE_ITERATION(reverse_iterator, container) \
	for(auto (reverse_iterator) = (container).rbegin(); (reverse_iterator) != (container).rend(); )

template<class T> void SafeRelease(T **ppT)
{
	if(*ppT != nullptr)
	{
		(*ppT)->Release();
		*ppT = nullptr;
	}
}

#endif //COMMON_H
