﻿#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <stdio.h>
#include <chrono>
#include <random>
#include "precompute.h"

using namespace std;

mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

const int C = 400;
const int fontUnit = 10;
const int scrollUnit = 10;
const int timeUnit = 500;
const char cursorChar = ' ';

int charHeight[C][C];
int fontSize = 60;

int navBarOffset = 50;
int cntRowsOffset = 200;
int globalHeightLine = fontSize + 5;
const int HEIGHT = 1000;
const int WIDTH = 1000;

vector < string > renderLines(1000);

namespace String
{
    void precalculateCharDim()
    {
        for (int fnt = fontUnit; fnt < C; fnt += fontUnit) ///precalculeaza width-urile si height-urile caracterilor in functie de marimea fontului 
        {
            for (int i = 0; i <= 255; i++)
                charHeight[fnt][i] = fnt + 5;
        }
    }

    int getDim(char ch)
    {
        return charWidth[fontSize][ch];
    }

    struct Treap
    {
        char ch;
        Treap* L, * R;
        bool flagCursor = 0;
        bool flagEndline = 0;
        int width;
        int sumCursor;
        int sumEndline;
        int sumWidth;
        int cnt;
        int priority;

        Treap(char ch, bool flagCursor = 0)
        {
            this->ch = ch;
            L = R = 0;
            this->sumCursor = this->flagCursor = flagCursor;
            this->sumEndline = this->flagEndline = (ch == 10);
            this->width = this->sumWidth = getDim(ch);
            cnt = 1;
            priority = rng();
        }
    };

    int sumCursor(Treap* T)
    {
        if (T == 0) return 0;
        return T->sumCursor;
    }

    int sumEndline(Treap* T)
    {
        if (T == 0) return 0;
        return T->sumEndline;
    }

    int cnt(Treap* T)
    {
        if (T == 0) return 0;
        return T->cnt;
    }

    int sumWidth(Treap* T)
    {
        if (T == 0) return 0;
        return T->sumWidth;
    }

    int len(Treap* T)
    {
        if (T == 0) return 0;
        return T->cnt;
    }

    void recalculate(Treap*& T)
    {
        if (T == 0) cerr << "flag!!!", exit(0);
        T->sumCursor = sumCursor(T->L) + sumCursor(T->R) + T->flagCursor;
        T->sumEndline = sumEndline(T->L) + sumEndline(T->R) + T->flagEndline;
        T->sumWidth = sumWidth(T->L) + sumWidth(T->R) + T->width;
        T->cnt = cnt(T->L) + cnt(T->R) + 1;
    }

    void merge(Treap*& T, Treap* L, Treap* R)
    {
        if (L == 0 || R == 0)
            return T = (L ? L : R), void();

        if (L->priority > R->priority)
        {
            T = L;
            merge(T->R, T->R, R);
        }
        else
        {
            T = R;
            merge(T->L, L, T->L);
        }

        recalculate(T);
    }

    void split(Treap* T, Treap*& L, Treap*& R, int key, int add = 0)
    {
        if (T == 0)
            return L = R = 0, void();

        int currKey = add + cnt(T->L) + 1;

        if (currKey <= key)
        {
            L = T;
            split(T->R, T->R, R, key, currKey);
        }
        else
        {
            R = T;
            split(T->L, L, T->L, key, add);
        }

        recalculate(T);
    }

    void print(Treap* T)
    {
        if (T == 0) return;
        print(T->L);
        cerr << T->ch << ' ';
        print(T->R);
    }

    char get(int pos, Treap*& T)
    {
        if (pos > len(T) || pos <= 0)
            exit(10);

        Treap* t1 = 0, * t2 = 0, * t3 = 0;
        split(T, t2, t3, pos);
        split(t2, t1, t2, pos - 1);
        char ch = t2->ch;
        merge(T, t1, t2);
        merge(T, T, t3);
        return ch;
    }

    void del(int pos, Treap*& T)
    {
        Treap* t1 = 0, * t2 = 0, * t3 = 0;
        split(T, t2, t3, pos);
        split(t2, t1, t2, pos - 1);
        if (t2) delete t2;
        merge(T, t1, t3);
    }

