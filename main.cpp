#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>

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

/* #### Polymorphic types
 * Runtime concept idiom allows polymorphism when needed, without inheritance. Polymorphic types are used like any
 * other types, including built-in types. This code is as efficient as base class mechanism. I don't have to wrap
 * integers, strings, ... into the objects.
 * There is a piece of code within which I want to deal with a set of types that share a particular attribute (they
 * might be drawable, ...). So I want to handle those objects as if they were the same. That's what we mean by
 * polymorphic type.
 */
void draw(const std::string& x, std::ostream& out, size_t position) {
    out << std::string(position, ' ') << x << std::endl;
}

void draw(const int& x, std::ostream& out, size_t position) {
    out << std::string(position, ' ') << x << std::endl;
}

void draw(const Wild&, std::ostream& out, size_t position) {
    out << std::string(position, ' ') << "Wild..." << std::endl;
}

class object_t {
public:
    object_t(std::string x) : self_(std::make_unique<string_model_t>(std::move(x))) {
        std::cout << "ctor" << std::endl;
    }
    object_t(int x) : self_(std::make_unique<int_model_t>(std::move(x))) {
        std::cout << "ctor" << std::endl;
    }
    object_t(Wild x) : self_(std::make_unique<wild_model_t>(std::move(x))) {
        std::cout << "ctor" << std::endl;
    }

    object_t(const object_t& x) : self_(x.self_->copy_()) {}
    object_t(object_t&&) noexcept = default;
    object_t& operator=(const object_t& x) { // copy assignment operator
        return *this = object_t(x);
    }
    object_t& operator=(object_t&&) noexcept = default; // move assignment operator

    friend void draw(const object_t& x, std::ostream& out, size_t position) {
        x.self_->draw_(out, position);
    }

private:
    struct concept_t { // a base class for implementation goes here...
        virtual ~concept_t() = default;
        virtual std::unique_ptr<concept_t> copy_() const = 0; // virtual copy_ function...
        virtual void draw_(std::ostream&, size_t) const = 0; // virtual draw_ function...
    };
    struct string_model_t final : concept_t {
        string_model_t(std::string x) : data_(std::move(x)) {}
        std::unique_ptr<concept_t> copy_() const override {
            return std::make_unique<string_model_t>(*this);
        }
        void draw_(std::ostream& out, size_t position) const override {
            draw(data_, out, position);
        }
        std::string data_;
    };
    struct int_model_t final : concept_t {
        int_model_t(int x) : data_(std::move(x)) {}
        std::unique_ptr<concept_t> copy_() const override {
            return std::make_unique<int_model_t>(*this);
        }
        void draw_(std::ostream& out, size_t position) const override {
            draw(data_, out, position);
        }
        int data_;
    };
    struct wild_model_t final : concept_t {
        wild_model_t(Wild x) : data_(std::move(x)) {}
        std::unique_ptr<concept_t> copy_() const override {
            return std::make_unique<wild_model_t>(*this);
        }
        void draw_(std::ostream& out, size_t position) const override {
            draw(data_, out, position);
        }
        Wild data_;
    };

    std::unique_ptr<concept_t> self_;
};

using document_t = std::vector<object_t>;

void draw(const document_t& x, std::ostream& out, size_t position) {
    out << std::string(position, ' ') << "<document>" << std::endl;
    for (const auto& e : x) draw(e, out, position);
    out << std::string(position, ' ') << "</document>" << std::endl;
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

    widget so; // creates a widget object in automatic storage
    widget test(so); // copy
    test.do_internal_work();

    /*
    widget so; // creates a widget object in automatic storage
    widget test;
    test = so; // copy assignment
    test.do_internal_work(); */

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

    // #### better code
    std::cout << "is_copy_constructible<object_t>: " << std::is_copy_constructible<object_t>::value << '\n';
    std::cout << "is_move_constructible<object_t>: " << std::is_move_constructible<object_t>::value << '\n';

    document_t document;
    document.reserve(10);

    document.emplace_back(0);
    document.emplace_back(std::string("Hello!"));
    document.emplace_back(2);
    document.emplace_back(Wild()); // creates a temporary object...

    auto start0 = std::chrono::system_clock::now();
    for (int i=0; i<10000; ++i) {
        std::reverse(document.begin(), document.end());
    }
    std::chrono::duration<double> duration0 = std::chrono::system_clock::now() - start0;
    std::cout << "Time native: " << duration0.count() << " seconds" << std::endl;
    /*
     * Time native: 0.000272 seconds
     * Time native: 0.000281 seconds
     * Time native: 0.000272 seconds
     */
    draw(document, std::cout, 0);

    return app.exec();
}
