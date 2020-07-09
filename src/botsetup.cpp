//
// YaPB - Counter-Strike Bot based on PODBot by Markus Klinge.
// Copyright © 2004-2020 YaPB Development Team <team@yapb.ru>.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//

#define WIN32_LEAN_AND_MEAN

#undef UNICODE
#undef _UNICODE

#include <crlib/cr-complete.h>

using namespace cr;

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' " "version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"") 

#pragma comment (lib, "comctl32.lib")
#pragma comment (lib, "imagehlp.lib")
#pragma comment (lib, "gdiplus.lib")
#pragma comment (lib, "msimg32.lib")

#include "windows.h"
#include "windowsx.h"
#include "resource.h"
#include "unzip.h"

#include <stdlib.h>
#include <stdio.h>
#include <commctrl.h>
#include <shellapi.h>
#include <commdlg.h>
#include <winnt.h>

#include <objidl.h>
#include <gdiplus.h>

using namespace Gdiplus;

// how much to shrink path?
constexpr int32 kShrinkPathAmount = 32;
class Lang : public Singleton <Lang> {
public:
   enum : int32 {
      English,
      Russian
   };

   enum : int32 {
      SelectExeFile,
      Install,
      Browse,
      Version,
      Cancel,
      Application,
      FileDamaged,
      CopyingFiles,
      PatchingFiles,
      Exit,
      Done,
      LicenseInfo,
      CancelCopy,
      SelectHLExeFile,
      CannotPatchFiles,
      BadGameDirectory,
      CannotOpenArchive,
      InfoText
   };

private:
   HashMap <int32, HashMap <int32, String>> m_data;
   uint32 m_lang;

private:
   void regLangTab () {
      m_data[Russian][SelectExeFile] = "Выберите hl.exe";
      m_data[Russian][Install] = "Установить";
      m_data[Russian][Browse] = "Обзор";
      m_data[Russian][Version] = "YaPB v%s";
      m_data[Russian][Cancel] = "Отмена";
      m_data[Russian][Application] = "Установка YaPB";
      m_data[Russian][FileDamaged] = "Установщик поврежден.\n\nПожалуйста получите новую копию установщика at https://yapb.ru/files/\n\nОшибка: %s";
      m_data[Russian][CopyingFiles] = "(%d/%d): %s%s";
      m_data[Russian][PatchingFiles] = "Правка Файлов";
      m_data[Russian][Exit] = "Выход";
      m_data[Russian][Done] = "OK. Нажмите Выход";
      m_data[Russian][LicenseInfo] = "YaPB Setup v2.0 (" __DATE__ " " __TIME__ ")";
      m_data[Russian][CancelCopy] = "Вы уверены, что хотите отменить установку ботов?";
      m_data[Russian][SelectHLExeFile] = "Исполняемый файл Half-Life (hl.exe/hlds.exe)\0hl.exe;hlds.exe\0";
      m_data[Russian][CannotPatchFiles] = "Невозможно записать изменения в liblist.gam/plugins.ini. Вам придется сделать это самостоятельно.";
      m_data[Russian][BadGameDirectory] = "В этой папке нету ничего похожего на CS... Попробуйте другую...";
      m_data[Russian][CannotOpenArchive] = "Невозможно открыть архив установки.";
      m_data[Russian][InfoText] =
         "Установка YaPB для CS.\r\n"
         "\r\n"
         "Выберите папку с игрой и нажмите установить.\r\n"
         "\r\n"
         "Удачных фрагов!\r\n";

      m_data[English][SelectExeFile] = "Select hl.exe";
      m_data[English][Install] = "Install";
      m_data[English][Browse] = "Browse";
      m_data[English][Version] = "YaPB v%s";
      m_data[English][Cancel] = "Cancel";
      m_data[English][Application] = "YaPB Setup";
      m_data[English][FileDamaged] = "Setup corrupted.\n\nPlease obtain new copy of installer at https://yapb.ru/files/\n\nError: %s";
      m_data[English][CopyingFiles] = "(%d/%d): %s%s";
      m_data[English][PatchingFiles] = "Patching Files";
      m_data[English][Exit] = "Exit";
      m_data[English][Done] = "OK. Press Exit";
      m_data[English][LicenseInfo] = "YaPB Setup v2.0 (" __DATE__ " " __TIME__ ")";
      m_data[English][CancelCopy] = "Are you sure you want to cancel installation?";
      m_data[English][SelectHLExeFile] = "Half-Life Executable (hl.exe/hlds.exe)\0hl.exe;hlds.exe\0";
      m_data[English][CannotPatchFiles] = "Unable to write changes to liblist.gam/plugins.ini. You have to modify these files yourself.";
      m_data[English][BadGameDirectory] = "There is no CS-compatible mods detected in this dir. Please select something better...";
      m_data[English][CannotOpenArchive] = "Unable to open installer archive.";
      m_data[English][InfoText] =
         "Welcome to YaPB Setup.\r\n"
         "\r\n"
         "Please select location of your game, then press install.\r\n"
         "\r\n"
         "Good luck! And have fun!\r\n";
   }

public:
   Lang () : m_lang (English) {
      regLangTab ();
   }

public:
   decltype (auto) tr (const uint32 code) {
      return m_data[m_lang][code].chars ();
   }

public:
   void trySet () {
      auto lcid = GetUserDefaultLCID ();

      if (lcid == 1049 || lcid == 1059 || lcid == 1058) {
         m_lang = Russian;
      }
      String cmdline = GetCommandLineA ();

      if (!cmdline.empty ()) {
         if (cmdline.contains ("/ru")) {
            m_lang = Russian;
         }
         else if (cmdline.contains ("/en")) {
            m_lang = English;
         }
#if 0
         if (cmdline.contains ("testing")) {
            global.test = true;
#endif
      }
   }

   uint32 getLang () const {
      return m_lang;
   }
};

struct Static : public Singleton <Static> {
   String target;
   HINSTANCE inst = nullptr;
   HWND hwnd = nullptr;
   HWND tip = nullptr;
   HANDLE mutex = nullptr;

