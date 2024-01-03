/*
 * Copyright 2010-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/graphics/blob/master/LICENSE
 */

#ifndef DIALOG_H_HEADER_GUARD
#define DIALOG_H_HEADER_GUARD

namespace base { class FilePath; class StringView; }

namespace graphics {

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
	base::FilePath& _inOutFilePath
	, FileSelectionDialogType::Enum _type
	, const base::StringView& _title
	, const base::StringView& _filter = "All Files | *"
	);

///
bool openDirectorySelectionDialog(
	base::FilePath& _inOutFilePath
	, const base::StringView& _title
);

///
void openUrl(const base::StringView& _url);

// 
void dragPath(const base::FilePath& _path);

}

#endif // DIALOG_H_HEADER_GUARD
