/*
 * Copyright 2010-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/graphics/blob/master/LICENSE
 */

#include <base/allocator.h>
#include <base/filepath.h>
#include <base/string.h>
#include <base/readerwriter.h>
#include <base/process.h>

#include "graphics/dialog.h"

#ifdef BASE_PLATFORM_WINDOWS
#include <windows.h>
#include <shlobj_core.h>
#endif

namespace graphics {

#if BASE_PLATFORM_WINDOWS
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

#endif // BASE_PLATFORM_WINDOWS

void openUrl(const base::StringView& _url)
{
	char tmp[4096];

#if BASE_PLATFORM_WINDOWS
#	define OPEN ""
#elif BASE_PLATFORM_OSX
#	define OPEN "open "
#else
#	define OPEN "xdg-open "
#endif // BASE_PLATFORM_OSX

	base::snprintf(tmp, BASE_COUNTOF(tmp), OPEN "%.*s", _url.getLength(), _url.getPtr() );

#undef OPEN

#if BASE_PLATFORM_WINDOWS
	void* result = ShellExecuteA(NULL, NULL, tmp, NULL, NULL, false);
	BASE_UNUSED(result);
#elif !BASE_PLATFORM_IOS
	int32_t result = system(tmp);
	BASE_UNUSED(result);
#endif // BASE_PLATFORM_*
}

#if BASE_PLATFORM_WINDOWS

void dragPath(const base::FilePath& _path)
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
	Split(const base::StringView& _str, char _ch)
	: m_str(_str)
	, m_token(_str.getPtr(), base::strFind(_str, _ch).getPtr() )
	, m_ch(_ch)
	{
	}

	base::StringView next()
	{
		base::StringView result = m_token;
		m_token = base::strTrim(
			  base::StringView(m_token.getTerm()+1, base::strFind(base::StringView(m_token.getTerm()+1, m_str.getTerm() ), m_ch).getPtr() )
			, " \t\n"
			);
		return result;
	}

	bool isDone() const
	{
		return m_token.isEmpty();
	}

private:
	const base::StringView& m_str;
	base::StringView m_token;
	char m_ch;
};

#if BASE_PLATFORM_WINDOWS
extern "C" typedef bool(__stdcall* OPENFILENAMEFUNCTION)(OPENFILENAMEA* _ofn);
static const struct { OPENFILENAMEFUNCTION m_function; uint32_t m_flags; }
s_getFileNameA[] =
{
	{ GetOpenFileNameA, /* OFN_EXPLORER */ 0x00080000 | /* OFN_DONTADDTORECENT */ 0x02000000 | /* OFN_FILEMUSTEXIST */ 0x00001000 },
	{ GetSaveFileNameA, /* OFN_EXPLORER */ 0x00080000 | /* OFN_DONTADDTORECENT */ 0x02000000                                      },
};
BASE_STATIC_ASSERT(BASE_COUNTOF(s_getFileNameA) == FileSelectionDialogType::Count);
#endif

#if !BASE_PLATFORM_OSX
bool openFileSelectionDialog(
	  base::FilePath& _inOutFilePath
	, FileSelectionDialogType::Enum _type
	, const base::StringView& _title
	, const base::StringView& _filter
	)
{
#if BASE_PLATFORM_LINUX
	char tmp[4096];
	base::StaticMemoryBlockWriter writer(tmp, sizeof(tmp) );

	base::Error err;
	base::write(&writer, &err
		, "--file-selection%s --title \"%.*s\" --filename \"%s\""
		, FileSelectionDialogType::Save == _type ? " --save" : ""
		, _title.getLength()
		, _title.getPtr()
		, _inOutFilePath.getCPtr()
		);

	for (base::LineReader lr(_filter); !lr.isDone();)
	{
		const base::StringView line = lr.next();

		base::write(&writer, &err
			, " --file-filter \"%.*s\""
			, line.getLength()
			, line.getPtr()
			);
	}

	base::write(&writer, '\0', &err);

	if (err.isOk() )
	{
		base::ProcessReader pr;

		if (base::open(&pr, "zenity", tmp, &err) )
		{
			char buffer[1024];
			int32_t total = base::read(&pr, buffer, sizeof(buffer), &err);
			base::close(&pr);

			if (0 == pr.getExitCode() )
			{
				_inOutFilePath.set(base::strRTrim(base::StringView(buffer, total), "\n\r") );
				return true;
			}
		}
	}
#elif BASE_PLATFORM_WINDOWS
	if (_type < 0 || _type >= BASE_COUNTOF(s_getFileNameA))
		return false;

	char out[base::kMaxFilePath] = { '\0' };

	OPENFILENAMEA ofn;
	base::memSet(&ofn, 0, sizeof(ofn) );
	ofn.structSize = sizeof(OPENFILENAMEA);
	ofn.initialDir = _inOutFilePath.getCPtr();
	ofn.file       = out;
	ofn.maxFile    = sizeof(out);
	ofn.flags      = s_getFileNameA[_type].m_flags | OFN_FILEMUSTEXIST | OFN_EXTENSIONDIFFERENT;

	char tmp[4096];
	base::StaticMemoryBlockWriter writer(tmp, sizeof(tmp) );

	base::Error err;

	ofn.title = tmp;
	base::write(&writer, &err, "%.*s", _title.getLength(),  _title.getPtr() );
	base::write(&writer, '\0', &err);

	ofn.filter = tmp + uint32_t(base::seek(&writer) );

	for (base::LineReader lr(_filter); !lr.isDone() && err.isOk();)
	{
		const base::StringView line = lr.next();
		const base::StringView sep  = base::strFind(line, '|');

		if (!sep.isEmpty() )
		{
			base::write(&writer, base::strTrim(base::StringView(line.getPtr(), sep.getPtr() ), " "), &err);
			base::write(&writer, '\0', &err);

			bool first = true;

			for (Split split(base::strTrim(base::StringView(sep.getPtr()+1, line.getTerm() ), " "), ' '); !split.isDone() && err.isOk();)
			{
				const base::StringView token = split.next();
				if (!first)
				{
					base::write(&writer, ';', &err);
				}

				first = false;
				base::write(&writer, token, &err);
			}

			base::write(&writer, '\0', &err);
		}
		else
		{
			base::write(&writer, line, &err);
			base::write(&writer, '\0', &err);
			base::write(&writer, '\0', &err);
		}
	}

	base::write(&writer, '\0', &err);

	if (err.isOk()
	&& s_getFileNameA[_type].m_function(&ofn))
	{
		_inOutFilePath.set(ofn.file);
		return true;
	}
#else
	BASE_UNUSED(_inOutFilePath, _type, _title, _filter);
#endif // BASE_PLATFORM_LINUX

	return false;
}
#endif // !BASE_PLATFORM_OSX

#if !BASE_PLATFORM_OSX
bool openDirectorySelectionDialog(
	base::FilePath& _inOutFilePath
	, const base::StringView& _title
	)
{
#if BASE_PLATFORM_LINUX
#error "Implement linux function"
#elif BASE_PLATFORM_WINDOWS
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
	BASE_UNUSED(_inOutFilePath, _type, _title, _filter);
#endif // BASE_PLATFORM_LINUX

	return false;
}
#endif // !BASE_PLATFORM_OSX
}
