#pragma once

#include <vector>

#ifdef SHUZIXI_DEBUG
#include <iostream>
#define debug_print(x) \
    std::cout << __FUNCTION__ << " --- " << x << std::endl;

inline std::ostream& operator<<(std::ostream& o, const std::vector<unsigned char> &v) {
    o << "eigen <";
    for (unsigned char c : v) o << c << " ";
    return o << ">";
}
#else
#define debug_print(x)
#endif // SHUZIXI_DEBUG

namespace texture {

typedef unsigned char uchar;
typedef std::vector<uchar> Color;

class TSVQ {
private:
    class TSVQ_Node {
    public:
        TSVQ_Node *left;
        TSVQ_Node *right;

        std::vector<uchar> centroid;
        std::vector<std::vector<uchar>> eigens;
        std::vector<Color> colors;

        TSVQ_Node() : left(nullptr), right(nullptr) {}
        ~TSVQ_Node() {
            if (left) delete left;
            if (right) delete right;
        }

        bool isLeaf() const { return !left && !right; }
        bool isEmpty() const { return eigens.empty() || colors.empty(); }

        // Build method
        void split();
        void computeCentroid();

        // Access method
        Color bestMatch(const std::vector<uchar> &eigen) const;
    };

private:
    constexpr static double epsilon = 0.001;

private:
    TSVQ_Node *_root;

public:
    TSVQ(const std::vector<std::vector<uchar>> &eigens, const std::vector<Color> &colors);
    ~TSVQ() { if(_root) delete _root; }

    Color bestMatch(const std::vector<uchar> &eigen) const;
};

// Operators

inline std::vector<uchar> operator*(const std::vector<uchar> &v, double k) {
    int n = v.size();
    std::vector<uchar> ret(n);
    for (int i = 0; i < n; i++) {
        ret[i] = v[i] * k;
    }
    return ret;
}

inline std::vector<uchar> operator*(double k, const std::vector<uchar> &v) {
    return v * k;
}

inline std::vector<uchar> operator+(const std::vector<uchar> &a, const std::vector<uchar> &b) {
    _ASSERT(a.size() == b.size());
    int n = a.size();
    std::vector<uchar> ret(n);
    for (int i = 0; i < n; i++) {
        ret[i] = a[i] * b[i];
    }
    return ret;
}

inline std::vector<uchar>& operator+=(std::vector<uchar> &a, const std::vector<uchar> &b) {
    _ASSERT(a.size() == b.size());
    int n = a.size();
    for (int i = 0; i < n; i++) {
        a[i] += b[i];
    }
    return a;
}

inline double distance(const std::vector<uchar>& a, const std::vector<uchar>& b) {
    _ASSERT(a.size() == b.size());
    double dist = 0;
    for (int i = 0, n = a.size(); i < n; i++) {
        dist += (a[i] - b[i]) * (a[i] - b[i]);
    }
    return dist;
}

};