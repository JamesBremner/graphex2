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
    int myID;
    static int myLastID;
    cNode()
    {
        myID = ++myLastID;
    }
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
    bool operator==(const cNode &other)
    {
        return (x == other.x && y == other.y);
    }
};

int cNode::myLastID = 0;

/// @brief a link between two nodes
class cLink
{
public:
    int n1, n2; // IDs of nodes linked
    cLink(int a, int b)
        : n1(a), n2(b)
    {
    }
};

/// @brief A graph of nodes connected by links
class cGraph
{
public:
    std::vector<cNode> vN; ///< the nodes
    std::vector<cLink> vL; ///< the links
    int selected;          ///< index of currently selected node
    int prevSelected;

    cGraph()
        : selected(-1)
    {
    }

    cNode &addNode()
    {
        vN.push_back(cNode());
        return vN.back();
    }

    /// @brief Link selected and previously selected nodes
    void link();

    /// @brief node near location
    /// @param x
    /// @param y
    /// @return near node index, -1 if none
    int nearNode(int x, int y) const;

    /// @brief Select a node if there is one close to the mouse cursor
    /// @param x
    /// @param y
    void selectIfNear(int x, int y);

    void select(int id)
    {
        prevSelected = selected;
        selected = id;
    }

    void SelectedNodeMove(int x, int y)
    {
        if (selected >= 0)
            selectedNode().loc(x, y);
    }
    void SelectedRemove();

    bool isSelected() const
    {
        return selected >= 0;
    }
    cNode &findNode(int id);
    cNode &selectedNode();
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

    void remove();

    void draw(wex::shapes &S);
};

cNode &cGraph::findNode(int id)
{
    for (auto &n : vN)
    {
        if (n.myID == id)
            return n;
    }
    throw std::runtime_error("Cannot find node");
}
cNode &cGraph::selectedNode()
{
    if (selected < 0)
        return vN[0];
    return findNode(selected);
}

int cGraph::nearNode(int x, int y) const
{
    for (int kn = 0; kn < vN.size(); kn++)
    {
        if (vN[kn].isNear(x, y))
        {
            return vN[kn].myID;
        }
    }
    return -1;
}

void cGraph::link()
{
    if (selected < 0 || prevSelected < 0 ||
        selected == prevSelected)
        return;
    vL.push_back(cLink(selected, prevSelected));
}

void cGraph::selectIfNear(int x, int y)
{
    selected = nearNode(x, y);
}

void cGraph::SelectedRemove()
{
    if (selected < 0)
        return;

    // remove connected links
    vL.erase(std::remove_if(
                 vL.begin(), vL.end(),
                 [this](cLink l)
                 {
                     return (l.n1 == selected || l.n2 == selected);
                 }),
             vL.end());

    vN.erase(std::remove_if(
                 vN.begin(), vN.end(),
                 [this](cNode n)
                 {
                     return (n.myID == selected);
                 }),
             vN.end());
    selected = -1;
}

void cGUI::draw(wex::shapes &S)
{
    // draw links
    S.color(0x000000);
    for (int kl = 0; kl < G.vL.size(); kl++)
    {
        auto &l = G.vL[kl];
        auto &n1 = G.findNode(l.n1);
        auto &n2 = G.findNode(l.n2);
        S.line({n1.x, n1.y,
                n2.x, n2.y});
    }
    // draw nodes
    for (int kn = 0; kn < G.vN.size(); kn++)
    {
        cNode &n = G.vN[kn];
        // std::cout << n.myID << " " << n.x << " " << n.y << "\n";
        S.color(0x000000);
        if (n.myID == G.selected)
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
    mRightClick = new wex::menu(fm);
    mRightClick->append("Link with selected",
                        [&](const std::string &title)
                        {
                            G.link();
                            fm.update();
                        });
    mRightClick->append("Delete",
                        [&](const std::string &title)
                        {
                            G.SelectedRemove();
                            fm.update();
                        });
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
    std::cout << mouseStatus.x << " " << mouseStatus.y << "\n";
    int nodeClicked = G.nearNode(mouseStatus.x, mouseStatus.y);
    if (nodeClicked == -1)
    {
        // click not near a node
        // add new node at click location
        auto &n = G.addNode();
        n.loc(mouseStatus.x, mouseStatus.y);
        G.select(n.myID);
        fm.update();
        return;
    }

    // click on node, popup menu
    G.select(nodeClicked);
    mRightClick->popup(mouseStatus.x, mouseStatus.y);
}

main()
{
    cGUI theGUI;
    return 0;
}
