#ifndef AV_CORE_APP_HPP
#define AV_CORE_APP_HPP

#include "glad/glad.h"
#include "util/task_queue.hpp"

#include <SDL.h>
#include <algorithm>
#include <functional>
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
        bool vsync = true;
        /** @brief Window flags. */
        bool shown = true, fullscreen, resizable;
    };

    /** @brief Defines an application listener. */
    struct app_listener {
        /** @brief Called in `app::init(const app_config &)`. */
        virtual void init(app &application) {}
        /** @brief Convenience function. */
        static inline void s_init(app_listener &listener, app &application) {
            listener.init(application);
        }

        /** Called in `app::loop()`. */
        virtual void update(app &application) {}
        /** @brief Convenience function. */
        static inline void s_update(app_listener &listener, app &application) {
            listener.update(application);
        }
    };
    
    /**
     * @brief A non copy-constructible class defining an application. Should only be instantiated once. Holds an SDL
     * window, an OpenGL context, and dynamic listeners.
     */
    class app {
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

        public:
        app(const app &) = delete; // Delete the copy-constructor.
        /**
         * @brief Instantiates an empty application. Calls to `init(const app_config &)` and `loop()` respectively are
         * required to run the application. Add application listeners by invoking `add_listener(app_listener *const &)`.
         */
        app();
        /**
         * @brief Destroys the application. The OpenGL context the SDL window are destroyed here. The requirement for
         * the application listeners to not be destructed are released.
         */
        ~app();

        /**
         * @brief Adds an arbitrary application listener. Typically invoked before `init()`. Use with caution.
         * @param listener - The pointer to the application listener. This is all yours and won't be destructed in any of
         *                   the application class' codes, though you must make sure it is not destructed until `loop()`
         *                   returns.
         */
        inline void add_listener(app_listener *const &listener) {
            listeners.push_back(listener);
        }
        /**
         * @brief Removes an application listener from the list. When called inside `loop()`, it will be removed in the
         * next frame. Note that this does not destroy the listener itself, it is your responsibility to do such.
         * @param listener - The pointer to the application listener.
         */
        inline void remove_listener(app_listener *const &listener) {
            post([=](app &) {
                listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
            });
        }

        /**
         * @brief Initializes the application, creating an SDL window and an OpenGL context.
         * @param config - The application configuration.
         * 
         * @return `true` if succeeded and both the window and context are successfully created, `false` otherwise.
         */
        bool init(const app_config &config);
        /**
         * @brief Initializes the application loop. `init()` must be successfully invoked prior to this function. This
         * function won't return until `exitting` is `true`.
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
        /** @return The (read-only) SDL window this application holds. */
        inline const SDL_Window *get_window() const {
            return window;
        }
        /** @return The (read-only) OpenGL context this application holds. */
        inline const SDL_GLContext get_context() const {
            return context;
        }

        /**
         * @brief Invokes a function on all the application listeners.
         * @param T_func - The function, should be in a signature of `T(app_listener &, app &, ...)`.
         *
         * @return `true` if all the function is successfully invoked to all the listeners, `false` if one or more of
         * the listeners threw an exception.
         */
        template<auto T_func, typename... T_args>
        inline bool listen(const T_args &... args) {
            try {
                for(app_listener *const &listener : listeners) T_func(*listener, *this, args...);
            } catch(std::exception &e) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, e.what());
                return false;
            }

            return true;
        }

        /**
         * @brief Submits a function that will run at the end of the frame in `loop()`.
         * @param function - The lambda `void` function accepting `app &` parameter.
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
