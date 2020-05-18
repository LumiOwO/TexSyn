#include "TSVQ.h"

namespace texture {

TSVQ::TSVQ(const std::vector<std::vector<uchar>> &eigens,
           const std::vector<Color> &colors)
{
    _root = new TSVQ_Node();
    _root->eigens = eigens;
    _root->colors = colors;

    std::vector<TSVQ_Node*> nodes{_root};
    std::vector<TSVQ_Node*> splited;

    int height = log(eigens.size()) / log(2.0);
    while(height--) {
        // Update centroid for current nodes
        for (TSVQ_Node *node : nodes) {
            node->computeCentroid();
        }

        // Get splited nodes
        splited.clear();
        for (TSVQ_Node* node : nodes) {
            node->split();
            if (node->left) splited.push_back(node->left);
            if (node->right) splited.push_back(node->right);
        }
        if (splited.empty()) break;

        std::swap(splited, nodes);
    }
    
}

Color TSVQ::bestMatch(const std::vector<uchar>& eigen) const
{
    TSVQ_Node *node = _root;
    while (!node->isLeaf()) {
        if (!node->left) node = node->right;
        else if (!node->right) node = node->left;
        else {
            node = distance(node->left->centroid, eigen) < distance(node->right->centroid, eigen) ?
                   node->left : node->right;
        }
    }

    return node->bestMatch(eigen);
}

void TSVQ::TSVQ_Node::split()
{
    if (eigens.size() == 1) {
        return;
    }

    // Build new childs
    left = new TSVQ_Node();
    right = new TSVQ_Node();
    left->centroid = centroid * (1 - epsilon);
    right->centroid = centroid * (1 + epsilon);

    // Cluster
    for (int i = 0, n = eigens.size(); i < n; i++) {
        std::vector<uchar> &v = eigens[i];
        TSVQ_Node *dst =
            distance(v, left->centroid) < distance(v, right->centroid) ?
            left : right;

        dst->eigens.push_back(v);
        dst->colors.push_back(colors[i]);
    }

    // Clear empty nodes
    if (left->isEmpty()) { delete left; left = nullptr; }
    if (right->isEmpty()) { delete right; right = nullptr; }
}

void TSVQ::TSVQ_Node::computeCentroid()
{
    int n = eigens.size();
    if (!n) return;
    double inv_n = 1.0 / n;
    centroid = std::vector<uchar>(eigens[0].size(), 0);
    for (auto v : eigens) {
        centroid += v * inv_n;
    }
}

Color TSVQ::TSVQ_Node::bestMatch(const std::vector<uchar>& eigen) const
{
    int index = 0;
    double dist = distance(eigen, eigens[0]);
    for (int i = 1, n = eigens.size(); i < n; i++) {
        double temp = distance(eigen, eigens[i]);
        if (temp < dist) {
            index = i;
            dist = temp;
        }
    }
    return colors[index];
}

};