    void insert(int pos, Treap*& T, Treap* S = new Treap(cursorChar, 1))
    {
        Treap* t1 = 0, * t2 = 0;
        split(T, t1, t2, pos - 1);
        merge(T, t1, S);
        merge(T, T, t2);
    }

    void insert(int pos, Treap*& T, char ch)
    {
        Treap* c = new Treap(ch);
        insert(pos, T, c);
    }

    int findCursorPosition(Treap* T, int add = 0)
    {
        int curr = add + cnt(T->L) + 1;

        if (T->flagCursor == 1)
            return curr;
        else if (sumCursor(T->L) == 1)
            return findCursorPosition(T->L, add);
        else return findCursorPosition(T->R, curr);
    }

    int findWidth(Treap* T, int key, int add = 0)
    {
        if (T == 0)
            return 0;

        int currKey = add + 1 + cnt(T->L);

        if (currKey <= key)
            return findWidth(T->R, key, currKey) + sumWidth(T->L) + T->width;
        else return findWidth(T->L, key, add);
    }

    void construct(Treap* T, string& s)
    {
        if (T == 0) return;
        construct(T->L, s);
        s += T->ch;
        construct(T->R, s);
    }

    string constructString(Treap* T)
    {
        string S;
        construct(T, S);
        return S;
    }

    int findNumberOfEndlines(int l, int r, Treap*& T)
    {
        if (l > r) return 0;
        Treap* t1 = 0, * t2 = 0, * t3 = 0;
        split(T, t2, t3, r);
        split(t2, t1, t2, l - 1);
        int ans = sumEndline(t2);
        merge(T, t1, t2);
        merge(T, T, t3);
        return ans;
    }

    int findPrevEndline(Treap*& T, int add = 0)
    {
        if (T == 0) return 0;

        int currPos = add + 1 + cnt(T->L);

        if (sumEndline(T->R))
            return findPrevEndline(T->R, currPos);
        else if (T->ch == 10)
            return currPos;
        else return findPrevEndline(T->L, add);
    }

    int findPrevEndline(int pos, Treap*& T)
    {
        Treap* t1 = 0, * t2 = 0;
        split(T, t1, t2, pos - 1);
        int ans = findPrevEndline(t1);
        merge(T, t1, t2);
        return ans;
    }

    int findNextEndline(Treap*& T, int add = 0)
    {
        if (T == 0) return -1;

        int currPos = add + 1 + cnt(T->L);

        if (sumEndline(T->L))
            return findNextEndline(T->L, add);
        else if (T->ch == 10)
            return currPos;
        else return findNextEndline(T->R, currPos);
    }

    int findNextEndline(int pos, Treap*& T)
    {
        Treap* t1 = 0, * t2 = 0;
        split(T, t1, t2, pos);
        int ans = findNextEndline(t2);
        merge(T, t1, t2);
        if (ans == -1) ans = len(T) + 1;
        else ans += pos;
        return ans;
    }

    int findCurrentWidth(int pos, Treap*& T)
    {
        int p = findPrevEndline(pos, T);
        Treap* t1, * t2, * t3;
        split(T, t2, t3, pos);
        split(t2, t1, t2, p);
        int ans = sumWidth(t2);
        merge(T, t1, t2);
        merge(T, T, t3);
        return ans;
    }

    int findCurrentHeight(Treap*& T)
    {
        int lines = findNumberOfEndlines(1, findCursorPosition(T), T) + 1;
        int globalHeight = lines * globalHeightLine;
        return globalHeight;
    }

    int findKthLine(Treap*& T, int k, int lin = 0, int add = 0)
    {
        int currPos = 1 + add + cnt(T->L);
        int currLine = lin + sumEndline(T->L) + (T->ch == 10);

        if (currLine == k && T->ch == 10)
            return currPos;
        else if (currLine >= k)
            return findKthLine(T->L, k, lin, add);
        else return findKthLine(T->R, k, currLine, currPos);
    }

    int findKthLine(int k, Treap*& T)
    {
        if (k == 1) return 1;
        int ans = findKthLine(T, k - 1);
        return ans + 1;
    }

