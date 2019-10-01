#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <chrono>
#include <fstream>
#include <iostream>

#include <widget.h>

/* ## Type Erasure with Templates from jsmith cplusplus.com article (2010 jsmith)
 * Instances of type Object can be created with arbitrary types because it has a generic constructor.
 */
class Wild {
public:
    std::string getName() const {
        return "Wild";
    }
    double implementation(double param) const {
        return (param * param) / 2.5;
    }
};

class Running {
public:
    std::string getName() const {
        return "Running";
    }
    double implementation(double param) const {
        return (param * param) / 2.5;
    }
};

class Homeless {
public:
    std::string getName() const {
        return "Homeless";
    }
    double implementation(double param) const {
        return (param * param) / 2.5;
    }
};

class Object {
public:
    template<typename T>
    Object(const T& obj) : object(std::make_shared<ObjectModel<T>>(std::move(obj))) {} // let's heap-allocate the ObjectModel...
    /* Object(const T& obj) : object(new ObjectModel<T>(obj)) {}
     * Object(const T& obj) : object(std::make_shared<ObjectModel<T>>(obj)) {}
     */
    std::string getName() const {
        return object->getName();
    }
    double implementation(double param) const {
        return object->implementation(param);
    }

    struct ObjectConcept {
        virtual ~ObjectConcept() {}
        virtual std::string getName() const = 0;
        virtual double implementation(double param) const = 0;
    };

    template<typename T>
    struct ObjectModel : ObjectConcept {
        ObjectModel(const T& obj) : object(std::move(obj)) {}
        virtual ~ObjectModel() {}
        std::string getName() const override {
            return object.getName();
        }
        double implementation(double param) const override {
            return object.implementation(param);
        }
    private:
        T object;
    };

private:
    std::shared_ptr<const ObjectConcept> object;
};
// _________________________________________________________________________________________________________________

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
    Callback(T t) : ptr_(std::make_unique<WrappingCallback<T>>(std::move(t))) {} // let's heap-allocate the WrappingCallback...
    /* Callback(T t) : ptr_(new WrappingCallback<T>(std::move(t))) {}
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
    std::cout << "is_reference<Callback&>: "
              << std::is_reference<Callback&>::value
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

    // ## type erasure with templates
    std::cout << "is_copy_constructible<Object>: " << std::is_copy_constructible<Object>::value << '\n';
    std::cout << "is_move_constructible<Object>: " << std::is_move_constructible<Object>::value << '\n';

    std::vector<Object> vec; // instances of type Object

    auto start = std::chrono::system_clock::now();
    for (int i=0; i<10000; ++i) {
        vec.emplace_back(Object(Wild()));
        //vec.emplace_back(Object(Running()));
        //vec.emplace_back(Object(Homeless()));
    }
    std::chrono::duration<double> duration = std::chrono::system_clock::now() - start;
    std::cout << "Time native: " << duration.count() << " seconds" << std::endl;
    /*
     * new
     * Time native: 0.006446
     * Time native: 0.006671
     * Time native: 0.006354
     * make_shared
     * Time native: 0.003793
     * Time native: 0.003761
     * Time native: 0.003992
     * std::move
     * Time native: 0.00386433 seconds
     * Time native: 0.00396666 seconds
     * Time native: 0.00367069 seconds
     * const
     * Time native: 0.00379101 seconds
     */

    for (auto v: vec) {
        std::cout << v.getName() << ": " << v.implementation(10.6) << std::endl;
    }
    // _________________________________________________________________________________________________________________

    // ### function object
    auto y = run_once([](int x){ return x; }); // run lambda expression once
    std::cout << "run_once(lambda): " << y << std::endl;
    // _________________________________________________________________________________________________________________

    return app.exec();
}
