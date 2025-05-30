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
 * @file touchgfx/widgets/TextArea.hpp
 *
 * Declares the touchgfx::TextArea class.
 */
#ifndef TOUCHGFX_TEXTAREA_HPP
#define TOUCHGFX_TEXTAREA_HPP

#include <touchgfx/Font.hpp>
#include <touchgfx/TextProvider.hpp>
#include <touchgfx/TypedText.hpp>
#include <touchgfx/Unicode.hpp>
#include <touchgfx/hal/Types.hpp>
#include <touchgfx/widgets/Widget.hpp>

namespace touchgfx
{
/**
 * This widget is capable of showing a text area on the screen. The text must be a predefined
 * TypedText in the text sheet in the assets folder. In order to display a dynamic text,
 * use TextAreaWithOneWildcard or TextAreaWithTwoWildcards.
 *
 * @see TypedText, TextAreaWithOneWildcard, TextAreaWithTwoWildcards
 *
 * @note A TextArea just holds a pointer to the text displayed. The developer must ensure that
 *       the pointer remains valid when drawing.
 */
class TextArea : public Widget
{
public:
    TextArea()
        : Widget(), typedText(TYPED_TEXT_INVALID), color(0), linespace(0), alpha(255), indentation(0), rotation(TEXT_ROTATE_0), wideTextAction(WIDE_TEXT_NONE), boundingArea()
    {
    }

    virtual void setWidth(int16_t width)
    {
        Widget::setWidth(width);
        boundingArea = calculateBoundingArea();
    }

    virtual void setHeight(int16_t height)
    {
        Widget::setHeight(height);
        boundingArea = calculateBoundingArea();
    }

    virtual Rect getSolidRect() const
    {
        return Rect();
    }

    /**
     * Sets the color of the text. If no color is set, the default color (black) is used.
     *
     * @param  newColor The color to use.
     */
    FORCE_INLINE_FUNCTION void setColor(colortype newColor)
    {
        color = newColor;
    }

    /**
     * Gets the color of the text. If no color has been set, the default color, black, is
     * returned.
     *
     * @return The color to used for drawing the text.
     */
    FORCE_INLINE_FUNCTION colortype getColor() const
    {
        return color;
    }

    /**
     * @copydoc Image::setAlpha
     */
    virtual void setAlpha(uint8_t newAlpha)
    {
        alpha = newAlpha;
    }

    /**
     * @copydoc Image::getAlpha
     */
    uint8_t getAlpha() const
    {
        return alpha;
    }

    /**
     * Adjusts the TextArea y coordinate so the text will have its baseline at the specified
     * value. The placements is relative to the specified TypedText so if the TypedText is
     * changed, you have to set the baseline again.
     *
     * @param  baselineY The y coordinate of the baseline of the text.
     *
     * @note setTypedText() must be called prior to setting the baseline.
     */
    virtual void setBaselineY(int16_t baselineY)
    {
        setY(baselineY - getTypedText().getFont()->getBaseline());
    }

    /**
     * Adjusts the TextArea x and y coordinates so the text will have its baseline at the
     * specified y value. The placements is relative to the specified TypedText so if the
     * TypedText is changed you have to set the baseline again. The specified x coordinate
     * will be used as the x coordinate of the TextArea.
     *
     * @param  x         The x coordinate of the TextArea.
     * @param  baselineY The y coordinate of the baseline of the text.
     *
     * @note setTypedText() must be called prior to setting the baseline.
     */
    virtual void setXBaselineY(int16_t x, int16_t baselineY)
    {
        setX(x);
        setBaselineY(baselineY);
    }

    /**
     * Sets the line spacing of the TextArea. Setting a larger value will increase the space
     * between lines. It is possible to set a negative value to have lines (partially)
     * overlap. Default line spacing, if not set, is 0.
     *
     * @param  space The line spacing of use in the TextArea.
     *
     * @see getLinespacing
     */
    FORCE_INLINE_FUNCTION void setLinespacing(int16_t space)
    {
        linespace = space;
        boundingArea = calculateBoundingArea();
    }

    /**
     * Gets the line spacing of the TextArea. If no line spacing has been set, the line
     * spacing is 0.
     *
     * @return The line spacing.
     *
     * @see setLinespacing
     */
    FORCE_INLINE_FUNCTION int16_t getLinespacing() const
    {
        return linespace;
    }

