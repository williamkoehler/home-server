#pragma once
#include "common.hpp"

namespace server
{
    class Worker : public boost::enable_shared_from_this<Worker>
    {
      private:
        const size_t threadCount = 1;

        /// @brief Running flag
        ///
        boost::atomic_bool running;

        /// @brief IO Context
        ///
        Ref<boost::asio::io_context> context = nullptr;

        /// @brief IO Work
        ///
        Ref<boost::asio::io_context::work> work = nullptr;

      public:
        Worker();
        virtual ~Worker();
        static Ref<Worker> Create();
        static Ref<Worker> GetInstance();

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

        /// @brief Start worker
        ///
        void Run();

        /// @brief Stop worker
        ///
        void Stop();
    };
}