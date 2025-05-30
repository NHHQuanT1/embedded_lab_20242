/******************************************************************************
* Copyright (c) 2018(-2025) STMicroelectronics.
* All rights reserved.
*
* This file is part of the TouchGFX 4.25.0 distribution.
*
* This software is licensed under terms that can be found in the LICENSE file in
* the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
*******************************************************************************/

#include <touchgfx/Application.hpp>
#include <touchgfx/Drawable.hpp>
#include <touchgfx/Utils.hpp>
#include <touchgfx/containers/scrollers/ScrollBase.hpp>

namespace touchgfx
{
ScrollBase::ScrollBase()
    : Container(),
      list(),
      numberOfDrawables(0),
      distanceBeforeAlignedItem(0),
      itemSize(0),
      swipeAcceleration(10),
      dragAcceleration(10),
      maxSwipeItems(0),
      easingEquation(&EasingEquations::backEaseOut),
      defaultAnimationSteps(30),
      overshootPercentage(75),
      itemSelectedCallback(0),
      itemLockedInCallback(0),
      animationEndedCallback(0),
      itemPressedCallback(0),
      currentAnimationState(NO_ANIMATION),
      gestureStep(0),
      gestureStepsTotal(0),
      gestureStart(0),
      gestureEnd(0),
      xClick(0),
      yClick(0),
      initialSwipeOffset(0),
      draggableX(false),
      draggableY(false),
      isPressed(false),
      isScrolling(false)
{
    Container::add(list);
    list.setXY(0, 0);
    list.setHorizontal(false);
    list.setCircular(false);
    setTouchable(true);
}

void ScrollBase::setWidth(int16_t width)
{
    Container::setWidth(width);
    list.setWidth(width);
}

void ScrollBase::setHeight(int16_t height)
{
    Container::setHeight(height);
    list.setHeight(height);
}

void ScrollBase::setHorizontal(bool horizontal)
{
    allowVerticalDrag(horizontal);
    allowHorizontalDrag(!horizontal);
    list.setHorizontal(horizontal);
}

bool ScrollBase::getHorizontal() const
{
    return list.getHorizontal();
}

void ScrollBase::setCircular(bool circular)
{
    list.setCircular(circular);
}

bool ScrollBase::getCircular() const
{
    return list.getCircular();
}

void ScrollBase::setDrawableSize(int16_t drawableSize, int16_t drawableMargin)
{
    itemSize = drawableSize + drawableMargin * 2;
    list.setDrawableSize(drawableSize, drawableMargin);
}

int16_t ScrollBase::getDrawableSize() const
{
    return list.getDrawableSize();
}

int16_t ScrollBase::getDrawableMargin() const
{
    return list.getDrawableMargin();
}

void ScrollBase::setNumberOfItems(int16_t numberOfItems)
{
    if (numberOfItems != getNumberOfItems())
    {
        list.setNumberOfItems(numberOfItems);
        if (!getCircular())
        {
            animateToPosition(keepOffsetInsideLimits(getOffset(), 0));
        }
    }
}

int16_t ScrollBase::getNumberOfItems() const
{
    return list.getNumberOfItems();
}

void ScrollBase::setEasingEquation(EasingEquation equation)
{
    easingEquation = equation;
}

void ScrollBase::setAnimationSteps(int16_t steps)
{
    defaultAnimationSteps = steps;
}

uint16_t ScrollBase::getAnimationSteps() const
{
    return defaultAnimationSteps;
}

void ScrollBase::setSwipeAcceleration(uint16_t acceleration)
{
    swipeAcceleration = acceleration;
}

uint16_t ScrollBase::getSwipeAcceleration() const
{
    return swipeAcceleration;
}

void ScrollBase::setMaxSwipeItems(uint16_t maxItems)
{
    maxSwipeItems = maxItems;
}

uint16_t ScrollBase::getMaxSwipeItems() const
{
    return maxSwipeItems;
}

void ScrollBase::setDragAcceleration(uint16_t acceleration)
{
    dragAcceleration = acceleration;
}

uint16_t ScrollBase::getDragAcceleration() const
{
    return dragAcceleration;
}

void ScrollBase::allowHorizontalDrag(bool enable)
{
    draggableX = enable;
}

void ScrollBase::allowVerticalDrag(bool enable)
{
    draggableY = enable;
}

void ScrollBase::animateToItem(int16_t itemIndex, int16_t animationSteps /*= -1*/)
{
    const int32_t position = getPositionForItem(itemIndex);
    animateToPosition(position, animationSteps);
}

void ScrollBase::setItemSelectedCallback(GenericCallback<int16_t>& callback)
{
    itemSelectedCallback = &callback;
}

void ScrollBase::setAnimationEndedCallback(GenericCallback<>& callback)
{
    animationEndedCallback = &callback;
}

void ScrollBase::setItemPressedCallback(GenericCallback<int16_t>& callback)
{
    itemPressedCallback = &callback;
}

bool ScrollBase::isAnimating() const
{
    return currentAnimationState != NO_ANIMATION;
}

void ScrollBase::stopAnimation()
{
    if (currentAnimationState == ANIMATING_GESTURE)
    {
        Application::getInstance()->unregisterTimerWidget(this);
        setOffset(gestureEnd);
    }
    currentAnimationState = NO_ANIMATION;
}

void ScrollBase::handleDragEvent(const DragEvent& event)
{
    isScrolling = true;
    stopAnimation();
    currentAnimationState = ANIMATING_DRAG;
    int32_t newOffset = getOffset() + (getHorizontal() ? event.getDeltaX() : event.getDeltaY()) * dragAcceleration / 10;
    newOffset = keepOffsetInsideLimits(newOffset, muldiv(itemSize, overshootPercentage, 100));
    setOffset(newOffset);
}

void ScrollBase::handleGestureEvent(const GestureEvent& event)
{
    if (event.getType() == (getHorizontal() ? GestureEvent::SWIPE_HORIZONTAL : GestureEvent::SWIPE_VERTICAL))
    {
        isScrolling = true;
        const int16_t velocity = abs(event.getVelocity());
        const int16_t direction = event.getVelocity() < 0 ? -1 : 1;
        int16_t steps = MAX(1, velocity - 4) * 7;
        int32_t newOffset = getOffset() + direction * steps * swipeAcceleration / 10;
        if (maxSwipeItems > 0)
        {
            const int32_t maxDistance = maxSwipeItems * itemSize;
            newOffset = MIN(newOffset, initialSwipeOffset + maxDistance);
            newOffset = MAX(newOffset, initialSwipeOffset - maxDistance);
        }
        newOffset = keepOffsetInsideLimits(newOffset, 0);
        steps = MIN(steps, defaultAnimationSteps);
        animateToPosition(newOffset, steps);
    }
}

void ScrollBase::handleTickEvent()
{
    if (currentAnimationState == ANIMATING_GESTURE)
    {
        gestureStep++;
        const int newPosition = gestureStart + easingEquation(gestureStep, 0, gestureEnd - gestureStart, gestureStepsTotal);
        setOffset(newPosition);
        if (gestureStep > gestureStepsTotal)
        {
            currentAnimationState = NO_ANIMATION;
            gestureStep = 0;
            Application::getInstance()->unregisterTimerWidget(this);
            setOffset(getNormalizedOffset(gestureEnd));
            // Also adjust initialSwipeOffset in case it is being used.
            initialSwipeOffset += getOffset() - gestureEnd;

            // Item has settled, call back
            if (animationEndedCallback && animationEndedCallback->isValid())
            {
                animationEndedCallback->execute();
            }
        }
    }
}

void ScrollBase::itemChanged(int itemIndex)
{
    list.itemChanged(itemIndex);
}

void ScrollBase::setOffset(int32_t offset)
{
    list.setOffset(offset + distanceBeforeAlignedItem);
}

int32_t ScrollBase::getOffset() const
{
    return list.getOffset() - distanceBeforeAlignedItem;
}

int ScrollBase::getNormalizedOffset(int offset) const
{
    const int16_t numItems = getNumberOfItems();
    if (numItems == 0 || itemSize == 0)
    {
        return offset;
    }
    const int32_t listSize = numItems * itemSize;
    offset %= listSize;
    return offset > 0 ? offset - listSize : offset;
}

int32_t ScrollBase::getNearestAlignedOffset(int32_t offset) const
{
    if (itemSize == 0)
    {
        return offset;
    }
    if (getCircular())
    {
        if (offset < 0)
        {
            return (((offset - (itemSize / 2)) / itemSize) * itemSize);
        }
        return ((offset + (itemSize / 2)) / itemSize) * itemSize;
    }
    offset = keepOffsetInsideLimits(offset, 0);
    return ((offset - (itemSize / 2)) / itemSize) * itemSize;
}

void ScrollBase::animateToPosition(int32_t position, int16_t steps)
{
    const int32_t currentPosition = getOffset();
    position = getNearestAlignedOffset(position);
    if (steps < 0)
    {
        steps = defaultAnimationSteps;
    }
    const int32_t distance = abs(position - currentPosition);
    steps = MIN(steps, distance);
    if (steps < 1)
    {
        setOffset(position);
        currentAnimationState = NO_ANIMATION;
    }
    else
    {
        gestureStart = currentPosition;
        gestureEnd = position;
        gestureStep = 0;
        gestureStepsTotal = steps;
        if (currentAnimationState != ANIMATING_GESTURE)
        {
            Application::getInstance()->registerTimerWidget(this);
            currentAnimationState = ANIMATING_GESTURE;
        }
    }
}

bool ScrollBase::getIsScrolling() const
{
    return isScrolling;
}

bool ScrollBase::getIsPressed() const
{
    return isPressed;
}
} // namespace touchgfx
