﻿#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <random>
#include <windows.h>
#include <commdlg.h>
#include <ctime>
#include <cassert>
#include <set>
#include <bitset>

#include "helpers.hpp"
#include "replace.hpp"
#include "windows.hpp"
#include "globals.hpp"
#include "render.hpp"
#include "string.hpp"
#include "config.hpp"
#include "button.hpp"
#include "menu.hpp"

using namespace std;

vector<sf::RectangleShape> selectedBoxes;

sf::Event event;
sf::Text text, ptext1;

int cursorHeight = 0, cursorWidth = 0;

int main()
{
    window.create(sf::VideoMode(windowWidth, windowHeight), "Neon - Text Editor");
    window.setFramerateLimit(90);

    sf::RenderWindow modal;
    modal.create(sf::VideoMode(200, 200), "asdf", sf::Style::Close);

    sf::View view;
    sf::Image icon;

    icon.loadFromFile("assets/images/icon.png");
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

    sf::Texture texture;
    texture.loadFromFile("assets/images/povesti.png");
    sf::Sprite povesti;
    povesti.setTexture(texture, true);

    globalFont.loadFromFile("assets/fonts/kanit.ttf");
    textFont.loadFromFile("assets/fonts/kanit.ttf");
    checkmarkFont.loadFromFile("assets/fonts/arial_unicode_ms.otf");

    vector<sf::Vector2f> toolBarPositions = Helpers::getToolBarPositions(), statusBarPositions = Helpers::getStatusBarPositions();

    menus = new Menu *[5];
    for (int i = 0; i < 3; i++)
        menus[i] = new Menu(menuLabels[i], menuButtonLabels[i], toolBarPositions[i]);

    menus[3] = new Menu(menus[2]->getButtons()[3], menuButtonLabels[3], menus[2]->getButtons()[3]->getPosition(), true);
    menus[4] = new Menu(menus[2]->getButtons()[4], menuButtonLabels[4], menus[2]->getButtons()[4]->getPosition(), true);

    Menu *fileMenu = menus[0], *editMenu = menus[1], *optionsMenu = menus[2], *themeMenu = menus[3], *fontMenu = menus[4];
    Button **fileMenuButtons = fileMenu->getButtons(), **editMenuButtons = editMenu->getButtons(), **optionsMenuButtons = optionsMenu->getButtons(), **themeMenuButtons = themeMenu->getButtons(), **fontMenuButtons = fontMenu->getButtons();

    themeMenuButtons[0]->setIsActive(true);
    fontMenuButtons[0]->setIsActive(true);
    fileNameTextBox = new TextBox("", toolBarPositions[3], false);

    lineCountTextBox = new TextBox("", statusBarPositions[0]);
    lineColumnTextBox = new TextBox("", statusBarPositions[1]);
    selectedCharacterCountTextBox = new TextBox("", statusBarPositions[2]);
    zoomOutButton = new Button(zoomButtonLabels[0], statusBarPositions[3], true, ButtonSize::SMALL);
    zoomLevelTextBox = new TextBox("", statusBarPositions[4]);
    zoomInButton = new Button(zoomButtonLabels[1], statusBarPositions[5], true, ButtonSize::SMALL);

    lineHeight = Helpers::getLineHeight();

    text.setFont(textFont);
    text.setFillColor(currentThemeColors.text);
    text.setCharacterSize(fontSize);
    text.setStyle(sf::Text::Regular);

    ptext1.setFont(textFont);
    ptext1.setFillColor(currentThemeColors.text);
    ptext1.setCharacterSize(fontSize);
    ptext1.setStyle(sf::Text::Regular);

    cursorLineHighlight.setFillColor(currentThemeColors.cursorLineHighlight);

    topSeparator.setPosition(0, marginTop);
    topSeparator.setSize(sf::Vector2f(windowWidth, 1));
    topSeparator.setFillColor(currentThemeColors.separator);

    bottomSeparator.setPosition(0, windowHeight - marginBottom);
    bottomSeparator.setSize(sf::Vector2f(windowWidth, 1));
    bottomSeparator.setFillColor(currentThemeColors.separator);

    lineNumbersBackground.setPosition(0, marginTop);
    lineNumbersBackground.setSize(sf::Vector2f(marginLeft, windowHeight));
    lineNumbersBackground.setFillColor(currentThemeColors.lineNumbersBackground);

    toolBarBackground.setPosition(0, 0);
    toolBarBackground.setSize(sf::Vector2f(windowWidth, marginTop));
    toolBarBackground.setFillColor(currentThemeColors.bar);
    
    statusBarBackground.setPosition(0, windowHeight - marginBottom);
    statusBarBackground.setSize(sf::Vector2f(windowWidth, marginBottom));
    statusBarBackground.setFillColor(currentThemeColors.bar);

    int Yoffset = 0, Xoffset = 0;
    int scrollUnitX = charWidth[fontIndex][fontSize][0], scrollUnitY = Helpers::getLineHeight();

    bool firstExec = 1;
    int l1 = 0, l2 = 0;
    int lastCursorLine = 0;

    aboveCurrentLineText.create(maxRows, maxRows);
    belowCurrentLineText.create(maxRows, maxRows);
    lineNumbersText.create(maxRows, maxRows);

    string buffer = "";
    int cursorTimer = 0;
    bool cursorOnScreen = 0;
    bool cursorLineOnScreen = 1;
    bool leftButtonPressed = 0;
    bool selectFlag = 0;
    bool ctrlX = 0, ctrlV = 0, ctrlC = 0 , ctrlO = 0 , ctrlS = 0 , ctrlF = 0 , ctrlR = 0 , ctrlA = 0 , ctrlL = 0 , ctrlG = 0 , ctrlN = 0 , ctrlShiftS = 0;

    int posCursorOnHold = -1;

    int lastDone = 0;
    int nr = 0;
    int cntX = 0;
    FILE *fptr = NULL;

    vector<int> positions, bit, gone, prv, nxt;
    int currentAppearance = 0;
    bool matchCase = 0, wholeWord = 0, findFlag = 0, replaceFlag = 0, replaceAllFlag = 0;
    string word, rword;
    string param;
    set<int> notRemoved;
    char* data = NULL;
    char* originalData = NULL;
    int currPosNewFile = 0;
    bool readFileFlag = 0;

    HANDLE fileHandle = NULL , mappingHandle = NULL;
    DWORD fileSize = 0;

    auto start = std::chrono::high_resolution_clock::now();
    
    while (window.isOpen())
    {
        bool flag = 0;
        bool fontSizeChanged = 0;
        bool renderAgain = 0;
        bool updateFindReplace = 0;

        sf::Event ev;

        while (modal.pollEvent(ev))
        {
            if (ev.type == sf::Event::Closed)
                modal.close();

            if (ev.type == sf::Event::MouseButtonPressed)
            {
                sf::Vector2i localPosition = sf::Mouse::getPosition(modal);
                cerr << localPosition.x << ' ' << localPosition.y << '\n';
                break;
            }
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            sf::Vector2i localPosition = sf::Mouse::getPosition(window);

            if (!Helpers::isAnyButtonPressed())
            {
                if (localPosition.x >= marginLeft && localPosition.y >= marginTop && localPosition.x < windowWidth && localPosition.y < windowHeight - marginBottom)
                {
                    int posCursor = String::findCursorPosition(S);
                    int newPosCursor = Render::moveCursorToClick(localPosition, S, scrollUnitY, l1, l2, Xoffset);
                    newPosCursor -= (newPosCursor > posCursor);

                    if (leftButtonPressed == 0)
                    {
                        String::del(posCursor, S);
                        leftButtonPressed = 1;
                        selectFlag = 0;

                        String::insert(newPosCursor, S);
                        Xoffset |= Render::updateViewX(S, Xoffset, scrollUnitX);
                    }
                    else if (newPosCursor != posCursor)
                    {
                        selectFlag = 1;

                        if (newPosCursor < posCursor)
                            segmSelected = {newPosCursor, posCursor - 1};
                        else
                            segmSelected = {posCursor + 1, newPosCursor};
                    }

                    flag = 1;
                    renderAgain = 1;
                }

                if (selectFlag && localPosition.x < marginLeft)
                {
                    Xoffset -= scrollUnitX;
                    Xoffset = max(0, Xoffset);
                    flag = 1;
                    renderAgain = 1;
                }

                if (selectFlag && localPosition.x >= windowWidth)
                {
                    Xoffset += scrollUnitX;
                    flag = 1;
                    renderAgain = 1;
                }

                if (selectFlag && localPosition.y < marginTop)
                {
                    Yoffset -= scrollUnitY;
                    Yoffset = max(0, Yoffset);
                    flag = 1;
                    renderAgain = 1;
                }

                if (selectFlag && localPosition.y >= windowHeight - marginBottom)
                {
                    Yoffset += scrollUnitY;

                    if (Yoffset / lineHeight + 1 > String::findNumberOfEndlines(1, String::len(S), S) + 1)
                        Yoffset -= scrollUnitY;

                    flag = 1;
                    renderAgain = 1;
                }
            }
        }
        else if (selectFlag && ctrlX == 0 && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::X))
        {
            ctrlX = 1;
            int L = segmSelected.first;
            int R = segmSelected.second;

            String::Treap *s1 = 0, *s2 = 0, *s3 = 0;
            String::split(S, s2, s3, R);
            String::split(s2, s1, s2, L - 1);

            buffer = String::constructString(s2);
            String::copyTextToClipboard(buffer.c_str());
            String::del(s2);

            String::merge(S, s1, s3);
            selectFlag = 0;
            findFlag = 0;
            replaceFlag = 0;

            flag = 1;
            renderAgain = 1;
        }
        else if (selectFlag && ctrlC == 0 && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::C))
        {
            ctrlC = 1;
            int L = segmSelected.first;
            int R = segmSelected.second;

            String::Treap *s1 = 0, *s2 = 0, *s3 = 0;
            String::split(S, s2, s3, R);
            String::split(s2, s1, s2, L - 1);

            buffer = String::constructString(s2);
            String::copyTextToClipboard(buffer.c_str());

            String::merge(S, s1, s2);
            String::merge(S, S, s3);
        }
        else if (ctrlV == 0 && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::V))
        {
            int posCursor = String::findCursorPosition(S);
            ctrlV = 1;
            buffer = String::getTextFromClipboard();

            for (auto ch : buffer)
            {
                String::insert(posCursor, S, ch);
                posCursor++;
            }

            flag = 1;
            renderAgain = 1;
            selectFlag = 0;
        }
        else if (ctrlA == 0 && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        {
            ctrlA = 1;

            if (String::len(S) > 1)
            {
                segmSelected = {1, String::len(S) - 1};
                String::del(String::findCursorPosition(S), S);
                String::insert(String::len(S) + 1 , S);
                flag = 1;
                selectFlag = 1;
            }
        }
        else if (ctrlO == 0 && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::O))
        {
            ctrlO = 1;

            path = Windows::open();

            if (path.size() != 0)
            {
                start = std::chrono::high_resolution_clock::now();

                fileHandle = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

                if (fileHandle == INVALID_HANDLE_VALUE)
                {
                    Windows::throwMessage("Invalid Path");
                    break;
                }

                mappingHandle = CreateFileMapping(fileHandle, nullptr, PAGE_READONLY, 0, 0, nullptr);
                originalData = data = (char*)(MapViewOfFile(mappingHandle, FILE_MAP_READ, 0, 0, 0));
                fileSize = GetFileSize(fileHandle, nullptr);

                currPosNewFile = 0;

                String::del(S);
                S = new String::Treap(cursorChar, 1);

                readFileFlag = 1;
                renderAgain = 1;
                fileSaved = 1;
                flag = 1;
            }
        }
        else if (ctrlS == 0 && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        {
            ctrlS = 1;

            if(path.size() == 0)
                path = Windows::saveAS();

            FILE* fptr = fopen(path.c_str(), "w");

            if (fptr == NULL)
            {
                Windows::throwMessage("Wrong Path!");
            }
            else
            {
                String::saveText(fptr, S);
                fileSaved = 1;
                fclose(fptr);
            }
        }
        else if (ctrlF == 0 && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::F))
        {
            ctrlF = 1;

            selectFlag = findFlag = replaceFlag = 0;

            /// apelat dupa ce faci setarile cu wholeWord si matchCase
            word = Windows::getStringFromUser("Find");

            if (word.size() != 0)
            {
                string s = String::constructRawString(S);

                if (matchCase == 0)
                {
                    for (auto& i : word)
                        if (i >= 'A' && i <= 'Z')
                            i = i - 'A' + 'a';

                    for (auto& i : s)
                        if (i >= 'A' && i <= 'Z')
                            i = i - 'A' + 'a';
                }

                Replace::KMP(s, word, positions, wholeWord);

                if (positions.size() == 0)
                {
                    Windows::throwMessage("There are 0 matchings!");
                    renderAgain = 1;
                    flag = 1;
                }
                else
                {
                    updateFindReplace = 1;
                    currentAppearance = 0;
                    findFlag = 1;
                    renderAgain = 1;
                    flag = 1;
                }
            }
        }
        else if (ctrlR == 0 && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::R))
        {
            ctrlR = 1;

            selectFlag = findFlag = replaceFlag = 0;

            word = Windows::getStringFromUser("word");
            rword = Windows::getStringFromUser("word");

            if (word.size() != 0)
            {
                string s = String::constructRawString(S);

                if (matchCase == 0)
                {
                    for (auto& i : word)
                        if (i >= 'A' && i <= 'Z')
                            i = i - 'A' + 'a';

                    for (auto& i : s)
                        if (i >= 'A' && i <= 'Z')
                            i = i - 'A' + 'a';
                }

                Replace::KMP(s, word, positions, wholeWord);

                if (positions.size() == 0)
                {
                    Windows::throwMessage("There are 0 matchings!");
                    renderAgain = 1;
                    flag = 1;
                }
                else
                {
                    prv.clear();
                    bit.clear();
                    nxt.clear();
                    gone.clear();

                    prv.resize(positions.size(), -1);
                    nxt.resize(positions.size(), -1);
                    gone.resize(positions.size(), 0);
                    bit.resize(positions.size(), 0);
                    notRemoved.clear();

                    for (int i = 1; i < positions.size(); i++)
                        prv[i] = i - 1;

                    for (int i = 0; i + 1 < positions.size(); i++)
                        nxt[i] = i + 1;

                    for (int i = 0; i < positions.size(); i++)
                        notRemoved.insert(i);

                    currentAppearance = 0;
                    replaceFlag = 1;
                    renderAgain = 1;
                    updateFindReplace = 1;
                    flag = 1;
                }
            }
        }
        else if (ctrlL == 0 && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::L))
        {
            ctrlL = 1;
            showLineNumbers = !showLineNumbers;
            optionsMenuButtons[0]->setLabel(toggleLinesButtonLabels[showLineNumbers]);
            flag = 1;
            renderAgain = 1;
        }
        else if (ctrlG == 0 && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::G))
        {
            ctrlG = 1;
            string line = Windows::getStringFromUser("Go to line");
            if (line != "")
            {
                int l = min(String::findNumberOfEndlines(1, String::len(S), S) + 1, stoi(line));
                Xoffset = 0;
                Yoffset = (l - 1) * lineHeight;
                renderAgain = 1;
                flag = 1;
            }
        }
        else if (ctrlN == 0 && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::N))
        {
            ctrlN = 1;

            if (fileSaved == 0)
            {
                int closeOption = Windows::saveModal();

                if (closeOption == IDCANCEL)
                    break;
                else if (closeOption == IDYES)
                {
                    if (path.size() == 0)
                        path = Windows::saveAS();

                    FILE* fptr = fopen(path.c_str(), "w");

                    if (fptr == NULL)
                    {
                        Windows::throwMessage("Wrong Path!");
                        break;
                    }

                    String::saveText(fptr, S);
                    fileSaved = 1;
                    fclose(fptr);
                    break;
                }
            }

            String::del(S);
            S = new String::Treap(cursorChar, 1);
            renderAgain = 1;
            flag = 1;
            fileSaved = 1;
        }
        else if (ctrlShiftS == 0 && sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) && sf::Keyboard::isKeyPressed(sf::Keyboard::G))
        {
            ctrlShiftS = 1;
            path = Windows::saveAS();

            FILE* fptr = fopen(path.c_str(), "w");

            if (fptr == NULL)
            {
                Windows::throwMessage("Wrong Path!");
            }
            else
            {
                String::saveText(fptr, S);
                fileSaved = 1;
                fclose(fptr);
            }
        }
        else while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                {
                    if (fileSaved == 1)
                    {
                        window.close();
                        break;
                    }

                    int closeOption = Windows::saveModal();

                    if (closeOption == IDCANCEL)
                        break;
                    else if (closeOption == IDYES)
                    {
                        if (path.size() == 0)
                            path = Windows::saveAS();

                        FILE* fptr = fopen(path.c_str(), "w");

                        if (fptr == NULL)
                        {
                            Windows::throwMessage("Wrong Path!");
                            break;
                        }

                        String::saveText(fptr, S);
                        fileSaved = 1;
                        fclose(fptr);
                        break;
                    }
                    else if (closeOption == IDNO)
                    {
                        window.close();
                        break;
                    }
                }

                if (event.type == sf::Event::MouseMoved)
                {
                    zoomOutButton->setHoverState(zoomOutButton->isHovering());
                    zoomInButton->setHoverState(zoomInButton->isHovering());

                    // open menus if user is hovering over them
                    // set hover state for toggle menu buttons
                    for (int i = 0; i < 3; i++)
                    {
                        Button *toggleButton = menus[i]->getToggleButton();
                        if (toggleButton->isHovering())
                        {
                            toggleButton->setHoverState(true);
                            menus[i]->open();
                            if (i == 1)
                                menus[i]->setPosition(toolBarPositions[i]);
                        }
                        else
                            toggleButton->setHoverState(false);
                    }
                    if (optionsMenu->getIsOpen() && themeMenu->getToggleButton()->isHovering() && !fontMenu->getIsOpen())
                        themeMenu->open();
                    if (optionsMenu->getIsOpen() && fontMenu->getToggleButton()->isHovering() && !themeMenu->getIsOpen())
                        fontMenu->open();

                    // close menus if user isn't hovering over them anymore
                    if (fileMenu->getIsOpen() && !fileMenu->isHovering() && !fileMenu->getToggleButton()->isHovering())
                        fileMenu->close();
                    if (editMenu->getIsOpen() && !editMenu->isHovering() && !editMenu->getToggleButton()->isHovering() && editMenu->getPosition() == toolBarPositions[1])
                        editMenu->close();
                    if (optionsMenu->getIsOpen() && !optionsMenu->isHovering() && !optionsMenu->getToggleButton()->isHovering() && !themeMenu->isHovering() && !fontMenu->isHovering())
                        optionsMenu->close();
                    if (themeMenu->getIsOpen() && !themeMenu->isHovering() && !themeMenu->getToggleButton()->isHovering())
                        themeMenu->close();
                    if (fontMenu->getIsOpen() && !fontMenu->isHovering() && !fontMenu->getToggleButton()->isHovering())
                        fontMenu->close();

                    // set hover state for buttons inside menus
                    for (int i = 0; i < 5; i++)
                    {
                        Button **buttons = menus[i]->getButtons();
                        for (int j = 0; j < menus[i]->getButtonCount(); j++)
                            buttons[j]->setHoverState(buttons[j]->isHovering());
                    }
                }

                if (event.type == sf::Event::MouseButtonReleased)
                {
                    if (event.key.code == 0)
                        leftButtonPressed = 0;

                    ctrlC = ctrlV = ctrlX = ctrlF = ctrlR = ctrlA = ctrlO = ctrlS = ctrlG = ctrlL = ctrlN = 0;
                }

                if (event.type == sf::Event::KeyReleased)
                {
                    int key = event.key.code;
                    cerr << "Key is " << key << '\n';
                    ctrlC = ctrlV = ctrlX = ctrlF = ctrlR = ctrlA = ctrlO = ctrlS = ctrlG = ctrlL = ctrlN = 0;

                    break;
                }

                if (event.type == sf::Event::MouseButtonPressed)
                {
                    int key = event.key.code;

                    if (key == 0)
                    {
                        if (zoomOutButton->isHovering())
                        {
                            fontSize -= fontUnit;
                            fontSize = max(minFontSize, fontSize);
                            zoomLevel = 100 * fontSize / initialFontSize;
                            zoomLevelTextBox->setContent(to_string(zoomLevel) + "%");
                            lineHeight = Helpers::getLineHeight();

                            String::updateWidth(S);
                            fontSizeChanged = 1;
                            cursorTimer = 0;
                        }
                        else if (zoomInButton->isHovering())
                        {
                            fontSize += fontUnit;
                            fontSize = min(fontSize, maxFontSize);
                            zoomLevel = 100 * fontSize / initialFontSize;
                            zoomLevelTextBox->setContent(to_string(zoomLevel) + "%");

                            lineHeight = Helpers::getLineHeight();

                            String::updateWidth(S);
                            fontSizeChanged = 1;
                            cursorTimer = 0;
                        }

                        else if (fileMenu->getIsOpen())
                        {
                            if (fileMenuButtons[0]->isHovering())
                            {
                                fileMenu->close();

                                if (fileSaved == 0)
                                {
                                    int closeOption = Windows::saveModal();

                                    if (closeOption == IDCANCEL)
                                        break;
                                    else if (closeOption == IDYES)
                                    {
                                        if (path.size() == 0)
                                            path = Windows::saveAS();

                                        FILE* fptr = fopen(path.c_str(), "w");

                                        if (fptr == NULL)
                                        {
                                            Windows::throwMessage("Wrong Path!");
                                            break;
                                        }

                                        String::saveText(fptr, S);
                                        fileSaved = 1;
                                        fclose(fptr);
                                        break;
                                    }
                                }

                                String::del(S);
                                S = new String::Treap(cursorChar, 1);
                                renderAgain = 1;
                                flag = 1;
                                fileSaved = 1;
                                // new file logic
                            }
                            else if (fileMenuButtons[1]->isHovering())
                            {
                                fileMenu->close();

                                path = Windows::open();

                                if (path.size() == 0)
                                    break;

                                start = std::chrono::high_resolution_clock::now();

                                fileHandle = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

                                if (fileHandle == INVALID_HANDLE_VALUE)
                                {
                                    Windows::throwMessage("Invalid Path");
                                    break;
                                }

                                mappingHandle = CreateFileMapping(fileHandle, nullptr, PAGE_READONLY, 0, 0, nullptr);
                                originalData = data = (char *)(MapViewOfFile(mappingHandle, FILE_MAP_READ, 0, 0, 0));
                                fileSize = GetFileSize(fileHandle, nullptr);

                                currPosNewFile = 0;

                                String::del(S);
                                S = new String::Treap(cursorChar , 1);

                                readFileFlag = 1;
                                renderAgain = 1;
                                fileSaved = 1;
                                flag = 1;
                            }
                            else if (fileMenuButtons[2]->isHovering())
                            {
                                fileMenu->close();

                                if (path.size() == 0)
                                    path = Windows::saveAS();

                                FILE *fptr = fopen(path.c_str(), "w");

                                if (fptr == NULL)
                                {
                                    Windows::throwMessage("Wrong Path!");
                                    break;
                                }

                                String::saveText(fptr, S);
                                fileSaved = 1;
                                fclose(fptr);
                            }
                            else if (fileMenuButtons[3]->isHovering())
                            {
                                fileMenu->close();

                                path = Windows::saveAS();

                                if (path.size() == 0)
                                    break;

                                FILE *fptr = fopen(path.c_str(), "w");

                                if (fptr == NULL)
                                {
                                    Windows::throwMessage("Wrong Path!");
                                    break;
                                }
                                
                                String::saveText(fptr, S);
                                fileSaved = 1;
                                fclose(fptr);
                            }
                            else if (fileMenuButtons[4]->isHovering())
                            {
                                fileMenu->close();
                                if (fileSaved == 1)
                                {
                                    window.close();
                                    break;
                                }

                                int closeOption = Windows::saveModal();

                                if (closeOption == IDCANCEL)
                                    break;
                                else if (closeOption == IDYES)
                                {
                                    if (path.size() == 0)
                                        path = Windows::saveAS();

                                    FILE* fptr = fopen(path.c_str(), "w");

                                    if (fptr == NULL)
                                    {
                                        Windows::throwMessage("Wrong Path!");
                                        break;
                                    }

                                    String::saveText(fptr, S);
                                    fileSaved = 1;
                                    fclose(fptr);
                                    break;
                                }
                                else if (closeOption == IDNO)
                                {
                                    window.close();
                                    break;
                                }
                            }
                        }
                        else if (editMenu->getIsOpen())
                        {
                            if (editMenuButtons[0]->isHovering())
                            {
                                editMenu->close();

                                if (selectFlag && ctrlC == 0)
                                {
                                    ctrlC = 1;
                                    int L = segmSelected.first;
                                    int R = segmSelected.second;

                                    String::Treap *s1 = 0, *s2 = 0, *s3 = 0;
                                    String::split(S, s2, s3, R);
                                    String::split(s2, s1, s2, L - 1);

                                    buffer = String::constructString(s2);
                                    String::copyTextToClipboard(buffer.c_str());

                                    String::merge(S, s1, s2);
                                    String::merge(S, S, s3);
                                }
                            }
                            else if (editMenuButtons[1]->isHovering())
                            {
                                editMenu->close();

                                if (ctrlV == 0)
                                {
                                    int posCursor = String::findCursorPosition(S);
                                    ctrlV = 1;

                                    buffer = String::getTextFromClipboard();

                                    for (auto ch : buffer)
                                    {
                                        String::insert(posCursor, S, ch);
                                        posCursor++;
                                    }

                                    flag = 1;
                                    renderAgain = 1;
                                    fileSaved = 0;
                                    selectFlag = findFlag = replaceFlag = 0;
                                }
                            }
                            else if (editMenuButtons[2]->isHovering())
                            {
                                editMenu->close();

                                if (selectFlag && ctrlX == 0)
                                {
                                    ctrlX = 1;
                                    int L = segmSelected.first;
                                    int R = segmSelected.second;

                                    String::Treap *s1 = 0, *s2 = 0, *s3 = 0;
                                    String::split(S, s2, s3, R);
                                    String::split(s2, s1, s2, L - 1);

                                    buffer = String::constructString(s2);
                                    String::copyTextToClipboard(buffer.c_str());

                                    String::del(s2);

                                    String::merge(S, s1, s3);
                                    selectFlag = findFlag = replaceFlag = 0;
                                    fileSaved = 0;
                                    flag = 1;
                                    renderAgain = 1;
                                }
                            }
                            else if (editMenuButtons[3]->isHovering())
                            {
                                editMenu->close();
                                selectFlag = findFlag = replaceFlag = 0;

                                /// apelat dupa ce faci setarile cu wholeWord si matchCase
                                word = Windows::getStringFromUser("Find");

                                if (word.size() == 0)
                                    break;

                                string s = String::constructRawString(S);

                                if (matchCase == 0)
                                {
                                    for (auto &i : word)
                                        if (i >= 'A' && i <= 'Z')
                                            i = i - 'A' + 'a';

                                    for (auto &i : s)
                                        if (i >= 'A' && i <= 'Z')
                                            i = i - 'A' + 'a';
                                }

                                Replace::KMP(s, word, positions, wholeWord);

                                if (positions.size() == 0)
                                {
                                    Windows::throwMessage("There are 0 matchings!");
                                    renderAgain = 1;
                                    flag = 1;
                                    break;
                                }

                                updateFindReplace = 1;
                                currentAppearance = 0;
                                findFlag = 1;
                                renderAgain = 1;
                                flag = 1;

                                break;
                            }
                            else if (editMenuButtons[4]->isHovering())
                            {
                                editMenu->close();
                                selectFlag = findFlag = replaceFlag = 0;

                                word = Windows::getStringFromUser("word");
                                rword = Windows::getStringFromUser("word");

                                if (word.size() == 0)
                                    break;

                                string s = String::constructRawString(S);

                                if (matchCase == 0)
                                {
                                    for (auto &i : word)
                                        if (i >= 'A' && i <= 'Z')
                                            i = i - 'A' + 'a';

                                    for (auto &i : s)
                                        if (i >= 'A' && i <= 'Z')
                                            i = i - 'A' + 'a';
                                }

                                Replace::KMP(s, word, positions, wholeWord);

                                if (positions.size() == 0)
                                {
                                    Windows::throwMessage("There are 0 matchings!");
                                    renderAgain = 1;
                                    flag = 1;
                                    break;
                                }

                                prv.clear();
                                bit.clear();
                                nxt.clear();
                                gone.clear();

                                prv.resize(positions.size(), -1);
                                nxt.resize(positions.size(), -1);
                                gone.resize(positions.size(), 0);
                                bit.resize(positions.size(), 0);
                                notRemoved.clear();

                                for (int i = 1; i < positions.size(); i++)
                                    prv[i] = i - 1;

                                for (int i = 0; i + 1 < positions.size(); i++)
                                    nxt[i] = i + 1;

                                for (int i = 0; i < positions.size(); i++)
                                    notRemoved.insert(i);

                                currentAppearance = 0;
                                replaceFlag = 1;
                                renderAgain = 1;
                                updateFindReplace = 1;
                                flag = 1;

                                break;
                            }
                            else if (editMenuButtons[5]->isHovering())
                            {
                                editMenu->close();
                                selectFlag = findFlag = replaceFlag = 0;

                                if (String::len(S) > 1)
                                {
                                    segmSelected = { 1, String::len(S) - 1 };
                                    String::del(String::findCursorPosition(S), S);
                                    String::insert(String::len(S) + 1, S);
                                    flag = 1;
                                    selectFlag = 1;
                                }

                                break;
                            }
                            else
                                editMenu->close();
                        }
                        else if (optionsMenu->getIsOpen())
                        {
                            if (optionsMenuButtons[0]->isHovering())
                            {
                                optionsMenu->close();

                                showLineNumbers = !showLineNumbers;
                                optionsMenuButtons[0]->setLabel(toggleLinesButtonLabels[showLineNumbers]);
                                flag = 1;
                                renderAgain = 1;
                            }
                            else if (optionsMenuButtons[1]->isHovering())
                            {
                                optionsMenu->close();

                                string line = Windows::getStringFromUser("Go to line");
                                
                                if (line == "")
                                    break;

                                int l = min(String::findNumberOfEndlines(1, String::len(S), S) + 1, stoi(line));
                                Xoffset = 0;
                                Yoffset = (l - 1) * lineHeight;
                                renderAgain = 1;
                                flag = 1;
                                break;
                            }
                            else if (optionsMenuButtons[2]->isHovering())
                            {
                                optionsMenu->close();

                                if (selectFlag == 1)
                                {
                                    int L = segmSelected.first;
                                    int R = segmSelected.second;

                                    String::Treap* s1 = 0, * s2 = 0, * s3 = 0;
                                    String::split(S, s2, s3, R);
                                    String::split(s2, s1, s2, L - 1);

                                    String::del(s2);
                                    String::merge(S, s1, s3);
                                }

                                // TODO: let user customize format via modal
                                string data = Helpers::getTime("%I:%M %p %m/%d/%Y");
                                int posCursor = String::findCursorPosition(S);

                                for (auto i : data)
                                {
                                    String::insert(posCursor, S, i);
                                    posCursor++;
                                }

                                flag = 1;
                                renderAgain = 1;
                                fileSaved = 0;
                            }
                            
                            if (themeMenu->getIsOpen())
                                for (int i = 0; i < themeMenu->getButtonCount(); i++)
                                {
                                    if (themeMenuButtons[i]->isHovering())
                                    {
                                        for (int j = 0; j < themeMenu->getButtonCount(); j++)
                                            themeMenuButtons[j]->setIsActive(false);
                                        themeMenuButtons[i]->setIsActive(true);     
                                        Helpers::changeTheme(themeNamesMapping.at(menuButtonLabels[3][i].first), text, ptext1);
                                        renderAgain = 1;
                                        themeMenu->close();
                                        optionsMenu->close();
                                    }
                                }
                            
                            else if (fontMenu->getIsOpen())
                                for (int i = 0; i < fontMenu->getButtonCount(); i++)
                                {
                                    if (fontMenuButtons[i]->isHovering())
                                    {
                                        for (int j = 0; j < fontMenu->getButtonCount(); j++)
                                            fontMenuButtons[j]->setIsActive(false);
                                        fontMenuButtons[i]->setIsActive(true);                               
                                        Helpers::changeFont(fontNamesMapping.at(menuButtonLabels[4][i].first));
                                        fontIndex = i;
                                        String::updateWidth(S);
                                        scrollUnitX = charWidth[fontIndex][fontSize][0];
                                        scrollUnitY = Helpers::getLineHeight();
                                        cout << fontIndex << '\n';
                                        renderAgain = 1;
                                        fontMenu->close();
                                        optionsMenu->close();
                                    }
                                }
                        }
                        // change cursor position inside text visible in the viewport
                        else
                        {
                            cursorTimer = 0;
                            break;
                        }
                    }

                    // right click
                    // set edit menu position to cursor position and then open it
                    else if (key == 1)
                    {
                        sf::Vector2i localPosition = sf::Mouse::getPosition(window);
                        editMenu->setPosition(sf::Vector2f(localPosition.x, localPosition.y));
                        editMenu->open();
                        break;
                    }

                    flag = 1;

                    selectFlag = findFlag = 0;

                    renderAgain |= Render::updateViewX(S, Xoffset, scrollUnitX);
                    renderAgain |= Render::updateViewY(S, Yoffset, scrollUnitY);

                    break;
                }

                if (event.type == sf::Event::KeyPressed)
                {
                    int key = event.key.code;

                    if (key == 36) /// escape
                    {
                        if (replaceFlag == 1)
                        {
                            replaceFlag = 0;
                            updateFindReplace = 1;
                            renderAgain = 1;
                            flag = 1;
                        }
                        else if (findFlag == 1)
                        {
                            findFlag = 0;
                            updateFindReplace = 1;
                            renderAgain = 1;
                            flag = 1;
                        }
                        else
                        {
                            if (fileSaved == 1)
                            {
                                window.close();
                                break;
                            }

                            int closeOption = Windows::saveModal();

                            if (closeOption == IDCANCEL)
                                break;
                            else if (closeOption == IDYES)
                            {
                                if (path.size() == 0)
                                    path = Windows::saveAS();

                                FILE* fptr = fopen(path.c_str(), "w");

                                if (fptr == NULL)
                                {
                                    Windows::throwMessage("Wrong Path!");
                                    break;
                                }

                                String::saveText(fptr, S);
                                fileSaved = 1;
                                fclose(fptr);
                                break;
                            }
                            else if (closeOption == IDNO)
                            {
                                window.close();
                                break;
                            }
                        }

                        break;
                    }
                    else if (key == 73) /// up arrow
                    {
                        if (replaceFlag == 1 || findFlag == 1)
                            break;

                        int posCursor = String::findCursorPosition(S);
                        int p1 = String::findPrevEndline(posCursor, S);

                        if (p1)
                        {
                            int p2 = String::findPrevEndline(p1, S);
                            int chCurr = posCursor - p1 - 1;
                            int chPrev = p1 - p2 - 1;
                            String::del(posCursor, S);
                            String::insert(p2 + min(chCurr, chPrev) + 1, S);
                            renderAgain = 1;
                            cursorTimer = 0;
                        }

                        selectFlag = 0;

                        renderAgain = 1;
                        flag = 1;

                        renderAgain |= Render::updateViewX(S, Xoffset, scrollUnitX);
                        renderAgain |= Render::updateViewY(S, Yoffset, scrollUnitY);

                        break;
                    }
                    else if (key == 74) /// down arrow
                    {
                        if (replaceFlag == 1 || findFlag == 1)
                            break;

                        int posCursor = String::findCursorPosition(S);
                        int p1 = String::findNextEndline(posCursor, S);
                        int p0 = String::findPrevEndline(posCursor, S);

                        if (p1 <= len(S))
                        {
                            int p2 = String::findNextEndline(p1, S);
                            int chCurr = posCursor - p0 - 1;
                            int chNext = p2 - p1 - 1;
                            String::del(posCursor, S);
                            String::insert(p1 + min(chCurr, chNext) + 1 - 1, S);
                            renderAgain = 1;
                            cursorTimer = 0;
                        }

                        selectFlag = 0;

                        renderAgain = 1;
                        flag = 1;

                        renderAgain |= Render::updateViewX(S, Xoffset, scrollUnitX);
                        renderAgain |= Render::updateViewY(S, Yoffset, scrollUnitY);

                        break;
                    }
                    else if (key == 71) /// left arrow
                    {
                        if (replaceFlag == 1)
                        {
                            int pap = Replace::findPrevValidAppearance(currentAppearance, bit, positions, gone, rword, word, prv, nxt, notRemoved);

                            if (pap != -1)
                                currentAppearance = pap;
                            else
                                break;

                            renderAgain = 1;
                            flag = 1;

                            updateFindReplace = 1;
                            selectFlag = 0;
                            findFlag = 0;

                            break;
                        }
                        else if (findFlag == 1)
                        {
                            currentAppearance--;
                            currentAppearance = max(0, currentAppearance);

                            renderAgain = 1;
                            flag = 1;

                            selectFlag = 0;
                            replaceFlag = 0;
                            updateFindReplace = 1;

                            break;
                        }
                        else
                        {
                            int posCursor = String::findCursorPosition(S);

                            if (posCursor > 1)
                            {
                                String::del(posCursor, S);
                                String::insert(posCursor - 1, S);
                                cursorTimer = 0;
                            }

                            flag = 1;
                            selectFlag = 0;

                            renderAgain |= Render::updateViewX(S, Xoffset, scrollUnitX);
                            renderAgain |= Render::updateViewY(S, Yoffset, scrollUnitY);
                        }

                        break;
                    }
                    else if (key == 72) /// right arrow
                    {
                        if (replaceFlag == 1)
                        {
                            int nap = Replace::findNextValidAppearance(currentAppearance, bit, positions, gone, rword, word, prv, nxt, notRemoved);
                            if (nap != -1)
                                currentAppearance = nap;
                            else
                                break;

                            renderAgain = 1;
                            flag = 1;

                            selectFlag = 0;
                            findFlag = 0;
                            updateFindReplace = 1;

                            break;
                        }
                        else if (findFlag == 1)
                        {
                            currentAppearance++;
                            currentAppearance = min((int)positions.size() - 1, currentAppearance);

                            renderAgain = 1;
                            flag = 1;

                            selectFlag = 0;
                            replaceFlag = 0;
                            updateFindReplace = 1;

                            break;
                        }
                        else
                        {
                            int posCursor = String::findCursorPosition(S);
                            if (posCursor < String::len(S))
                            {
                                String::del(posCursor, S);
                                String::insert(posCursor + 1, S);
                                cursorTimer = 0;
                            }

                            flag = 1;
                            selectFlag = 0;

                            renderAgain |= Render::updateViewX(S, Xoffset, scrollUnitX);
                            renderAgain |= Render::updateViewY(S, Yoffset, scrollUnitY);
                        }

                        break;
                    }
                    else if (key == 58)
                    {
                        if (replaceFlag == 0 || findFlag == 1)
                            break;

                        if (currentAppearance == -1)
                        {
                            Windows::throwMessage("There are no more matchings!");
                            renderAgain = 1;
                            flag = 1;
                            replaceFlag = 0;
                            break;
                        }

                        int L = Replace::findRealPosition(currentAppearance, positions, bit, word, rword);
                        String::replace(L, L + word.size() - 1, rword, S);
                        Replace::delAp(currentAppearance, prv, nxt, bit, gone, notRemoved);
                        int nxtAppearance = Replace::findNextValidAppearance(currentAppearance, bit, positions, gone, rword, word, prv, nxt, notRemoved);
                        int prvAppearance = Replace::findPrevValidAppearance(currentAppearance, bit, positions, gone, rword, word, prv, nxt, notRemoved);

                        nxt[currentAppearance] = prv[currentAppearance] = -1;
                        currentAppearance = max(nxtAppearance, prvAppearance);

                        fileSaved = 0;
                        renderAgain = 1;
                        flag = 1;
                        updateFindReplace = 1;
                        break;
                    }
                    else if (key == 85)
                    {
                        replaceAllFlag = 1;

                        set<int> snapshot = notRemoved;

                        for (auto currentAppearance : snapshot)
                        {
                            if (Replace::canReplace(currentAppearance, bit, positions, gone, rword, word) == 0)
                                continue;

                            int L = Replace::findRealPosition(currentAppearance, positions, bit, word, rword);
                            String::replace(L, L + word.size() - 1, rword, S);
                            Replace::delAp(currentAppearance, prv, nxt, bit, gone, notRemoved);
                        }

                        fileSaved = 0;
                        renderAgain = 1;
                        flag = 1;
                        updateFindReplace = 1;
                    }
                    else
                        break;

                    break;
                }

                if (event.type == sf::Event::TextEntered) /// ce scrie user-ul
                {
                    fileSaved = 0;
                    int ch = event.text.unicode;
                    cerr << "char is " << ch << '\n';

                    if (ch == 27 || ch == 24 || ch == 3 || ch == 1 || ch == 22 || ch == 13 && replaceFlag == 1 || replaceFlag == 1 || findFlag == 1 || ch == 19 || ch == 1 || ch == 6 || ch == 18 || ch == 5 || ch == 7 || ch == 12)
                        break;

                    int posCursor = String::findCursorPosition(S);

                    if (ch == 8)
                    {
                        if (selectFlag == 1)
                        {
                            String::del(segmSelected.first, segmSelected.second, S);
                            selectFlag = 0;
                        }
                        else if (posCursor > 1)
                            String::del(posCursor - 1, S);

                        renderAgain = 1;
                    }
                    else
                    {
                        if (selectFlag == 1)
                        {
                            int L = segmSelected.first;
                            int R = segmSelected.second;

                            String::Treap *s1 = 0, *s2 = 0, *s3 = 0;
                            String::split(S, s2, s3, R);
                            String::split(s2, s1, s2, L - 1);

                            String::del(s2);
                            String::merge(S, s1, s3);
                            renderAgain = 1;
                        }

                        if (ch == 13)
                            ch = 10;
                        String::insert(String::findCursorPosition(S), S, ch);
                    }

                    fileSaved = 0;
                    flag = 1;
                    selectFlag = findFlag = 0;

                    renderAgain |= Render::updateViewX(S, Xoffset, scrollUnitX);
                    renderAgain |= Render::updateViewY(S, Yoffset, scrollUnitY);

                    break;
                }

                if (event.type == sf::Event::MouseWheelMoved) /// scroll up and down
                {
                    int direction = event.mouseWheel.delta;

                    if (direction == +1)
                        Yoffset -= scrollUnitY, Yoffset = max(0, Yoffset);
                    if (direction == -1)
                    {
                        Yoffset += scrollUnitY;
                        
                        if (Yoffset / lineHeight + 1 > String::findNumberOfEndlines(1, String::len(S), S) + 1)
                            Yoffset -= scrollUnitY;
                    }

                    renderAgain = 1;
                    flag = 1;

                    break;
                }

                if (event.type == sf::Event::Resized)
                {
                    sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                    window.setView(sf::View(visibleArea));
                    renderAgain = 1;
                    flag = 1;
                    windowWidth = event.size.width;
                    windowHeight = event.size.height;

                    bottomSeparator.setSize(sf::Vector2f(windowWidth, 1));
                    bottomSeparator.setPosition(0, windowHeight - marginBottom);
                    topSeparator.setSize(sf::Vector2f(windowWidth, 1));
                    topSeparator.setPosition(sf::Vector2f(0, marginTop));

                    lineNumbersBackground.setSize(sf::Vector2f(marginLeft, windowHeight));
                    lineNumbersBackground.setPosition(0, marginTop);
                    statusBarBackground.setSize(sf::Vector2f(windowWidth, marginBottom));
                    statusBarBackground.setPosition(sf::Vector2f(0, windowHeight - marginBottom));
                    toolBarBackground.setPosition(0, 0);
                    toolBarBackground.setSize(sf::Vector2f(windowWidth, marginTop));

                    break;
                }
            }

        window.clear(currentThemeColors.background);
        if (theme == Theme::POVESTI_DIN_FOLCLORUL_MAGHIAR)
            window.draw(povesti);

        cursorTimer++;
        cursorTimer %= timeUnit * 2;

        if (readFileFlag == 1 && currPosNewFile == fileSize)
        {
            readFileFlag = 0;

            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
            std::cout << "Time elapsed: " << duration.count() << " ms\n";

            data = NULL;
            UnmapViewOfFile(originalData);
            CloseHandle(mappingHandle);
            CloseHandle(fileHandle);
        }
        else if(readFileFlag == 1)
        {
            int lastIdx = std::min((int) fileSize, currPosNewFile + bucketSize);
            int sz = lastIdx - currPosNewFile;
            String::merge(S, S, String::build(sz, data));
            data += sz;
            currPosNewFile += sz;
            flag = 1;
            renderAgain = 1;
        }

        if (cursorTimer % (timeUnit * 2) <= timeUnit)
            cursorBox.setFillColor(currentThemeColors.text);
        else if (cursorTimer % (timeUnit * 2) != 0)
            cursorBox.setFillColor(currentThemeColors.background);

        if (flag || firstExec)
        {
            scrollUnitX = charWidth[fontIndex][fontSize][0], scrollUnitY = Helpers::getLineHeight();

            renderAgain |= firstExec;
            renderAgain |= fontSizeChanged;

            if (fontSizeChanged || firstExec)
            {
                cursorHeight = lineHeight;
                cursorWidth = 1;
            }

            fontSizeChanged = 0;
            firstExec = 0;

            int posCursor = String::findCursorPosition(S);
            int cursorLine = String::findNumberOfEndlines(1, posCursor, S) + 1;
            int p = String::findKthLine(cursorLine, S);
            int fp = String::getFirstSeen(p, posCursor, Xoffset, S);
            int lp = String::getLastSeen(p, posCursor, Xoffset + windowWidth - marginLeft, S);
            if (lp < posCursor)
                fp = -1;

            renderAgain |= lastCursorLine != cursorLine;

            if (findFlag == 1 && updateFindReplace == 1)
            {
                if (currentAppearance < positions.size() && !Replace::isApOnScreen(positions[currentAppearance], word.size()))
                {
                    int P = positions[currentAppearance];
                    int L = String::findNumberOfEndlines(1, P, S) + 1;
                    int F = String::findKthLine(L, S);
                    Yoffset = (L - 1) * lineHeight;
                    Xoffset = String::findWidth(F, P - 1, S);
                }

                renderAgain = 1;
            }

            if (replaceFlag == 1 && updateFindReplace == 1)
            {

                Render::render(l1, l2, S, Yoffset, Xoffset, cursorLine, text, scrollUnitY);

                if (currentAppearance != -1 && currentAppearance < positions.size() && !Replace::isApOnScreen(Replace::findRealPosition(currentAppearance, positions, bit, word, rword), word.size()))
                {
                    int P = Replace::findRealPosition(currentAppearance, positions, bit, word, rword);
                    int L = String::findNumberOfEndlines(1, P, S) + 1;
                    int F = String::findKthLine(L, S);
                    Yoffset = (L - 1) * lineHeight;
                    Xoffset = String::findWidth(F, P - 1, S);
                }

                renderAgain = 1;
            }

            if (renderAgain == 1)
                Render::render(l1, l2, S, Yoffset, Xoffset, cursorLine, text, scrollUnitY);
            else
            {
                if (cursorLine >= l1 && cursorLine <= l2)
                {
                    Render::updateTextLine(cursorLine - l1, renderLines, String::constructRenderedLine(cursorLine, S, Xoffset, cursorLine - l1));
                    text.setString(renderLines[cursorLine - l1]);
                }
            }

                string cursorTextLine = (cursorLine >= l1 && cursorLine <= l2 ? renderLines[cursorLine - l1] : "");
                float cw = Render::splitCursorLine(text, ptext1, cursorTextLine, posCursor - fp + 1, fp);

            if (cursorLine >= l1 && cursorLine <= l2)
                cursorLineOnScreen = 1;
            else
                cursorLineOnScreen = 0;

            if (cursorLine >= l1 && cursorLine <= l2 && fp != -1)
            {
                cursorBox.setSize(sf::Vector2f(cursorWidth, cursorHeight));
                cursorBox.setPosition((float)marginLeft + cw + paddingLeft, (cursorLine - l1) * lineHeight + marginTop);

                cursorOnScreen = 1;
            }
            else
                cursorOnScreen = 0;

            cursorLineHighlight.setSize(sf::Vector2f(windowWidth, lineHeight));
            cursorLineHighlight.setPosition(0, (cursorLine - l1) * lineHeight + marginTop);

            lastCursorLine = cursorLine;

            selectedBoxes.clear();

            if (selectFlag)
            {
                int L = segmSelected.first;
                int R = segmSelected.second;

                for (int i = 0; i < sizeRLines; i++)
                {
                    int l = segmOnScreen[i].first;
                    int r = segmOnScreen[i].second;

                    int li = max(l, L);
                    int ri = min(r, R);

                    if (li > ri)
                        continue;

                    int y = i * lineHeight + marginTop;
                    int x = marginLeft;

                    int w = String::findWidth(l, li - 1, S);
                    int W = String::findWidth(li, ri, S);

                    box.setPosition(w + marginLeft + paddingLeft + (cursorLine - l1 == i && li == posCursor + 1 ? -charWidth[fontIndex][fontSize][' '] : 0), y);
                    box.setSize(sf::Vector2f(W, lineHeight));
                    box.setFillColor(currentThemeColors.selectHighlight);
                    selectedBoxes.push_back(box);
                }
            }

            if (findFlag)
            {
                for (int i = 0; i < sizeRLines; i++)
                {
                    int l = segmOnScreen[i].first;
                    int r = segmOnScreen[i].second;

                    if (l == -1)
                        continue;

                    int p = lower_bound(positions.begin(), positions.end(), l) - positions.begin();
                    int y = i * lineHeight + marginTop;

                    while (p < positions.size() && positions[p] <= r)
                    {
                        int w = String::findWidth(l, positions[p] - 1, S);
                        int W = String::findWidth(positions[p], positions[p] + word.size() - 1, S);

                        box.setPosition(marginLeft + w + paddingLeft, y);
                        box.setSize(sf::Vector2f(W, lineHeight));
                        if (p != currentAppearance)
                            box.setFillColor(sf::Color(255, 255, 0, 128));
                        else
                            box.setFillColor(sf::Color(255, 187, 0, 128));
                        selectedBoxes.push_back(box);
                        p++;
                    }
                }
            }

            if (replaceFlag)
            {
                for (int i = 0; i < sizeRLines; i++)
                {
                    int l = segmOnScreen[i].first;
                    int r = segmOnScreen[i].second;

                    if (l == -1)
                        continue;

                    int p = Replace::traceFirstApToRender(l, positions, bit, notRemoved, word, rword);
                    int y = i * lineHeight + marginTop;

                    while (p != -1 && p < positions.size() && Replace::findRealPosition(p, positions, bit, word, rword) <= r)
                    {
                        int P = Replace::findRealPosition(p, positions, bit, word, rword);
                        int w = String::findWidth(l, P - 1, S);
                        int W = String::findWidth(P, P + word.size() - 1, S);

                        box.setPosition(marginLeft + w + paddingLeft, y);
                        box.setSize(sf::Vector2f(W, lineHeight));
                        if (p != currentAppearance)
                            box.setFillColor(sf::Color(255, 255, 0, 128));
                        else
                            box.setFillColor(sf::Color(255, 187, 0, 128));
                        selectedBoxes.push_back(box);
                        p = Replace::findNextValidAppearance(p, bit, positions, gone, rword, word, prv, nxt, notRemoved);
                    }
                }
            }
        }

        if ((findFlag | replaceFlag) && flag == 1)
        {
            int highlighted = 0;

            for (int i = 0; i < selectedBoxes.size(); i++)
                if (selectedBoxes[i].getFillColor() == sf::Color(255, 187, 0, 128))
                    highlighted = i;

            for (int i = 0; i < highlighted; i++)
                if (selectedBoxes[i].getPosition().y == selectedBoxes[i + 1].getPosition().y)
                    selectedBoxes[i].setSize(sf::Vector2f(min(selectedBoxes[i + 1].getPosition().x - selectedBoxes[i].getPosition().x, selectedBoxes[i].getSize().x), lineHeight));

            for (int i = highlighted + 1; i < selectedBoxes.size(); i++)
                if (selectedBoxes[i].getPosition().y == selectedBoxes[i - 1].getPosition().y)
                {
                    int oldX = selectedBoxes[i].getPosition().x;
                    selectedBoxes[i].setPosition(max(selectedBoxes[i].getPosition().x, selectedBoxes[i - 1].getPosition().x + selectedBoxes[i - 1].getSize().x), selectedBoxes[i].getPosition().y);
                    selectedBoxes[i].setSize(sf::Vector2f(selectedBoxes[i].getSize().x - (selectedBoxes[i].getPosition().x - oldX), lineHeight));
                }
        }

        if (selectFlag | findFlag | replaceFlag)
            for (auto box : selectedBoxes)
                window.draw(box);

        window.draw(lineNumbersBackground);
        if (showLineNumbers)
            window.draw(lineNumbersSprite);

        if (cursorOnScreen)
            window.draw(cursorBox);
        if (cursorLineOnScreen && selectFlag == 0 && findFlag == 0 && replaceFlag == 0)
            window.draw(cursorLineHighlight);

        window.draw(aboveCurrentLineSprite);
        window.draw(ptext1);
        window.draw(belowCurrentLineSprite);
        
        Helpers::updateStatusBarInfo();
        Helpers::updateStatusBarPositions();

        Helpers::updateToolBarInfo();
        Helpers::updateToolBarPositions();
    
        window.draw(statusBarBackground);
        window.draw(toolBarBackground);
        
        lineCountTextBox->draw();
        lineColumnTextBox->draw();
        if (selectedBoxes.size())
            selectedCharacterCountTextBox->draw();
        zoomOutButton->draw();
        zoomLevelTextBox->draw();
        zoomInButton->draw();
        window.draw(topSeparator);
        window.draw(bottomSeparator);

        for (int i = 0; i < 5; i++)
            menus[i]->draw();
        
        fileNameTextBox->draw();

        window.display();
        modal.display();
    }

    return 0;
}
