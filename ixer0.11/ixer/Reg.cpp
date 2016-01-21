//////////////////////////////////////////////////////////////////////////
// Reg.cpp

#include "StdAfx.h"
#include "Reg.h"


#define  hKey_Classes_Root     HKEY_CLASSES_ROOT;     //RootIndex=1
#define  hKey_Current_Config   HKEY_CURRENT_CONFIG;   //RootIndex=2
#define  hKey_Current_User     HKEY_CURRENT_USER;     //RootIndex=3
#define  hKey_Local_Machine    HKEY_LOCAL_MACHINE;    //RootIndex=4
#define  hKey_Users            HKEY_USERS;            //RootIndex=5


HKEY  GetRegistory_HKey(CString  strPath)
{
	CString   strleft;
    int       h = 0;

	if ( strPath == "" )
	{
		return NULL;
	}

	if ( strstr(strPath,"HKEY") )
	{
       
		h = strPath.Find("\\"); 
		if ( h > 0 )
		  strleft = strPath.Left( h ); 

        strleft =strPath;
		strleft.MakeUpper();
		
		if ( strstr(strleft,"HKEY_CLASSES_ROOT") )
			return HKEY_CLASSES_ROOT;
		
		else if ( strstr(strleft,"HKEY_CURRENT_USER") )
			return HKEY_CURRENT_USER;
		
		else if ( strstr(strleft,"HKEY_LOCAL_MACHINE") )
			return HKEY_LOCAL_MACHINE;
		
		else if ( strstr(strleft,"HKEY_USERS") )
			return HKEY_USERS;

		else if ( strstr(strleft,"HKEY_CURRENT_CONFIG") )
			return HKEY_CURRENT_CONFIG;
	}
    else
	{
		return NULL;
	}
    return NULL;

}
HKEY  OpenNextKey(CString pszFullPath)
{

    HKEY      rKey,mKey;
	CString   szpath = pszFullPath;
	CString   strleft,strright;    
   
    if ( szpath == "" )
		return NULL;
	if ( szpath.Right(1) == "\\" )
       szpath = szpath.Left( szpath.GetLength()-1 ); 

	int   u  =  szpath.Find("\\"); 

	if ( u > 0 )
	{
	   strleft  =  szpath.Left( u ); 
	   strright =  szpath.Right( szpath.GetLength() - ( u+1 ) );
	}
	else
       strleft = szpath;

	strleft.MakeUpper();
	
    if ( strleft == "HKEY_CLASSES_ROOT" )
		rKey = HKEY_CLASSES_ROOT;
	else if ( strleft == "HKEY_CURRENT_USER" )
	    rKey = HKEY_CURRENT_USER;
	else if ( strleft == "HKEY_LOCAL_MACHINE" )
		rKey = HKEY_LOCAL_MACHINE;
	else if ( strleft == "HKEY_CURRENT_CONFIG" )
		rKey = HKEY_CURRENT_CONFIG;
	else if ( strleft == "HKEY_USERS" )
		rKey = HKEY_USERS;

	if ( ::RegOpenKey(rKey,strright,&mKey) == ERROR_SUCCESS )
       return mKey;
    else
       return NULL;

}
HKEY  OpenNextWriteKey(CString pszFullPath)
{

    HKEY      rKey,mKey;
	CString   szpath = pszFullPath;
	CString   strleft,strright;    
   
    if ( szpath == "" )
		return NULL;
    szpath = szpath.Left( szpath.GetLength()-1 ); 
	int   u  =  szpath.Find("\\"); 

	if ( u > 0 )
	{
	   strleft  =  szpath.Left( u ); 
	   strright =  szpath.Right( szpath.GetLength() - ( u+1 ) );
	}
	else
       strleft = szpath;

	strleft.MakeUpper();
	
    if ( strleft == "HKEY_CLASSES_ROOT" )
		rKey = HKEY_CLASSES_ROOT;
	else if ( strleft == "HKEY_CURRENT_USER" )
	    rKey = HKEY_CURRENT_USER;
	else if ( strleft == "HKEY_LOCAL_MACHINE" )
		rKey = HKEY_LOCAL_MACHINE;
	else if ( strleft == "HKEY_CURRENT_CONFIG" )
		rKey = HKEY_CURRENT_CONFIG;
	else if ( strleft == "HKEY_USERS" )
		rKey = HKEY_USERS;

	if ( ::RegOpenKeyEx( rKey,strright,0,KEY_WRITE,&mKey ) == ERROR_SUCCESS )
       return mKey;
    else
       return NULL;

}
HKEY  OpenNextDelKey(CString pszFullPath)
{

    HKEY      rKey,mKey;
	CString   szpath = pszFullPath;
	CString   strleft,strright;    
   
    if ( szpath == "" )
		return NULL;
    szpath = szpath.Left( szpath.GetLength()-1 );
	int r  = szpath.ReverseFind('\\');   
	szpath = szpath.Left( r ); 

	int   u  =  szpath.Find("\\"); 

	if ( u > 0 )
	{
	   strleft  =  szpath.Left( u ); 
	   strright =  szpath.Right( szpath.GetLength() - ( u+1 ) );
	}
	else
       strleft = szpath;

	strleft.MakeUpper();
	
    if ( strleft == "HKEY_CLASSES_ROOT" )
		rKey = HKEY_CLASSES_ROOT;
	else if ( strleft == "HKEY_CURRENT_USER" )
	    rKey = HKEY_CURRENT_USER;
	else if ( strleft == "HKEY_LOCAL_MACHINE" )
		rKey = HKEY_LOCAL_MACHINE;
	else if ( strleft == "HKEY_CURRENT_CONFIG" )
		rKey = HKEY_CURRENT_CONFIG;
	else if ( strleft == "HKEY_USERS" )
		rKey = HKEY_USERS;

	if ( ::RegOpenKeyEx(rKey,strright,0,KEY_WRITE,&mKey) == ERROR_SUCCESS )
       return mKey;
    else
       return NULL;

}

