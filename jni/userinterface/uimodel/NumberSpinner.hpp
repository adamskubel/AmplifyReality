#include "UIElement.hpp"
#include "Label.hpp"
#include "Button.hpp"
#include "GridLayout.hpp"
#include "userinterface/events/EventArgs.hpp"
#include "userinterface/events/EventDelegates.hpp"

#ifndef NUMBER_SPINNER_HPP_
#define NUMBER_SPINNER_HPP_

using namespace std;

class NumberSpinner : public GridLayout
{
public:
	NumberSpinner(string description, float initialValue = 0, float delta = 1, string valueFormat = "%f", Point2i position = Point2i(0,0), Size2i size = Size2i(100,100));
	void SetPosition(Point2i position);
	void SetSize(Size2i size);
	void SetMaximum(float max);
	void SetMinimum(float min);

	//Delegates
	void DecreaseClick(void * sender, EventArgs args);
	void IncreaseClick(void * sender, EventArgs args);

	void AddValueChangedDelegate(NumberSpinnerEventDelegate newDelegate);

private:
	vector<NumberSpinnerEventDelegate> delegateVector;
	void UpdateLabel();
	float value, clickDelta, maxValue, minValue;
	std::string valueFormat;	
	Label * valueLabel;


};

#endif