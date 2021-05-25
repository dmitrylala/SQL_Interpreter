#ifndef __TEST__
#define __TEST__


class Test
{
public:
    virtual ~Test();

    virtual void setup() = 0;
    virtual void execute(int verbose) = 0;
    virtual void teardown() = 0;
};

#endif
