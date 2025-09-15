/* ==========================================================================
	CTextFile

	Author :		Johan Rosengren, Abstrakt Mekanik AB

	Date :			2004-03-22

	Purpose :		The class is a helper-package for text files and 
					windows. It allows loading and saving text files in a 
					single operation, as well as getting text to and 
					from edit- and listboxes. If an empty filename is given 
					as a parameter to a call, the standard file dialog will 
					be displayed, to let the user select a file.
					Error handling is managed internally, and the different 
					API-functions return a BOOL to signal success or 
					failure. In case of failure, FALSE returned, the member 
					function GetErrorMessage can be called to retrieve a 
					CString with the error message.
					If this string is empty, the file selection was aborted 
					in the case of an empty input name.
   ========================================================================
					14/4 2005	Added Dave Pritchards class CStdioFileEx
								for MBCS/UNICODE-support.
   ========================================================================*/

#include "stdafx.h"
#include "TextFile.h"
#include "../External/StdioFileEx.h"

////////////////////////////////////////
// CTextFile construction/destruction

CTextFile::CTextFile( const CString& ext, const CString& eol )
/* ============================================================
	Function :		CTextFile::CTextFile
	Description :	constructor
					
	Return :		void
	Parameters :	const CString& ext	-	Standard extension 
											to use in case no 
											file name is given.
					const CString& eol	-	The end-of-line to 
											use. Defaults to 
											"\r\n".

   ============================================================*/
{

	m_extension = ext;
	m_eol = eol;

}

CTextFile::~CTextFile()
/* ============================================================
	Function :		CTextFile::~CTextFile
	Description :	destructor
					
	Return :		void
	Parameters :	none

   ============================================================*/
{
}

////////////////////////////////////////
// CTextFile operations
//

BOOL CTextFile::ReadTextFile( CString& filename, CStringArray& contents )
/* ============================================================
	Function :		CTextFile::ReadTextFile
	Description :	Will read the contents of the file filename 
					into the CStringArray contents, one line 
					from the file at a time.
					If filename is empty, the standard file 
					dialog will be displayed, and - if OK is 
					selected - filename will contain the 
					selected filename on return.

	Return :		BOOL					-	TRUE if OK. 
												GetErrorMessage 
												will contain errors.
	Parameters :	CString& filename		-	file to read from
					CStringArray& contents	-	will be filled 
												with the contents 
												of the file

   ============================================================*/
{

	ClearError();
	BOOL result = TRUE;

	if( filename.IsEmpty() )
		result = GetFilename( FALSE, filename );

	if( result )
	{
		CStdioFileEx file;
		CFileException feError;

		if( file.Open( filename, CFile::modeRead , &feError ) )
		{

			contents.RemoveAll();

			CString line;
			while( file.ReadString( line ) )
				contents.Add( line );

			file.Close();

		}
		else
		{

			TCHAR	errBuff[256];
			feError.GetErrorMessage( errBuff, 256 );
			m_error = errBuff;
			result = FALSE;

		}
	}

	return result;

}

BOOL CTextFile::ReadTextFile( CString& filename, CString& contents )
/* ============================================================
	Function :		CTextFile::ReadTextFile
	Description :	Will read the contents of the file filename 
					into contents.
					If filename is empty, the standard file 
					dialog will be displayed, and - if OK is 
					selected - filename will contain the 
					selected filename on return.
					
	Return :		BOOL				-	TRUE if OK. 
											GetErrorMessage will 
											contain errors.
	Parameters :	CString& filename	-	file to read from
					CString& contents	-	will be filled with 
											the contents of the 
											file

   ============================================================*/
{

	contents = _T( "" );

	// Error handling
	ClearError();

	CStdioFileEx file;
	CFileException feError;
	BOOL result = TRUE;

	if( filename.IsEmpty() )
		result = GetFilename( FALSE, filename );

	if( result )
	{

		// Reading the file
		if( file.Open( filename, CFile::modeRead | CFile::typeText, &feError ) )
		{

			CString line;
			while( file.ReadString( line ) )
				contents += line + m_eol;

			file.Close();

		}
		else
		{

			// Setting error message
			TCHAR	errBuff[256];
			feError.GetErrorMessage( errBuff, 256 );
			m_error = errBuff;
			result = FALSE;

		}
	}

	return result;

}

