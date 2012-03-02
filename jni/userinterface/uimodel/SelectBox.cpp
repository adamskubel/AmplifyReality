#include "SelectBox.hpp"

SelectBoxItem::SelectBoxItem(std::string label) : Button(label,Colors::Green)
{
	Name = label;
}



///SELECTBOX
SelectBox::SelectBox(int itemsToDisplay, cv::Scalar _foregroundColor, cv::Scalar _backgroundColor) : GridLayout(Size2i(1,itemsToDisplay))
{
	selected = NULL;
	backgroundColor = _backgroundColor;
	foregroundColor = _foregroundColor;
}

void SelectBox::AddItem(SelectBoxItem * newItem)
{
	newItem->AddClickDelegate(ClickEventDelegate::from_method<SelectBox,&SelectBox::ItemSelected>(this));
	newItem->SetFillColor(backgroundColor);
	
	int childCount = Children.size();
	if (gridSize.height <= childCount)
	{
		//Grid is too small, need to resize
		ResizeGrid(controlSize,Size2i(1,childCount+1),Position);
	}
	AddChild(newItem,Point2i(0,childCount));
}

void SelectBox::AddSelectionChangedDelegate(SelectionChangedEventDelegate newDelegate)
{
	selectionChangedEvents.push_back(newDelegate);
}

void SelectBox::SetSelectedIndex(int index)
{
	GraphicalUIElement * child = GetElementAtCell(Point2i(0,index));
	SetSelected((SelectBoxItem*)child);
}

void SelectBox::SetSelected(SelectBoxItem * item)
{
	if (selected != NULL)
		selected->SetFillColor(backgroundColor);

	selected = item;
	
	if (selected != NULL)
		selected->SetFillColor(UI_BUTTON_ALT_STATE_COLOR);
}

void SelectBox::ItemSelected(void * sender, EventArgs args)
{
	SelectBoxItem * item = (SelectBoxItem*)sender;
	if (item != selected)
	{
		LOGD(LOGTAG_INPUT,"Selection changed.");
		for (int i=0;i<selectionChangedEvents.size();i++)
		{
			selectionChangedEvents.at(i)(this, SelectionChangedEventArgs(item,selected));
		}

		//Selected might be null if this is first selection
		if (selected != NULL)
			selected->SetFillColor(backgroundColor);
		
		selected = item;
		selected->SetFillColor(UI_BUTTON_ALT_STATE_COLOR);
	}
}

