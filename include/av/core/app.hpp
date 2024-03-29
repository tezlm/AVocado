#ifndef AV_CORE_APP_HPP
#define AV_CORE_APP_HPP

#include <glad/glad.h>
#include <av/core/input.hpp>
#include <av/util/log.hpp>
#include <av/util/task_queue.hpp>
#include <av/util/time.hpp>

#include <SDL2/SDL.h>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

namespace av {
    class app;

    /** @brief An application configuration, for creation of SDL windows. */
    struct app_config {
        /** @brief Window title. */
        std::string title;
        /** @brief Window dimension. */
        int width = 800, height = 600;
        /** @brief Whether to turn on VSync at startup or not. */
        bool vsync;
        /** @brief Window flags. */
        bool shown = true, fullscreen, resizable;
    };

    /** @brief Defines an application listener. */
    class app_listener {
        friend class app;

        protected:
        /** @brief Called in the beginning of `app::loop()`. */
        virtual void init([[maybe_unused]] app &application) {}

        /** @brief Called every frame in `app::loop()`. */
        virtual void update([[maybe_unused]] app &application) {}

        /** @brief Called in the end of `app::loop()`. */
        virtual void dispose([[maybe_unused]] app &application) {}
    };

    /**
     * @brief A non copy-constructible class defining an application. Should only be instantiated once. Holds an SDL
     * window, an OpenGL context, and dynamic listeners.
     */
    class app {
        /** @brief Whether the application successfully initialized. Implicitly set in the constructor. */
        bool initialized;

        /** @brief The SDL window this application holds. Initialized in `init(const app_config &)`. */
        SDL_Window *window;
        /** @brief The OpenGL context this application holds. Initialized in `init(const app_config &)`. */
        SDL_GLContext context;
        /** @brief The application listeners. */
        std::vector<app_listener *> listeners;
        /** @brief Frame-delayed runnables submitted by `post(function<void(app &)> &&)`. */
        task_queue<void, app &> posts;
        /** @brief Whether the main loop in `loop()` should end. */
        bool exitting;

        /** @brief The SDL input event manager of the application. */
        input_manager input;
        /** @brief Global time manager of the application. */
        time_manager time;

        public:
        app(const app &) = delete; // Delete the copy-constructor.
        /**
         * @brief Instantiates and initializes an application. Add application listeners by invoking
         * `add_listener(app_listener *const &)`. Enter the main loop afterwards by calling `loop()`.
         * 
         * @param config The application configuration.
         */
        app(const app_config &config = {});
        /**
         * @brief Destroys the application. The OpenGL context the SDL window are destroyed here. The requirement for
         * the application listeners to not be destructed are released.
         */
        ~app();

        /** @return Whether the application was successfully instantiated. */
        inline bool has_initialized() const {
            return initialized;
        }

        /**
         * @brief Adds an arbitrary application listener. Typically invoked before `loop()`. Use with caution.
         * @param listener The pointer to the application listener. This is all yours and won't be destructed in any of
         *                 the application class' codes, though you must make sure it is not destructed until `loop()`
         *                 returns.
         */
        inline void add_listener(app_listener *const &listener) {
            listeners.push_back(listener);
        }
        /**
         * @brief Removes an application listener from the list. When called inside `loop()`, it will be removed in the
         * next frame. Note that this does not destroy the listener itself, it is your responsibility to do such.
         *
         * @param listener The pointer to the application listener.
         */
        inline void remove_listener(app_listener *const &listener) {
            post([&](app &) {
                listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
            });
        }

        /**
         * @brief Initializes the application loop. This function won't return until `exitting` is `true`.
         *
         * @return `true` if the main loop ended purely due to the application reaching exit state, `false` if the loop
         *         encountered exception(s).
         */
        bool loop();
        /**
         * @brief Stops `loop()` of the application. This can be called outside of `loop()` itself, but there really is
         * no sane reason to do such.
         */
        inline void exit() {
            exitting = true;
        }

        /** @return The SDL window this application holds. */
        inline SDL_Window *get_window() const {
            return window;
        }
        /** @return The OpenGL context this application holds. */
        inline SDL_GLContext get_context() const {
            return context;
        }
        /** @return The application's input manager. */
        inline input_manager &get_input() {
            return input;
        }
        /** @return The (read-only) application's input manager. */
        inline const input_manager &get_input() const {
            return input;
        }
        /** @return The application's time manager. */
        inline time_manager &get_time() {
            return time;
        }
        /** @return The (read-only) application's time manager. */
        inline const time_manager &get_time() const {
            return time;
        }

        /**
         * @brief Invokes a function on all the application listeners.
         *
         * @param T_func The function, in a signature of `void(app_listener &, app &)`.
         * @return `true` if all the function is successfully invoked to all the listeners, `false` if one or more of
         * the listeners threw an exception.
         */
        inline bool accept(const std::function<void(app_listener &, app &)> &acceptor) {
            try {
                for(app_listener *const &listener : listeners) acceptor(*listener, *this);
            } catch(std::exception &e) {
                log::msg<log_level::error>(e.what());
                return false;
            }

            return true;
        }

        /**
         * @brief Submits a function that will run at the end of the frame in `loop()`.
         *
         * @param function The lambda `void` function accepting `app &` parameter.
         */
        inline void post(const std::function<void(app &)> &function) {
            posts.submit(function);
        }

        protected:
        /** @brief Runs all functions submitted by `post(const function<void(app &)> &)` and clears the delayed run list. */
        inline void run_posts() {
            posts.run(*this);
        }
    };
}

#endif // !AV_CORE_APP_HPP
