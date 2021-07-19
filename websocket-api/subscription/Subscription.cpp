#include <libdevcore/SHA3.h>
#include <libdevcore/FileSystem.h>
#include "Subscription.h"

namespace WebsocketAPI {

Subscription::Subscription(IWebsocketClient* client, Subscription::Type type)
    : m_client(client), m_type(type)
{
    auto fh = dev::FixedHash<16>::random();
    m_id = "0x" + fh.hex();
}

std::string Subscription::getId()
{
    return m_id;
}

IWebsocketClient* Subscription::getClient()
{
    return m_client;
}

Subscription::Type Subscription::getType()
{
    return m_type;
}

void Subscription::cleanUp()
{
	if(m_connection.connected())
		m_connection.disconnect();
}

}