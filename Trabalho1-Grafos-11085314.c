#define _CRT_SECURE_NO_WARNINGS // Used to allow visual studio C++ to compile scanf(deprecated)

#include <stdio.h> // printf, scanf
#include <stdlib.h> // malloc, calloc

typedef struct Edge
{
    short from, to;
} Edge;

char Equals(Edge a, Edge b) // All edges are 2 way, therefore going from a to b == b to a
{
    return (a.from == b.from && a.to == b.to)
        || (a.from == b.to && b.from == a.to);
}

typedef struct ListNode // Generic list node
{
    void* value;
    struct ListNode* next;
} ListNode;

typedef struct List // Generic list
{
    short count;
    ListNode* root;
    ListNode* tail;
} List;

void CreateList(List* list)
{
    *list = { 0, (ListNode*)calloc(1, sizeof(ListNode)), NULL};
    list->tail = list->root; // tail always starts pointing to root
}

void AddList(List* list, void* value)
{
    // Always alloc memory for next node, even when it still holds no value
    *list->tail = { value, (ListNode*)calloc(1, sizeof(ListNode)) }; // Uses calloc to initialize ptr to zero (NULL)

    list->tail = list->tail->next;
    list->count++;
}

typedef struct Graph
{
    short nVertices, nEdges;
    List* nodes; // Adjacency list
    Edge* edges;
} Graph;

short FindEdge(Graph* graph, Edge edge) // Finds corresponding edge index
{
    for (int i = 0; i < graph->nEdges; i++)
        if (Equals(edge, graph->edges[i]))
            return i;

    return -1;
}

void CreateGraph(Graph* graph)
{
    scanf("%hd %hd", &graph->nVertices, &graph->nEdges);

    graph->nodes = (List*)malloc(graph->nVertices * sizeof(List)); // 

    for (int i = 0; i < graph->nVertices; i++)
        CreateList(graph->nodes + i);

    graph->edges = (Edge*)malloc(graph->nEdges * sizeof(Edge));

    for (short i = 0, aux = 0; i < graph->nEdges; i++)
    {
        Edge* cur = graph->edges + i;
        scanf("%hd %hd", &cur->from, &(cur->to));

        // 2 way connections
        AddList(graph->nodes + cur->from, &cur->to);
        AddList(graph->nodes + cur->to, &cur->from);
    }
}

typedef struct TreeNode
{
    struct TreeNode* parent;
    short value;
    short height;
    List children;
} TreeNode;

typedef struct Tree
{
    short nVertices, nEdges;
    TreeNode* nodes;
    List cutEdges;
    short* isBridge; // Boolean array. If isBridge[index] != 0, is bridge; else, critical edge;
} Tree;

int GetCriticalEdgesN(Tree* tree)
{
    int count = 0;
    for (int i = 0; i < tree->nEdges; i++)
        if (tree->isBridge[i])
            count++;

    return tree->nEdges - count;
}

// Sets every edge in the cycle formed by Edge {from, to} in isBridge array
void BackTrack(Graph * graph, Tree * tree, short from, short to)
{
    TreeNode* a = tree->nodes + from, * b = tree->nodes + to;

    tree->isBridge[FindEdge(graph, { from, to })]++;

    while (a->value != b->value) // Goes backwards in tree until a == b
    {
        short edge;
        if (a->height >= b->height)
        {
            edge = FindEdge(graph, { a->value, a->parent->value });
            a = a->parent;
        }
        else
        {
            edge = FindEdge(graph, { b->value, b->parent->value });
            b = b->parent;
        }
        tree->isBridge[edge]++;
    }
}

void AddCutEdge(Tree* tree, short from, short to)
{
    Edge* edge = (Edge*)malloc(sizeof(Edge));
    *edge = { from, to };
    AddList(&tree->cutEdges, edge);
}


void AddChild(Tree* tree, short parent, short child)
{
    TreeNode* parentNode = tree->nodes + parent;
    TreeNode* childNode = tree->nodes + child;

    AddList(&parentNode->children, childNode);
    childNode->parent = parentNode;
    childNode->height = parentNode->height + 1;
}

void DFSTree(Graph* graph, Tree* tree, short cur, short visited[])
{
    visited[cur]++;

    for (ListNode* curNode = graph->nodes[cur].root; curNode->next; curNode = curNode->next)
    {
        short n = *(short*)curNode->value;

        if (tree->nodes[cur].parent == &tree->nodes[n]) continue;

        if (!visited[n]) // If first time visitting, adds to tree and visits its children
        {
            AddChild(tree, cur, n);
            DFSTree(graph, tree, n, visited);
        }
        else // Else, edge is cut edge. Adds to cut edge list and backtracks to check cycles.
        {
            AddCutEdge(tree, cur, n);
            BackTrack(graph, tree, cur, n);
        }
    }
}

// Converts graph to Tree form (also contains cut edges list)
void GraphToTree(Graph* graph, Tree * tree)
{
    tree->nVertices = graph->nVertices;
    tree->nEdges = graph->nEdges;
    tree->nodes = (TreeNode*)calloc(tree->nVertices, sizeof(TreeNode));
    CreateList(&tree->cutEdges);
    tree->isBridge = (short*)calloc(graph->nEdges, sizeof(short));

    for (int i = 0; i < tree->nVertices; i++)
    {
        CreateList(&tree->nodes[i].children);
        tree->nodes[i].value = i;
    }

    short * visited = (short *) calloc(graph->nVertices, sizeof(short));

    DFSTree(graph, tree, 0, visited);
}

void printDFS(TreeNode* node) // Prints each tree's node's value and height in DFS order
{
    printf("%hd - %hd\n", node->value, node->height);
    if (node->children.count == 0) return;

    for (ListNode* n = node->children.root; n->value; n = n->next)
        printDFS((TreeNode*)n->value);
}

// Arthur N. Guedes - 11085314
int main()
{
    Graph graph;
    CreateGraph(&graph);
        
    Tree tree;
    GraphToTree(&graph, &tree);

    //printDFS(tree.nodes);
    printf("%hd", GetCriticalEdgesN(&tree));

    return 0;
}