    /**
     * Sets the indentation for the text. This can be very useful when a font is an italic
     * font where letters such as "j" and "g" extend a lot to the left under the previous
     * character(s). if a line starts with a "j" or "g" this letter would either have to be
     * pushed to the right to be able to see all of it, e.g. using spaces (which would ruin
     * a multi line text which is left aligned) - or by clipping the first letter (which
     * could ruin the nice graphics). The solution is to change
     * @code
     *      textarea.setPosition(50, 50, 100, 100);
     * @endcode
     * to
     * @code
     *      textarea.setPosition(45, 50, 110, 100);
     *      textarea.setIndentation(5);
     * @endcode
     * Characters that do not extend to the left under the previous characters will be drawn
     * in the same position in either case, but "j" and "g" will be aligned with other lines.
     *
     * The function Font::getMaxPixelsLeft() will give you the maximum number of pixels any glyph
     * in the font extends to the left.
     *
     * @param  indent The indentation from left (when left aligned text) and right (when right
     *                aligned text).
     *
     * @see Font::getMaxPixelsLeft
     */
    FORCE_INLINE_FUNCTION void setIndentation(uint8_t indent)
    {
        indentation = indent;
        boundingArea = calculateBoundingArea();
    }

    /**
     * Gets the indentation of text inside the TextArea.
     *
     * @return The indentation.
     *
     * @see setIndentation
     */
    FORCE_INLINE_FUNCTION uint8_t getIndentation()
    {
        return indentation;
    }

    /**
     * Gets the alignment of text inside the TextArea.
     *
     * @return The alignment.
     *
     */
    virtual Alignment getAlignment() const;

    /**
     * Gets the total height needed by the text, taking number of lines and line spacing
     * into consideration.
     *
     * @return the total height needed by the text.
     */
    virtual int16_t getTextHeight() const;

    /**
     * Gets the width in pixels of the current associated text in the current selected
     * language. In case of multi-lined text the width of the widest line is returned.
     *
     * @return The width in pixels of the current text.
     */
    virtual uint16_t getTextWidth() const;

    virtual void draw(const Rect& area) const;

    /**
     * Sets the TypedText of the text area. If no prior size has been set, the TextArea will
     * be resized to fit the new TypedText.
     *
     * @param  t The TypedText for this widget to display.
     *
     * @see resizeToCurrentText
     */
    void setTypedText(const TypedText& t);

    /**
     * Gets the TypedText of the text area.
     *
     * @return The currently used TypedText.
     */
    const TypedText& getTypedText() const
    {
        return typedText;
    }

    /**
     * Sets rotation of the text in the TextArea. The value TEXT_ROTATE_0 is the default for
     * normal text. The value TEXT_ROTATE_90 will rotate the text clockwise, thus writing
     * from the top of the display and down. Similarly TEXT_ROTATE_180 and TEXT_ROTATE_270
     * will each rotate the text further 90 degrees clockwise.
     *
     * @param  textRotation The rotation of the text.
     */
    FORCE_INLINE_FUNCTION void setRotation(const TextRotation textRotation)
    {
        rotation = textRotation;
        boundingArea = calculateBoundingArea();
    }

    /**
     * Gets rotation of the text in the TextArea.
     *
     * @return The rotation of the text.
     *
     * @see setRotation
     */
    TextRotation getRotation() const
    {
        return rotation;
    }

    /**
     * Sets the dimensions of the TextArea to match the width and height of the current
     * associated text for the current selected language.
     *
     * If WordWrap is turned on for the TextArea, the height might be set to an unexpected
     * value, as only manually insert line breaks in the text will be respected - use
     * resizeHeightToCurrentText() to keep the width of the TextArea and therefore retain
     * word wrapping.
     *
     * If the text is centered or right aligned, calling resizeToCurrentText() will actually
     * move the text on the screen, as the x and y coordinates of the TextArea widget is not
     * changed. To simply minimize the size of the TextArea but keep the TypedText in the
     * same position on the screen, use resizeToCurrentTextWithAlignment(). This is also the
     * case if the text is rotated, e.g. 180 degrees.
     *
     * @see setRotation, resizeHeightToCurrentText
     *
     * @note If the current text rotation is either 90 or 270 degrees, the width of the text area
     *       will be set to the height of the text and vice versa, as the text is rotated.
     */
    void resizeToCurrentText();

    /**
     * Sets the dimensions of the TextArea to match the width and height of the current
     * associated text for the current selected language, and for centered and right aligned
     * text, the position of the TextArea widget is also updated to keep the text in the
     * same position on the display. Text that is rotated is also handled properly.
     *
     * @see setRotation, resizeHeightToCurrentText
     *
     * @note If the current text rotation is either 90 or 270 degrees, the width of the text area
     *       will be set to the height of the text and vice versa, as the text is rotated.
     */
    void resizeToCurrentTextWithAlignment();

    /**
     * Sets the height of the TextArea to match the height of the current associated text
     * for the current selected language. This is especially useful for texts with WordWrap
     * enabled.
     *
     * @see resizeToCurrentText, setWideTextAction, setRotation,
     *      resizeHeightToCurrentTextWithRotation
     *
     * @note If the current text rotation is either 90 or 270 degrees, the width of the text area
     *       will be set and not the height, as the text is rotated.
     * @note If the current text is rotated, the x/y coordinate is not updated, which means that
     *       the text will be repositioned on the display.
     */
    void resizeHeightToCurrentText();

