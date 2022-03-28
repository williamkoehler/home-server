#include "Worker.hpp"

namespace server
{
    namespace threading
    {
        Worker::Worker(const std::string& name, size_t threadCount)
            : name(name), threadCount(std::max(threadCount, 1ul)), running(false)
        {
        }
        Worker::~Worker()
        {
            running = false;
        }
        Ref<Worker> Worker::Create(const std::string& name, size_t threadCount)
        {
            Ref<Worker> worker = boost::make_shared<Worker>(name, threadCount);

            if (worker != nullptr)
            {
                // Initialize context
                worker->context = boost::make_shared<boost::asio::io_service>(threadCount);
                if (worker->context == nullptr)
                {
                    LOG_ERROR("Create worker context.");
                    return nullptr;
                }

                // Initialize notifier
                worker->work = boost::make_shared<boost::asio::io_service::work>(worker->GetContext());
                if (worker->work == nullptr)
                {
                    LOG_ERROR("Create worker context notifier.");
                    return nullptr;
                }
            }

            return worker;
        }

        void Worker::Start(bool withCurrentThread)
        {
            if (!running)
            {
                // Start threads
                {
                    // Lock main mutex
                    boost::lock_guard lock(mutex);

                    LOG_INFO("Starting {0} threads(s) for {1} worker.", threadCount, name);

                    running = true; // Lock thread loop

                    // Start threads
                    size_t t = threadCount - (withCurrentThread ? 1 : 0);
                    for (size_t i = 0; i < t; i++)
                        threads.create_thread(boost::bind(&Worker::Handler, shared_from_this()));
                }

                // Run worker in the current thread
                if (withCurrentThread)
                    Handler();
            }
        }

        void Worker::Handler()
        {
            std::string nameCopy;

            // Make a safe copy
            {
                boost::lock_guard lock(mutex);
                nameCopy = name;
            }

            // Start
            LOG_INFO("Started thread for {0} worker", nameCopy);

            // Running
            {
                while (running)
                {
                    try
                    {
                        context->run();
                        boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
                    }
                    catch (const std::exception& e)
                    {
                        LOG_ERROR("Ops... Something bad happend in worker {0}!\n{1}", nameCopy, std::string(e.what()));
                    }
                }
            }

            // Stop
            LOG_INFO("Stopped thread for {0} worker", nameCopy);
        }

        void Worker::Stop()
        {
            if (running)
            {
                // Lock main mutex
                boost::lock_guard lock(mutex);

                LOG_INFO("Stopping {0} threads(s) for {1} worker.", threadCount, name);

                running = false; // Unlock thread loop

                context->stop(); // Force threads to stop

                // Wait for threads
                threads.join_all();
            }
        }
    }
}