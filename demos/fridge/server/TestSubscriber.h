#ifndef TESTSUBSCRIBER_H
#define TESTSUBSCRIBER_H

#include "zg/messagetree/gateway/ITreeGatewaySubscriber.h"

/**
 * Subscribes to a given node, and logs calls to TreeNodeUpdated().
 */
class TestSubscriber : public zg::ITreeGatewaySubscriber
{
public:
    TestSubscriber(zg::ITreeGateway * optGateway, const muscle::String& nodePath);
    ~TestSubscriber();

    void TreeNodeUpdated(const muscle::String & nodePath, const muscle::MessageRef & optPayloadMsg, const muscle::String & optOpTag);
};

/**
 * Dynamically instantiates a TestSubscriber for every magnet in the database.
 */
class TestSubscriberFactory : public zg::ITreeGatewaySubscriber
{
public:
    TestSubscriberFactory(zg::ITreeGateway* optGateway);

    void TreeNodeUpdated(const muscle::String & nodePath, const muscle::MessageRef & optPayloadMsg, const muscle::String & optOpTag);

private:
    muscle::Hashtable<muscle::String, std::shared_ptr<TestSubscriber>> mTestSubscribers;
};

#endif // TESTSUBSCRIBER_H
