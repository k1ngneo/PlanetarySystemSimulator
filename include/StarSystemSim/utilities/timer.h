#pragma once

namespace utils {

    class Timer {
    public:
        float deltaTime = 0.0f;

        float measureTime();

    private:
        float lastTime = 0.0f;
    };

}