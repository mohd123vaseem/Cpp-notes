// ============================================================================
// WITHOUT pimpl — the "normal" class. Shows the two problems.
// ----------------------------------------------------------------------------
// In a real project this class would live in a HEADER (widget.h) that many
// files #include. Read the comments: everything private is EXPOSED in the
// header, and changing any private member forces every includer to recompile.
//
// Build & run:
//   g++ -std=c++23 -Wall -Wextra pimpl_without.cpp -o pimpl_without
//   .\pimpl_without
// ============================================================================

#include <iostream>
#include <string>
#include <vector>
using namespace std;

// ---- Imagine this whole class is in "widget.h" ----------------------------
class Widget {
public:
    Widget(string name) : name_(name), clicks_(0) { }

    void click() {
        clicks_++;
        history_.push_back("clicked");           // uses a private detail
    }
    void report() {
        cout << name_ << " was clicked " << clicks_ << " times\n";
    }

private:
    // ⚠️ PROBLEM 1 — LEAK: every one of these private details is written in
    //    the header, so anyone who #includes widget.h SEES them (the types,
    //    and must transitively #include <string>/<vector> too).
    string name_;
    int    clicks_;
    vector<string> history_;

    // ⚠️ PROBLEM 2 — RECOMPILATION CASCADE: if you add a member here
    //    (e.g. `double weight_;`) or change a type, the header changes, so
    //    EVERY .cpp that #includes widget.h must recompile — even though the
    //    public interface (click/report) never changed.
};
// ---------------------------------------------------------------------------

int main() {
    Widget w("Button");
    w.click();
    w.click();
    w.report();

    cout << "\nWITHOUT pimpl: private members (name_, clicks_, history_) are\n"
            "visible in the header, and changing any of them recompiles every\n"
            "file that includes it.\n";
}
