#include <iostream>
#include <random>
#include <typeinfo>
int main()
{
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_real_distribution<double> dis(-0.5, 0.5); // define the range

    for(int n=0; n<40; ++n)
        std::cout << dis(gen) << ' '; // generate numbers
    std::cout << "\n\n";

    decltype(dis.param()) new_range (100.2, 500.0);
    dis.param(new_range);

    for(int n=0; n<40; ++n)
        std::cout << dis(gen) << ' '; // generate numbers
    std::cout << "\n\n";


    std::cout << typeid(dis(gen)).name() << ' ';

}