   volatile bool install = false;
   volatile bool pause = false;
   volatile bool test = false;
   volatile bool imageSet = false;
};

CR_EXPOSE_GLOBAL_SINGLETON (Lang, lang);
CR_EXPOSE_GLOBAL_SINGLETON (Static, global);

class BotSetup : public Singleton <BotSetup> {
public:
   using SetupCallback = Lambda <void (StringRef, uint32)>;

private:
   HZIP m_hz;
   String m_botVersion;
   uint32 m_fileCount;

public:
   template <typename ...Args> void __declspec (noreturn) abort (Args &&...args) {
      MessageBoxA (GetActiveWindow (), strings.format (lang.tr (Lang::FileDamaged), args...), lang.tr (Lang::Application), MB_ICONSTOP | MB_TOPMOST);
      ExitProcess (EXIT_FAILURE);
   }
   
   template <typename ...Args> void warning (Args &&...args) {
      MessageBoxA (GetActiveWindow (), strings.format (args...), lang.tr (Lang::Application), MB_ICONWARNING | MB_TOPMOST);
   }

   bool dirExists (StringRef dirname) const {
      auto type = GetFileAttributesA (dirname.chars ());

      if (type == INVALID_FILE_ATTRIBUTES) {
         return false;
      }

      if (type & FILE_ATTRIBUTE_DIRECTORY) {
         return true;
      }
      return false;
   }

   void makeFileBackup (StringRef file) {
      CopyFileA (file.chars (), strings.format ("%s.bak", file), FALSE);
   }

public:
   BotSetup () : m_hz (nullptr) {
      auto sfx = strings.chars ();

      if (global.test) {
         strings.copy (sfx, "botsetup.zip", StringBuffer::StaticBufferSize);
      }
      else {
         GetModuleFileNameA (nullptr, sfx, StringBuffer::StaticBufferSize);
      }
      m_hz = OpenZip (sfx, nullptr);

      if (!m_hz) {
         abort (lang.tr (Lang::CannotOpenArchive));
      }
      auto buffer = strings.chars ();
      GetZipGlobalComment (m_hz, buffer, StringBuffer::StaticBufferSize);

      ZIPENTRY ze {};
      GetZipItem (m_hz, -1, &ze);

      m_fileCount = ze.index;
      m_botVersion.assign (buffer);

      if (m_botVersion.empty ()) {
         abort (lang.tr (Lang::CannotOpenArchive));
      }
   }

