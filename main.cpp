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

    const char* wavPath = "input.wav";
    const char* mfcPath = "output.mfc";

    // Check if input is readable
    wavFp.open(wavPath);
    if (!wavFp.is_open()) {
        std::cerr << "Unable to open input file: " << wavPath << std::endl;
    }

    // Check if output is writable
    mfcFp.open(mfcPath);
    if (!mfcFp.is_open()) {
        std::cerr << "Unable to open output file: " << mfcPath << std::endl;
        wavFp.close();
    }

    return app.exec();
}
