#ifndef FILE_H
#define FILE_H

#include<windows.h>
#include<iostream>
#include<commdlg.h>

using namespace std;

bool opn_return=0;
bool sav_return=0;

string open_filename(HWND hwnd = nullptr)
{
    char filename[150];

    OPENFILENAME ofn;
    ZeroMemory( &filename, sizeof( filename ) );
    ZeroMemory( &ofn,      sizeof( ofn ) );
    ofn.lStructSize  = sizeof( ofn );
    ofn.hwndOwner    = hwnd;  // If you have a window to center over, put its HANDLE here
    ofn.lpstrFilter  = "Text Files\0*.txt\0Any File\0*.*\0";
    ofn.lpstrFile    = filename;
    ofn.nMaxFile     = 150;
    ofn.lpstrTitle   = "Select a File, yo!";
    ofn.Flags        = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName( &ofn ))
    {
        std::cout << "You chose the file \"" << filename << "\"\n";
        opn_return=true;
        return string(filename);
    }
    else
    {
        // All this stuff below is to tell you exactly how you messed up above.
        // Once you've got that fixed, you can often (not always!) reduce it to a 'user cancelled' assumption.
        switch (CommDlgExtendedError())
        {
            case CDERR_DIALOGFAILURE   : std::cout << "CDERR_DIALOGFAILURE\n";   break;
            case CDERR_FINDRESFAILURE  : std::cout << "CDERR_FINDRESFAILURE\n";  break;
            case CDERR_INITIALIZATION  : std::cout << "CDERR_INITIALIZATION\n";  break;
            case CDERR_LOADRESFAILURE  : std::cout << "CDERR_LOADRESFAILURE\n";  break;
            case CDERR_LOADSTRFAILURE  : std::cout << "CDERR_LOADSTRFAILURE\n";  break;
            case CDERR_LOCKRESFAILURE  : std::cout << "CDERR_LOCKRESFAILURE\n";  break;
            case CDERR_MEMALLOCFAILURE : std::cout << "CDERR_MEMALLOCFAILURE\n"; break;
            case CDERR_MEMLOCKFAILURE  : std::cout << "CDERR_MEMLOCKFAILURE\n";  break;
            case CDERR_NOHINSTANCE     : std::cout << "CDERR_NOHINSTANCE\n";     break;
            case CDERR_NOHOOK          : std::cout << "CDERR_NOHOOK\n";          break;
            case CDERR_NOTEMPLATE      : std::cout << "CDERR_NOTEMPLATE\n";      break;
            case CDERR_STRUCTSIZE      : std::cout << "CDERR_STRUCTSIZE\n";      break;
            case FNERR_BUFFERTOOSMALL  : std::cout << "FNERR_BUFFERTOOSMALL\n";  break;
            case FNERR_INVALIDFILENAME : std::cout << "FNERR_INVALIDFILENAME\n"; break;
            case FNERR_SUBCLASSFAILURE : std::cout << "FNERR_SUBCLASSFAILURE\n"; break;
            default                    : std::cout << "You cancelled.\n"; opn_return=false; return string("");
        }
    }
}

string save_filename(string title="", HWND hwnd = nullptr)
{
    char filename[150];

    OPENFILENAME ofn;
    ZeroMemory( &filename, sizeof( filename ) );
    ZeroMemory( &ofn,      sizeof( ofn ) );
    ofn.lStructSize  = sizeof( ofn );
    ofn.hwndOwner    = hwnd;  // If you have a window to center over, put its HANDLE here
    ofn.lpstrFilter  = "Text Files\0*.txt\0Any File\0*.*\0";
    ofn.lpstrFile    = filename;
    ofn.nMaxFile     = 150;
    ofn.lpstrTitle   = title.c_str();
    ofn.Flags        = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetSaveFileName( &ofn ))
    {
        std::cout << "You chose the file \"" << filename << "\"\n";
        sav_return=true;
        return string(filename);
    }
    else
    {
        // All this stuff below is to tell you exactly how you messed up above.
        // Once you've got that fixed, you can often (not always!) reduce it to a 'user cancelled' assumption.
        switch (CommDlgExtendedError())
        {
            case CDERR_DIALOGFAILURE   : std::cout << "CDERR_DIALOGFAILURE\n";   break;
            case CDERR_FINDRESFAILURE  : std::cout << "CDERR_FINDRESFAILURE\n";  break;
            case CDERR_INITIALIZATION  : std::cout << "CDERR_INITIALIZATION\n";  break;
            case CDERR_LOADRESFAILURE  : std::cout << "CDERR_LOADRESFAILURE\n";  break;
            case CDERR_LOADSTRFAILURE  : std::cout << "CDERR_LOADSTRFAILURE\n";  break;
            case CDERR_LOCKRESFAILURE  : std::cout << "CDERR_LOCKRESFAILURE\n";  break;
            case CDERR_MEMALLOCFAILURE : std::cout << "CDERR_MEMALLOCFAILURE\n"; break;
            case CDERR_MEMLOCKFAILURE  : std::cout << "CDERR_MEMLOCKFAILURE\n";  break;
            case CDERR_NOHINSTANCE     : std::cout << "CDERR_NOHINSTANCE\n";     break;
            case CDERR_NOHOOK          : std::cout << "CDERR_NOHOOK\n";          break;
            case CDERR_NOTEMPLATE      : std::cout << "CDERR_NOTEMPLATE\n";      break;
            case CDERR_STRUCTSIZE      : std::cout << "CDERR_STRUCTSIZE\n";      break;
            case FNERR_BUFFERTOOSMALL  : std::cout << "FNERR_BUFFERTOOSMALL\n";  break;
            case FNERR_INVALIDFILENAME : std::cout << "FNERR_INVALIDFILENAME\n"; break;
            case FNERR_SUBCLASSFAILURE : std::cout << "FNERR_SUBCLASSFAILURE\n"; break;
            default                    : std::cout << "You cancelled.\n"; sav_return=false; return string("");
        }
    }
}


#endif // FILE_H
