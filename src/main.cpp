#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <wex.h>
#include <inputbox.h>
#include "cStarterGUI.h"
#include "graphexgraph.h"

class cGUI : public cStarterGUI
{
public:
    cGUI();

private:
    cGraphExGraph G; ///< the graph
    wex::menubar *mbar;
    wex::menu *mFile;
    wex::menu *mView;
    wex::menu *mRightClick;  ///< pop-up menu to show when user clicks right mouse button
    wex::sMouse mouseStatus; ///< position of mouse cursor when a mouse bottun was pressed
    bool fTracking;
    bool fLabel;

    void ConstructMenu();
    void registerHandlers();

    void rightClick();
    void leftClick();

    void draw(wex::shapes &S);
};

cNode &cGraphExGraph::findNode(int id)
{
    for (auto &n : vN)
    {
        if (n.myID == id)
            return n;
    }
    throw std::runtime_error("Cannot find node");
}
cNode &cGraphExGraph::selectedNode()
{
    if (selected < 0)
        return vN[0];
    return findNode(selected);
}

int cGraphExGraph::nearNode(int x, int y) const
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

void cGraphExGraph::link()
{
    if (selected < 0 || prevSelected < 0 ||
        selected == prevSelected)
        return;
    vL.push_back(cLink(selected, prevSelected));
}

void cGraphExGraph::selectIfNear(int x, int y)
{
    selected = nearNode(x, y);
}
void cGraphExGraph::selectedLabel(const std::string &l)
{
    if (selected < 0)
        return;
    selectedNode().myLabel = l;
}
void cGraphExGraph::selectedRemove()
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

void cGraphExGraph::save(const std::string fname)
{
    if (fname.empty())
        return;
    std::ofstream ofs(fname);
    if (!ofs.is_open())
        throw std::runtime_error("Cannot open save file");
    for (auto &n : vN)
        ofs << n.save();
    for (auto &l : vL)
        ofs << l.save();
}

void cGraphExGraph::read(const std::string fname)
{
    if (fname.empty())
        return;
    std::ifstream ifs(fname);
    if (!ifs.is_open())
        throw std::runtime_error("Cannot open read file");
    clear();
    char type;
    while (ifs.good())
    {
        ifs >> type;
        if (type == 'n')
        {
            int id;
            ifs >> id;
            cNode n(id);
            ifs >> n.x >> n.y;
            ifs >> n.myLabel;
            vN.push_back(n);
        }
        else
        {
            int n1, n2;
            ifs >> n1 >> n2;
            vL.push_back(cLink(n1, n2));
        }
    }
}

cGUI::cGUI()
    : cStarterGUI(
          "GraphEx",
          {50, 50, 1000, 500}),
      fTracking(false),
      fLabel(false)
{
    ConstructMenu();
    registerHandlers();

    show();
    run();
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
        if (fLabel)
            S.text(n.myLabel, {n.x + 10, n.y});
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
    mbar = new wex::menubar(fm);

    mFile = new wex::menu(fm);
    mFile->append("New",
                  [&](const std::string &title)
                  {
                      G.clear();
                      fm.update();
                  });
    mFile->append("Save",
                  [&](const std::string &title)
                  {
                      wex::filebox fb(fm);
                      G.save(fb.save());
                  });
    mFile->append("Load",
                  [&](const std::string &title)
                  {
                      wex::filebox fb(fm);
                      G.read(fb.open());
                      fm.update();
                  });
    mbar->append("File", *mFile);

    mView = new wex::menu(fm);
    mView->append("Node Labels",
                  [&](const std::string &title)
                  {
                      fLabel = !fLabel;
                      mView->check(0, fLabel);
                      fm.update();
                  });
    mbar->append("View", *mView);

    mRightClick = new wex::menu(fm);
    mRightClick->append("Link with selected",
                        [&](const std::string &title)
                        {
                            G.link();
                            fm.update();
                        });
    mRightClick->append("Label",
                        [&](const std::string &title)
                        {
                            wex::inputbox ib;
                            ib.add("Label", "");
                            ib.showModal();
                            G.selectedLabel(ib.value("Label"));
                            fm.update();
                        });
    mRightClick->append("Delete",
                        [&](const std::string &title)
                        {
                            G.selectedRemove();
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
