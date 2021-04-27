#include <gtest/gtest.h>
#include <ModuleLoader.hpp>

TEST(MODULE_LOADER, LoadAndCall)
{
    using namespace GL;

    ModuleLoader loader("./t_Module.module");
    auto add_func = loader.RETRIEVE(int, add, int, int);
    auto times3 = loader.RETRIEVE(float, times3, float);

    EXPECT_EQ(add_func(1, 2), 3);
    EXPECT_EQ(times3(2.0f), 6.0f);
}

TEST(MODULE_LOADER, UnLoad)
{
    using namespace GL;

    ModuleLoader loader("./t_Module.module");
    loader.UnLoad();
    loader.UnLoad();
}

TEST(MODULE_LOADER, LoadTwice)
{
    using namespace GL;

    ModuleLoader loader1("./t_Module.module");
    ModuleLoader loader2("./t_Module.module");

    auto add_func = loader1.RETRIEVE(int, add, int, int);
    add_func = loader2.RETRIEVE(int, add, int, int);

    loader1.UnLoad();

    add_func(1, 2);
}

TEST(MODULE_LOADER, InvalidModule)
{
    try
    {
        GL::ModuleLoader loader("./void");
        FAIL();
    }
    catch (std::runtime_error)
    {
    }
}

TEST(MODULE_LOADER, InvalidFunction)
{
    using namespace GL;

    ModuleLoader loader("./t_Module.module");
    try
    {
        auto func = loader.RETRIEVE(int, none, int, int);
        func(1,2);
        FAIL();
    }
    catch (std::runtime_error)
    {
    }
}