    int getFirstSeen(Treap*& T, int X, int width = 0, int add = 0)
    {
        if (T == 0) return INT_MAX;

        int currPos = add + 1 + cnt(T->L);
        int currWidth = width + T->width + sumWidth(T->L);

        if (currWidth >= X)
            return min(currPos, getFirstSeen(T->L, X, width, add));
        else return getFirstSeen(T->R, X, currWidth, currPos);
    }

    int getFirstSeen(int l, int r, int X, Treap*& T)
    {
        if (l > r) return -1;
        Treap* t1 = 0, * t2 = 0, * t3 = 0;
        split(T, t2, t3, r);
        split(t2, t1, t2, l - 1);

        int ans = -1;
        if (sumWidth(t2) >= X) ans = getFirstSeen(t2, X) + l - 1;

        merge(T, t1, t2);
        merge(T, T, t3);

        return ans;
    }

    int getLastSeen(Treap*& T, int  X, int width = 0, int add = 0)
    {
        if (T == 0) return -1;

        int currPos = add + 1 + cnt(T->L);
        int currWidth = width + T->width + sumWidth(T->L);

        if (currWidth <= X)
            return max(currPos, getLastSeen(T->R, X, currWidth, currPos));
        else return getLastSeen(T->L, X, width, add);
    }

    int getLastSeen(int l, int r, int X, Treap*& T)
    {
        Treap* t1 = 0, * t2 = 0, * t3 = 0;
        split(T, t2, t3, r);
        split(t2, t1, t2, l - 1);

        int ans = getLastSeen(t2, X);
        if (ans != -1) ans += l - 1;

        merge(T, t1, t2);
        merge(T, T, t3);

        return ans;
    }

    void traverseString(Treap*& T, string& txt)
    {
        if (T == 0) return;
        traverseString(T->L, txt);
        txt += T->ch;
        traverseString(T->R, txt);
    }

    void traverseString(int l, int r, Treap*& T, string& txt)
    {
        Treap* t1 = 0, * t2 = 0, * t3 = 0;
        split(T, t2, t3, r);
        split(t2, t1, t2, l - 1);

        traverseString(t2, txt);

        merge(T, t1, t2);
        merge(T, T, t3);
    }

    void updateWidth(Treap*& T)
    {
        if (T == 0) return;
        T->width = getDim(T->ch);
        updateWidth(T->L);
        updateWidth(T->R);
        recalculate(T);
    }

    string constructRenderedLine(int i, Treap*& T, int Xoffset)
    {
        string txt = "";
        int p1 = String::findKthLine(i, T);
        if (String::len(T) + 1 == p1 || String::get(p1, T) == 10) return txt;
        int p2 = String::findNextEndline(p1, T) - 1;

        int t1 = String::getFirstSeen(p1, p2, Xoffset , T);
        int t2 = String::getLastSeen(p1, p2, Xoffset + WIDTH - cntRowsOffset , T);

        if (t1 == -1 || t2 == -1) return txt;
        String::traverseString(t1, t2, T, txt);
        return txt;
    }

    int findWidth(int l, int r, Treap*& T)
    {
        if (l > r) return 0;
        Treap* t1 = 0, * t2 = 0, * t3 = 0;
        split(T, t2, t3, r);
        split(t2, t1, t2, l - 1);

        int ans = sumWidth(t2);

        merge(T, t1, t2);
        merge(T, T, t3);

        return ans;
    }
}

namespace Windows
{
    string getPathFromUser(string name)///ia path-ul introdus de catre utilizator pentru a salva fisierul
    {
        const int DIM = 200;
        sf::RenderWindow window(sf::VideoMode(DIM * 4, DIM / 4 - 10), name);
        sf::Image mainIcon;
        mainIcon.loadFromFile("assets/images/main_icon.png");
        window.setIcon(mainIcon.getSize().x, mainIcon.getSize().y, mainIcon.getPixelsPtr());

        sf::Event event;
        sf::Text text, pth;
        sf::Font font;

        font.loadFromFile("assets/fonts/cour.ttf");

        text.setFont(font);
        text.setFillColor(sf::Color::Black);
        text.setString("EnterPath: ");

        pth.setFont(font);
        pth.setFillColor(sf::Color::Green);
        pth.setString("");
        pth.setPosition(200, 0);

        string path;

        while (window.isOpen())
        {
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::KeyPressed)
                {
                    //cerr << event.key.code << ' ';
                    int key = event.key.code;

                    if (key == 58 || key == 36)
                    {
                        window.close();
                    }

                    break;
                }
                if (event.type == sf::Event::TextEntered)
                {
                    int ch = event.text.unicode;

                    if (ch == 8)
                    {
                        if (path.size())
                            path.pop_back();
                    }
                    else path += ch;

                    break;
                }
            }

