#define CAMERA_INITIALIZATION_EXCEPTION 1001


#ifndef AR_EXCEPTIONS_HPP_
#define AR_EXCEPTIONS_HPP_

using namespace std;

class CodeNotFoundException : exception
{

};

class OpenGLInitializationException : exception
{
public:
	const char* what()
	{
		return "Failed to init OpenGL. See logs";
	}
};


#endif