#pragma once

#include <SFML/Graphics.hpp>

#include <string>

using namespace std;

class Button
{
public:
    sf::RectangleShape container;
    sf::Text content;
    sf::Texture texture;
    
    Button(string &string, sf::Font &font, sf::Vector2f &size, sf::Vector2f &position);
    void setTexture(const sf::Texture *texture);
    void setOpacity(bool hover);
    bool isHovering(sf::RenderWindow &window);
    sf::FloatRect getGlobalBounds();
};