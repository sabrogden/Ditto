#ifndef _TEXTFILE_H_
#define _TEXTFILE_H_

class CTextFile
{

public:

	// ctor( s )
	CTextFile( const CString& ext = _T( "" ), const CString& eol = _T( "\r\n" ) );

	// dtor
	virtual ~CTextFile();

	// File operations
	BOOL	ReadTextFile( CString& filename, CStringArray& contents );
	BOOL	ReadTextFile( CString& filename, CString& contents );

	BOOL	WriteTextFile( CString& filename, const CStringArray& contents );
	BOOL	WriteTextFile( CString& filename, const CString& contents );

	BOOL	AppendFile( CString& filename, const CString& contents );
	BOOL	AppendFile( CString& filename, const CStringArray& contents );

	// Window operations
	BOOL	Load( CString& filename, CEdit* edit );
	BOOL	Load( CString& filename, CListBox* list );
	BOOL	Save( CString& filename, CEdit* edit );
	BOOL	Save( CString& filename, CListBox* list );

	// Error handling
	CString GetErrorMessage();

protected:

	virtual BOOL GetFilename( BOOL save, CString& filename );
	CString GetExtension();

private:

	CString m_error;
	CString m_extension;
	CString m_eol;

	void	ClearError();
	BOOL	ValidParam( CWnd* wnd );

};

#endif // _TEXTFILE_H_
