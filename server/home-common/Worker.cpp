#include "Worker.hpp"

namespace server
{
    Worker::Worker() : running(false)
    {
    }
    Worker::~Worker()
    {
        running = false;
    }
    Ref<Worker> Worker::Create()
    {
        Ref<Worker> worker = boost::make_shared<Worker>();

        if (worker != nullptr)
        {
            // Initialize context
            worker->context = boost::make_shared<boost::asio::io_context>(1);
            if (worker->context == nullptr)
            {
                LOG_ERROR("Create worker context.");
                return nullptr;
            }

            // Initialize notifier
            worker->work = boost::make_shared<boost::asio::io_context::work>(worker->GetContext());
            if (worker->work == nullptr)
            {
                LOG_ERROR("Create worker context notifier.");
                return nullptr;
            }
        }

        return worker;
    }

    void Worker::Run()
    {
        if (!running)
        {
            running = true; // Lock thread loop

            LOG_INFO("Starting worker.");

            while (running)
            {
                try
                {
                    context->run();
                    boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
                }
                catch (const std::exception& e)
                {
                    LOG_ERROR("Ops... Something bad happend!\n{1}", std::string(e.what()));
                }
            }

            LOG_INFO("Stopping worker.");
        }
    }

    void Worker::Stop()
    {
        if (running)
        {
            running = false; // Unlock thread loop

            context->stop(); // Force threads to stop
        }
    }
}