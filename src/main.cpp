#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <wex.h>
#include "cStarterGUI.h"

/// A node with a location on the screen
class cNode
{
public:
    int x, y;
    void loc(int X, int Y)
    {
        x = X;
        y = Y;
    }
    /// @brief Is node near mouse curor
    /// @param X
    /// @param Y
    /// @return true if cursor is close to node
    bool isNear(int X, int Y) const
    {
        return (abs(x - X) < 10 && abs(y - Y) < 10);
    }
};
/// @brief a link between two nodes
class cLink
{
public:
    cNode &n1, &n2;
    cLink(cNode &a, cNode &b)
        : n1(a), n2(b)
    {
    }
};

/// @brief A graph pf nodes connected by links
class cGraph
{
public:
    std::vector<cNode> vN; ///< the nodes
    std::vector<cLink> vL; ///< the links
    int selected;          ///< index of currently selected node

    cNode &addNode()
    {
        vN.push_back(cNode());
        return vN.back();
    }
    void addLink(int n1, int n2)
    {
        vL.push_back(cLink(vN[n1], vN[n2]));
    }
    /// @brief Select a node if there is one close to the mouse cursor
    /// @param x
    /// @param y
    void selectIfNear(int x, int y);

    void SelectedNodeMove(int x, int y)
    {
        if (selected >= 0)
            vN[selected].loc(x, y);
    }
};

class cGUI : public cStarterGUI
{
public:
    cGUI()
        : cStarterGUI(
              "GraphEx",
              {50, 50, 1000, 500}),
          fTracking(false)

    {
        ConstructMenu();
        registerHandlers();

        show();
        run();
    }

private:
    cGraph G;                ///< the graph
    wex::menu *mRightClick;  ///< pop-up menu to show when user clicks right mouse button
    wex::sMouse mouseStatus; ///< position of mouse cursor when a mouse bottun was pressed
    bool fTracking;

    void ConstructMenu();
    void registerHandlers();

    void rightClick();
    void leftClick();

    void newNode();
    void link();

    void draw(wex::shapes &S);
};

void cGraph::selectIfNear(int x, int y)
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

void cGUI::draw(wex::shapes &S)
{
    // draw links
    S.color(0x000000);
    for (int kl = 0; kl < G.vL.size(); kl++)
    {
        auto &l = G.vL[kl];
        S.line({l.n1.x, l.n1.y,
                l.n2.x, l.n2.y});
    }
    // draw nodes
    for (int kn = 0; kn < G.vN.size(); kn++)
    {
        cNode &n = G.vN[kn];
        S.color(0x000000);
        if (kn == G.selected)
            S.color(0x0000FF);
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
    fm.events().mouseUp(
        [this]
        {
            fTracking = false;
        });
    fm.events().mouseMove(
        [this](wex::sMouse &m)
        {
            if (!fTracking)
                return;
            G.SelectedNodeMove(m.x, m.y);
            fm.update();
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
    fTracking = true;
    auto m = fm.getMouseStatus();
    G.selectIfNear(m.x, m.y);
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
    n.loc(mouseStatus.x, mouseStatus.y);
    fm.update();
}

void cGUI::link()
{
    if (G.selected < 0)
        return;
    int link = G.selected;

    G.selectIfNear(mouseStatus.x, mouseStatus.y);
    if (G.selected < 0)
        return;
    if (G.selected == link)
        return;
    G.addLink(link, G.selected);
    fm.update();
}

main()
{
    cGUI theGUI;
    return 0;
}
