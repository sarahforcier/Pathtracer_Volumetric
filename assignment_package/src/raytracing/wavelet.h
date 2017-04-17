#pragma once
#include <globals.h>
#include "filt.h"

using namespace std;

class Wavelet {
public:

    Wavelet(vector<vector<Color3f>> img, int w, int h);
    //Color3f MedAbsDev(vector<vector<Color3f>> img, int w, int h, Point2i pixel);
    void HighPass(vector<Color3f> &a,vector<Color3f> &cx);
    Color3f Evaluate(Point2i pixel);
    vector<vector<vector<Color3f>>> medians;

};

static inline void rowToCol(vector<vector<Color3f>> in, vector<vector<Color3f>> &out)
{

    int m = in.size();
    int n = in[0].size();
    for (int j = 0; j < n; j ++) {
        vector<Color3f> col;
        for (int i = 0; i < m; i ++) {
            col.push_back(in[i][j]);
        }
        out.push_back(col);
    }
}
