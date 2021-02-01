#include "qmainloop.hh"

ClipboardRegister::ClipboardRegister() {
    Mime = nullptr;
}

ClipboardRegister::~ClipboardRegister() {
    if(Mime != nullptr) {
        delete Mime;
    }
}

void QMainLoop::clearConsoleBuffer(char fill_character) {
    static HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(console_handle, &csbi);

    unsigned long written;

    FillConsoleOutputCharacter(console_handle, fill_character, csbi.dwSize.X * csbi.dwSize.Y, {0, 0}, &written);
    FillConsoleOutputAttribute(console_handle, csbi.wAttributes, csbi.dwSize.X * csbi.dwSize.Y, {0, 0}, &written);
    SetConsoleCursorPosition(console_handle, {0, 0});
}

bool QMainLoop::mimeCopy(const QMimeData* mime_from, QMimeData* mime_to) {
    bool data_set = false;

    for(QString format : mime_from->formats()) {
        QByteArray data = mime_from->data(format);

        if(format.startsWith("application/x-qt")) {
            int index_begin = format.indexOf('"') + 1;
            int index_end = format.indexOf('"', index_begin);

            format = format.mid(index_begin, index_end - index_begin);
        }

        mime_to->setData(format, data);
        data_set = true;
    }

    return data_set;
}

void QMainLoop::mainLoop() {
    const constexpr char valid_register_chars[46] {
        113, 119, 101, 114, 116, 121, 117, 105, 111, 112,
        108, 107, 106, 104, 103, 102, 100, 115, 97,  122,
        120, 99,  118, 98,  110, 109, 49,  50,  51,  52,
        53,  54,  55,  56,  57,  48,  49,  50,  51,  52,
        53,  54,  55,  56,  57,  48
    };

    if(registerMap.size() == 0) {
        for(const char& register_character : valid_register_chars) {
            registerMap[register_character] = ClipboardRegister();
        }
    }

    const auto lambda_draw_ui = [&] () {
        clearConsoleBuffer();

        for(const uint8_t& ch_register_name : registerMap.keys()) {
            const ClipboardRegister& cb_register = registerMap[ch_register_name];

            if(cb_register.Text.size() || cb_register.Image.sizeInBytes()) {
                std::cout << ch_register_name << " [" << (cb_register.Image.sizeInBytes() ? cb_register.Image.sizeInBytes() : cb_register.Text.size()) << " bytes]: ";
            }

            if(cb_register.Text.size()) {
                std::cout << QString(cb_register.Text).replace('\n', "\\n").replace('\r', "\\r").toStdString() << std::endl;
            } else if(cb_register.Image.sizeInBytes()) {
                std::cout << "<Image>" << std::endl;
            }
        }

        std::cout << std::endl << std::string(10, '-') << std::endl;
        std::cout << "(s)tore, (l)oad, (c)lear, e(x)it? " << std::endl;
    };
    
    lambda_draw_ui();

    char ch_operation = 0;
    if(_kbhit()) ch_operation = _getch();

    if(ch_operation == 's') {
        std::cout << "Store To Register: ";
        const char& ch_register = _getch();

        if(registerMap.count(ch_register)) {
            ClipboardRegister& cb_register = registerMap[ch_register];

            if(QApplication::clipboard()->text().size() || QApplication::clipboard()->image().sizeInBytes()) {
                if(cb_register.Mime != nullptr) delete cb_register.Mime;
                cb_register.Mime = new QMimeData;
                mimeCopy(QApplication::clipboard()->mimeData(), cb_register.Mime);
            }

            if(QApplication::clipboard()->text().size()) {
                cb_register.Text = QApplication::clipboard()->text();
                cb_register.Image = QImage();

                std::cout << std::endl << " Stored Text In Register '" << ch_register << '\'' << std::endl;
                Sleep(200);
            } else if(QApplication::clipboard()->image().sizeInBytes()) {
                cb_register.Image = QApplication::clipboard()->image();
                cb_register.Text = QString();

                std::cout << std::endl << " Stored Image In Register '" << ch_register << '\'' << std::endl;
                Sleep(200);
            } else {

                std::cout << std::endl << " No Content To Copy" << std::endl;
                Sleep(200);
            }
        } else {
            std::cout << std::endl << " Invalid Register '" << ch_register << '\'' << std::endl;
            std::cin.get();
        }
    } else if(ch_operation == 'l') {
        std::cout << "Load From Register: ";
        const char& ch_register = _getch();

        if(registerMap.count(ch_register)) {
            ClipboardRegister& cb_register = registerMap[ch_register];

            if(cb_register.Text.size() || cb_register.Image.sizeInBytes()) {
                QMimeData* clone = new QMimeData;
                mimeCopy(cb_register.Mime, clone);
                QApplication::clipboard()->setMimeData(cb_register.Mime);
                cb_register.Mime = clone;
            }

            if(cb_register.Text.size()) {
                QApplication::clipboard()->setText(cb_register.Text);

                std::cout << " Loaded Text From Register '" << ch_register << '\'' << std::endl;
                Sleep(200);
            } else if(cb_register.Image.sizeInBytes()) {
                QApplication::clipboard()->setImage(cb_register.Image);
                std::cout << " Loaded Image From Register '" << ch_register << '\'' << std::endl;
                Sleep(200);
            } else {
                std::cout << std::endl << " No Content In Register" << std::endl;
                Sleep(200);
            }
        } else {
            std::cout << std::endl << " Invalid Register '" << ch_register << '\'' << std::endl;
            std::cin.get();
        }
    } else if(ch_operation == 'c') {
        for(const auto& cb_register_name : registerMap.keys()) {
            registerMap[cb_register_name] = ClipboardRegister();
        }

        std::cout << " Cleared Registers" << std::endl;
        Sleep(100);
    } else if(ch_operation == 'x') {
        quit();
    }

    lambda_draw_ui();

    emit restartTimer();
}

void QMainLoop::timerTimeout() {
    kbhitPollTimer.stop();

    if(_kbhit()) {
        mainLoop();
    } else {
        emit restartTimer();
    }
}

void QMainLoop::run() {
    mainLoop();
}

QMainLoop::QMainLoop(int kbhit_poll_timeout) {
    kbhitPollTimer.setInterval(kbhit_poll_timeout);

    connect(this, SIGNAL(restartTimer()), &kbhitPollTimer, SLOT(start()));
    connect(&kbhitPollTimer, SIGNAL(timeout()), this, SLOT(timerTimeout()));
}
