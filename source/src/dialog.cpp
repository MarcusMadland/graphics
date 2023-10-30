/*
 * Copyright 2010-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#include <mapp/allocator.h>
#include <mapp/filepath.h>
#include <mapp/string.h>
#include <mapp/readerwriter.h>
#include <mapp/process.h>

#include "mrender/dialog.h"

#ifdef BX_PLATFORM_WINDOWS
#include <windows.h>
#include <shlobj_core.h>
#endif

namespace mrender {

#if BX_PLATFORM_WINDOWS
typedef uintptr_t (__stdcall *LPOFNHOOKPROC)(void*, uint32_t, uintptr_t, uint64_t);

struct OPENFILENAMEA
{
	uint32_t      structSize;
	void*         hwndOwner;
	void*         hinstance;
	const char*   filter;
	const char*   customFilter;
	uint32_t      maxCustomFilter;
	uint32_t      filterIndex;
	const char*   file;
	uint32_t      maxFile;
	const char*   fileTitle;
	uint32_t      maxFileTitle;
	const char*   initialDir;
	const char*   title;
	uint32_t      flags;
	uint16_t      fileOffset;
	uint16_t      fileExtension;
	const char*   defExt;
	uintptr_t     customData;
	LPOFNHOOKPROC hook;
	const char*   templateName;
	void*         reserved0;
	uint32_t      reserved1;
	uint32_t      flagsEx;
};

extern "C" bool     __stdcall GetOpenFileNameA(OPENFILENAMEA* _ofn);
extern "C" bool     __stdcall GetSaveFileNameA(OPENFILENAMEA * _ofn);
extern "C" void*    __stdcall GetModuleHandleA(const char* _moduleName);
extern "C" uint32_t __stdcall GetModuleFileNameA(void* _module, char* _outFilePath, uint32_t _size);
extern "C" void*    __stdcall ShellExecuteA(void* _hwnd, void* _operation, void* _file, void* _parameters, void* _directory, int32_t _showCmd);

#endif // BX_PLATFORM_WINDOWS

void openUrl(const bx::StringView& _url)
{
	char tmp[4096];

#if BX_PLATFORM_WINDOWS
#	define OPEN ""
#elif BX_PLATFORM_OSX
#	define OPEN "open "
#else
#	define OPEN "xdg-open "
#endif // BX_PLATFORM_OSX

	bx::snprintf(tmp, BX_COUNTOF(tmp), OPEN "%.*s", _url.getLength(), _url.getPtr() );

#undef OPEN

#if BX_PLATFORM_WINDOWS
	void* result = ShellExecuteA(NULL, NULL, tmp, NULL, NULL, false);
	BX_UNUSED(result);
#elif !BX_PLATFORM_IOS
	int32_t result = system(tmp);
	BX_UNUSED(result);
#endif // BX_PLATFORM_*
}

#if BX_PLATFORM_WINDOWS

void dragPath(const bx::FilePath& _path)
{
	// Initialize COM if not already done
	CoInitialize(nullptr);

	// Create an IDataObject
	IDataObject* dataObject = nullptr;
	HRESULT hr = OleGetClipboard(&dataObject);
	if (SUCCEEDED(hr)) {
		// Set text data into the data object
		FORMATETC fmtetc = { CF_TEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		STGMEDIUM stgmed = {};
		const char* text = _path.getCPtr();
		size_t textLen = strlen(text);

		stgmed.hGlobal = GlobalAlloc(GHND, textLen + 1);
		if (stgmed.hGlobal) {
			char* data = static_cast<char*>(GlobalLock(stgmed.hGlobal));
			if (data) {
				memcpy(data, text, textLen);
				data[textLen] = '\0';
				GlobalUnlock(stgmed.hGlobal);
				stgmed.tymed = TYMED_HGLOBAL;

				// Set the text data into the data object
				dataObject->SetData(&fmtetc, &stgmed, true);

				// Create a drop source
				class YourDropSourceImplementation : public IDropSource
				{
				public:
					// Constructor
					YourDropSourceImplementation()
					{
					}

					// IDropSource interface methods
					STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject) override
					{
						if (riid == IID_IDropSource || riid == IID_IUnknown)
						{
							*ppvObject = static_cast<IDropSource*>(this);
							AddRef();
							return S_OK;
						}

						*ppvObject = nullptr;
						return E_NOINTERFACE;
					}

					STDMETHOD_(ULONG, AddRef)() override
					{
						return 1;
					}

					STDMETHOD_(ULONG, Release)() override
					{
						return 0;
					}

					STDMETHOD(GiveFeedback)(DWORD dwEffect) override
					{
						// Return DRAGDROP_S_USEDEFAULTCURSORS for default behavior.
						return DRAGDROP_S_USEDEFAULTCURSORS;
					}

					STDMETHOD(QueryContinueDrag)(BOOL fEscapePressed, DWORD grfKeyState) override
					{
						if (fEscapePressed || !(grfKeyState & MK_LBUTTON))
						{
							return DRAGDROP_S_CANCEL;
						}

						return S_OK;
					}
				};
				IDropSource* dropSource = new YourDropSourceImplementation;

				// Initialize and perform the drag-and-drop operation
				DWORD dwEffect = 0;
				hr = DoDragDrop(dataObject, dropSource, DROPEFFECT_COPY | DROPEFFECT_MOVE, &dwEffect);

				// Release resources
				dataObject->Release();
				dropSource->Release();
			}
		}
	}

	// Uninitialize COM
	CoUninitialize();
}

#endif

class Split
{
public:
	Split(const bx::StringView& _str, char _ch)
	: m_str(_str)
	, m_token(_str.getPtr(), bx::strFind(_str, _ch).getPtr() )
	, m_ch(_ch)
	{
	}

	bx::StringView next()
	{
		bx::StringView result = m_token;
		m_token = bx::strTrim(
			  bx::StringView(m_token.getTerm()+1, bx::strFind(bx::StringView(m_token.getTerm()+1, m_str.getTerm() ), m_ch).getPtr() )
			, " \t\n"
			);
		return result;
	}

	bool isDone() const
	{
		return m_token.isEmpty();
	}

private:
	const bx::StringView& m_str;
	bx::StringView m_token;
	char m_ch;
};

#if BX_PLATFORM_WINDOWS
extern "C" typedef bool(__stdcall* OPENFILENAMEFUNCTION)(OPENFILENAMEA* _ofn);
static const struct { OPENFILENAMEFUNCTION m_function; uint32_t m_flags; }
s_getFileNameA[] =
{
	{ GetOpenFileNameA, /* OFN_EXPLORER */ 0x00080000 | /* OFN_DONTADDTORECENT */ 0x02000000 | /* OFN_FILEMUSTEXIST */ 0x00001000 },
	{ GetSaveFileNameA, /* OFN_EXPLORER */ 0x00080000 | /* OFN_DONTADDTORECENT */ 0x02000000                                      },
};
BX_STATIC_ASSERT(BX_COUNTOF(s_getFileNameA) == FileSelectionDialogType::Count);
#endif

