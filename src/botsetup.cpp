//
// YaPB - Counter-Strike Bot based on PODBot by Markus Klinge.
// Copyright © 2004-2022 YaPB Project <yapb@jeefo.net>.
//
// SPDX-License-Identifier: MIT
// 

#define WIN32_LEAN_AND_MEAN

#undef UNICODE
#undef _UNICODE

#include <crlib/crlib.h>

using namespace cr;

#if !defined(CR_CXX_GCC)
#  pragma comment (lib, "comctl32.lib")
#  pragma comment (lib, "imagehlp.lib")
#  pragma comment (lib, "gdiplus.lib")
#  pragma comment (lib, "msimg32.lib")
#endif

#include "windows.h"
#include "windowsx.h"
#include "resource.h"

#include <stdlib.h>
#include <stdio.h>
#include <commctrl.h>
#include <shellapi.h>
#include <commdlg.h>
#include <winnt.h>

#include <objidl.h>
#include <gdiplus.h>

#include <unzip/unzip.h>

using namespace Gdiplus;

struct Static : public Singleton <Static> {
   String target;
   HINSTANCE inst = nullptr;
   HWND hwnd = nullptr;
   HWND tip = nullptr;
   HANDLE mutex = nullptr;

   bool install = false;
   bool pause = false;
   bool test = false;
   bool imageSet = false;

   wchar_t title[32];

public:
   WCHAR *m2s (const char *str) {
      constexpr auto MaxBuffers = 12;

      static wchar_t buf[MaxBuffers][1024];
      static int rtr = 0;

      static wchar_t *ret = buf[rtr];
      ret[0] = 0;

      if (++rtr > MaxBuffers - 1) {
         rtr = 0;
      }

      const auto len = strnlen (str, 1023);
      const auto chars = MultiByteToWideChar (CP_UTF8, 0, str, len, NULL, 0);

      MultiByteToWideChar (CP_UTF8, 0, str, len, ret, chars);
      ret[chars] = L'\0';

      return ret;
   }
};

CR_EXPOSE_GLOBAL_SINGLETON (Static, global);

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
      CannotPatchFiles,
      BadGameDirectory,
      CannotOpenArchive,
      InfoText,
      InfoText2
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
      m_data[Russian][FileDamaged] = "Установщик поврежден.\n\nПожалуйста получите новую копию установщика на:\n\n https://yapb.jeefo.net/latest/\n\nОшибка: %s";
      m_data[Russian][CopyingFiles] = "(%d/%d): %s%s";
      m_data[Russian][PatchingFiles] = "Правка Файлов";
      m_data[Russian][Exit] = "Выход";
      m_data[Russian][Done] = "OK. Нажмите Выход";
      m_data[Russian][LicenseInfo] = "YaPB Setup v2.0 (" __DATE__ " " __TIME__ ")";
      m_data[Russian][CancelCopy] = "Вы уверены, что хотите отменить установку ботов?";
      m_data[Russian][CannotPatchFiles] = "Невозможно записать изменения в liblist.gam/plugins.ini. Вам придется сделать это самостоятельно.";
      m_data[Russian][BadGameDirectory] = "В этой папке нету ничего похожего на CS... Попробуйте другую...";
      m_data[Russian][CannotOpenArchive] = "Невозможно открыть архив установки.";
      m_data[Russian][InfoText] =
         "Установка YaPB для CS.\r\n"
         "\r\n"
         "Выберите папку с игрой и нажмите установить.\r\n"
         "\r\n"
         "Удачных фрагов!\r\n";
      m_data[Russian][InfoText2] =
         "Удачных фрагов!\r\n"
         "\r\n"
         "Если у вас есть вопросы, обращайтесь.\r\n"
         "\r\n"
         "https://github.com/yapb/yapb\r\n";

      m_data[English][SelectExeFile] = "Select hl.exe";
      m_data[English][Install] = "Install";
      m_data[English][Browse] = "Browse";
      m_data[English][Version] = "YaPB v%s";
      m_data[English][Cancel] = "Cancel";
      m_data[English][Application] = "YaPB Setup";
      m_data[English][FileDamaged] = "Setup corrupted.\n\nPlease obtain new copy of installer at:\n\nhttps://yapb.jeefo.net/latest\n\nError: %s";
      m_data[English][CopyingFiles] = "(%d/%d): %s%s";
      m_data[English][PatchingFiles] = "Patching Files";
      m_data[English][Exit] = "Exit";
      m_data[English][Done] = "OK. Press Exit";
      m_data[English][LicenseInfo] = "YaPB Setup v2.0 (" __DATE__ " " __TIME__ ")";
      m_data[English][CancelCopy] = "Are you sure you want to cancel installation?";
      m_data[English][CannotPatchFiles] = "Unable to write changes to liblist.gam/plugins.ini. You have to modify these files yourself.";
      m_data[English][BadGameDirectory] = "There is no CS-compatible mods detected in this dir. Please select something better...";
      m_data[English][CannotOpenArchive] = "Unable to open installer archive.";
      m_data[English][InfoText] =
         "Welcome to YaPB Setup.\r\n"
         "\r\n"
         "Please select location of your game, then press install.\r\n"
         "\r\n"
         "Good luck! And have fun!\r\n";
      m_data[English][InfoText2] =
         "Have fun!\r\n"
         "\r\n"
         "Сontact us, if you're have any questions.\r\n"
         "\r\n"
         "https://github.com/yapb/yapb\r\n";
   }

