#ifndef AR_COMMUNICATOR_HPP_
#define AR_COMMUNICATOR_HPP_

#include "model/network/RealmDefinition.hpp"
#include "model/network/NetworkMessages.hpp"
#include "model/network/StringMessage.hpp"
#include "model/network/WavefrontModel.hpp"

class ARCommunicator
{

public:
	ARCommunicator()
	{
		outgoingMessageQueue.clear();
	}

	void SendMessage(OutgoingMessage * message)
	{
		outgoingMessageQueue.push_back(message);
	}


	void GetOutgoingMessages(std::vector<OutgoingMessage*> & newMsgQueue)
	{
		while (outgoingMessageQueue.empty() == false)
		{
			newMsgQueue.push_back(outgoingMessageQueue.back());
			outgoingMessageQueue.pop_back();
		}
	}

	bool HasOutgoingMessages()
	{
		return (!outgoingMessageQueue.empty());
	}


	void AddIncomingMessage(IncomingMessage * message)
	{
		incomingMessageQueue.push_back(message);
	}


	bool FilterMessages(std::string filter, std::vector<IncomingMessage*> & newMsgs)
	{
		if (incomingMessageQueue.empty())
			return false;
		
		LOGD(LOGTAG_NETWORKING,"%d messages in queue. Returning messages with tag %s", incomingMessageQueue.size(), filter.c_str());
		bool found = false;
		for (int i=0;i<incomingMessageQueue.size();i++)
		{
			if (incomingMessageQueue.at(i)->GetAction().compare(filter) == 0)
			{
				found = true;
				newMsgs.push_back(incomingMessageQueue.at(i));
				incomingMessageQueue.erase(incomingMessageQueue.begin() + i);
				i--;
			}
		}
		return found;
	}

private:


	
	//Message queues used to communicate with outside
	std::vector<IncomingMessage*> incomingMessageQueue;
	std::vector<OutgoingMessage*> outgoingMessageQueue;
	
};

#endif