HKEY Create_New_KeyItem(HKEY  m_hKey,CString szNewKeyItem)
{

    if ( szNewKeyItem.IsEmpty() )
        return ( FALSE );
    if ( m_hKey == NULL )
        return ( FALSE );
    
	HKEY mkey;
	int v = ::RegCreateKey( m_hKey,szNewKeyItem,&mkey ); 
    
	if ( mkey != NULL )
		return mkey;
	else
		return NULL;
	 
	
}
BOOL Set_Key_Value(HKEY m_wKey,CString pszSubKeyName,DWORD dwType,CString pszValue)
{
    
    if ( (pszSubKeyName == "") || (pszValue == "") )
        return ( FALSE );
    if ( m_wKey == NULL )
        return ( FALSE );

	DWORD   dwLen = 0;
    int     v = 0;
	dwLen = pszValue.GetLength();
    LPBYTE  nbyte = new BYTE[ dwLen+1 ];
    
	for ( int n =0; n<(int)dwLen; n++ )
		nbyte[ n ] =pszValue.GetAt( n ); 
    nbyte[ dwLen ] = 0;

	switch ( dwType )
	{
	case 1:   //REG_SZ

	   v = ::RegSetValue( m_wKey,pszSubKeyName,dwType,pszValue,dwLen );
	   if ( v == ERROR_SUCCESS )
	 	  return ( TRUE );
	   else
          return ( FALSE );
	   break;

	case 2:   //REG_EXPAND_SZ

       v = ::RegSetValue( m_wKey,pszSubKeyName,dwType,pszValue,dwLen );
	   if ( v == ERROR_SUCCESS )
	 	  return ( TRUE );
	   else
          return ( FALSE );

	   break;	
    case 3:   //REG_BINARY
		break;
	case 4:   //REG_DWORD

		break;
	case 5:   //REG_DWORD_BIG_ENDIAN
		break;
	case 7:   //REG_MULTI_SZ
		break;
	default:
		break;
	}

	return TRUE;
}
BOOL Delete_KEY_Item (HKEY  m_hKey,CString szDelKeyItem)
{
	if ( szDelKeyItem == "" )
        return ( FALSE );
    if ( m_hKey == NULL )
        return ( FALSE );

	int v = RegDeleteKey( m_hKey,szDelKeyItem );
	if ( v == ERROR_SUCCESS )
		return ( TRUE );
	else
	    return ( FALSE );

}

BOOL Delete_VALUE_Item (HKEY  m_hKey,CString szDelValueItem)
{
	if ( szDelValueItem == "" )
        return ( FALSE );
    if ( m_hKey == NULL )
        return ( FALSE );

	int v = ::RegDeleteValue( m_hKey,szDelValueItem );
	if ( v == ERROR_SUCCESS )
		return ( TRUE );
	else
	    return ( FALSE );

}

char HexChar( char c )
{

	if( ( c >= '0' )&&( c <= '9' ) )
		return c - 0x30;
	else if( ( c >= 'A' )&&( c <= 'F' ) )
		return c - 'A' + 10;
	else if( ( c >= 'a' )&&( c <= 'f' ) )
		return c - 'a' + 10;
	else 
		return 0x10;
}

CString HexToTen(CString hex)
{
	CString  v,endv;
	CString  hex1;
	CString  pu;
	
	double   fv     = 0;
	int      h      = 0;
    int      len    = 0;
	int      len1   = 0;
    double   endfv  = 1; 
    double   endfv1 = 0; 
    char     tmm;
	char     mm[500];
	
    hex.MakeLower();
	
	int hi = 0;
	for (hi=0; hi<hex.GetLength(); hi++ )
	{

		tmm = hex[ hi ]; 
		
        if ( (tmm >= '0') && (tmm <='9') )
		   mm[hi] = (tmm - 0x30);

        if ( (tmm >='A') && (tmm <='F') )
		   mm[hi] = (tmm -'A' + 0xA);
		
        if ( (tmm>='a') && (tmm<='f') )
		   mm[hi] = (tmm -'a' + 0xA);
		
	}

	len = hi;
    
	for ( int b=0; b<len; b++ )
    {
		 endfv = 1;
		 len1 = len - b;
		 char  vl[2];
		 vl[0] = mm[b];
         int p = vl[0];
         double bk = 0;

	     for (int u = 1; u < len1; u++)
		 {
			//endfv = u * 16;
			endfv = endfv*16;
		 }

		 bk =  p * endfv;
		 endfv1+= bk;
	}
    
	if ( hex == "ffffffff" )
       endv.Format("%10.0f",endfv1);
	else
       endv.Format("%.0f",endfv1);
	return  endv; 

}

CString DecimalToHex(int iDecimal)
{
	char c[20];
	sprintf(c, "%x", iDecimal);	// _ltoa( l, buffer, 16 );
	CString  str(c);
	return str;
}