BOOL CTextFile::WriteTextFile( CString& filename, const CStringArray& contents )
/* ============================================================
	Function :		CTextFile::WriteTextFile
	Description :	Writes contents to filename. Will create 
					the file if it doesn't already exist, 
					overwrite it otherwise.
					If filename is empty, the standard file 
					dialog will be displayed, and - if OK is 
					selected - filename will contain the 
					selected filename on return.
					
	Return :		BOOL							-	TRUE if OK. 
														GetErrorMessage 
														will return 
														errors
	Parameters :	CString& filename				-	file to 
														write to
					conste CStringArray& contents	-	contents 
														to write

   ============================================================*/
{

	// Error handling
	ClearError();

	CStdioFileEx file;
	CFileException feError;
	BOOL result = TRUE;

	if(filename.IsEmpty())
		result = GetFilename(TRUE, filename);

	if(result)
	{
		// Write file
		if(file.Open( filename, CFile::modeWrite | CFile::modeCreate , &feError))
		{
			INT_PTR max = contents.GetSize();
			for(int t = 0 ; t < max ; t++)
			{
				file.WriteString(contents[t] + m_eol);
			}

			file.Close();
		}
		else
		{
			// Set error message
			TCHAR	errBuff[256];
			feError.GetErrorMessage(errBuff, 256);
			m_error = errBuff;
			result = FALSE;
		}
	}

	return result;

}

BOOL CTextFile::WriteTextFile( CString& filename, const CString& contents )
/* ============================================================
	Function :		CTextFile::WriteTextFile
	Description :	Writes contents to filename. Will create 
					the file if it doesn't already exist, 
					overwrite it otherwise.
					If filename is empty, the standard file 
					dialog will be displayed, and - if OK is 
					selected - filename will contain the 
					selected filename on return.
					
	Return :		BOOL					-	TRUE if OK. 
												GetErrorMessage 
												will return 
												errors
	Parameters :	CString& filename		-	file to write to
					const CString& contents	-	contents to write

   ============================================================*/
{

	// Error checking
	ClearError();

	CStdioFileEx file;
	CFileException feError;
	BOOL result = TRUE;

	if( filename.IsEmpty() )
		result = GetFilename( TRUE, filename );

	if( result )
	{
		// Write the file
		if( file.Open( filename, CFile::modeWrite | CFile::modeCreate , &feError ) ) 
		{

			file.WriteString( contents );
			file.Close();

		}
		else
		{

			// Set error message
			TCHAR	errBuff[256];
			feError.GetErrorMessage( errBuff, 256 );
			m_error = errBuff;
			result = FALSE;

		}
	}

	return result;

}

BOOL CTextFile::AppendFile( CString& filename, const CString& contents )
/* ============================================================
	Function :		CTextFile::AppendFile
	Description :	Appends contents to filename. Will create 
					the file if it doesn't already exist.
					If filename is empty, the standard file 
					dialog will be displayed, and - if OK is 
					selected - filename will contain the 
					selected filename on return.
					AppendFile will not add eols.
					
	Return :		BOOL					-	TRUE if OK. 
												GetErrorMessage 
												will return errors
	Parameters :	CString& filename		-	file to write to
					const CString& contents	-	contents to write

   ============================================================*/
{

	CFile file;
	CFileException feError;
	BOOL result = TRUE;

	if( filename.IsEmpty() )
		result = GetFilename( TRUE, filename );

	if( result )
	{
		// Write the file
		if( file.Open( filename, CFile::modeWrite | CFile::modeCreate | CFile::modeNoTruncate, &feError ) ) 
		{

			file.SeekToEnd();
			file.Write( contents, contents.GetLength() );
			file.Close();

		}
		else
		{

			// Set error message
			TCHAR	errBuff[256];
			feError.GetErrorMessage( errBuff, 256 );
			m_error = errBuff;
			result = FALSE;

		}
	}

	return result;

}

