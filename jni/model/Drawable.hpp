#include "model/Engine.hpp"
#include "model/FrameItem.hpp"

#include <opencv2/core/core.hpp>

#ifndef DRAWABLE_HPP_
#define DRAWABLE_HPP_

class Drawable
{
public:
	virtual ~Drawable()
	{
		;
	}
	virtual void Draw(Mat * rgbaImage)
	{
		;
	}
};
 
#endif