#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <filesystem>
#include "graphexgraph.h"

int cNode::myLastID = 0;

std::string cNode::save()
{
    std::stringstream ss;
    ss << "n " << myID << " " << x << " " << y << " " 
        << label() << "\n";
    return ss.str();
}

std::string cNode::label() const
{
    if( myLabel.empty() )
        return std::to_string( myID );
    return myLabel;
}

std::string cLink::save()
{
    std::stringstream ss;
    ss << "l " << n1 << " " << n2 << "\n";
    return ss.str();
}

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
        if (n.label() == label)
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
    return findNode(selected).label();
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
        selectedNode().label( l );
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
            std::string lbl;
            ifs >> lbl;
            n.label(lbl );
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

std::vector< cLink*  > cGraphExGraph::findLinksSelectedNode()
{
    std::vector< cLink*  > ret;
    if( ! isSelected() )
        return ret;
    for( auto& e : vL )
    {
        if( e.n1 == selected || e.n2 == selected )
        {
            ret.push_back( &e );
        }
    }
    return ret;
}

// void cGraphExGraph::graphViz()
// {
//     bool isDirected = false;

//     auto path = std::filesystem::temp_directory_path();
//     std::cout << "RunDOT " << path << "\n";
//     auto gdot = path / "g.dot";
//     std::ofstream f(gdot);
//     if (!f.is_open())
//         throw std::runtime_error("Cannot open " + gdot.string());

//     std::string graphvizgraph = "graph";
//     std::string graphvizlink = "--";
//     // if (isDirected())
//     // {
//     //     graphvizgraph = "digraph";
//     //     graphvizlink = "->";
//     // }

//     f << graphvizgraph << " G {\n";
//     for (auto& n : vN)
//     {
//         f << n.myLabel
//           << " [  penwidth = 3.0 ];\n";
//     }

//     // std::cout << "pathViz " << pathText() << "\n";

//     // loop over links
//     for (auto &e : vL)
//     {
//         f << findNode(e.n1).myLabel << graphvizlink << findNode(e.n2).myLabel << "\n";
//     }

//     f << "}\n";

//     f.close();

//     STARTUPINFO si;
//     PROCESS_INFORMATION pi;

//     ZeroMemory(&si, sizeof(si));
//     si.cb = sizeof(si);
//     ZeroMemory(&pi, sizeof(pi));

//     auto sample = path / "sample.png";
//     std::string scmd = "dot -Kfdp -n -Tpng -Tdot -o " + sample.string() + " " + gdot.string();

//     //std::cout << scmd << "\n";

//     // Retain keyboard focus, minimize module2 window
//     si.wShowWindow = SW_SHOWNOACTIVATE | SW_MINIMIZE;
//     si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USEPOSITION;
//     si.dwX = 600;
//     si.dwY = 200;

//     if (!CreateProcess(NULL,                 // No module name (use command line)
//                        (char *)scmd.c_str(), // Command line
//                        NULL,                 // Process handle not inheritable
//                        NULL,                 // Thread handle not inheritable
//                        FALSE,                // Set handle inheritance to FALSE
//                        CREATE_NEW_CONSOLE,   //  creation flags
//                        NULL,                 // Use parent's environment block
//                        NULL,                 // Use parent's starting directory
//                        &si,                  // Pointer to STARTUPINFO structure
//                        &pi)                  // Pointer to PROCESS_INFORMATION structure
//     )
//     {
//         int syserrno = GetLastError();
//         if (syserrno == 2)
//         {
//             // wex::msgbox mb(
//             //     "Cannot find executable file\n"
//             //     "Is graphViz installed?");

//             return;
//         }
//         //wex::msgbox mb("system error");
//         // SetStatusText(wxString::Format("Sysem error no (%d)\n", GetLastError()));
//         // wchar_t *lpMsgBuf;
//         // FormatMessage(
//         //     FORMAT_MESSAGE_ALLOCATE_BUFFER |
//         //         FORMAT_MESSAGE_FROM_SYSTEM |
//         //         FORMAT_MESSAGE_IGNORE_INSERTS,
//         //     NULL,
//         //     (DWORD)syserrno,
//         //     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
//         //     (LPWSTR)&lpMsgBuf,
//         //     0, NULL);

//         // char errorbuf[200];
//         // snprintf(errorbuf, 199,
//         //          "Error is %S",
//         //          lpMsgBuf);
//         // LocalFree(lpMsgBuf);

//         return;
//     }

//     // Close process and thread handles.
//     CloseHandle(pi.hProcess);
//     CloseHandle(pi.hThread);

//     Sleep(1000);
// }
