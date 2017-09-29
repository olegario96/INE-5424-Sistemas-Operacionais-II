#include <smart_data.h>

using namespace EPOS;

int main()
{
    Olegson *olegs = new Olegson(10, 10, Olegson::Mode::ADVERTISED);

    // Do not change below
    Thread::self()->suspend();

    return 0;
}
