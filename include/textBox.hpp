#pragma once

#include <SFML/Graphics.hpp>
#include <string>

class TextBox
{
    sf::RectangleShape container;
    sf::Text content;

public:
    TextBox(const std::string &label, const sf::Vector2f &position, bool const &outline = true);

    void setContent(const std::string &_content);
    void setPosition(const sf::Vector2f &position);
    void updateThemeColors();
    sf::Vector2f getSize();
    void draw();
    void draw(sf::RenderWindow &window);
};

extern TextBox *lineColumnTextBox, *zoomLevelTextBox, *selectedCharacterCountTextBox, *lineCountTextBox, *fileNameTextBox;
