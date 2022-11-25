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
                            ib.add("Label", G.selectedLabel() );
                            ib.showModal();
                            try
                            {
                                G.selectedLabel(ib.value("Label"));
                            }
                            catch (...)
                            {
                                wex::msgbox mb("Duplicate label");
                            }
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
