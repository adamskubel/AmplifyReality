#include "ARCommunicator.hpp"


ARCommunicator::ARCommunicator()
{
	outgoingMessageQueue.clear();
}

void ARCommunicator::SendMessage(OutgoingMessage * message)
{
	outgoingMessageQueue.push_back(message);
}


void ARCommunicator::GetOutgoingMessages(std::vector<OutgoingMessage*> & newMsgQueue)
{
	while (outgoingMessageQueue.empty() == false)
	{
		newMsgQueue.push_back(outgoingMessageQueue.back());
		outgoingMessageQueue.pop_back();
	}
}

bool ARCommunicator::HasOutgoingMessages()
{
	return (!outgoingMessageQueue.empty());
}


void ARCommunicator::AddIncomingMessage(IncomingMessage * message)
{
	if (message != NULL)
		incomingMessageQueue.push_back(message);
}


bool ARCommunicator::FilterMessages(std::string filter, std::vector<IncomingMessage*> & newMsgs)
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
			//LOGV(LOGTAG_NETWORKING,"Erasing msg at i=%d",i);
			incomingMessageQueue.erase(incomingMessageQueue.begin() + i);
			i--;
		}
	}		
	//LOGD(LOGTAG_NETWORKING,"%d matching messages found", newMsgs.size());
	return found;
}

