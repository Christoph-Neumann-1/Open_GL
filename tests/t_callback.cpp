#include <gtest/gtest.h>
#include <Callback.hpp>

void CheckIfCalled(bool *called)
{
    *called = true;
}

void CountCalls(int *count)
{
    (*count)++;
}

void RemoveSelf(GL::CallbackList *ls, uint *id, bool *wascalled)
{
    ls->Remove(*id);
    *wascalled = true;
}

TEST(CALLBACK, TestSingleCallback)
{
    //Test adding calling and removing a single callback.
    using namespace GL;

    CallbackHandler handler;
    CallbackList &list = handler.GetList(CallbackType::Render);
    bool wascalled = false;
    uint callback_id = list.Add(std::bind(CheckIfCalled, &wascalled));
    list();
    EXPECT_TRUE(wascalled);

    wascalled = false;
    list.Remove(callback_id);
    list();
    EXPECT_FALSE(wascalled);
}

TEST(CALLBACK, TestMultipleCallbacks)
{
    //Test adding and removing multiple callbacks. ASSERT is used because a failure means later tests won't work.
    using namespace GL;

    CallbackHandler handler;
    CallbackList &list = handler.GetList(CallbackType::Render);
    uint id = handler.GenId();

    int count = 0;

    list.Add(std::bind(CountCalls, &count), id);
    list.Add(std::bind(CountCalls, &count), id);
    list.Add(std::bind(CountCalls, &count), id);
    list();
    ASSERT_EQ(count, 3);

    count = 0;
    list.RemoveAll(id);
    list();
    ASSERT_EQ(count, 0);

    count = 0;
    list.Add(std::bind(CountCalls, &count), id);
    list.Add(std::bind(CountCalls, &count), id);
    list.Add(std::bind(CountCalls, &count), id);
    handler.RemoveAll(id);
    list();
    ASSERT_EQ(count, 0);

    count = 0;
    uint id1 = list.Add(std::bind(CountCalls, &count), id);
    uint id2 = list.Add(std::bind(CountCalls, &count), id);
    uint id3 = list.Add(std::bind(CountCalls, &count), id);
    
    list.Remove(id1);
    list.Remove(id2);
    list.Remove(id3);
    list();
    ASSERT_EQ(count,0);
}

TEST(CALLBACK, RemoveSelfFromList)
{
    //Test removing a callback while it is being called.
    using namespace GL;

    CallbackHandler handler;
    CallbackList &list = handler.GetList(CallbackType::Render);
    bool wascalled = false;
    uint id = list.Add(std::bind(RemoveSelf, &list, &id, &wascalled));
    list();
    ASSERT_TRUE(wascalled);

    wascalled = false;
    list();
    ASSERT_FALSE(wascalled);
}