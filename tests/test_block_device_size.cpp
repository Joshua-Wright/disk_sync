#include <string>   // string
#include <iostream> // cout, cin, endl
#include "../lib/block_device_size.h"

/*
g++ -Wall -Wextra -std=gnu++11 ../lib/block_device_size.o test_block_device_size.cpp -o test_block_device_size
*/

int main(int argc, char const *argv[])
{
	if (argc < 2) return 1;
	using std::cout;
	using std::endl;
	using std::string;
	string input = argv[1];
	cout << input << endl;
	cout << block_device_size(input) << endl;
	return 0;
}