#if !BX_PLATFORM_OSX
bool openFileSelectionDialog(
	  bx::FilePath& _inOutFilePath
	, FileSelectionDialogType::Enum _type
	, const bx::StringView& _title
	, const bx::StringView& _filter
	)
{
#if BX_PLATFORM_LINUX
	char tmp[4096];
	bx::StaticMemoryBlockWriter writer(tmp, sizeof(tmp) );

	bx::Error err;
	bx::write(&writer, &err
		, "--file-selection%s --title \"%.*s\" --filename \"%s\""
		, FileSelectionDialogType::Save == _type ? " --save" : ""
		, _title.getLength()
		, _title.getPtr()
		, _inOutFilePath.getCPtr()
		);

	for (bx::LineReader lr(_filter); !lr.isDone();)
	{
		const bx::StringView line = lr.next();

		bx::write(&writer, &err
			, " --file-filter \"%.*s\""
			, line.getLength()
			, line.getPtr()
			);
	}

	bx::write(&writer, '\0', &err);

	if (err.isOk() )
	{
		bx::ProcessReader pr;

		if (bx::open(&pr, "zenity", tmp, &err) )
		{
			char buffer[1024];
			int32_t total = bx::read(&pr, buffer, sizeof(buffer), &err);
			bx::close(&pr);

			if (0 == pr.getExitCode() )
			{
				_inOutFilePath.set(bx::strRTrim(bx::StringView(buffer, total), "\n\r") );
				return true;
			}
		}
	}
#elif BX_PLATFORM_WINDOWS
	if (_type < 0 || _type >= BX_COUNTOF(s_getFileNameA))
		return false;

	char out[bx::kMaxFilePath] = { '\0' };

	OPENFILENAMEA ofn;
	bx::memSet(&ofn, 0, sizeof(ofn) );
	ofn.structSize = sizeof(OPENFILENAMEA);
	ofn.initialDir = _inOutFilePath.getCPtr();
	ofn.file       = out;
	ofn.maxFile    = sizeof(out);
	ofn.flags      = s_getFileNameA[_type].m_flags | OFN_FILEMUSTEXIST | OFN_EXTENSIONDIFFERENT;

	char tmp[4096];
	bx::StaticMemoryBlockWriter writer(tmp, sizeof(tmp) );

	bx::Error err;

	ofn.title = tmp;
	bx::write(&writer, &err, "%.*s", _title.getLength(),  _title.getPtr() );
	bx::write(&writer, '\0', &err);

	ofn.filter = tmp + uint32_t(bx::seek(&writer) );

	for (bx::LineReader lr(_filter); !lr.isDone() && err.isOk();)
	{
		const bx::StringView line = lr.next();
		const bx::StringView sep  = bx::strFind(line, '|');

		if (!sep.isEmpty() )
		{
			bx::write(&writer, bx::strTrim(bx::StringView(line.getPtr(), sep.getPtr() ), " "), &err);
			bx::write(&writer, '\0', &err);

			bool first = true;

			for (Split split(bx::strTrim(bx::StringView(sep.getPtr()+1, line.getTerm() ), " "), ' '); !split.isDone() && err.isOk();)
			{
				const bx::StringView token = split.next();
				if (!first)
				{
					bx::write(&writer, ';', &err);
				}

				first = false;
				bx::write(&writer, token, &err);
			}

			bx::write(&writer, '\0', &err);
		}
		else
		{
			bx::write(&writer, line, &err);
			bx::write(&writer, '\0', &err);
			bx::write(&writer, '\0', &err);
		}
	}

	bx::write(&writer, '\0', &err);

	if (err.isOk()
	&& s_getFileNameA[_type].m_function(&ofn))
	{
		_inOutFilePath.set(ofn.file);
		return true;
	}
#else
	BX_UNUSED(_inOutFilePath, _type, _title, _filter);
#endif // BX_PLATFORM_LINUX

	return false;
}
#endif // !BX_PLATFORM_OSX

#if !BX_PLATFORM_OSX
bool openDirectorySelectionDialog(
	bx::FilePath& _inOutFilePath
	, const bx::StringView& _title
	)
{
#if BX_PLATFORM_LINUX
#error "Implement linux function"
#elif BX_PLATFORM_WINDOWS
	BROWSEINFOA bi = { 0 };
	bi.hwndOwner = NULL;
	bi.pszDisplayName = NULL;
	bi.lpszTitle = _title.getPtr();
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
	if (pidl != NULL)
	{
		char folderPath[MAX_PATH];
		if (SHGetPathFromIDListA(pidl, folderPath))
		{
			_inOutFilePath.set(folderPath);
			CoTaskMemFree(pidl);
			return true;
		}
	}
	return false;
#else
	BX_UNUSED(_inOutFilePath, _type, _title, _filter);
#endif // BX_PLATFORM_LINUX

	return false;
}
#endif // !BX_PLATFORM_OSX
}
