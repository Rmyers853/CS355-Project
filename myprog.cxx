#include <cryptopp/cryptlib.h>
#include <cryptopp/osrng.h>
#include <cryptopp/integer.h>
#include <iostream>
#include <iomanip>

int main(int argc, char* argv[])
{
    using namespace CryptoPP;

    AutoSeededRandomPool prng;
    Integer x(prng, 64), y(prng, 64);

    std::cout << "x = " << std::hex << x << std::endl;
    std::cout << "y = " << std::hex << y << std::endl;
    std::cout << "x*y = " << std::hex << x*y << std::endl;

    return 0;
}