BOOL CTextFile::AppendFile( CString& filename, const CStringArray& contents )
/* ============================================================
	Function :		CTextFile::AppendFile
	Description :	Appends contents to filename. Will create 
					the file if it doesn't already exist.
					If filename is empty, the standard file 
					dialog will be displayed, and - if OK is 
					selected - filename will contain the 
					selected filename on return.
					
	Return :		BOOL					-	TRUE if OK. 
												GetErrorMessage 
												will return 
												errors
	Parameters :	CString& filename		-	file to write to
					CStringArray contents	-	contents to write

   ============================================================*/
{

	CStdioFileEx file;
	CFileException feError;
	BOOL result = TRUE;

	if(filename.IsEmpty())
		result = GetFilename(TRUE, filename);

	if(result)
	{
		// Write the file
		if(file.Open(filename, CFile::modeWrite | CFile::modeCreate | CFile::modeNoTruncate, &feError)) 
		{

			file.SeekToEnd();

			INT_PTR max = contents.GetSize();
			for(int t = 0 ; t < max ; t++)
				file.WriteString(contents[t] + m_eol);

			file.Close();
		}
		else
		{
			// Set error message
			TCHAR errBuff[256];
			feError.GetErrorMessage(errBuff, 256);
			m_error = errBuff;
			result = FALSE;

		}
	}

	return result;

}

////////////////////////////////////////
// Window operations
//

BOOL CTextFile::Load( CString& filename, CEdit* edit )
/* ============================================================
	Function :		CTextFile::Load
	Description :	Loads a text file from filename to the 
					CEdit edit.
					If filename is empty, the standard file 
					dialog will be displayed, and - if OK is 
					selected - filename will contain the 
					selected filename on return.
					No translation of eols will be made.

	Return :		BOOL				-	FALSE if failure. 
											GetErrorMessage will 
											return the error.
	Parameters :	CString& filename	-	name of file to load
					CEdit* edit			-	pointer to CEdit to 
											set text to

   ============================================================*/
{
	BOOL result = FALSE;

	// Error checking
	if( ValidParam( edit ) )
	{
		CString contents;
		if( ReadTextFile( filename, contents ) )
		{

			edit->SetWindowText( contents );
			result = TRUE;

		}
	}

	return result;

}

BOOL CTextFile::Load( CString& filename, CListBox* list )
/* ============================================================
	Function :		CTextFile::Load
	Description :	Loads a text file from filename to the 
					CListBox list.
					If filename is empty, the standard file 
					dialog will be displayed, and - if OK is 
					selected - filename will contain the 
					selected filename on return.

	Return :		BOOL				-	FALSE if failure. 
											GetErrorMessage will 
											return the error.
	Parameters :	CString& filename	-	name of file to load
					CListBox* list		-	pointer to CListBox 
											to set text to

   ============================================================*/
{
	BOOL result = FALSE;

	// Error checking
	if(ValidParam(list))
	{

		// Read the file
		CStringArray contents;
		if(ReadTextFile( filename, contents))
		{
			// Set to listbox
			INT_PTR max = contents.GetSize();
			for(int t = 0 ; t < max ; t++)
			{
				if(contents[t].GetLength())
				{
					list->AddString(contents[t]);
				}
			}

			result = TRUE;
		}
	}

	return result;

}

BOOL CTextFile::Save( CString& filename, CEdit* edit )
/* ============================================================
	Function :		CTextFile::Save
	Description :	Saves the contents of the CEdit edit to the 
					file filename. The file will be created or 
					overwritten.
					If filename is empty, the standard file 
					dialog will be displayed, and - if OK is 
					selected - filename will contain the 
					selected filename on return.
					Note that the eol-öarkers from the editbox
					will be used.

	Return :		BOOL				-	FALSE if failure. 
											GetErrorMessage will 
											return the error.
	Parameters :	CString& filename	-	name of file to save 
											to. Will be 
											overwritten
					CEdit* edit			-	pointer to CEdit to 
											get text from

   ============================================================*/
{
	BOOL result = FALSE;

	// Error checking
	if( ValidParam( edit ) )
	{

		// Get text
		CString contents;
		edit->GetWindowText( contents );

		// Write file
		if( WriteTextFile( filename, contents ) )
			result = TRUE;

	}

	return result;

}

