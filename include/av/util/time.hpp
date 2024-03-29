#ifndef AV_UTIL_TIME_HPP
#define AV_UTIL_TIME_HPP

#include <chrono>
#include <initializer_list>
#include <vector>

namespace av {
    class time_manager {
        using clock = std::chrono::high_resolution_clock;

        float delta_time;
        clock::time_point last_time;

        std::vector<float> times;

        public:
        time_manager(std::initializer_list<float> init = {0.0f}):
            delta_time(0.0f),
            last_time(clock::now()),
            times(init) {}

        ~time_manager() = default;

        inline void update(std::initializer_list<int> indices = {0}) {
            const clock::time_point &now = clock::now();
            delta_time = (now - last_time).count() / 1000000000.0f;

            for(int index : indices) times[index] += delta_time;

            last_time = std::move(now);
        }

        inline float get(int index = 0) const {
            return times[index];
        }

        inline float delta() const {
            return delta_time;
        }
    };
}

#endif // !AV_UTIL_TIME_HPP
