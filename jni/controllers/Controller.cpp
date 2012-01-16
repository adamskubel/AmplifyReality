#include "Controller.hpp"

void Controller::ProcessFrame(Engine * engine, FrameItem * frame)
{
	LOGE("Called Controller::ProcessFrame(...) on base!");
	throw exception();
}

void Controller::Initialize(Engine * engine)
{
	;
}

bool Controller::isExpired()
{
	return false;
}


bool Controller::wasSuccessful()
{
	return false;
}

Controller::~Controller()
{
	LOGE("Called ~Controller() on base");
}