#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include "graphexgraph.h"

int cNode::myLastID = 0;

cNode &cGraphExGraph::findNode(int id)
{
    for (auto &n : vN)
    {
        if (n.myID == id)
            return n;
    }
    throw std::runtime_error("Cannot find node");
}

cNode &cGraphExGraph::findNode(const std::string &label) 
{
    for (auto &n : vN)
    {
        if (n.myLabel == label)
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
std::string cGraphExGraph::selectedLabel()
{
    if (selected < 0)
        return "";
    return findNode(selected).myLabel;
}
void cGraphExGraph::selectedLabel(const std::string &l)
{
    if (selected < 0)
        return;
    try
    {
        // check for duplicate label
        findNode(l);
    }
    catch (...)
    {
        // no duplicate
        // go ahead
        selectedNode().myLabel = l;
        return;
    }
    // dulpicate label
    throw std::runtime_error("Dup label");
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
