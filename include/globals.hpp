#pragma once

#include <SFML/Graphics.hpp>

#include "textBox.hpp"
#include "button.hpp"
#include "config.hpp"
#include "menu.hpp"

extern sf::Font globalFont, textFont;
extern sf::RenderWindow window;
extern sf::RectangleShape topSeparator, bottomSeparator, lineNumbersBackground, toolBarBackground, statusBarBackground, cursorBox, cursorLineHighlight, box;
extern sf::RenderTexture aboveCurrentLineText, belowCurrentLineText, lineNumbersText;
extern sf::Sprite aboveCurrentLineSprite, belowCurrentLineSprite, lineNumbersSprite;

extern bool fileSaved;

extern int windowWidth, windowHeight;

extern bool showLineNumbers;
extern Theme theme;
extern ThemeColors currentThemeColors;

extern int lineNumbersMaxDigits;
extern int fontSize;
extern int zoomLevel;
extern float lineHeight;
extern float marginLeft, paddingLeft;
extern std::pair<int, int> segmSelected, segmOnScreen[];
extern std::vector<std::string> renderLines;
extern int sizeRLines;

extern std::string path;
