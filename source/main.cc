#include <QCoreApplication>
#include <QApplication>
#include <QObject>

#include <iostream>
#include <thread>

#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif

#include <Windows.h>
#include <WinUser.h>

#include "qmainloop.hh"

#define DEFAULT_CONSOLE_VIS SW_HIDE // SW_HIDE and SW_SHOW Are acceptable.
#define BACK_BUTTON_CHAR 0x08

#define CONSOLE_WINDOW_WIDTH 500
#define CONSOLE_WINDOW_HEIGHT 200

bool KeyPressed(const uint16_t& keyboard_vkid) {
    return (GetAsyncKeyState(keyboard_vkid) & 0x8000) != 0;
}

void SetConsoleVisibility(const uint32_t& visibility) {
    static HWND console_window = GetConsoleWindow();
    ShowWindow(console_window, visibility);
    
    if(visibility == SW_SHOW) {
        SetForegroundWindow(console_window);
    }
}

void MonitorRControlWorker(int console_width, int console_height, bool fullscreen_overlay = false) {
    static HWND console_window = GetConsoleWindow();
    static uint8_t console_shown = DEFAULT_CONSOLE_VIS;

    for(;;Sleep(100)) {
        for(;!KeyPressed(0xA3);Sleep(50));
        for(;KeyPressed(0xA3);Sleep(50));

        for(int i = 0; i < 40; ++i) {
            if(KeyPressed(0xA3)) {
               if((console_shown ^ SW_SHOW) == SW_SHOW) {
                   HWND foreground_window = GetForegroundWindow();

                   if(foreground_window != console_window) {
                       /* Get the WINDOWPLACEMENT of the foreground window. The console WINDOWPLACEMENT
                        * will later be set to a modified version of this WINDOWPLACEMENT. */
                       WINDOWPLACEMENT foreground_window_placement;
                       foreground_window_placement.length = sizeof(foreground_window_placement);
                       GetWindowPlacement(foreground_window, &foreground_window_placement);

                       /* If the foreground window is maximized, manually get its coordinates using GetWindowRect,
                        * and replace WINDOWPLACEMENT::rcNormalPosition with the GetWindowRect coordinates.
                        * This is necessary because WinApi doesn't update rcNormalPosition for maximized windows,
                        * and using RC_MAXIMIZE on the console window causes strange behavior with multiple monitors
                        * where a window can be moved while staying maximized by dragging it to the top of another monitor. */
                       if(foreground_window_placement.showCmd == SW_MAXIMIZE) {
                           RECT foreground_window_rect;
                           GetWindowRect(foreground_window, &foreground_window_rect);
                           foreground_window_placement.rcNormalPosition = foreground_window_rect;
                       }

                       // Map the desktop position of the foreground window to its relative position.
                       RECT relative_position = foreground_window_placement.rcNormalPosition;
                       MapWindowPoints(HWND_DESKTOP, foreground_window, reinterpret_cast<POINT*>(&relative_position), 2);

                       if(!fullscreen_overlay) {
                           // Adjust the relative position to center the console window in the middle of the foreground window.
                           relative_position.left += (relative_position.right / 2) - CONSOLE_WINDOW_WIDTH;
                           relative_position.top += (relative_position.bottom / 2) - CONSOLE_WINDOW_HEIGHT;
                           relative_position.bottom = (relative_position.bottom / 2) + CONSOLE_WINDOW_HEIGHT;
                           relative_position.right = (relative_position.right / 2) + CONSOLE_WINDOW_WIDTH;
                       }

                       // Remap the relative position of the foreground window to its desktop position.
                       RECT adjusted_desktop_position = relative_position;
                       MapWindowPoints(foreground_window, HWND_DESKTOP, reinterpret_cast<POINT*>(&adjusted_desktop_position), 2);

                       // Replace the WINDOWPLACEMENT::rcNormalPosition for the foreground window with the adjuisted position.
                       foreground_window_placement.rcNormalPosition = adjusted_desktop_position;

                       // Remove any visibility flags from WINDOWPLACEMENT::showCmd
                       // Needed to stop the console window from maximizing.
                       foreground_window_placement.showCmd = NULL;

                       // Sets the window placement of the console window to the adjusted foreground window placement.
                       SetWindowPlacement(console_window, &foreground_window_placement);

                       SetForegroundWindow(console_window);
                   }
               }

               // Invert the console's visibility.
               SetConsoleVisibility(console_shown ^ SW_SHOW);
               console_shown ^= SW_SHOW;
               
               break;
           }

           Sleep(5);
        }
    }
}


