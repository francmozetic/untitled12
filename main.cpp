#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <fstream>
#include <iostream>

#include <widget.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    // Initialise input and output streams
    std::ifstream wavFp;
    std::ofstream mfcFp;

    const char* wavPath = "partita.wav";
    const char* mfcPath = "output.mfc";

    std::cout << "Initialise input stream: " << wavPath << std::endl;
    std::cout << "Initialise output stream: " << mfcPath << std::endl;

    // Check if input is readable
    wavFp.open(wavPath);
    if (!wavFp.is_open()) {
        std::cout << "Unable to open input file: " << wavPath << std::endl;
    }

    // Check if output is writable
    mfcFp.open(mfcPath);
    if (!mfcFp.is_open()) {
        std::cout << "Unable to open output file: " << mfcPath << std::endl;
    }

    widget test;
    test.do_internal_work();

    wavFp.close();
    mfcFp.close();

    return app.exec();
}
