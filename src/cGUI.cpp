#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <wex.h>
#include <inputbox.h>
#include "graphexgraph.h"
#include "cGUI.h"

cGUI::cGUI()
    : cStarterGUI(
          "GraphEx",
          {50, 50, 1000, 500}),
      fTracking(false),
      fLabel(false),
      fDirected(false)
{
    ConstructMenu();
    registerHandlers();

    show();
    run();
}

static void drawDirectedArrow(
    wex::shapes &S,
    int x1, int y1,
    int x2, int y2)
{
    // construct arrow at origin pointing along positive x-axis
    const int arrowsize = 10;
    std::vector<std::vector<float>> varrow = {
        {-arrowsize, arrowsize}, {0, 0}, {-arrowsize, -arrowsize}};

    // rotate arrow to align with connection
    // https://matthew-brett.github.io/teaching/rotation_2d.html

    int vx = x2 - x1;
    int vy = y1 - y2;            // y pixel is 0 at top, increasing going down
    float angle = asin(vy / sqrt(vx * vx + vy * vy));
    if (vx < 0)
            angle = 3.14 - angle;
    if( vx > 0 && vy < 0)
            angle = 6.28 + angle;
    angle *= -1;
    float cosv = cos(angle);
    float sinv = sin(angle);
    auto rot = varrow;
    rot[0][0] = cosv * varrow[0][0] - sinv * varrow[0][1];
    rot[0][1] = sinv * varrow[0][0] + cosv * varrow[0][1];
    rot[2][0] = cosv * varrow[2][0] - sinv * varrow[2][1];
    rot[2][1] = sinv * varrow[2][0] + cosv * varrow[2][1];

    // move arrow to midpoint
    for (auto &p : rot)
    {
        p[0] += (x1 + x2) / 2;;
        p[1] += (y1 + y2) / 2;;
    }

    // draw arrow
    S.line({(int)rot[0][0], (int)rot[0][1],
            (int)rot[1][0], (int)rot[1][1]});
    S.line({(int)rot[1][0], (int)rot[1][1],
            (int)rot[2][0], (int)rot[2][1]});
}

void cGUI::draw(wex::shapes &S)
{
    // draw links
    S.color(0x000000);
    for (int kl = 0; kl < G.vL.size(); kl++)
    {
        auto &l = G.vL[kl];
        if (!fDirected)
            if (l.n1 > l.n2)
                continue;
        auto &n1 = G.findNode(l.n1);
        auto &n2 = G.findNode(l.n2);
        S.line({n1.x, n1.y,
                n2.x, n2.y});
        if (fDirected)
            drawDirectedArrow(
                S,
                n1.x, n1.y,
                n2.x, n2.y);
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
            S.text(n.label(), {n.x + 10, n.y});
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
                      G.save(fb.save(),fDirected);
                  });
    mFile->append("Load",
                  [&](const std::string &title)
                  {
                      wex::filebox fb(fm);
                      G.read(fb.open(),fDirected);
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

    mGraph = new wex::menu(fm);
    mGraph->append("Directed",
                   [&](const std::string &title)
                   {
                       if (G.nodeCount())
                       {
                           wex::msgbox(
                               "Cannot toggle directed on populated graph");
                           return;
                       }
                       fDirected = !fDirected;
                       mGraph->check(0, fDirected);
                       fm.update();
                   });
    mbar->append("Graph", *mGraph);

    mRightClick = new wex::menu(fm);
    mRightClick->append("Link to selected",
                        [&](const std::string &title)
                        {
                            if (!fDirected)
                                G.linkUndirected();
                            else
                                G.link();
                            fm.update();
                        });
    mRightClick->append("Label",
                        [&](const std::string &title)
                        {
                            wex::inputbox ib;
                            ib.add("Label", G.selectedLabel());
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
    mRightClick->append("Link Costs",
                        [&](const std::string &title)
                        {
                            linkCosts();
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

void cGUI::linkCosts()
{
    auto ve = G.findLinksSelectedNode();
    if (!ve.size())
        return;
    wex::inputbox ib;
    ib.text("Link costs from " + G.selectedNode().label());
    for (cLink *e : ve)
    {
        int ni = e->n1;
        if (ni == G.selected)
            ni = e->n2;
        auto &n = G.findNode(ni);
        ib.add(
            n.label(),
            std::to_string(e->cost()));
    }

    ib.showModal();

    for (cLink *e : ve)
    {
        int ni = e->n1;
        if (ni == G.selected)
            ni = e->n2;
        auto &n = G.findNode(ni);
        e->cost(atof(ib.value(n.label()).c_str()));
    }
}