// ApiJtag.cpp : 定義 DLL 應用程式的匯出函式。
//

#include "stdafx.h"
#include "ApiJtag.h"


// 這是匯出變數的範例
APIJTAG_API int nApiJtag=0;

// 這是匯出函式的範例。
APIJTAG_API int fnApiJtag(void)
{
	return 42;
}

// 這是已匯出的類別建構函式。
// 請參閱 ApiJtag.h 中的類別定義
CApiJtag::CApiJtag()
{
	return;
}