constexpr const char* HELP_TEXT =
"--fullscreen-overlay (-f)      |    Overlay the console on the entire window, not the center.\n"
"--borderless (-b)              |    Disable the console window's titlebar.\n"
"--console-width (-cw)          |    Sets the center relative width of the console window, if fullscreen is disabled. Default: 500\n"
"--console-height (-ch)         |    Sets the center relative height of the console window, if fullscreen is disabled. Default: 200\n"
"--opacity (-o) [0-255]         |    Set the console opacity, valid range is 0 - 255.\n"
"--help (-h)                    |    This help message.\n";

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);

    int arg_console_width = 500;
    int arg_console_height = 200;
    
    bool arg_fullscreen_overlay = false;
    bool arg_borderless_console = false;
    uint8_t arg_opacity_factor = 200;

    // Parse command line arguments.
    for(int i = 0; i < argc; ++i) {
        const char* argument = argv[i];
        const char* next_argument = (i + 1) < argc ? argv[i + 1] : nullptr;

        if(!_stricmp(argument, "--help") || !_stricmp(argument, "-h")) {
            std::cout << HELP_TEXT << std::endl;
            return 0;
        } else if(!_stricmp(argument, "--fullscreen-overlay") || !_stricmp(argument, "-f")) {
            arg_fullscreen_overlay = true;
        } else if(!_stricmp(argument, "--borderless") || !_stricmp(argument, "-b")) {
            arg_borderless_console = true;
        }

        if(next_argument != nullptr) {
            bool is_num = std::all_of(&next_argument[0], &next_argument[strlen(next_argument) - 1], [](char character) -> bool {
                return isdigit(character);
            });

            if(is_num && (!_stricmp(argument, "--opacity") || !_stricmp(argument, "-o"))) {
                uint32_t opacity_factor = std::atoi(next_argument);

                if(opacity_factor <= 255) {
                    arg_opacity_factor = static_cast<uint8_t>(opacity_factor);
                } else {
                    std::cout << "Opacity out of range: " << opacity_factor << " (valid range 0 - 255), default " << arg_opacity_factor << std::endl;
                    std::cout << "Press any key to continue.." << std::endl;
                    _getch();
                }
            } else if(is_num && (!_stricmp(argument, "--console-width") || !_stricmp(argument, "-cw"))) {
                arg_console_width = std::atoi(next_argument);
            } else if(is_num && (!_stricmp(argument, "--console-height") || !_stricmp(argument, "-ch"))) {
                arg_console_height = std::atoi(next_argument);
            }
        }
    }

    static HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);

    SetConsoleScreenBufferSize(console_handle, {1000, 1000});

    static HWND console_window = GetConsoleWindow();

    if(arg_borderless_console) {
        SetWindowLong(console_window, GWL_STYLE, GetWindowLong(console_window, GWL_STYLE) & ~(WS_BORDER | WS_CAPTION | WS_THICKFRAME));
    }

    SetLayeredWindowAttributes(console_window, RGB(255, 255, 255), arg_opacity_factor, LWA_COLORKEY | LWA_ALPHA);

    SetConsoleVisibility(SW_HIDE);

    // Start thread that monitors rcontrol shortcut.
    const std::thread monitor_rcontrol_thread(MonitorRControlWorker, arg_console_width, arg_console_height, arg_fullscreen_overlay);

    // The main Qt program loop where UI and input takes place.
    QMainLoop main_program_loop;
    main_program_loop.run();
    
    QObject::connect(&main_program_loop, SIGNAL(quit()), &application, SLOT(quit()));

    return application.exec();
}
