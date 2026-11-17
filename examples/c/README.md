This directory contains an example implementation in C++.

As of October 2026, the the two unicodedata-* files are generated
using Unicode 18.0.0, however, nothing in the specification is tied to
a specific Unicode version. I'll upgrade those two data files
occasionally.

The NFC check uses ICU, so building it needs something like this:

    g++ -std=c++17 smtputf8-syntax.cpp $(pkg-config --cflags --libs icu-uc)
