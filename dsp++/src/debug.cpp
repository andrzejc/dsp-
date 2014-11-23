#include <dsp++/debug.h>
#include <fstream>
#include <iterator>
#include <sstream>

#ifdef _WIN32
#include <windows.h> // Clipboard access functions
#include <vector>
#endif // _WIN32

using namespace dsp;

namespace {

static inline void dump_stream(std::ostream& s, const float* vec, size_t len) {
	s.precision(11);
	for (size_t i = 0; i < len; ++i, ++vec) {
		if (i != 0)
			s << ", ";
		s << *vec;
	}
}

static inline void dump_stream(std::ostream& s, const double* vec, size_t len) {
	s.precision(18);
	for (size_t i = 0; i < len; ++i, ++vec) {
		if (i != 0)
			s << ", ";
		s << *vec;
	}
}

}

void dsp::dbg::dump_csv(const char* path, const float* vec, size_t len)
{
	std::ofstream s(path);
	dump_stream(s, vec, len);
}

void dsp::dbg::dump_csv(const char* path, const double* vec, size_t len)
{
	std::ofstream s(path);
	dump_stream(s, vec, len);
}

void dsp::dbg::dump_str(std::string& str, const float* vec, size_t len)
{
	std::ostringstream s;
	dump_stream(s, vec, len);
	str = s.str();
}

void dsp::dbg::dump_str(std::string& str, const double* vec, size_t len)
{
	std::ostringstream s;
	dump_stream(s, vec, len);
	str = s.str();
}

namespace {

#ifdef _WIN32

struct EnumWindowsCallbackArgs {
    EnumWindowsCallbackArgs(std::vector<HWND>& h, DWORD p): pid(p), handles(h) { }
    const DWORD pid;
    std::vector<HWND>& handles;
};

static BOOL CALLBACK EnumWindowsCallback(HWND hnd, LPARAM lParam)
{
    EnumWindowsCallbackArgs *args = (EnumWindowsCallbackArgs *)lParam;
    DWORD windowPID;
    (void)::GetWindowThreadProcessId(hnd, &windowPID);
    if (windowPID == args->pid) {
        args->handles.push_back(hnd);
    }
    return TRUE;
}

static void enum_windows(std::vector<HWND>& wnd)
{
    EnumWindowsCallbackArgs args(wnd, ::GetCurrentProcessId());
    ::EnumWindows(&EnumWindowsCallback, (LPARAM) &args);
}


static bool do_clipbrd_copy(const std::string& str) 
{
	HWND win = NULL;
	std::vector<HWND> wnds;
	enum_windows(wnds);
	for (size_t i = 0; i < wnds.size(); ++i)
		if (::IsWindow(wnds[i])) {
			win = wnds[i];
			break;
		}

	if (NULL == win)
		win = ::GetDesktopWindow();

	if (!::OpenClipboard(win))
		return false;

	// Empty the Clipboard. This also has the effect
	// of allowing Windows to free the memory associated
	// with any data that is in the Clipboard
	::EmptyClipboard();

	// Ok. We have the Clipboard locked and it's empty. 
	// Now let's allocate the global memory for our data.

	// Here I'm simply using the GlobalAlloc function to 
	// allocate a block of data equal to the text in the
	// "to clipboard" edit control plus one character for the
	// terminating null character required when sending
	// ANSI text to the Clipboard.
	HGLOBAL hClipboardData;
	if (NULL == (hClipboardData = ::GlobalAlloc(GMEM_DDESHARE, str.length() + 1)))
	{
		::CloseClipboard();
		return false;
	}

	// Calling GlobalLock returns to me a pointer to the 
	// data associated with the handle returned from 
	// GlobalAlloc
	char * pchData;
	if (NULL == (pchData = (char*)::GlobalLock(hClipboardData)))
	{
		::GlobalFree(hClipboardData);
		::CloseClipboard();
		return false;
	}

	// At this point, all I need to do is use the standard 
	// C/C++ strcpy function to copy the data from the local 
	// variable to the global memory.
	memcpy(pchData, str.c_str(), str.length() + 1);

	// Once done, I unlock the memory - remember you 
	// don't call GlobalFree because Windows will free the 
	// memory automatically when EmptyClipboard is next 
	// called. 
	::GlobalUnlock(hClipboardData);

	// Now, set the Clipboard data by specifying that 
	// ANSI text is being used and passing the handle to
	// the global memory.
	if (NULL == ::SetClipboardData(CF_TEXT, hClipboardData))
	{
		::GlobalFree(hClipboardData);
		::CloseClipboard();
		return false;
	}

	// Finally, when finished I simply close the Clipboard
	// which has the effect of unlocking it so that other
	// applications can examine or modify its contents.
	::CloseClipboard();
	return true;
}
#else // !_WIN32
static bool do_clipbrd_copy(const std::string&) {return false;}
#endif // !_WIN32

}

void dsp::dbg::clipbrd_copy(const float* vec, size_t len)
{
	std::string str;
	dump_str(str, vec, len);
	do_clipbrd_copy(str);
}

void dsp::dbg::clipbrd_copy(const double* vec, size_t len)
{
	std::string str;
	dump_str(str, vec, len);
	do_clipbrd_copy(str);
}
