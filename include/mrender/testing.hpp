#pragma once

class Backend
{
public:
    static void init(const int width, const int height, void* nativeWindow, void* nativeDisplay = nullptr);
    static void shutdown();

	static void render();
};
