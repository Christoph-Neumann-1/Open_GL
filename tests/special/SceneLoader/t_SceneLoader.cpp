#include <gtest/gtest.h>
#include <SceneLoader.hpp>

TEST(SCENE_LOADER, LoadAndUnload)
{
    GL::CallbackHandler cbh;
    GL::Window window(NULL, cbh.GetList(GL::CallbackType::OnWindowResize));
    GL::TimeInfo timeinfo(cbh);
    GL::SceneLoader loader(window, cbh, timeinfo);

    auto &preupdate = cbh.GetList(GL::CallbackType::PreUpdate);
    auto &prerender = cbh.GetList(GL::CallbackType::PreRender);

    loader.Load("./Scene.scene");
    preupdate();
    prerender();
    EXPECT_TRUE(loader.HasScene());

    loader.UnLoad();
    preupdate();
    prerender();
    EXPECT_FALSE(loader.HasScene());

    loader.UnLoad();
    preupdate();
    prerender();
    EXPECT_FALSE(loader.HasScene());

    loader.Load("./Scene.scene");
    preupdate();
    prerender();

    loader.Load("./Scene.scene");
    preupdate();
    prerender();
    EXPECT_TRUE(loader.HasScene());

    loader.UnLoad();
    preupdate();
    prerender();
    EXPECT_FALSE(loader.HasScene());
}

TEST(SCENE_LOADER, TestInvalidScene)
{
    GL::CallbackHandler cbh;
    GL::Window window(NULL, cbh.GetList(GL::CallbackType::OnWindowResize));
    GL::TimeInfo timeinfo(cbh);
    GL::SceneLoader loader(window, cbh, timeinfo);
    auto &preupdate = cbh.GetList(GL::CallbackType::PreUpdate);
    auto &prerender = cbh.GetList(GL::CallbackType::PreRender);

    try
    {
        loader.Load(".");
        FAIL();
    }
    catch (const GL::SceneLoader::InvalidScene &e)
    {
        loader.UnLoad();
        prerender();
        preupdate();
    }
}

TEST(SCENE_LOADER, TestFlags)
{
    GL::CallbackHandler cbh;
    GL::Window window(NULL, cbh.GetList(GL::CallbackType::OnWindowResize));
    GL::TimeInfo timeinfo(cbh);
    GL::SceneLoader loader(window, cbh, timeinfo);
    auto &preupdate = cbh.GetList(GL::CallbackType::PreUpdate);
    auto &prerender = cbh.GetList(GL::CallbackType::PreRender);

    loader.Load("./Scene.scene");
    preupdate();
    prerender();
    ASSERT_TRUE(loader.HasScene());
    EXPECT_TRUE(loader.GetFlag("_VALID_"));

    auto &flag = loader.GetFlag("Test");
    flag = 1;

    EXPECT_EQ(flag, 1);

    flag = 0;
    EXPECT_EQ(flag, 0);

    loader.UnLoad();
    prerender();
    preupdate();
    ASSERT_FALSE(loader.HasScene());
    EXPECT_FALSE(loader.GetFlag("_VALID_"));
}