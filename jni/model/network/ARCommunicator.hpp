#ifndef AR_COMMUNICATOR_HPP_
#define AR_COMMUNICATOR_HPP_

#include "model/network/RealmDefinition.hpp"
#include "model/network/NetworkMessages.hpp"
#include "model/network/StringMessage.hpp"
#include "model/network/WavefrontModel.hpp"

class ARCommunicator
{
public:
	ARCommunicator();

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

	void AddIncomingMessage(IncomingMessage * message)
	{
		incomingMessageQueue.push_back(message);
	}

	bool GetNewResourceMessages(std::vector<WavefrontModel*> & newMsgs)
	{
		if (incomingMessageQueue.empty())
			return false;
		else
		{
			bool found = false;
			for (int i=0;i<incomingMessageQueue.size();i++)
			{
				if (incomingMessageQueue.at(i)->GetAction().compare("Wavefront"))
				{
					found = true;
					newMsgs.push_back((WavefrontModel*)incomingMessageQueue.at(i));
				}
			}
			return found;
		}
	}

	bool FilterMessages(std::string filter, std::vector<IncomingMessage*> & newMsgs)
	{
		if (incomingMessageQueue.empty())
			return false;
		else
		{
			bool found = false;
			for (int i=0;i<incomingMessageQueue.size();i++)
			{
				if (incomingMessageQueue.at(i)->GetAction().compare(filter))
				{
					found = true;
					newMsgs.push_back(incomingMessageQueue.at(i));
				}
			}
			return found;
		}
	}

private:


	
	//Message queues used to communicate with outside
	std::vector<IncomingMessage*> incomingMessageQueue;
	std::vector<OutgoingMessage*> outgoingMessageQueue;
	
};

#endif