            pth.setString(path);
            window.clear(sf::Color::White);
            window.draw(text);
            window.draw(pth);
            window.display();
        }

        return path;
    }

    string getStringFromUser(string name)///citeste cuvantul pe care vrea user-ul sa-l caute
    {
        const int DIM = 200;
        sf::RenderWindow window(sf::VideoMode(DIM * 4, DIM / 4 - 10), name);
        sf::Image mainIcon;
        mainIcon.loadFromFile("assets/images/main_icon.png");
        window.setIcon(mainIcon.getSize().x, mainIcon.getSize().y, mainIcon.getPixelsPtr());

        sf::Event event;
        sf::Text text, pth;
        sf::Font font;

        font.loadFromFile("assets/fonts/cour.ttf");

        text.setFont(font);
        text.setFillColor(sf::Color::Black);
        text.setString("Enter keyword: ");

        pth.setFont(font);
        pth.setFillColor(sf::Color::Green);
        pth.setString("");
        pth.setPosition(250, 0);

        string path;

        while (window.isOpen())
        {
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::KeyPressed)
                {
                    //cerr << event.key.code << ' ';
                    int key = event.key.code;

                    if (key == 58 || key == 36)
                    {
                        window.close();
                    }

                    break;
                }
                if (event.type == sf::Event::TextEntered)
                {
                    int ch = event.text.unicode;

                    if (ch == 8)
                    {
                        if (path.size())
                            path.pop_back();
                    }
                    else path += ch;

                    break;
                }
            }

            pth.setString(path);
            window.clear(sf::Color::White);
            window.draw(text);
            window.draw(pth);
            window.display();
        }

        return path;
    }

    void throwMessage(string message)
    {
        const int DIM = 200;
        sf::RenderWindow window(sf::VideoMode(DIM * 4, DIM / 4 - 10), "");

        sf::Event event;
        sf::Text text;
        sf::Font font;

        font.loadFromFile("assets/fonts/cour.ttf");

        text.setFont(font);
        text.setFillColor(sf::Color::Black);
        text.setString(message);

        while (window.isOpen())
        {
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::KeyPressed)
                {
                    //cerr << event.key.code << ' ';
                    int key = event.key.code;

                    if (key == 58 || key == 36)
                    {
                        window.close();
                    }

                    break;
                }
            }

            window.clear(sf::Color::White);
            window.draw(text);
            window.display();
        }
    }
}

bool updateViewX(String::Treap*& S, int& Xoffset, int scrollUnitX)
{
    int currLineWidth = String::findCurrentWidth(String::findCursorPosition(S), S);
    bool modif = 0;

    while (currLineWidth <= Xoffset )
        Xoffset -= scrollUnitX, modif = 1;

    if (modif) Xoffset -= scrollUnitX;
    Xoffset = max(0, Xoffset);

    while (currLineWidth >= Xoffset + WIDTH - cntRowsOffset)
        Xoffset += scrollUnitX, modif = 1;

    return modif;
}

bool updateViewY(String::Treap*& S, int& Yoffset, int scrollUnitY)
{
    int globalHeight = String::findCurrentHeight(S);
    int height = fontSize + 5;
    bool modif = 0;

    while (globalHeight - height < Yoffset)
        Yoffset -= scrollUnitY, modif = 1;

    //if (modif) Yoffset -= scrollUnitY;
    Yoffset = max(0, Yoffset);

    // cerr << "globalHeight is: " << globalHeight << '\n' << "height is " << height << '\n';

    while (globalHeight > Yoffset + HEIGHT - navBarOffset - height)
        Yoffset += scrollUnitY, modif = 1;

    return modif;
}

