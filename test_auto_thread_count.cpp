#include <iostream>
#include <boost/thread.hpp>

int main() {
    using namespace std;
    cout << boost::thread::hardware_concurrency() << endl;
}