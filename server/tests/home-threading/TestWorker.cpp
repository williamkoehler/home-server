#include "Testworker.hpp"
#include "home-threading/worker.hpp"

static Ref<server::Worker> worker;

boost::atomic_size_t load = 0;

boost::atomic_uint64_t counter = 0;
#define JOB_COUNT_LOW 10000
#define JOB_COUNT_MEDIUM 100000
#define JOB_COUNT_HIGH 1000000

void JobHandler()
{
    // Count
    counter++;

    // Simulate load
    if (load > 0)
        boost::this_thread::sleep_for(boost::chrono::milliseconds(load));
}

BOOST_AUTO_TEST_CASE(test_empty_worker)
{
    LOG_INFO("Test empty worker");

    worker = server::Worker::Create("test worker", 1);
    BOOST_CHECK_MESSAGE(worker != nullptr, "Create worker");

    // Start worker
    worker->Start();

    // Wait for 8 seconds
    boost::this_thread::sleep_for(boost::chrono::seconds(8));

    // Post jobs
    load = 0;
    counter = 0;
    for (size_t index = 0; index < JOB_COUNT_LOW; index++)
        worker->GetContext().post(JobHandler);

    // Wait for 2 seconds for jobs to finish
    boost::this_thread::sleep_for(boost::chrono::seconds(2));

    // Check job count
    BOOST_CHECK_MESSAGE(counter >= JOB_COUNT_LOW, "Too few jobs executed");
    BOOST_CHECK_MESSAGE(counter <= JOB_COUNT_LOW, "Too many jobs executed");

    // Stop worker
    worker->Stop();
}

BOOST_AUTO_TEST_CASE(test_worker_threads)
{
    LOG_INFO("Test worker threads");

    worker = server::Worker::Create("test worker", 50);
    BOOST_CHECK_MESSAGE(worker != nullptr, "Create worker");

    // Start worker
    worker->Start();

    // Post jobs
    load = 10;
    counter = 0;
    for (size_t index = 0; index < JOB_COUNT_LOW; index++)
        worker->GetContext().post(JobHandler);

    // Wait for 5 seconds for jobs to finish
    boost::this_thread::sleep_for(boost::chrono::milliseconds(2500));

    // Check job count
    BOOST_CHECK_MESSAGE(counter >= JOB_COUNT_LOW, "Too few jobs executed");
    BOOST_CHECK_MESSAGE(counter <= JOB_COUNT_LOW, "Too many jobs executed");

    // Stop worker
    worker->Stop();
}

void TestPostJob(size_t jobCount)
{
    worker = server::Worker::Create("test worker", 5);
    BOOST_CHECK_MESSAGE(worker != nullptr, "Create worker");

    // Start worker
    worker->Start();

    // Post jobs
    load = 0;
    counter = 0;
    for (size_t index = 0; index < jobCount; index++)
        worker->GetContext().post(JobHandler);

    // Wait for 2 seconds for jobs to finish
    boost::this_thread::sleep_for(boost::chrono::seconds(5));

    // Check job count
    BOOST_CHECK_MESSAGE(counter >= jobCount, "Too few jobs executed");
    BOOST_CHECK_MESSAGE(counter <= jobCount, "Too many jobs executed");

    // Stop worker
    worker->Stop();
}

BOOST_AUTO_TEST_CASE(test_worker_under_low_stress)
{
    LOG_INFO("Test worker under low stress");

    TestPostJob(JOB_COUNT_LOW);
}

BOOST_AUTO_TEST_CASE(test_worker_under_stress_medium)
{
    LOG_INFO("Test worker under medium stress");

    TestPostJob(JOB_COUNT_MEDIUM);
}

BOOST_AUTO_TEST_CASE(test_worker_under_stress_high)
{
    LOG_INFO("Test worker under high stress");

    TestPostJob(JOB_COUNT_HIGH);
}