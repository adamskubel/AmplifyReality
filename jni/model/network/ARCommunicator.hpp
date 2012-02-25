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
	void SendMessage(OutgoingMessage * message);
	void GetOutgoingMessages(std::vector<OutgoingMessage*> & newMsgQueue);
	bool HasOutgoingMessages();
	void AddIncomingMessage(IncomingMessage * message);
	bool FilterMessages(std::string filter, std::vector<IncomingMessage*> & newMsgs);

private:	
	//Message queues used to communicate with outside
	std::vector<IncomingMessage*> incomingMessageQueue;
	std::vector<OutgoingMessage*> outgoingMessageQueue;	
};

#endif