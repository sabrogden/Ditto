#pragma once

//Contains the actual data of a clip format
// Type is the type of clipboard format
// Data is a HGLOBAL object pointing to the clipboard format data
class IClipFormat
{
public:
	virtual CLIPFORMAT Type() = 0;
	virtual HGLOBAL Data() = 0;
	virtual bool AutoDeleteData() = 0;
	virtual void Type(CLIPFORMAT type) = 0;
	virtual void Data(HGLOBAL data) = 0;
	virtual void AutoDeleteData(bool autoDelete) = 0;
};

//Contains a list of IClipFormats
//This is a list of all data associated with a clip
class IClipFormats
{
public:
	virtual int Size() = 0;
	virtual IClipFormat *GetAt(int nPos) = 0;
	virtual void DeleteAt(int nPos) = 0;
	virtual void DeleteAll() = 0;
	virtual INT_PTR AddNew(CLIPFORMAT type, HGLOBAL data) = 0;
	virtual IClipFormat *FindFormatEx(CLIPFORMAT type) = 0;
};

class IClip
{
public:
	virtual CString Description() = 0;
	virtual void Description(CString csValue) = 0;
	virtual CTime PasteTime() = 0;
	virtual int ID() = 0;
	virtual int Parent() = 0;
	virtual void Parent(int nParent) = 0;
	virtual int DontAutoDelete() = 0;
	virtual void DontAutoDelete(int Dont) = 0;
	virtual CString QuickPaste() = 0;
	virtual void QuickPaste(CString csValue) = 0;

	virtual IClipFormats *Clips() = 0;
};