public:
   Lang () : m_lang (English) {
      regLangTab ();
   }

public:
   decltype (auto) tr (const uint32 code) {
      return m_data[m_lang][code].chars ();
   }

   decltype (auto) trw (const uint32 code) {
      return global.m2s (tr (code));
   }

public:
   void startup () {
      auto legacy = [] () {
         return DWORD ((LOBYTE (LOWORD (GetVersion ())))) <= 5;
      };

      if (legacy ()) {
         return;
      }
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

         if (cmdline.contains ("/local")) {
            global.test = true;
         }
      }
      wcscpy (global.title, trw (Lang::Application));
   }

   uint32 getLang () const {
      return m_lang;
   }
};

CR_EXPOSE_GLOBAL_SINGLETON (Lang, lang);

class BotSetup : public Singleton <BotSetup> {
public:
   using SetupCallback = Lambda <void (StringRef, uint32)>;

private:
   HZIP m_hz;
   String m_botVersion;
   uint32 m_fileCount;

public:
   template <typename ...Args> void __declspec (noreturn) abort (Args &&...args) {
      MessageBoxW (GetActiveWindow (), global.m2s (strings.format (lang.tr (Lang::FileDamaged), args...)), global.title, MB_ICONSTOP | MB_TOPMOST);
      ExitProcess (EXIT_FAILURE);
   }

   template <typename ...Args> void warning (Args &&...args) {
      MessageBoxW (GetActiveWindow (), global.m2s (strings.format (args...)), global.title, MB_ICONWARNING | MB_TOPMOST);
   }


