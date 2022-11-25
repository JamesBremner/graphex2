/// A node with a location on the screen
class cNode
{
public:
    int x, y;
    int myID;
    std::string myLabel;
    static int myLastID;

    /// CTOR with default ID
    cNode()
    {
        myID = ++myLastID;
    }
    /// @brief CTOR with specified ID
    /// @param id 
    cNode( int id )
    : myID( id )
    {}

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
    std::string save()
    {
        std::stringstream ss;
        std::string lbl = myLabel;
        if( myLabel.empty() )
            lbl = std::to_string( myID );
        ss << "n "<< myID << " " << x << " " << y << " " << lbl << "\n";
        return ss.str();
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
    std::string save()
    {
        std::stringstream ss;
        ss <<"l "<< n1 << " " << n2 << "\n";
        return ss.str();
    }
};

/// @brief A graph of nodes connected by links for the Graphex application
class cGraphExGraph
{
public:
    std::vector<cNode> vN; ///< the nodes
    std::vector<cLink> vL; ///< the links
    int selected;          ///< index of currently selected node
    int prevSelected;

    cGraphExGraph()
        : selected(-1)
    {
    }

    void clear()
    {
        vN.clear();
        vL.clear();
        selected = -1;
        prevSelected = -1;
    }

    void save(const std::string fname);
    void read(const std::string fname);

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
    void selectedRemove();

    void selectedLabel(const std::string &l);

    bool isSelected() const
    {
        return selected >= 0;
    }
    cNode &findNode(int id);
    cNode &findNode(const std::string& label );
    cNode &selectedNode();
};
