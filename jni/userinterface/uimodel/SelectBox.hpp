#ifndef SELECT_BOX_HPP_
#define SELECT_BOX_HPP_

#include "GridLayout.hpp"
#include "Button.hpp"
#include "userinterface/events/EventDelegates.hpp"

class SelectBoxItem : public Button
{
public:	
	SelectBoxItem(std::string label);
	SelectBoxItem(std::string label, ButtonEventDelegate onSelect);
	int index;
};

class SelectBox : public GridLayout
{
public:
	SelectBox(int itemsToDisplay = 2,cv::Scalar textColor = Colors::Black, cv::Scalar backgroundColor = Colors::Transparent);
	void AddItem(SelectBoxItem * newItem);
	void SetSelectedIndex(int selectedIndex);
	void SetSelected(SelectBoxItem * selectedItem);
	void AddSelectionChangedDelegate(SelectionChangedEventDelegate newDelegate);

private:
	void ItemSelected(void * sender, EventArgs args);

	vector<SelectionChangedEventDelegate> selectionChangedEvents;
	SelectBoxItem * selected;
	cv::Scalar backgroundColor, foregroundColor;
};

#endif