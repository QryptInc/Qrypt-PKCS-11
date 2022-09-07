/**
 * This is the cpp file for the RandomGetter class.
*/

#include "RandomGetter.h"
#include <cstdio>   // remove
#include <iostream> //cout

namespace meteringclientlib
{
const char *RandomGetter::RANDOM_FILE_NAME = "random_output.txt";

void RandomGetter::removeRandomFile()
{
    // should add location info here.
    remove(RandomGetter::RANDOM_FILE_NAME);
}

RandomGetter::RandomGetter() = default;

RandomGetter::~RandomGetter() = default;
} // namespace meteringclientlib
