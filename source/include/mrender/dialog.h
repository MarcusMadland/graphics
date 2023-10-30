/*
 * Copyright 2010-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#ifndef DIALOG_H_HEADER_GUARD
#define DIALOG_H_HEADER_GUARD

namespace bx { class FilePath; class StringView; }

namespace mrender {

struct FileSelectionDialogType
{
	enum Enum
	{
		Open,
		Save,

		Count
	};
};

///
bool openFileSelectionDialog(
	bx::FilePath& _inOutFilePath
	, FileSelectionDialogType::Enum _type
	, const bx::StringView& _title
	, const bx::StringView& _filter = "All Files | *"
	);

///
bool openDirectorySelectionDialog(
	bx::FilePath& _inOutFilePath
	, const bx::StringView& _title
);

///
void openUrl(const bx::StringView& _url);

// 
void dragPath(const bx::FilePath& _path);

}

#endif // DIALOG_H_HEADER_GUARD
