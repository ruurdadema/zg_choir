#include "TestSubscriber.h"

TestSubscriber::TestSubscriber(zg::ITreeGateway * optGateway, const muscle::String& nodePath) : zg::ITreeGatewaySubscriber(optGateway)
{
    muscle::LogTime(muscle::MUSCLE_LOG_INFO, "TestSubscriber(\"%s\")\n", nodePath.Cstr());
    muscle::status_t ret;

    // Subscribe to given node path.
    if (AddTreeSubscription(nodePath).IsError(ret))
        muscle::LogTime(muscle::MUSCLE_LOG_ERROR, "Error: %s\n", ret.GetDescription());
}

TestSubscriber::~TestSubscriber()
{
    muscle::LogTime(muscle::MUSCLE_LOG_INFO, "~TestSubscriber()\n");
}

void TestSubscriber::TreeNodeUpdated(const muscle::String & nodePath, const muscle::MessageRef & optPayloadMsg, const muscle::String&)
{
    muscle::LogTime(muscle::MUSCLE_LOG_INFO, "TestSubscriber::TreeNodeUpdated: \n", nodePath.Cstr());
    if (optPayloadMsg())
        optPayloadMsg()->PrintToStream();

    // NOTE: This method never gets called.
}

TestSubscriberFactory::TestSubscriberFactory(zg::ITreeGateway * optGateway) : zg::ITreeGatewaySubscriber(optGateway)
{
    muscle::status_t ret;

    // Subscribe to magnet nodes, using them as trigger to instantiate TestSubscribers (see TestSubscriberFactory::TreeNodeUpdated).
    if (AddTreeSubscription("project/magnets/*").IsError(ret))
        muscle::LogTime(muscle::MUSCLE_LOG_ERROR, "Error: %s\n", ret.GetDescription());
}

void TestSubscriberFactory::TreeNodeUpdated(const muscle::String &nodePath, const muscle::MessageRef & optPayloadMsg, const muscle::String &)
{
    if (optPayloadMsg())
    {
        // Instantiate a TestSubscriber, which will subscribe itself to given node path.
        if (auto* value = mTestSubscribers.GetOrPut(nodePath))
            *value = std::make_shared<TestSubscriber>(GetGateway(), "project/test");
    }
    else
    {
        mTestSubscribers.Remove(nodePath);
    }
}
