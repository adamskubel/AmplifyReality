#ifndef AR_COMMUNICATOR_HPP_
#define AR_COMMUNICATOR_HPP_

#include "model/network/RealmDefinition.hpp"
#include "model/network/NetworkMessages.hpp"
#include "model/network/StringMessage.hpp"
#include "model/network/WavefrontModel.hpp"



namespace CommunicatorStates
{
	enum CommunicatorState
	{
		Starting,
		Ready,
		ConnectingNext,
		Connected,
		ConnectFailed,
		AuthFailed,
		InvalidHost
	};
}

class ARCommunicator
{
public:
	ARCommunicator();
	void SendMessage(OutgoingMessage * message);
	void GetOutgoingMessages(std::vector<OutgoingMessage*> & newMsgQueue);
	bool HasOutgoingMessages();
	void AddIncomingMessage(IncomingMessage * message);
	bool FilterMessages(std::string filter, std::vector<IncomingMessage*> & newMsgs);
	bool IsConnected();
	void ConnectTo(std::string connectionString, std::string userName, std::string password);
	void SetClientObject(jobject arClientObject);
	void Update(JavaVM * jvm);
	CommunicatorStates::CommunicatorState GetState();

	bool CanConnect();

private:	
	jobject arClientObject;
	std::string connectionString, userName, password;
	//bool isConnected, tryConnect, connectFailed;
	CommunicatorStates::CommunicatorState state;
	//Message queues used to communicate with outside
	std::vector<IncomingMessage*> incomingMessageQueue;
	std::vector<OutgoingMessage*> outgoingMessageQueue;	
};

#endif