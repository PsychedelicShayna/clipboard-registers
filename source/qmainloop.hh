#ifndef QMAINLOOP_H_
#define QMAINLOOP_H_

#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif

#include <Windows.h>
#include <conio.h>

#include <QApplication>
#include <QObject>

#include <QWinEventNotifier>

#include <QClipboard>
#include <QMimeData>
#include <QImage>
#include <QTimer>
#include <QMap>

#include <iostream>
#include <string>

/* Struct that represents a "clipboard register"
 * groups clipboard information together. */
struct ClipboardRegister {
    QString Text;
    QImage Image;
    QMimeData* Mime;

    ClipboardRegister();
    ~ClipboardRegister();
};

/* This is the main class responsible for displaying the console
 * UI and interacting with user input. It's a timer based event loop
 * that is started via the run() method.
 *
 *
 *              --------------------------------------------------------------
 *              |                                                            ^
 *              |              -----------------------------                 ^
 *              |              |                           ^                 ^
 * run() -> mainLoop() -> restartTimer() -> timerTimeout() ^-> mainLoop() -> ^
 *
 *
 * run() causes mainLoop() to run once.
 * ->
 * restartTimer() is emitted when mainLoop() finishes execution.
 * ->
 * The kbhitPollTimer restarts when restarTimer() is emitted.
 * ->
 * When kbitPollTimer emits timeout(), timerTimeout() is run.
 * ->
 * timerTimeout() re-runs mainLoop() if there is keyboard input,
 * and just emitts restartTimer() if there isn't.
 *
 * */
class QMainLoop : public QObject {
private: Q_OBJECT
    QMap<char, ClipboardRegister> registerMap;
    QTimer kbhitPollTimer;

    // Copies the data within QMimeData from one instance to another.
    bool mimeCopy(const QMimeData* mime_from, QMimeData* mime_to);

    // Method that uses WinAPI to clear the console.
    void clearConsoleBuffer(char fill_character =  ' ');

private slots:
    /* This is where user input and text rendering takes place.
     * It runs every time timerTimeout() detects _kbhit(). When
     * this method is done, it emitts restartTimer(), which
     * triggers timerTimeout() when it times out. If timerTimeout()
     * does not call mainLoop() then it will emit restartTimer() itself. */
    void mainLoop();

    /* This method is responsible for calling mainLoop()
     * when _kbhit() is detected. It is connected to
     * kbhitPollTimer's timeout() signal, and will restart
     * its own timer if mainLoop() isn't called / no
     * keyboard input is detected. */
    void timerTimeout();

public:
    /* Calls mainLoop() once, which by proxy, starts the kbhitPollTimer
     * and renders the UI a single time without the need for keyboard input. */
    void run();

    QMainLoop(int kbhit_poll_timeout = 300);

signals:
    void quit();

    // Emitted by mainLoop() and timerTimeout(), restarts kbhitPolLTimer.
    void restartTimer();
};

#endif // QMAINLOOP_H_
