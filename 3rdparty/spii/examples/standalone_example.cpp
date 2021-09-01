// Petter Strandmark 2013.
//
// When everything is built and installed correctly, it is possible
// to build this file with:
//
//     g++ -std=c++11 standalone_example.cpp -lspii
//
// (on Linux, that is. Windows uses Visual Studio.)
//

#include <iostream>

#include <spii/function.h>

int main()
{
	spii::Function function;
	std::cout << "OK.\n";
}