   ~BotSetup () {
      CloseZipU (m_hz);
   }

   void clearReadOnly (StringRef target) {
      SetFileAttributesA (target.chars (), GetFileAttributesA (target.chars ()) & ~(FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY));
   }

   void patchModFiles (StringRef target, StringRef mod) {
      auto plugins = strings.format ("%s\\%s\\addons\\metamod\\plugins.ini", target, mod);

      if (File::exists (plugins)) {
         makeFileBackup (plugins);
         clearReadOnly (plugins);

         String line, contents;
         File fp (plugins, "rt");

         if (fp) {
            while (fp.getLine (line)) {
               if (!line.contains ("yapb") && !line.contains ("YaPB") && !line.contains ("ubot") && !line.contains ("Ubot")) {
                  contents += line;
               }
            }
            fp.close ();

            contents += "\n";
            contents += "; Added by YaPB Setup\n";
            contents += "; See: https://yapb.ru/\n";
            contents += "win32 addons\\yapb\\bin\\yapb.dll YaPB for Counter-Strike\n";
            contents += "linux addons/yapb/bin/yapb.so YaPB for Counter-Strike\n";
            contents += "osx addons/yapb/bin/yapb.dylib YaPB for Counter-Strike\n";
            contents += "\n";

            if (fp.open (plugins, "wt")) {
               fp.puts (contents.chars ());
               fp.close ();
            }
            else {
               warning (lang.tr (Lang::CannotPatchFiles));
            }
         }
      }
      auto liblist = strings.format ("%s\\%s\\liblist.gam", target, mod);

      if (File::exists (liblist)) {
         makeFileBackup (liblist);
         clearReadOnly (liblist);

         String line, contents;
         File fp (liblist, "rt");

         if (fp) {
            auto gamedll = false;

            while (fp.getLine (line)) {
               if (!line.contains ("gamedll")) {
                  contents += line;
               }
               else if (!gamedll) {
                  contents += "gamedll \"addons\\yapb\\bin\\yapb.dll\"\n";
                  contents += "gamedll_linux \"addons/yapb/bin/yapb.so\"\n";
                  contents += "gamedll_osx \"addons/yapb/bin/yapb.dylib\"\n";

                  gamedll = true;
               }
            }
            fp.close ();

            if (fp.open (liblist, "wt")) {
               fp.puts (contents.chars ());
               fp.close ();
            }
            else {
               warning (lang.tr (Lang::CannotPatchFiles));
            }
         }
      }
   }

   bool isCSBinary (StringRef target, StringRef dir) {
      auto mod = strings.format ("%s\\%s\\dlls\\mp.dll", target, dir);

      if (!File::exists (mod)) {
         return false;
      }
      SharedLibrary mp;

      if (mp.load (mod)) {
         if (mp.resolve <SharedLibrary::Handle> ("weapon_usp") && mp.resolve <SharedLibrary::Handle> ("weapon_m4a1")) {
            mp.unload ();
            return true;
         }
      }
      return false;
   }

