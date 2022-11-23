#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <wex.h>
#include "cStarterGUI.h"

class cNode
{
public:
    int x, y;
    void loc(int X, int Y)
    {
        x = X;
        y = Y;
    }
    bool isNear(int X, int Y) const
    {
        return (abs(x - X) < 10 && abs(y - Y) < 10);
    }
};

class cLink
{
    public:
    cNode& n1, n2;
    cLink( cNode& a, cNode& b )
    : n1(a), n2(b)
    {}

};

class cGraph
{
public:
    std::vector<cNode> vN;
    std::vector<cLink> vL;
    int selected;

    cNode &addNode()
    {
        vN.push_back(cNode());
        return vN.back();
    }
        void addLink( int n1, int n2 )
    {
        vL.push_back( cLink( vN[n1], vN[n2] ));
    }
    void selectIfNear(int x, int y)
    {
        for (int kn = 0; kn < vN.size(); kn++)
        {
            if (vN[kn].isNear(x, y))
            {
                selected = kn;
                return;
            }
        }
        selected = -1;
    }

};

class cGUI : public cStarterGUI
{
public:
    cGUI()
        : cStarterGUI(
              "GraphEx",
              {50, 50, 1000, 500})

    {
        ConstructMenu();
        registerHandlers();

        show();
        run();
    }

private:
    cGraph G;
    wex::menu *mRightClick;
    wex::sMouse mouseStatus;

    void ConstructMenu();
    void registerHandlers();

    void rightClick();
    void leftClick();

    void newNode();
    void link();

    void draw(wex::shapes &S);
};

void cGUI::draw(wex::shapes &S)
{
    for( int kl = 0; kl < G.vL.size(); kl++)
    {
        auto& l = G.vL[kl];
        S.line({l.n1.x,l.n1.y,
           l.n2.x,l.n2.y });
    }
    for( int kn = 0; kn < G.vN.size(); kn++ )
    {
        cNode& n = G.vN[kn];
        S.color( 0x000000 );
        if( kn == G.selected )
            S.color(0x0000FF );
        S.circle(n.x, n.y, 10);
    }
}
void cGUI::registerHandlers()
{
    fm.events().draw(
        [&](PAINTSTRUCT &ps)
        {
            wex::shapes S(ps);
            draw(S);
        });

    fm.events().click(
        [this]
        {
            leftClick();
        });
    fm.events().clickRight(
        [this]
        {
            rightClick();
        });
}
void cGUI::ConstructMenu()
{
    int clicked;
    mRightClick = new wex::menu(fm);
    mRightClick->append("New Node",
                        [&](const std::string &title)
                        { newNode(); });
    mRightClick->append("Link with selected", [&](const std::string &title)
                        { link(); });

}

void cGUI::leftClick()
{
    auto m = fm.getMouseStatus();
    G.selectIfNear( m.x,m.y);
    fm.update();
}
void cGUI::rightClick()
{
    mouseStatus = fm.getMouseStatus();
    mRightClick->popup(mouseStatus.x, mouseStatus.y);
}

void cGUI::newNode()
{
    auto &n = G.addNode();
    n.loc(mouseStatus.x,mouseStatus.y);
    fm.update();
}

void cGUI::link()
{
    if( G.selected < 0 )
        return;
    int link = G.selected;

    G.selectIfNear( mouseStatus.x,mouseStatus.y);
    if( G.selected < 0 )
        return;
    if( G.selected == link )
        return;
    G.addLink( link, G.selected );
    fm.update();
}

main()
{
    cGUI theGUI;
    return 0;
}
