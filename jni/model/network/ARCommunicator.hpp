#ifndef AR_COMMUNICATOR_HPP_
#define AR_COMMUNICATOR_HPP_

#include "model/network/RealmDefinition.hpp"
#include "model/network/NetworkMessages.hpp"
#include "model/network/StringMessage.hpp"
#include "model/network/WavefrontModel.hpp"
#include "util/JNIUtils.hpp"


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
	ARCommunicator(jobject arClientObject);
	void SendMessage(OutgoingMessage * message);
	void AddIncomingMessage(IncomingMessage * message);
	bool FilterMessages(std::string filter, std::vector<IncomingMessage*> & newMsgs);
	bool IsConnected();
	void ConnectTo(std::string connectionString, std::string userName, std::string password);
	void SetClientObject(jobject arClientObject);

	CommunicatorStates::CommunicatorState GetState();
	bool CanConnect();

	void Update(JavaVM * jvm);

private:	
	jobject arClientObject;
	std::string connectionString, userName, password;
	CommunicatorStates::CommunicatorState state;
	std::vector<IncomingMessage*> incomingMessageQueue;
	std::vector<OutgoingMessage*> outgoingMessageQueue;

	void Connect(JavaVM * javaVM);
	void SendMessages(JavaVM * jvm);
};

#endif