int sizeRLines = 0;

void updateTextLine(int line, vector < string >& renderLines, string L)
{
    if (line == sizeRLines) renderLines[sizeRLines++] = L;
    else renderLines[line] = L;
    // textLines[line].setString(L);
}

int findLineOnScreen(float y)
{
    return (int)((y - navBarOffset) / globalHeightLine) + 1;
}

int moveCursorToClick(sf::Vector2i localPosition, String::Treap*& S, int scrollUnitY, int l1, int l2, int Xoffset)
{
    int l = findLineOnScreen(localPosition.y);
    int w = localPosition.x - cntRowsOffset;

    //cerr << averageLineHeight << ' ' << l << ' ' << l1 << ' ' << l2 << ' ' << localPosition.y << '\n';

    if (l + l1 - 1 > l2) return String::len(S) + 1;
    int p1 = String::findKthLine(l + l1 - 1, S);
    if (w <= 0) return p1;

    if (String::len(S) + 1 == p1) return String::len(S) + 1;
    if (String::get(p1, S) == 10) return p1;

    int p2 = String::findNextEndline(p1, S) - 1;
    int p = String::getFirstSeen(p1, p2, w + Xoffset , S);
    // cerr << p << ' ' << p2 << '\n';
    if (p == -1) p = p2;
    return p + 1;
}

string txt1, txt2, txt, all;

void updateSmartRender(sf::Text& text, sf::RenderTexture& text1, sf::RenderTexture& text2, sf::Sprite& img1, sf::Sprite& img2, int l1, int l2, int cursorLine, int scrollUnitY)
{
    txt1.clear(), txt2.clear(), txt.clear();
    all.clear();
    int h1 = 0;
    int L = min(l2 - l1 + 1, cursorLine - l1);

    text.setCharacterSize(fontSize);

    text1.clear(sf::Color(0, 0, 0, 0));
    text2.clear(sf::Color(0, 0, 0, 0));

    int lastHeight = -globalHeightLine;

    for (int i = 0; i < L; i++)
    {
        text.setString(renderLines[i]);
        text.setPosition(cntRowsOffset , navBarOffset + lastHeight + globalHeightLine);
        text1.draw(text);
        lastHeight += globalHeightLine;
    }

    int textHeight = lastHeight;

    if (l1 <= cursorLine && cursorLine <= l2) txt = renderLines[cursorLine - l1];
    else txt = " ";

    lastHeight += globalHeightLine;

    for (int i = max(0, cursorLine - l1 + 1); i < sizeRLines; i++)
    {
        text.setString(renderLines[i]);
        text.setPosition(cntRowsOffset , navBarOffset + lastHeight + globalHeightLine);
        text2.draw(text);
        lastHeight += globalHeightLine;
    }

    img1.setTexture(text1.getTexture());
    img2.setTexture(text2.getTexture());

    text.setString(txt);
    text.setPosition(cntRowsOffset , navBarOffset + textHeight + globalHeightLine);

    text1.display();
    text2.display();
}

class Button
{
public:
    sf::RectangleShape container;
    sf::Text content;
    sf::Texture texture;
    Button(string& content, sf::Font& font, sf::Vector2f& size, sf::Vector2f& position)
    {
        this->container.setSize(size);
        this->container.setPosition(position);
        this->container.setFillColor(sf::Color(0, 0, 0, 0));

        this->content.setString(content);
        this->content.setFont(font);
        this->content.setCharacterSize(13);
        this->content.setStyle(sf::Text::Bold);
        this->content.setFillColor(sf::Color::Black);

        sf::Vector2f center(
            ((2 * this->container.getGlobalBounds().left + this->container.getGlobalBounds().width) / 2.0),
            ((2 * this->container.getGlobalBounds().top + this->container.getGlobalBounds().height) / 2.0)
        );

        this->content.setPosition
        (
            center.x - this->content.getLocalBounds().width / 2.0,
            center.y - this->content.getLocalBounds().height / 2.0
        );
    }
    void setTexture(const sf::Texture* texture)
    {
        this->container.setTexture(texture);
    }
    sf::FloatRect getGlobalBounds()
    {
        return container.getGlobalBounds();
    }
    bool isInside(sf::RenderWindow& window)
    {
        sf::Vector2i localPosition = sf::Mouse::getPosition(window);
        return this->getGlobalBounds().contains(window.mapPixelToCoords(localPosition));
    }
    void setOpacity(bool hover)
    {
        this->container.setFillColor(sf::Color(0, 0, 0, hover ? 32 : 0));
    }
};

