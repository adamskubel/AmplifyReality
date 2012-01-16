#include "model/Engine.hpp"
#include "model/FrameItem.hpp"

#include <opencv2/core/core.hpp>

#ifndef RENDERABLE_HPP_
#define RENDERABLE_HPP_

class Updateable
{
public:
	virtual void Update(FrameItem * frameItem);
	virtual ~Updateable()
	{
		;
	}
};
 
#endif