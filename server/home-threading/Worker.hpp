#pragma once
#include <home-common/common.hpp>

namespace server
{
    namespace threading
    {
        class Worker : public boost::enable_shared_from_this<Worker>
        {
          private:
            boost::mutex mutex;

            /// @brief Worker name
            ///
            std::string name;

            const size_t threadCount = 1;

            /// @brief Running flag
            ///
            boost::atomic_bool running;

            /// @brief Thread pool
            ///
            boost::thread_group threads;

            /// @brief IO Context
            ///
            Ref<boost::asio::io_context> context = nullptr;

            /// @brief IO Work
            ///
            Ref<boost::asio::io_context::work> work = nullptr;

            void Handler();

          public:
            Worker(const std::string& name, size_t threadCount);
            virtual ~Worker();
            static Ref<Worker> Create(const std::string& name, size_t threadCount = 1);

            inline boost::asio::io_context& GetContext()
            {
                return *context.get();
            }
            inline boost::asio::io_context::work GetWork()
            {
                return *work.get();
            }

            inline bool IsRunning() const
            {
                return running;
            }

            /// @brief Start threads
            /// 
            /// @param withCurrentThread Run worker on the current thread
            void Start(bool withCurrentThread = false);

            /// @brief Stop threads
            ///
            void Stop();
        };
    }
}