   template <typename ...Args> void error (Args &&...args) {
      MessageBoxW (GetActiveWindow (), global.m2s (strings.format (args...)), global.title, MB_ICONEXCLAMATION | MB_TOPMOST);
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
   BotSetup () : m_hz (nullptr) { }

   ~BotSetup () {
      CloseZipU (m_hz);
      m_hz = nullptr;
   }

   void startup () {
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

   void clearReadOnly (StringRef target) {
      SetFileAttributesA (target.chars (), GetFileAttributesA (target.chars ()) & ~(FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY));
   }

   void patchModFiles (StringRef target, StringRef mod) {
      auto liblist = strings.format ("%s\\%s\\liblist.gam", target, mod);

      // no liblist - no fun
      if (!File::exists (liblist)) {
         return;
      }
      auto plugins = strings.format ("%s\\%s\\addons\\metamod\\plugins.ini", target, mod);

      // load liblist into buffer, and check whether metamod is there (even if commented out currently)
      int32 size = 0;
      auto buffer = MemFileStorage::defaultLoad (liblist, &size);

      String liblistBuffer (reinterpret_cast <const char *> (buffer));
      MemFileStorage::defaultUnload (buffer);

      if (File::exists (plugins) && liblistBuffer.contains ("metamod")) {
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
            contents += "; See: https://yapb.jeefo.net/\n";
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
         return;
      }

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
         } while (FindNextFileA (handle, &search));
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

   int getFileCount () const {
      return m_fileCount;
   }

   const char *getBotVersion () {
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

   static void setText (const int item, const char *text) {
      SetDlgItemTextW (global.hwnd, item, global.m2s (text));
   }

public:
   static void progress (StringRef file, uint32 num) {
      const auto &inst = BotSetup::instance ();

      SendMessageW (GetDlgItem (global.hwnd, IDC_PROGRESS_BAR), PBM_SETRANGE, 0, MAKELPARAM (0, inst.getFileCount () + 1));
      SendMessageW (GetDlgItem (global.hwnd, IDC_PROGRESS_BAR), PBM_SETSTEP, 1, 0);
      SendMessageW (GetDlgItem (global.hwnd, IDC_PROGRESS_BAR), PBM_STEPIT, 0, 0);

      char dst[64];
      char ext[32];
      _splitpath (file.chars (), nullptr, nullptr, dst, ext);

      setText (IDC_STATUS, strings.format (lang.tr (Lang::CopyingFiles), num, inst.getFileCount (), dst, ext));
      Sleep (30);

      if (num == static_cast <uint32> (inst.getFileCount ())) {
         setText (IDC_STATUS, lang.tr (Lang::PatchingFiles));
         Sleep (800);
      }
   }

   static HICON loadPNG (uint32 resId) {
      static HICON result;

      if (result) {
         return result;
      }
      auto resHandle = FindResourceW (global.inst, MAKEINTRESOURCEW (resId), L"PNG");

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

   static DWORD CR_STDCALL thread (CONST LPVOID) {
      auto &inst = BotSetup::instance ();
      global.install = true;

      EnableWindow (GetDlgItem (global.hwnd, IDOK), FALSE);
      setText (IDCANCEL, lang.tr (Lang::Cancel));

      ShowWindow (GetDlgItem (global.hwnd, IDC_BROWSE), SW_HIDE);
      ShowWindow (GetDlgItem (global.hwnd, IDC_INSTALL_PATH), SW_HIDE);

      ShowWindow (GetDlgItem (global.hwnd, IDC_STATUS), SW_SHOW);
      ShowWindow (GetDlgItem (global.hwnd, IDC_PROGRESS_BAR), SW_SHOW);

      inst.setText (IDC_TEXT_INFO, lang.tr (Lang::InfoText2));

      global.target.trim ();

      if (inst.install (global.target, BotSetup::progress)) {
         setText (IDCANCEL, lang.tr (Lang::Exit));
         setText (IDC_STATUS, lang.tr (Lang::Done));

         auto pb = GetDlgItem (global.hwnd, IDC_PROGRESS_BAR);

         SetWindowLongW (pb, GWL_STYLE, GetWindowLongW (pb, GWL_STYLE) | PBS_MARQUEE);

         SendMessageW (pb, (UINT) PBM_SETMARQUEE, (WPARAM) 1, (LPARAM) nullptr);
         UpdateWindow (pb);
      }
      else {
         EnableWindow (GetDlgItem (global.hwnd, IDOK), TRUE);

         ShowWindow (GetDlgItem (global.hwnd, IDC_BROWSE), SW_SHOW);
         ShowWindow (GetDlgItem (global.hwnd, IDC_INSTALL_PATH), SW_SHOW);

         ShowWindow (GetDlgItem (global.hwnd, IDC_PROGRESS_BAR), SW_HIDE);

         inst.error (lang.tr (Lang::BadGameDirectory));
      }

      global.install = false;
      ExitThread (EXIT_SUCCESS);
   }

   void addTooltip (String text, int32 id) {
      TOOLINFO ti {};

      ti.cbSize = sizeof (TOOLINFO);
      ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
      ti.hwnd = global.hwnd;
      ti.uId = (WPARAM) GetDlgItem (global.hwnd, id);
      ti.lpszText = const_cast <char *> (text.chars ());
      
      SendMessageW (global.tip, TTM_ADDTOOL, 0, reinterpret_cast <LPARAM> (&ti));
   };

   static void detectSetupDirectory () {
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

            inst.setText (IDOK, lang.tr (Lang::Install));
            inst.setText (IDC_INSTALL_PATH, inst.shrinkPath (res.lowercase (), kShrinkPathAmount).chars ());

            inst.addTooltip (res.lowercase (), IDC_INSTALL_PATH);

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
   LOGFONTW lf {};
   HCURSOR m_hand;

public:
   UrlWrap () {
      GetObjectW (GetStockObject (DEFAULT_GUI_FONT), sizeof (LOGFONTW), &lf);

      m_hand = LoadCursorA (nullptr, IDC_HAND);
   }

   void handleMouseMove (WPARAM, LPARAM l) {
      m_urls.foreach ([&] (const int32 &key, const auto &) {
         auto hwnd = GetDlgItem (global.hwnd, key);

         if (ChildWindowFromPoint (global.hwnd, { LOWORD (l), HIWORD (l) }) == hwnd) {
            SetCursor (m_hand);
         }
      });
   }

   void handleMouseDown (WPARAM, LPARAM l) {
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
         SetTextColor (HDC (w), RGB (255, 21, 11));
      });

      return handle;
   }

   void push (int id, Callback action, int w = 8, int h = 4) {
      m_urls[id] = action;

      auto font = CreateFontW (-w, h,
                               lf.lfEscapement, lf.lfOrientation, FW_DEMIBOLD,
                               lf.lfItalic, 0, lf.lfStrikeOut, lf.lfCharSet,
                               lf.lfOutPrecision, lf.lfClipPrecision, ANTIALIASED_QUALITY,
                               lf.lfPitchAndFamily, lf.lfFaceName);

      SendMessageW (GetDlgItem (global.hwnd, id), WM_SETFONT, (WPARAM) font, 0);
   }
};

CR_EXPOSE_GLOBAL_SINGLETON (BotSetup, setup);
CR_EXPOSE_GLOBAL_SINGLETON (UrlWrap, urls);

LRESULT CR_STDCALL installerProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
   auto enableTransparency = [&] (HWND hwnd, BYTE opacity) {
      auto wl = GetWindowLongW (hwnd, GWL_EXSTYLE);

      if (opacity < 100) {
         if ((wl & WS_EX_LAYERED) == 0) {
            SetWindowLongW (hwnd, GWL_EXSTYLE, wl | WS_EX_LAYERED);
         }
         SetLayeredWindowAttributes (hwnd, 0, (255 * opacity) / 100, LWA_ALPHA);
         RedrawWindow (hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
      }
      else {
         if ((wl & WS_EX_LAYERED) != 0) {
            SetWindowLongW (hwnd, GWL_EXSTYLE, wl & ~WS_EX_LAYERED);
         }
         RedrawWindow (hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
      }
   };

   static HBRUSH whiteBrush = CreateSolidBrush (RGB (255, 255, 255));

   switch (message) {
   case WM_INITDIALOG:
   {
      global.hwnd = hwnd;
      global.tip = CreateWindowExW (WS_EX_TOPMOST, TOOLTIPS_CLASSW, NULL, TTS_NOPREFIX | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwnd, NULL, global.inst, NULL);

      urls.push (IDC_BOTVER, [&] () {
         ShellExecuteW (hwnd, L"open", L"https://yapb.jeefo.net", nullptr, nullptr, SW_SHOWNORMAL);
      });
      global.mutex = CreateMutexW (nullptr, TRUE, L"YaPBSetupMutexHandle");

      if (GetLastError () == ERROR_ALREADY_EXISTS) {
         PostQuitMessage (EXIT_FAILURE);
      }
      SetWindowTextW (hwnd, global.title);

      LOGFONTW lf;
      GetObjectW (GetStockObject (DEFAULT_GUI_FONT), sizeof (LOGFONTW), &lf);

      auto fontSmall = CreateFontW (lf.lfHeight, 4,
                                    lf.lfEscapement, lf.lfOrientation, lf.lfWeight,
                                    lf.lfItalic, lf.lfUnderline, lf.lfStrikeOut, lf.lfCharSet,
                                    lf.lfOutPrecision, lf.lfClipPrecision, lf.lfQuality,
                                    lf.lfPitchAndFamily, lf.lfFaceName);

      auto fontDialog = CreateFontW (8, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET,
                                     OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                     DEFAULT_PITCH | FF_DONTCARE, lf.lfFaceName);

      SendMessageW (GetDlgItem (hwnd, IDC_INSTALL_PATH), WM_SETFONT, (WPARAM) fontSmall, 0);
      SendMessageW (hwnd, WM_SETFONT, (WPARAM) fontDialog, 0);

      SetClassLongW (hwnd, GCL_HICON, (LONG) LoadIconW (global.inst, MAKEINTRESOURCEW (IDI_ICON)));

      enableTransparency (hwnd, 70);
      setup.setText (IDC_BOTVER, strings.format (lang.tr (Lang::Version), setup.getBotVersion ()));

      setup.setText (IDC_BROWSE, lang.tr (Lang::Browse));
      setup.setText (IDCANCEL, lang.tr (Lang::Exit));
      setup.setText (IDOK, lang.tr (Lang::SelectExeFile));

      setup.setText (IDC_TEXT_INFO, lang.tr (Lang::InfoText));

      AppendMenuW (GetSystemMenu (hwnd, FALSE), MF_SEPARATOR, 0, nullptr);
      AppendMenuW (GetSystemMenu (hwnd, FALSE), MF_STRING, 0xDEADBEEF, lang.trw (Lang::LicenseInfo));

      SetTimer (hwnd, 1, 350, 0);
   }
   break;

   case WM_TIMER:
      KillTimer (hwnd, 1);
      BotSetup::detectSetupDirectory ();
   break;

   case WM_ACTIVATEAPP:
   case WM_PAINT:
   {
      auto icon = setup.loadPNG (IDB_LOGO);

      if (icon) {
         SetLayeredWindowAttributes (hwnd, 255, 255, LWA_COLORKEY);
         DrawIconEx (GetDC (hwnd), 0, 0, icon, 0, 0, 0, NULL, DI_NORMAL | DI_IMAGE | DI_MASK);

         auto display = [&] (uint32 id) {
            auto itm = GetDlgItem (hwnd, id);
            enableTransparency (itm, 100);
         };

         for (const auto &itm : IntArray { IDCANCEL, IDOK, IDC_PROGRESS_BAR, IDC_BROWSE, IDC_INSTALL_PATH, IDC_STATUS, IDC_BOTVER, IDC_TEXT_INFO }) {
            display (itm);
         }
      }
   }
   break;

   case WM_COMMAND:
      switch (LOWORD (wParam)) {
      case IDCANCEL:
         global.pause = true;

         if ((global.install && MessageBoxW (hwnd, lang.trw (lang.CancelCopy), global.title, MB_ICONQUESTION | MB_YESNO) == IDYES) || !global.install) {
            PostQuitMessage (EXIT_SUCCESS);
         }
         global.pause = false;
         break;

      case IDOK:
         CreateThread (nullptr, 0, &BotSetup::thread, nullptr, 0, nullptr);
         break;

      case IDC_BROWSE:
      {
         OPENFILENAME ofn {};
         auto filename = strings.chars ();

         ofn.lStructSize = sizeof (ofn);
         ofn.hwndOwner = hwnd;
         ofn.lpstrFile = filename;
         ofn.lpstrFile[0] = kNullChar;
         ofn.nMaxFile = StringBuffer::StaticBufferSize;
         ofn.lpstrFilter = "Half-Life EXE (hl.exe/hlds.exe)\0hl.exe;hlds.exe;xash.exe;xash_vc.exe;xash_mingw.exe\0";
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

               setup.setText (IDOK, lang.tr (Lang::Install));
               setup.setText (IDC_INSTALL_PATH, setup.shrinkPath (res.lowercase (), kShrinkPathAmount).chars ());

               setup.addTooltip (res.lowercase (), IDC_INSTALL_PATH);

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
      SetBkMode ((HDC) wParam, TRANSPARENT);
      SetTextColor (GetDC (HWND (lParam)), RGB (255, 255, 255));

      urls.handleColorStatic (HWND (lParam), wParam);

      if (HWND (lParam) == GetDlgItem (hwnd, IDC_STATUS)) {
         SetBkMode (HDC (wParam), TRANSPARENT);
         SetTextColor (HDC (wParam), RGB (34, 0, 204));
      }
      else if (HWND (lParam) == GetDlgItem (hwnd, IDC_INSTALL_PATH)) {
         SetBkMode ((HDC) wParam, TRANSPARENT);
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
   lang.startup ();
   setup.startup ();

   GdiplusStartupInput gdiplusStartupInput;
   ULONG_PTR gdiplusToken;

   GdiplusStartup (&gdiplusToken, &gdiplusStartupInput, nullptr);

   INITCOMMONCONTROLSEX controls {
      sizeof (INITCOMMONCONTROLSEX),
      ICC_PROGRESS_CLASS
   };
   InitCommonControlsEx (&controls);

   global.inst = instance;
   global.hwnd = CreateDialogW (global.inst, MAKEINTRESOURCEW (IDD_SETUP_DIALOG), nullptr, (DLGPROC) installerProcedure);

   ShowWindow (global.hwnd, SW_SHOW);

   Twin <BOOL, MSG> pump;

   while ((pump.first = GetMessageW (&pump.second, nullptr, 0, 0)) != 0) {
      if (pump.first == -1) {
         return -1;
      }

      if (!IsDialogMessageW (global.hwnd, &pump.second)) {
         TranslateMessage (&pump.second);
         DispatchMessageW (&pump.second);
      }
   }
   return EXIT_SUCCESS;
}