BOOL CTextFile::Save( CString& filename, CListBox* list )
/* ============================================================
	Function :		CTextFile::Save
	Description :	Saves the contents of the CListBox list to 
					the file filename. The file will be created 
					or overwritten.
					If filename is empty, the standard file 
					dialog will be displayed, and - if OK is 
					selected - filename will contain the 
					selected filename on return.
					
	Return :		BOOL				-	FALSE if failure. 
											GetErrorMessage will 
											return the error.
	Parameters :	CString& filename	-	name of file to save 
											to. Will be 
											overwritten
					CListBox* list		-	pointer to CListBox 
											to get text from

   ============================================================*/
{
	BOOL result = FALSE;

	// Error checking
	if( ValidParam( list ) )
	{

		// Get listbox contents
		CStringArray contents;
		int max = list->GetCount();
		for( int t = 0; t < max ; t++ )
		{

			CString line;
			list->GetText( t, line );
			contents.Add( line );

		}

		// Write file
		if( WriteTextFile( filename, contents ) )
			result = TRUE;

	}

	return result;

}

////////////////////////////////////////
// Error handling
//

CString CTextFile::GetErrorMessage()
/* ============================================================
	Function :		CTextFile::GetErrorMessage
	Description :	Retrieves the error message. Should be 
					called after any of the file operations 
					returns FALSE and the file name is not 
					empty.

	Return :		CString	-	The current error string
	Parameters :	none

   ============================================================*/
{

	return m_error;

}

////////////////////////////////////////
// Private functions
//

void CTextFile::ClearError()
/* ============================================================
	Function :		CTextFile::ClearError
	Description :	Clears the internal error string. Should 
					be called first by all functions setting 
					the error message string.

	Return :		void
	Parameters :	none

   ============================================================*/
{

	m_error = _T( "" );

}

BOOL CTextFile::ValidParam( CWnd* wnd )
/* ============================================================
	Function :		CTextFile::ValidParam
	Description :	Used to check parameters of the Save/Load
					functions. The pointer to the window must 
					be valid and the window itself must exist.
					
	Return :		BOOL			-	FALSE if any parameter 
										was invalid
	Parameters :	CWnd* wnd		-	a window pointer, that 
										must be valid, to a 
										window

   ============================================================*/
{

	ClearError();
	BOOL result = TRUE;

	if( wnd == NULL )
	{

		ASSERT( FALSE );
		result = FALSE;

	}

	if( !IsWindow( wnd->m_hWnd ) )
	{

		ASSERT( FALSE );
		result = FALSE;

	}

	if( !result )
		m_error = _T( "Bad Window handle as parameter" );

	return result;

}

BOOL CTextFile::GetFilename( BOOL save, CString& filename )
/* ============================================================
	Function :		CTextFile::GetFilename
	Description :	The function will display a standard file 
					dialog. If the instance is created with an 
					extension, the extension will be used to 
					filter files.
					
	Return :		BOOL				-	TRUE if a file was 
											selected
	Parameters :	BOOL save			-	TRUE if the file 
											should be saved.
					CString& filename	-	Placeholder for the 
											selected filename

   ============================================================*/
{
	CString filter;
	CString extension = GetExtension();
	if( extension.GetLength() )
		filter = extension + _T( "-files (*." + extension + ")|*." ) + extension + _T( "|All Files (*.*)|*.*||" );

	BOOL result = FALSE;
	CFileDialog dlg( !save, extension, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter );

	if( dlg.DoModal() == IDOK )
	{
		filename = dlg.GetPathName();
		result = TRUE;
	}

	return result;

}

CString CTextFile::GetExtension()
/* ============================================================
	Function :		CTextFile::GetExtension
	Description :	An accessor for the m_extension field.
					
	Return :		CString	-	the extension.
	Parameters :	none

   ============================================================*/
{

	return m_extension;

}