   decltype (auto) locateGameMods (StringRef target) {
      StringArray result;

      WIN32_FIND_DATA search {};
      HANDLE handle = FindFirstFileA (strings.format ("%s\\*", target), &search);

      if (handle != INVALID_HANDLE_VALUE) {
         do {
            auto filename = search.cFileName;

            if (filename[0] == '.' || !(search.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
               continue;
            }
            
            if (isCSBinary (target, filename)) {
               result.emplace (filename);
            }
         }
         while (FindNextFileA (handle, &search));
      }
      return result;
   }

   void patchLang (StringRef target, StringRef mod) {
      auto botConfig = strings.format ("%s\\%s\\addons\\yapb\\conf\\yapb.cfg", target, mod);
      clearReadOnly (botConfig);

      String line, contents;
      File fp (botConfig, "rt");

      if (!fp) {
         return;
      }
      constexpr StringRef cvarName = "yb_language";

      while (fp.getLine (line)) {
         if (!line.contains (cvarName)) {
            contents.append (line);
            
         }
         else {
            contents.appendf ("%s \"%s\"\n", cvarName, lang.getLang () == Lang::Russian ? "ru" : "en");
         }
      }
      fp.close ();

      if (fp.open (botConfig, "wt")) {
         fp.puts (contents.chars ());
         fp.close ();
      }
      else {
         warning (lang.tr (Lang::CannotPatchFiles));
      }
   }

   bool install (StringRef target, SetupCallback fn) {
      auto targets = locateGameMods (target);

      if (targets.empty ()) {
         return false;
      }

      for (const auto &mod : targets) {
         extract (target, mod, fn);

         patchModFiles (target, mod);
         patchLang (target, mod);
      }
      return true;
   }

   int getFileCount (void) const {
      return m_fileCount;
   }

    const char *getBotVersion (void) {
       return m_botVersion.empty () ? "<null>" : m_botVersion.chars ();
   }

   void extract (StringRef target, StringRef mod, SetupCallback fn) {
      for (auto i = 0U; i <= m_fileCount; ++i) {
         ZIPENTRY ze;
         GetZipItem (m_hz, i, &ze);

         fn (ze.name, i);

         while (global.pause) {
            Sleep (100);
         }
         UnzipItem (m_hz, i, strings.format ("%s\\%s\\%s", target, mod, ze.name));
      }
   }

   String shrinkPath (const String &path, int maxLength) {
      auto parts = path.split ("\\");
      auto output = String::join (parts, "\\");
      auto endIndex = (parts.length () - 1);
      auto startIndex = endIndex / 2;
      auto index = startIndex;
      auto step = 0;

      while (output.length () >= static_cast <size_t> (maxLength) && index != 0 && index != endIndex) {
         if (index == 1) {
            index++;
            continue;
         }
         parts[index] = "...";
         output = String::join (parts, "\\");

         if (step >= 0) {
            step++;
         }
         step = (step * -1);
         index = startIndex + step;
      }
      return output;
   }

public:
   static void progress (StringRef file, uint32 num) {
      const auto &inst = BotSetup::instance ();

      SendMessageA (GetDlgItem (global.hwnd, IDC_PROGRESS1), PBM_SETRANGE, 0, MAKELPARAM (0, inst.getFileCount () + 1));
      SendMessageA (GetDlgItem (global.hwnd, IDC_PROGRESS1), PBM_SETSTEP, 1, 0);
      SendMessageA (GetDlgItem (global.hwnd, IDC_PROGRESS1), PBM_STEPIT, 0, 0);

      char dst[64];
      char ext[32];
      _splitpath (file.chars (), nullptr, nullptr, dst, ext);

      SetDlgItemTextA (global.hwnd, IDC_STATUS, strings.format (lang.tr (Lang::CopyingFiles), num, inst.getFileCount (), dst, ext));
      Sleep (30);

      if (num == inst.getFileCount ()) {
         SetDlgItemTextA (global.hwnd, IDC_STATUS, lang.tr (Lang::PatchingFiles));
         Sleep (800);
      }
   }

   static HICON loadPNG (uint32 resId) {
      static HICON result;

      if (result) {
         return result;
      }

      auto resHandle = FindResourceA (global.inst, MAKEINTRESOURCEA (resId), "PNG");

      if (!resHandle) {
         return nullptr;
      }
      auto resSize = SizeofResource (global.inst, resHandle);

      if (!resSize) {
         return nullptr;
      }
      auto resInstance = LoadResource (global.inst, resHandle);

      if (!resInstance) {
         return nullptr;
      }
      auto resData = LockResource (resInstance);

      if (!resData) {
         FreeResource (resInstance);
         return nullptr;
      }
      auto buffer = GlobalAlloc (GMEM_MOVEABLE, resSize);

      if (!buffer) {
         FreeResource (resInstance);
         return nullptr;
      }
      IStream *stream = nullptr;

      if (auto holder = GlobalLock (buffer)) {
         memcpy (holder, resData, resSize);

         if (CreateStreamOnHGlobal (buffer, false, &stream) == S_OK) {
            auto bm = new Bitmap (stream, PixelFormat32bppARGB);
            stream->Release ();

            bm->GetHICON (&result);
           
            return result;
         }
         GlobalUnlock (buffer);
      }
      GlobalFree (buffer);

      UnlockResource (resInstance);
      FreeResource (resInstance);

      return nullptr;
   }


   static DWORD CR_STDCALL thread (CONST LPVOID lpParam) {
      auto &inst = BotSetup::instance ();
      global.install = true;

      EnableWindow (GetDlgItem (global.hwnd, IDOK), FALSE);
      SetDlgItemTextA (global.hwnd, IDCANCEL, lang.tr (Lang::Cancel));

      ShowWindow (GetDlgItem (global.hwnd, IDC_BROWSE), SW_HIDE);
      ShowWindow (GetDlgItem (global.hwnd, IDC_EDIT2), SW_HIDE);

      ShowWindow (GetDlgItem (global.hwnd, IDC_STATUS), SW_SHOW);
      ShowWindow (GetDlgItem (global.hwnd, IDC_PROGRESS1), SW_SHOW);

      global.target.trim ();

      if (inst.install (global.target, BotSetup::progress)) {
         SetDlgItemTextA (global.hwnd, IDCANCEL, lang.tr (Lang::Exit));
         SetDlgItemTextA (global.hwnd, IDC_STATUS, lang.tr (Lang::Done));

         auto pb = GetDlgItem (global.hwnd, IDC_PROGRESS1);

         SetWindowLongA (pb, GWL_STYLE, GetWindowLongA (pb, GWL_STYLE) | PBS_MARQUEE);

         SendMessageA (pb, (UINT) PBM_SETMARQUEE, (WPARAM) 1, (LPARAM) nullptr);
         UpdateWindow (pb);
      }
      else {
         EnableWindow (GetDlgItem (global.hwnd, IDOK), TRUE);

         ShowWindow (GetDlgItem (global.hwnd, IDC_BROWSE), SW_SHOW);
         ShowWindow (GetDlgItem (global.hwnd, IDC_EDIT2), SW_SHOW);

         ShowWindow (GetDlgItem (global.hwnd, IDC_PROGRESS1), SW_HIDE);
         MessageBoxA (global.hwnd, lang.tr (Lang::BadGameDirectory), lang.tr (Lang::Application), MB_TOPMOST | MB_ICONEXCLAMATION);
      }

      global.install = false;
      ExitThread (EXIT_SUCCESS);
   }

   void addTooltip (char *text, int32 id) {
      TOOLINFO ti {};
      ti.cbSize = sizeof (TOOLINFO);
      ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
      ti.hwnd = global.hwnd;
      ti.uId = (WPARAM)GetDlgItem (global.hwnd, id);
      ti.lpszText = text;

      SendMessageA (global.tip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
   };

   static void detectSetupDirectory (void) {
      auto &inst = BotSetup::instance ();

      HKEY key;
      auto data = strings.chars ();

      if (RegOpenKeyExA (HKEY_CURRENT_USER, "software\\valve\\steam", 0, KEY_READ, &key) == ERROR_SUCCESS) {
         DWORD length = 255;
         DWORD type = 0;

         if ((RegQueryValueExA (key, "SteamPath", nullptr, &type, reinterpret_cast <LPBYTE> (data), &length) != ERROR_SUCCESS) || (type != REG_SZ && type != REG_EXPAND_SZ)) {
            data[255] = kNullChar;
         }
         RegCloseKey (key);
      }

      auto applyDir = [&inst] (StringRef data, StringRef exe) {
         String res = strings.format ("%s/steamapps/common/Half-Life/%s.exe", data, exe);

         if (!File::exists (res)) {
            return false;
         }
         auto pos = res.findLastOf ("\\/");

         if (pos != String::InvalidIndex) {
            res = res.substr (0, pos);
         }
         res.replace ("/", "\\");

         if (inst.dirExists (res)) {
            global.target = res;

            SetDlgItemTextA (global.hwnd, IDOK, lang.tr (Lang::Install));
            SetDlgItemTextA (global.hwnd, IDC_EDIT2, inst.shrinkPath (res.lowercase (), kShrinkPathAmount).chars ());

            inst.addTooltip (const_cast <char *> (res.lowercase ().chars ()), IDC_EDIT2);

            EnableWindow (GetDlgItem (global.hwnd, IDOK), TRUE);
            return true;
         }
         return false;
      };

      for (const auto &exe : StringArray { "hl", "hlds", "xash", "xash_vc6", "xash_sdl", "cs", "cstrike" }) {
         if (applyDir (data, exe)) {
            break;
         }
      }
   }
};

class UrlWrap : public Singleton <UrlWrap> {
public:
   using Callback = Lambda <void ()>;

private:
   HashMap <int32, Callback> m_urls;
   LOGFONT lf {};
   HCURSOR m_hand;

public:
   UrlWrap () {
      GetObjectA (GetStockObject (DEFAULT_GUI_FONT), sizeof (LOGFONT), &lf);
      m_hand = LoadCursorA (nullptr, IDC_HAND);
   }

   void handleMouseMove (WPARAM w, LPARAM l) {
      m_urls.foreach ([&] (const int32 &key, const auto &) {
         auto hwnd = GetDlgItem (global.hwnd, key);

         if (ChildWindowFromPoint (global.hwnd, { LOWORD (l), HIWORD (l) }) == hwnd) {
            SetCursor (m_hand);
         }
      });
   }

   void handleMouseDown (WPARAM w, LPARAM l) {
      m_urls.foreach ([&] (const int32 &key, const Callback &cb) {
         auto hwnd = GetDlgItem (global.hwnd, key);

         if (ChildWindowFromPoint (global.hwnd, { LOWORD (l), HIWORD (l) }) == hwnd) {
            const_cast <Callback &> (cb) ();
         }
      });
   }

   bool handleColorStatic (HWND hwndDialog, WPARAM w) {
      bool handle = false;

      m_urls.foreach ([&] (const int32 &key, const auto &) {
         auto hwnd = GetDlgItem (global.hwnd, key);

         if (hwnd != hwndDialog) {
            return;
         }
         handle = true;

         SetBkMode (HDC (w), TRANSPARENT);
         SetTextColor (HDC (w), RGB (34, 0, 204));
      });

      return handle;
   }

   void push (int id, Callback action, int w = 8, int h = 4) {
      m_urls[id] = action;

      auto font = CreateFontA (-w, h,
         lf.lfEscapement, lf.lfOrientation, FW_DEMIBOLD,
         lf.lfItalic, 1, lf.lfStrikeOut, lf.lfCharSet,
         lf.lfOutPrecision, lf.lfClipPrecision, lf.lfQuality,
         lf.lfPitchAndFamily, lf.lfFaceName);

      SendMessageA (GetDlgItem (global.hwnd, id), WM_SETFONT, (WPARAM) font, 0);
   }
};

CR_EXPOSE_GLOBAL_SINGLETON (BotSetup, setup);
CR_EXPOSE_GLOBAL_SINGLETON (UrlWrap, urls);

LRESULT CR_STDCALL installerProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
   auto enableTransparency = [](HWND hwnd, BYTE opacity) {
      auto wl = GetWindowLongA (hwnd, GWL_EXSTYLE);

      if (opacity < 100) {
         if ((wl & WS_EX_LAYERED) == 0) {
            SetWindowLongA (hwnd, GWL_EXSTYLE, wl | WS_EX_LAYERED);
         }
         SetLayeredWindowAttributes (hwnd, 0, (255 * opacity) / 100, LWA_ALPHA);
         RedrawWindow (hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
      }
      else {
         if ((wl & WS_EX_LAYERED) != 0) {
            SetWindowLongA (hwnd, GWL_EXSTYLE, wl & ~WS_EX_LAYERED);
         }
         RedrawWindow (hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
      }
   };

   static HBRUSH whiteBrush = CreateSolidBrush (RGB (255, 255, 255));

   switch (message) {
   case WM_INITDIALOG: {
      global.hwnd = hwnd;

      global.tip = CreateWindowExA (WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, TTS_NOPREFIX | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwnd, NULL, global.inst, NULL);

      urls.push (IDC_BOTVER, [&] () {
         ShellExecuteA (hwnd, "open", "https://yapb.ru", nullptr, nullptr, SW_SHOWNORMAL);
      });
      global.mutex = CreateMutexA (nullptr, TRUE, "BotSetupMutexHandle");

      if (GetLastError () == ERROR_ALREADY_EXISTS) {
         PostQuitMessage (EXIT_FAILURE);
      }
     
      SetWindowTextA (hwnd, lang.tr (Lang::Application));

      LOGFONT lf;
      GetObjectA (GetStockObject (DEFAULT_GUI_FONT), sizeof (LOGFONT), &lf);

      auto fontSmall = CreateFontA (lf.lfHeight, 4,
         lf.lfEscapement, lf.lfOrientation, lf.lfWeight,
         lf.lfItalic, lf.lfUnderline, lf.lfStrikeOut, lf.lfCharSet,
         lf.lfOutPrecision, lf.lfClipPrecision, lf.lfQuality,
         lf.lfPitchAndFamily, lf.lfFaceName);

      SendMessageA (GetDlgItem (hwnd, IDC_EDIT2), WM_SETFONT, (WPARAM)fontSmall, 0);

      SetClassLongA (hwnd, GCL_HICON, (LONG)LoadIcon (global.inst, MAKEINTRESOURCE (IDI_ICON)));

      enableTransparency (hwnd, 75);
      SetDlgItemTextA (hwnd, IDC_BOTVER, strings.format (lang.tr (Lang::Version), setup.getBotVersion ()));

      SetDlgItemTextA (hwnd, IDC_BROWSE, lang.tr (Lang::Browse));
      SetDlgItemTextA (hwnd, IDCANCEL, lang.tr (Lang::Exit));
      SetDlgItemTextA (hwnd, IDOK, lang.tr (Lang::SelectExeFile));

      SetDlgItemTextA (hwnd, IDC_TEXT_INFO, lang.tr (Lang::InfoText));

      AppendMenuA (GetSystemMenu (hwnd, FALSE), MF_SEPARATOR, 0, nullptr);
      AppendMenuA (GetSystemMenu (hwnd, FALSE), MF_STRING, 0xDEADBEEF, lang.tr (Lang::LicenseInfo));

      SetTimer (hwnd, 1, 350, 0);

      setup.addTooltip (const_cast <char *> (lang.tr (Lang::SelectExeFile)), IDC_BROWSE);
      setup.addTooltip ("https://yapb.ru/", IDC_BOTVER);
   }
   break;

   case WM_TIMER: {
      KillTimer (hwnd, 1);
      BotSetup::detectSetupDirectory ();
   }
   break;

   case WM_ACTIVATEAPP:
   case WM_PAINT: {
      auto icon = setup.loadPNG (IDB_PNG1);

      if (icon) {
         SetLayeredWindowAttributes (hwnd, 255, 255, LWA_COLORKEY);
         DrawIconEx (GetDC (hwnd), 0, 0, icon, 0, 0, 0, NULL, DI_NORMAL | DI_IMAGE | DI_MASK);

         auto display = [&](uint32 id) {
            auto itm = GetDlgItem (hwnd, id);
            enableTransparency (itm, 100);
         };

         for (const auto &itm : IntArray{ IDCANCEL, IDOK, IDC_PROGRESS1, IDC_BROWSE, IDC_EDIT2, IDC_STATUS, IDC_BOTVER, IDC_TEXT_INFO }) {
            display (itm);
         }
      }
   }
   break;

   case WM_COMMAND:
      switch (LOWORD (wParam)) {

      case IDCANCEL:
         global.pause = true;

         if ((global.install && MessageBoxA (hwnd, lang.tr (lang.CancelCopy), lang.tr (Lang::Application), MB_ICONQUESTION | MB_YESNO) == IDYES) || !global.install) {
            PostQuitMessage (EXIT_SUCCESS);
         }
         global.pause = false;
         break;

      case IDOK:
         CreateThread (nullptr, 0, &BotSetup::thread, nullptr, 0, nullptr);
         break;

      case IDC_BROWSE: {
         OPENFILENAME ofn {};
         auto filename = strings.chars ();

         ofn.lStructSize = sizeof (ofn);
         ofn.hwndOwner = hwnd;
         ofn.lpstrFile = filename;
         ofn.lpstrFile[0] = kNullChar;
         ofn.nMaxFile = StringBuffer::StaticBufferSize;
         ofn.lpstrFilter = lang.tr (Lang::SelectHLExeFile);
         ofn.nFilterIndex = 1;
         ofn.lpstrFileTitle = nullptr;
         ofn.nMaxFileTitle = 0;
         ofn.lpstrInitialDir = nullptr;
         ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

         if (GetOpenFileNameA (&ofn) && File::exists (filename)) {
            String res = filename;

            auto pos = res.findLastOf ("\\/");

            if (pos != String::InvalidIndex) {
               res = res.substr (0, pos);
            }
            res.replace ("/", "\\");

            if (setup.dirExists (res)) {
               global.target = res;

               SetDlgItemTextA (global.hwnd, IDOK, lang.tr (Lang::Install));
               SetDlgItemTextA (hwnd, IDC_EDIT2, setup.shrinkPath (res.lowercase (), kShrinkPathAmount).chars ());

               setup.addTooltip (const_cast <char *> (res.lowercase ().chars ()), IDC_EDIT2);

               EnableWindow (GetDlgItem (hwnd, IDOK), TRUE);
            }
         }
         return true;
      }
      }
      break;

   case WM_MOUSEMOVE: 
      urls.handleMouseMove (wParam, lParam);
      break;

   case WM_LBUTTONDOWN:
      urls.handleMouseDown (wParam, lParam);
      break;

   case WM_CTLCOLORSTATIC:
      SetBkMode ((HDC)wParam, TRANSPARENT);
      SetTextColor (GetDC (HWND (lParam)), RGB (255, 255, 255));

      urls.handleColorStatic (HWND (lParam), wParam);

      if (HWND (lParam) == GetDlgItem (hwnd, IDC_STATUS)) {
         SetBkMode (HDC (wParam), TRANSPARENT);
         SetTextColor (HDC (wParam), RGB (34, 0, 204));
      }
      else if (HWND (lParam) == GetDlgItem (hwnd, IDC_EDIT2)) {
         SetBkMode ((HDC)wParam, TRANSPARENT);
         SetTextColor (GetDC (HWND (lParam)), RGB (255, 255, 255));
      }
      return (INT_PTR) whiteBrush;

   case WM_CLOSE:
   case WM_DESTROY:
      DeleteObject (whiteBrush);
      ReleaseMutex (global.mutex);
      CloseHandle (global.mutex);

      PostQuitMessage (0);
      ExitProcess (EXIT_SUCCESS);
      break;
   }

   return 0;
}

int CR_STDCALL WinMain (HINSTANCE instance, HINSTANCE, LPSTR, int) {
   lang.trySet ();

   GdiplusStartupInput gdiplusStartupInput;
   ULONG_PTR gdiplusToken;

   GdiplusStartup (&gdiplusToken, &gdiplusStartupInput, nullptr);

   INITCOMMONCONTROLSEX controls {
      sizeof (INITCOMMONCONTROLSEX),
      ICC_PROGRESS_CLASS
   };
   InitCommonControlsEx (&controls);

   global.inst = instance;
   global.hwnd = CreateDialogA (global.inst, MAKEINTRESOURCE (IDD_SETUP_DIALOG), nullptr,  (DLGPROC) installerProcedure);

   ShowWindow (global.hwnd,  SW_SHOW);

   Twin <BOOL, MSG> pump;

   while ((pump.first = GetMessageA (&pump.second, nullptr, 0, 0)) != 0) {
      if (pump.first == -1) {
         return -1;
      }

      if (!IsDialogMessageA (global.hwnd, &pump.second)) {
         TranslateMessage (&pump.second);
         DispatchMessageA (&pump.second);
      }
   }
   return EXIT_SUCCESS;
}

