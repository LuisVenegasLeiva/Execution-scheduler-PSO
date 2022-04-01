#include <iostream>
#include <chrono>
#include <thread>
#include <locale.h>
using namespace std;
void sleep(int ms)
{
    this_thread::sleep_for(static_cast<chrono::milliseconds>(ms));
}
int main()
{
    cout << "hola" << endl;
    int burst = 2000;
    int round = 4000;
    sleep(burst < round ? burst : round);
    cout << "adios!" << endl;
    return 0;
}