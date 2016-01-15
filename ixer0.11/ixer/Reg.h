//////////////////////////////////////////////////////////////////////////
// Reg.h

#ifndef  ___REG___
#define  ___REG___


typedef struct _REGMGR
{
	HKEY         m_hKey;
    HKEY         m_wKey;
	HKEY         m_DelKey;
	HTREEITEM    m_ClickNode;
	HTREEITEM    hRegParentNode;
	CImageList   m_imageList;
    CString      openPath;
	CString      fullPath;
	CString      szValueName;
    CString      newKeyItem;
    CString      szLastValueName;
	CString      szLastItemName;
    CString      szExPath;
	
    BOOL         bLastValueItem;
    
	CTreeCtrl   *m_regTree;
	CListCtrl   *m_listTop;
    CListCtrl   *m_listButtom;
} REGMGR, FAR *LPREGMGR;


HKEY	 GetRegistory_HKey(CString  strPath);
HKEY  OpenNextKey(CString pszFullPath);
HKEY  OpenNextWriteKey(CString pszFullPath);
HKEY  OpenNextDelKey(CString pszFullPath);
HKEY	 Create_New_KeyItem(HKEY  m_hKey,CString szNewKeyItem);
BOOL Set_Key_Value(HKEY m_wKey,CString pszSubKeyName,DWORD dwType,CString pszValue);
BOOL Delete_KEY_Item (HKEY  m_hKey,CString szDelKeyItem);
BOOL Delete_VALUE_Item (HKEY  m_hKey,CString szDelValueItem);
char HexChar(char c);
CString HexToTen(CString hex);
CString DecimalToHex(int iDecimal);
//////////////////////////////////////////////////////////////////////////
#endif