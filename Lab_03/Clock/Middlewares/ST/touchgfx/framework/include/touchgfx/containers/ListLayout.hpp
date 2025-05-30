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

/**
 * @file touchgfx/containers/ListLayout.hpp
 *
 * Declares the touchgfx::ListLayout class.
 */
#ifndef TOUCHGFX_LISTLAYOUT_HPP
#define TOUCHGFX_LISTLAYOUT_HPP

#include <touchgfx/Drawable.hpp>
#include <touchgfx/containers/Container.hpp>
#include <touchgfx/hal/Types.hpp>

namespace touchgfx
{
/**
 * This class provides a layout mechanism for arranging Drawable instances adjacent in the
 * specified Direction. The first element in the ListLayout is positioned in the
 * ListLayout origin (0,0). The dimensions of this class is automatically expanded to
 * cover the area of the added Drawable instances, which may grow larger than the
 * dimensions of the physical screen. Place the ListLayout inside e.g. a
 * ScrollableContainer to allow all the children to be viewed.
 *
 * @see ScrollableContainer
 */
class ListLayout : public Container
{
public:
    /**
     * Initializes a new instance of the ListLayout class.
     *
     * @param  d (Optional) The direction to place the elements. ::SOUTH (Default)
     *           places the elements vertically, ::EAST places the elements horizontally.
     *
     * @see setDirection
     */
    ListLayout(const Direction d = SOUTH)
        : Container(), direction(d), offset(0)
    {
        assert((d == SOUTH || d == EAST) && "Chosen direction not supported");
    }

    /**
     * Sets the direction of the ListLayout. If elements have already been added to the
     * ListLayout, these elements will be repositioned to adhere to the new direction.
     *
     * @param  d The new Direction to grow in when added children (either ::SOUTH or ::EAST).
     *
     * @see getDirection
     */
    virtual void setDirection(const Direction d);

    /**
     * Gets the direction of the ListLayout.
     *
     * @return The current direction to grow in when added children (either ::SOUTH or ::EAST).
     *
     * @see setDirection
     */
    virtual Direction getDirection() const
    {
        return direction;
    }

    /**
     * Adds a Drawable instance to the end of the list. The Drawable dimensions shall be set
     * prior to addition. The coordinates of the Drawable will be updated to reflect the
     * position in the ListLayout.
     *
     * @param [in] d The Drawable to add.
     */
    virtual void add(Drawable& d);

    /**
     * Removes a Drawable. Safe to call even if drawable has not been added. Other Drawable
     * elements in the ListLayout are repositioned and the size of the ListLayout is
     * adjusted.
     *
     * @param [in] d The drawable to remove.
     */
    virtual void remove(Drawable& d);

    virtual void insert(Drawable* previous, Drawable& d);

    virtual void removeAll();

private:
    void internalAddElementAt(Drawable& d, int16_t coord);
    void internalAddElement(Drawable& d);
    void internalRemoveElement(Drawable& d, int16_t coord);
    Direction direction;
    int16_t offset;
};

} // namespace touchgfx

#endif // TOUCHGFX_LISTLAYOUT_HPP