int main()
{
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Text Editor");
    sf::View view;
    sf::Image mainIcon;

    mainIcon.loadFromFile("assets/images/main_icon.png");
    window.setIcon(mainIcon.getSize().x, mainIcon.getSize().y, mainIcon.getPixelsPtr());

    sf::Event event;
    sf::Text text;
    sf::Font font;

    font.loadFromFile("assets/fonts/cour.ttf");
    text.setFont(font);
    text.setFillColor(sf::Color::Black);
    text.setCharacterSize(fontSize);
    text.setStyle(sf::Text::Regular);

    string contents[] = {"+", "-", "Open", "Save", "Save as", "Find"};

    sf::Vector2f positions[6];

    for (int i = 0; i < 6; i++)
    {
        positions[i] = sf::Vector2f(i * 90, 0);
    }

    sf::Vector2f size(90.0, 30.0);

    Button* buttons[6];

    for (int i = 0; i < 6; i++)
    {
        buttons[i] = new Button(contents[i], font, size, positions[i]);
    }

    // sf::Texture zoomIn, zoomOut;
    // zoomIn.loadFromFile("assets/images/plus.png");
    // zoomOut.loadFromFile("assets/images/minus.png");

    // buttons[0]->setTexture(&zoomIn);
    // buttons[1]->setTexture(&zoomOut);

    String::precalculateCharDim();

    String::Treap* S = new String::Treap(cursorChar, 1); ///string doar cu pointer-ul de text

    int Yoffset = 0, Xoffset = 0;
    int scrollUnitX = charWidth[fontSize][0], scrollUnitY = charHeight[fontSize]['a'];

    txt.reserve(100000);
    txt1.reserve(100000);
    txt2.reserve(100000);
    all.reserve(100000);

    bool firstExec = 1;
    int l1 = 0, l2 = 0;
    int lastCursorLine = 0;

    sf::RenderTexture text1, text2;
    sf::Sprite img1, img2;

    text1.create(WIDTH, HEIGHT);
    text2.create(WIDTH, HEIGHT);

    vector < sf::Color > colorCursor(2);
    colorCursor[1] = sf::Color(0, 0, 0, 255);
    colorCursor[0] = sf::Color(0, 0, 0, 0);

    sf::RectangleShape cursorBox;

    string path = "";
    int Timer = 0;
    bool cursorOnScreen = 0;

    while (window.isOpen())
    {
        bool flag = 0;
        bool fontChanged = 0;
        bool renderAgain = 0;

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::MouseMoved) 
            {
                for (int i = 0; i < 6; i++)
                {
                    if (buttons[i]->isInside(window))
                    {
                        buttons[i]->setOpacity(true);
                    }
                    else
                    {
                        buttons[i]->setOpacity(false);
                    }
                }
                    break;
            }
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed)
            {
                int key = event.key.code;
                sf::Vector2i localPosition = sf::Mouse::getPosition(window);

                if (key == 0 && buttons[2]->isInside(window))
                {
                    path = Windows::getPathFromUser("Open File");
                    FILE* fptr = fopen(path.c_str(), "r");

                    if (fptr == NULL)
                    {
                        Windows::throwMessage("Wrong Path!");
                        break;
                    }

                    char ch;
                    int nr = 0;

                    for (int i = String::len(S); i >= 1; i--)
                        String::del(i, S);

                    S = new String::Treap(cursorChar, 1);

                    while ((ch = fgetc(fptr)) != EOF)
                    {
                        int posCursor = String::findCursorPosition(S);
                        String::insert(posCursor, S, ch);
                        nr++;
                        //cerr << ch << '\n';
                    }

                    int posCursor = String::findCursorPosition(S);
                    String::del(posCursor, S);
                    String::insert(1, S);
                    renderAgain = 1;
                    cerr << "done" << ' ' << nr << '\n';
                }
                else if (key == 0 && buttons[4]->isInside(window))
                {
                    path = Windows::getPathFromUser("Save As");
                    FILE* fptr = fopen(path.c_str(), "w");

                    if (fptr == NULL)
                    {
                        Windows::throwMessage("Wrong Path!");
                        break;
                    }

                    int posCursor = String::findCursorPosition(S);

                    for (int i = 1; i <= String::len(S); i++)
                    {
                        char ch = String::get(i, S);
                        if (posCursor != i) fprintf(fptr, "%c", ch);
                    }

                    fclose(fptr);

                    break;
                }
                else if (key == 0 && buttons[5]->isInside(window))
                {
                    string word = Windows::getStringFromUser("Find");

                    if (word.size() == 0)
                    {
                        break;
                    }

                    string s = String::constructString(S);
                    int pos = -1;
                    int matchings = 0;

                    while ((pos = s.find(word, pos + 1)) < s.size())
                    {
                        matchings++;

                        for (int i = pos; i <= pos + word.size() - 1; i++)
                        {

                        }

                        pos += word.size() - 1;
                    }

                    Windows::throwMessage("There are " + to_string(matchings) + " matchings!");
                    break;
                }
                else if (key == 0 && buttons[0]->isInside(window))
                {
                    fontSize += fontUnit;
                    fontSize = min(fontSize, C - fontUnit);
                    String::updateWidth(S);
                    globalHeightLine = fontSize + 5;
                    fontChanged = 1;
                }
                else if (key == 0 && buttons[1]->isInside(window))
                {
                    fontChanged = 1;
                    fontSize -= fontUnit;
                    fontSize = max(fontUnit, fontSize);
                    String::updateWidth(S);
                    globalHeightLine = fontSize + 5;
                }
                else  if (key == 0 && buttons[3]->isInside(window))
                {
                    if (path.size() == 0)
                    {
                        path = Windows::getPathFromUser("Save");
                    }

                    FILE* fptr = fopen(path.c_str(), "w");

                    if (fptr == NULL)
                    {
                        Windows::throwMessage("Wrong Path!");
                        break;
                    }

                    int posCursor = String::findCursorPosition(S);

                    for (int i = 1; i <= String::len(S); i++)
                    {
                        char ch = String::get(i, S);
                        if (posCursor != i) fprintf(fptr, "%c", ch);
                    }

                    fclose(fptr);
                    break;
                }
                else if (key == 0) ///click random pe ecran ca sa schimbi unde e cursorul
                {
                    int newPosCursor = moveCursorToClick(localPosition, S, scrollUnitY, l1, l2, Xoffset);
                    cerr << "newpos: " << newPosCursor << '\n';
                    int posCursor = String::findCursorPosition(S);
                    String::del(posCursor, S);

                    if (newPosCursor <= posCursor) String::insert(newPosCursor, S);
                    else String::insert(newPosCursor - 1, S);

                    flag = 1;
                    renderAgain = 1;

                    break;
                }
                else break;

                flag = 1;

                renderAgain |= updateViewX(S, Xoffset, scrollUnitX);
                renderAgain |= updateViewY(S, Yoffset, scrollUnitY);

                break;
            }

            if (event.type == sf::Event::KeyPressed)
            {
                cerr << event.key.code << '\n';
                int key = event.key.code;

                if (key == 36) ///escape
                {
                    window.close();

                    break;
                }
                else if (key == 73) ///up arrow
                {
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
                    }
                }
                else if (key == 74) ///down arrow
                {
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
                    }
                }
                else if (key == 71) ///left arrow
                {
                    int posCursor = String::findCursorPosition(S);

                    if (posCursor > 1)
                    {
                        String::del(posCursor, S);
                        String::insert(posCursor - 1, S);
                    }
                }
                else if (key == 72)  ///right arrow
                {
                    int posCursor = String::findCursorPosition(S);

                    if (posCursor < String::len(S))
                    {
                        String::del(posCursor, S);
                        String::insert(posCursor + 1, S);
                    }
                }
                else break;

                flag = 1;

                renderAgain |= updateViewX(S, Xoffset, scrollUnitX);
                renderAgain |= updateViewY(S, Yoffset, scrollUnitY);

                break;
            }

            if (event.type == sf::Event::TextEntered) ///ce scrie user-ul
            {
                int ch = event.text.unicode;

                if (ch == 27)
                    break;

                int posCursor = String::findCursorPosition(S);

                if (ch == 8)
                {
                    if (posCursor > 1)
                        String::del(posCursor - 1, S);
                }
                else
                {
                    if (ch == 13) ch = 10;
                    String::insert(posCursor, S, ch);
                }

                flag = 1;

                renderAgain |= updateViewX(S, Xoffset, scrollUnitX);
                renderAgain |= updateViewY(S, Yoffset, scrollUnitY);

                break;
            }

            if (event.type == sf::Event::MouseWheelMoved) ///scroll up and down
            {
                int direction = event.mouseWheel.delta;

                if (direction == +1) Yoffset -= scrollUnitY, Yoffset = max(0, Yoffset);
                if (direction == -1) Yoffset += scrollUnitY;

                renderAgain = 1;
                flag = 1;

                break;
            }
        }

        window.clear(sf::Color::White);

        if (flag || firstExec)
        {
            scrollUnitX = charWidth[fontSize][0], scrollUnitY = charHeight[fontSize]['a'];

            renderAgain |= firstExec;
            renderAgain |= fontChanged;

            fontChanged = 0;
            firstExec = 0;

            int cursorLine = String::findNumberOfEndlines(1, String::findCursorPosition(S), S) + 1;
            renderAgain |= lastCursorLine != cursorLine;

            if (renderAgain == 1)
            {
                // cerr << renderAgain << '\n';

                int numberOfLines = String::findNumberOfEndlines(1, String::len(S), S) + 1;

                l1 = min(numberOfLines , max(1 , (Yoffset - 1 - navBarOffset) / scrollUnitY + 1));
                l2 = min(numberOfLines , max(1 , (Yoffset - 1 + HEIGHT - navBarOffset) / scrollUnitY));

                cerr << l1 << ' ' << l2 << '\n';

                sizeRLines = 0;

                for (int i = l1; l1 > 0 && l2 > 0 && l1 <= l2 && i <= l2; i++)
                    updateTextLine(sizeRLines, renderLines, String::constructRenderedLine(i, S, Xoffset));

                updateSmartRender(text, text1, text2, img1, img2, l1, l2, cursorLine, scrollUnitY);
            }
            else
            {
                if (cursorLine >= l1 && cursorLine <= l2)
                {
                    cerr << "here flag" << '\n';
                    updateTextLine(cursorLine - l1, renderLines, String::constructRenderedLine(cursorLine, S, Xoffset));
                    text.setString(renderLines[cursorLine - l1]);
                }
            }

            if (cursorLine >= l1 && cursorLine <= l2)
            {
                int posCursor = String::findCursorPosition(S);
                int p = String::findPrevEndline(posCursor, S) + 1;
                int p1 = String::getFirstSeen(p, posCursor, Xoffset , S);
                int width = String::findWidth(p1, posCursor - 1, S);

                int cursorHeight = globalHeightLine - 2;
                int cursorWidth = charWidth[fontSize][' '] / 6;

                cursorBox.setSize(sf::Vector2f(cursorWidth, cursorHeight));
                cursorBox.setPosition(width + cntRowsOffset , text.getPosition().y + globalHeightLine * 1.1 - cursorHeight);

                cursorOnScreen = 1;
            }
            else cursorOnScreen = 0;

            lastCursorLine = cursorLine;
        }

        Timer++;

        if (Timer % timeUnit == 0)
        {
            swap(colorCursor[0], colorCursor[1]);
            cursorBox.setFillColor(colorCursor[0]);
        }

        window.draw(img1);
        window.draw(text);
        window.draw(img2);

        for (auto button : buttons)
        {
            window.draw(button->container);
            window.draw(button->content);
        }
        if (cursorOnScreen) window.draw(cursorBox);
        window.display();
    }


    return 0;
}