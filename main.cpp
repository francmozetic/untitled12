#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <fstream>
#include <iostream>

#include <widget.h>

/* ### What is Type Erasure? – Arthur O'Dwyer – Stuff mostly about C++
 * We need to move the object into a region of storage where we can control its lifetime via explicit delete or
 * placement-destruction syntax. The by-far easiest way to do that is to heap-allocate our WrappingCallback.
 */

struct AbstractCallback {
    virtual int call(int) const = 0;
    virtual ~AbstractCallback() = default;
};

template<class T>
struct WrappingCallback : AbstractCallback {
    explicit WrappingCallback(T&& cb) : cb_(std::move(cb)) {} // explicit move constructor...
    int call(int x) const override {
        return cb_(x);
    }

    T cb_;
};

struct Callback {
    std::unique_ptr<AbstractCallback> ptr_;

    // Instances of type Callback can be created with arbitrary types because it has a generic constructor.
    template<class T>
    Callback(T t) : ptr_(std::make_unique<WrappingCallback<T>>(std::move(t))) {} // to je pomembno...
    /*
    Callback(T t) {
        ptr_ = std::make_unique<WrappingCallback<T>>(std::move(t)); // to je pomembno...
    }
    */
    int operator()(int x) const { // call operator...
        return ptr_->call(x);
    }
};

int run_once(const Callback& callback) {
    std::cout << "is_abstract<AbstractCallback>: "
              << std::is_abstract<AbstractCallback>::value
              << '\n';
    std::cout << "is_polymorphic<AbstractCallback>: "
              << std::is_polymorphic<AbstractCallback>::value
              << '\n';
    std::cout << "is_member_function_pointer<&AbstractCallback::call>: "
              << std::is_member_function_pointer<decltype(&AbstractCallback::call)>::value
              << '\n';
    std::cout << "is_copy_constructible<Callback>: "
              << std::is_copy_constructible<Callback>::value
              << '\n';
    std::cout << "is_move_constructible<Callback>: "
              << std::is_move_constructible<Callback>::value
              << '\n';
    return callback(10);
}
// _________________________________________________________________________________________________________________



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

    // function object (run lambda expression once)
    auto y = run_once([](int x){ return x; });
    std::cout << "run_once(lambda): " << y << std::endl;

    return app.exec();
}
