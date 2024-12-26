#pragma once

#include <SFML/Graphics.hpp>

#include <string>
#include <vector>

#include "button.hpp"

class Menu 
{
    bool isOpen;
    int buttonCount;
    sf::RectangleShape container;
    Button **buttons;
    Button *toggleButton;

public:
    Menu(Button *_toggleButton, const std::vector<std::string> &buttonLabels, const sf::Vector2f &position);

    bool isHovering();

    bool getIsOpen();
    sf::Vector2f getPosition();
    int getButtonCount();
    Button** getButtons();
    Button* getToggleButton();

    void setIsOpen(bool _isOpen);
    void setPosition(const sf::Vector2f &position);

    void draw();
};
