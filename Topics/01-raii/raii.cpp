// ============================================================================
// RAII — Resource Acquisition Is Initialization
// ----------------------------------------------------------------------------
// Goal of this file: SEE with your own eyes that a stack object's destructor
// runs automatically on every exit path — normal end, early return, AND when
// an exception is thrown. That automatic destructor call is the whole trick
// behind RAII.
//
// Build & run:
//   g++ -std=c++23 -Wall -Wextra raii.cpp -o raii && ./raii
// ============================================================================

#include <iostream>
#include <stdexcept>
#include <string>

// ----------------------------------------------------------------------------
// A toy "resource". Pretend this is a file handle, network socket, mutex lock,
// or heap allocation — anything that must be released exactly once.
// We just print on acquire (constructor) and release (destructor) so we can
// watch the lifetime.
// ----------------------------------------------------------------------------
class FileHandle {
public:
    explicit FileHandle(std::string name) : name_(std::move(name)) {
        std::cout << "  [acquire]  opened \"" << name_ << "\"\n";
    }

    // Destructor = release. This runs AUTOMATICALLY when the object dies.
    ~FileHandle() {
        std::cout << "  [release]  closed \"" << name_ << "\"\n";
    }

    void write(const std::string& msg) {
        std::cout << "  [use]      " << name_ << " <- " << msg << '\n';
    }

private:
    std::string name_;
};

// ----------------------------------------------------------------------------
// Demo 1: normal scope exit.
// ----------------------------------------------------------------------------
void demoNormal() {
    FileHandle f("data.txt");   // constructor runs here
    f.write("hello");
}                               // destructor runs here, automatically

// ----------------------------------------------------------------------------
// Demo 2: early return. We leave before the closing brace, but the destructor
// still runs as control exits the function.
// ----------------------------------------------------------------------------
void demoEarlyReturn(bool bail) {
    FileHandle f("config.txt");
    f.write("reading settings");

    if (bail) {
        std::cout << "  (bailing out early with return)\n";
        return;                 // <-- ~FileHandle runs right here, before return
    }

    f.write("done reading");
}

// ----------------------------------------------------------------------------
// Demo 3: the important one — exception. We never reach the end of the
// function, yet the destructor still runs during STACK UNWINDING as the
// exception propagates outward looking for a catch.
// ----------------------------------------------------------------------------
void demoException() {
    FileHandle f("transaction.log");
    f.write("begin transaction");

    throw std::runtime_error("disk full!");   // <-- ~FileHandle runs as the
                                               //     stack unwinds toward catch

    f.write("commit");   // never reached (unreachable — compiler may warn)
}

int main() {
    std::cout << "Demo 1 : normal scope exit:\n";
    demoNormal();

    std::cout << "\nDemo 2a : early return path:\n";
    demoEarlyReturn(true);

    std::cout << "\nDemo 2b : falls through to the end:\n";
    demoEarlyReturn(false);

    std::cout << "\nDemo 3 : exception thrown mid-function:\n";
    try {
        demoException();
    } catch (const std::exception& e) {
        std::cout << "  caught in main: " << e.what() << '\n';
    }

    std::cout << "\nNotice: every [acquire] has a matching [release], no\n"
                 "matter how we left the function. That is RAII.\n";
}