    /**
     * Sets the height of the TextArea to match the height of the current associated text
     * for the current selected language. This is especially useful for texts with WordWrap
     * enabled.
     *
     * @see resizeToCurrentText, setWideTextAction, setRotation, resizeHeightToCurrentText
     *
     * @note If the current text rotation is either 90 or 270 degrees, the width of the text area
     *       will be set and not the height, as the text is rotated. Also, the x or y
     *       coordinates will be updated.
     */
    void resizeHeightToCurrentTextWithRotation();

    /**
     * Defines what to do if a line of text is wider than the text area. Default action is
     * ::WIDE_TEXT_NONE which means that text lines are only broken if there is a manually
     * inserted newline in the text.
     *
     * If wrapping is enabled and the text would occupy more lines than the size of the
     * TextArea, the end of the last line will get an ellipsis (often &hellip;) to signal
     * that some text is missing. The character used for ellipsis is taken from the text
     * spreadsheet.
     *
     * @param  action The action to perform for wide lines of text.
     *
     * @see WideTextAction, getWideTextAction, resizeHeightToCurrentText
     */
    FORCE_INLINE_FUNCTION void setWideTextAction(WideTextAction action)
    {
        wideTextAction = action;
        boundingArea = calculateBoundingArea();
    }

    /**
     * Gets wide text action previously set using setWideTextAction.
     *
     * @return current WideTextAction setting.
     *
     * @see setWideTextAction, WideTextAction
     */
    WideTextAction getWideTextAction() const
    {
        return wideTextAction;
    }

    /**
     * Gets the total height needed by the text. Determined by number of lines and
     * linespace. The number of parameters passed after the format, must match the number of
     * wildcards in the TypedText.
     *
     * @param  format The text containing &lt;placeholder> wildcards.
     * @param  ...    Variable arguments providing additional information.
     *
     * @return the total height needed by the text.
     */
    virtual int16_t calculateTextHeight(const Unicode::UnicodeChar* format, ...) const;

    /**
     * Gets the first wildcard used in the TypedText.
     *
     * @return A pointer to the first wildcard, if this text area has a wildcard, otherwise 0.
     *
     * @see TextAreaWithOneWildcard, TextAreaWithTwoWildcards
     */
    virtual const Unicode::UnicodeChar* getWildcard1() const
    {
        return 0;
    }

    /**
     * Gets the second wildcard used in the TypedText.
     *
     * @return A pointer to the second wildcard, if this text area has two wildcards, otherwise 0.
     *
     * @see TextAreaWithOneWildcard, TextAreaWithTwoWildcards
     */
    virtual const Unicode::UnicodeChar* getWildcard2() const
    {
        return 0;
    }

    virtual void invalidateContent() const;

protected:
    /** Structure for the relationship between a bounding rectangle and the contained text. */
    class BoundingArea
    {
    public:
        /**
         * Initializes a new instance of the BoundingArea class.
         *
         * @param  boundingRect  The bounding rectangle of this text area.
         * @param  containedText  A pointer to the text contained in the bounding rectangle.
         */
        BoundingArea(const Rect& boundingRect, const Unicode::UnicodeChar* containedText)
            : rect(boundingRect), text(containedText)
        {
        }

        /**
         * Initializes a new instance of the BoundingArea class which is invalid by default.
         */
        BoundingArea()
            : rect(Rect(0, 0, -1, -1)), // Negative width and height means invalid rectangle
              text(0)
        {
        }

        /**
         * Gets bounding rectangle.
         *
         * @return The bounding rectangle.
         */
        Rect getRect() const
        {
            return rect;
        }

        /**
         * Query if the bounding area is valid.
         *
         * @param  currentText  A pointer to the current text of this text area.
         *
         * @return True if valid otherwise false.
         */
        bool isValid(const Unicode::UnicodeChar* currentText) const
        {
            return (rect.height >= 0 && rect.width >= 0 && text == currentText);
        }

    private:
        Rect rect;
        const Unicode::UnicodeChar* text;
    };

    /**
     * Calculates the minimum bounding rectangle of this text area and correlates it
     * with the containing text, to get the bounding area.
     * Note: The bounding rectangle is adjusted according to alignment and rotation.
     *
     * @return The bounding area.
     */
    virtual TextArea::BoundingArea calculateBoundingArea() const;

    TypedText typedText;                ///< The TypedText to display
    colortype color;                    ///< The color to use for the text.
    int16_t linespace;                  ///< The extra space between lines of text, measured in pixels.
    uint8_t alpha;                      ///< The alpha to use.
    uint8_t indentation;                ///< The indentation of the text inside the text area.
    TextRotation rotation;              ///< The text rotation to use in steps of 90 degrees.
    WideTextAction wideTextAction;      ///< What to do if the lines of text are wider than the text area.
    static const uint16_t newLine = 10; ///< NewLine value.
    BoundingArea boundingArea;          ///< Bounding area of this text area.
};

} // namespace touchgfx

#endif // TOUCHGFX_TEXTAREA_HPP
