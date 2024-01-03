/*
 * Copyright 2019-2019 Attila Kocsis. All rights reserved.
 * License: https://github.com/bkaradzic/graphics/blob/master/LICENSE
 */

#include "entry_p.h"
#if BASE_PLATFORM_OSX

#include <base/allocator.h>
#include <base/filepath.h>
#include <base/string.h>
#include <base/readerwriter.h>
#include <base/process.h>
#include <base/semaphore.h>

#import <AppKit/AppKit.h>

#include "dialog.h"

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
			  base::StringView(m_token.getTerm()+1, base::strFind(base::StringView(m_token.getTerm()+1, m_str.getTerm() ), m_ch).getPtr())
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

bool openFileSelectionDialog(
	  base::FilePath& _inOutFilePath
	, FileSelectionDialogType::Enum _type
	, const base::StringView& _title
	, const base::StringView& _filter
	)
{
	NSMutableArray* fileTypes = [NSMutableArray arrayWithCapacity:10];

	for (base::LineReader lr(_filter); !lr.isDone();)
	{
		const base::StringView line = lr.next();
		const base::StringView sep  = base::strFind(line, '|');

		if (!sep.isEmpty() )
		{
			for (Split split(base::strTrim(base::StringView(sep.getPtr()+1, line.getTerm() ), " "), ' ')
				; !split.isDone()
				;
				)
			{
				const base::StringView token = split.next();

				if (token.getLength() >= 3
				&&  token.getPtr()[0] == '*'
				&&  token.getPtr()[1] == '.'
				&&  base::isAlphaNum(token.getPtr()[2]) )
				{
					NSString* extension = [[NSString alloc] initWithBytes:token.getPtr()+2 length:token.getLength()-2 encoding:NSASCIIStringEncoding];
					[fileTypes addObject: extension];
				}
			}
		}
	}

	__block NSString* fileName = nil;

	void (^invokeDialog)(void) =
	^{
		NSSavePanel* panel = nil;

		if (FileSelectionDialogType::Open == _type)
		{
			NSOpenPanel* openPanel = [NSOpenPanel openPanel];
			openPanel.canChooseFiles = TRUE;
			openPanel.allowsMultipleSelection = FALSE;
			openPanel.canChooseDirectories = FALSE;
			panel = openPanel;
		}
		else
		{
			panel = [NSSavePanel savePanel];
		}

		panel.message = [[NSString alloc] initWithBytes:_title.getPtr() length:_title.getLength() encoding:NSASCIIStringEncoding];
		panel.directoryURL = [NSURL URLWithString:@(_inOutFilePath.getCPtr())];
		panel.allowedContentTypes = fileTypes;

		if ([panel runModal] == NSModalResponseOK)
		{
			NSURL* url = [panel URL];

			if (nil != url)
			{
				fileName = [url path];
				[fileName retain];
			}
		}

		[panel close];
	};

	if ([NSThread isMainThread])
	{
		invokeDialog();
	}
	else
	{
		base::Semaphore semaphore;
		base::Semaphore* psemaphore = &semaphore;

		CFRunLoopPerformBlock(
			  [[NSRunLoop mainRunLoop] getCFRunLoop]
			, kCFRunLoopCommonModes
			, ^{
				invokeDialog();
				psemaphore->post();
			});
		semaphore.wait();
	}

	if (fileName != nil)
	{
		_inOutFilePath.set([fileName UTF8String]);
		[fileName release];
		return true;
	}

	return false;
}

#endif // BASE_PLATFORM_OSX
