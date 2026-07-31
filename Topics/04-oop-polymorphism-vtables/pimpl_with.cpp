// ============================================================================
// WITH pimpl — the fix. The header shows NOTHING about the internals.
// ----------------------------------------------------------------------------
// Normally split across widget.h + widget.cpp. Here it's one file so it runs,
// but the comments mark which part would be the HEADER vs the .cpp, so you can
// see that the header exposes only a forward declaration + one pointer.
//
// Build & run:
//   g++ -std=c++23 -Wall -Wextra pimpl_with.cpp -o pimpl_with
//   .\pimpl_with
// ============================================================================

#include <iostream>
#include <string>
#include <vector>
#include <memory>
using namespace std;

// ===================== would be "widget.h" (PUBLIC) ========================
class Widget {
public:
    Widget(string name);
    ~Widget();                       // declared here, DEFINED in the .cpp part
    void click();
    void report();

private:
    class Impl;                      // ✅ forward declaration ONLY — no details
    unique_ptr<Impl> pImpl;          // ✅ the single pointer to implementation
    // Notice: NO name_, NO clicks_, NO vector here. The header leaks nothing,
    // and it doesn't even need to #include <string>/<vector>.
};
// ===========================================================================


// ===================== would be "widget.cpp" (HIDDEN) ======================
// ALL the real details live here, invisible to anyone including the header.
class Widget::Impl {
public:
    string name_;
    int    clicks_ = 0;
    vector<string> history_;         // add/remove members here → only THIS
                                     // file recompiles (compilation firewall)

    Impl(string name) : name_(name) { }
    void click()  { clicks_++; history_.push_back("clicked"); }
    void report() { cout << name_ << " was clicked " << clicks_ << " times\n"; }
};

Widget::Widget(string name) : pImpl(make_unique<Impl>(name)) { }
Widget::~Widget() = default;         // ✅ defined HERE, where Impl is complete
                                     //    (unique_ptr needs the full Impl to destroy it)
void Widget::click()  { pImpl->click(); }
void Widget::report() { pImpl->report(); }
// ===========================================================================

int main() {
    Widget w("Button");
    w.click();
    w.click();
    w.report();

    cout << "\nWITH pimpl: the header shows only `class Impl;` + a unique_ptr.\n"
            "All internals live in the .cpp -> nothing leaks, and changing them\n"
            "recompiles ONLY the .cpp, not every file that includes the header.\n";
}
