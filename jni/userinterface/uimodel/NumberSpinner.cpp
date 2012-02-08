#include "NumberSpinner.hpp"



NumberSpinner::NumberSpinner(string description, float intialValue, float delta, string _valueFormat, Point2i position, Size2i size) : GridLayout(size,Size2i(3,2),position)
{
	LOGD(LOGTAG_INPUT, "Enter NumberSpinner constructor");
	//Initialize grid base
	//gridSize = Size2i(3,2);
	//cellSize = Size_<int>((int)((float)size.width/gridSize.width),(int)((float)size.height/gridSize.height));
	//Position = position;
	//
	LOGD(LOGTAG_INPUT, "Cellsize = [%d,%d]",cellSize.width,cellSize.height);

	value = intialValue;
	clickDelta = delta;
	maxValue = MAXFLOAT;
	minValue = -MAXFLOAT;
	valueFormat = _valueFormat;


	Label * descriptionLabel = new Label(description,Point2i(0,0),Colors::Black,Colors::White);	
	descriptionLabel->FontScale = 0.9f;
	AddChild(descriptionLabel,Point2i(0,0),Size2i(3,1));

	char * str = new char[valueFormat.size()];
	sprintf(str,valueFormat.c_str(),value);
	valueLabel = new Label(string(str),Point2i(0,0),Colors::Black,Colors::White);	
	valueLabel->FontScale = 0.9f;
	GridLayout::AddChild(valueLabel,Point2i(1,1));
	delete str;


	Button * increase = new Button("+",Colors::MediumSeaGreen);	
	increase->AddClickDelegate(ClickEventDelegate::from_method<NumberSpinner,&NumberSpinner::IncreaseClick>(this));
	GridLayout::AddChild(increase,Point2i(2,1));
	
	Button * decrease = new Button("-",Colors::Gold);
	decrease->AddClickDelegate(ClickEventDelegate::from_method<NumberSpinner,&NumberSpinner::DecreaseClick>(this));
	GridLayout::AddChild(decrease,Point2i(0,1));
	
	LOGD(LOGTAG_INPUT, "NumberSpinner instantiated");

}

void NumberSpinner::AddValueChangedDelegate(NumberSpinnerEventDelegate newDelegate)
{
	delegateVector.push_back(newDelegate);
}

void NumberSpinner::SetMaximum(float max)
{
	maxValue = max;
}

void NumberSpinner::SetMinimum(float min)
{
	minValue = min;
}

float NumberSpinner::GetValue()
{
	return value;
}

void NumberSpinner::IncreaseClick(void * sender, EventArgs args)
{
	if (value + clickDelta <= maxValue)
	{
		value += clickDelta;
		UpdateLabel();
	}
}

void NumberSpinner::DecreaseClick(void * sender, EventArgs args)
{
	if (value - clickDelta >= minValue)
	{		
		value -= clickDelta;		
		UpdateLabel();	
	}
}

void NumberSpinner::UpdateLabel()
{
	char * str = new char[valueFormat.size()];
	sprintf(str,valueFormat.c_str(),value);
	valueLabel->SetText(string(str));
	LOGD(LOGTAG_INPUT,"Updating label value to %s",str);
	delete str;

	NumberSpinnerEventArgs args = NumberSpinnerEventArgs(value);
	for (int i=0;i<delegateVector.size();i++)
	{
		delegateVector.at(i)(this,args);
	}
}

