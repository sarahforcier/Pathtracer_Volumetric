#include "wavelet.h"

using namespace std;

struct Complex {
    float r;
    float i;
};

vector<float> kernel = vector<float> {-1.f, -1.f, 9.f, -1.f, -1.f};

bool sortR(Color3f i, Color3f j) { return i.r < j.r; }
bool sortG(Color3f i, Color3f j) { return i.g < j.g; }
bool sortB(Color3f i, Color3f j) { return i.b < j.b; }

Wavelet::Wavelet(vector<vector<Color3f>> img, int w, int h) {
    // row filter and downsample
    vector<vector<Color3f>> rowFiltered1, rowFiltered2; // 4x8
    for (int i = 0; i < w; i++) {
        vector<Color3f> cx;
        HighPass(img[i], cx);
        if (i%2 == 0) rowFiltered1.push_back(cx);
        else rowFiltered2.push_back(cx);
    }

    // column filter
    vector<vector<Color3f>> byCol1, byCol2;
    rowToCol(rowFiltered1, byCol1);
    rowToCol(rowFiltered2, byCol2);
    vector<vector<Color3f>> colEE, colEO, colOE, colOO; // 4x8
    for (int i = 0; i < h; i ++) {
        if (i%2 == 0) {
            vector<Color3f> ce;
            HighPass(byCol1[i], ce);
            colEE.push_back(ce);
            vector<Color3f> co;
            HighPass(byCol2[i], co);
            colOE.push_back(co);
        }
        else {
            vector<Color3f> ce;
            HighPass(byCol1[i], ce);
            colEO.push_back(ce);
            vector<Color3f> co;
            HighPass(byCol2[i], co);
            colOO.push_back(co);
        }
    }
    medians.push_back(colEE); medians.push_back(colEO);
    medians.push_back(colOE); medians.push_back(colOO);
}

//Color3f MedAbsDev(vector<vector<Color3f>> img, int w, int h, Point2i pixel)
//{
//    // row filter and downsample
//    vector<vector<Color3f>> rowFiltered; // 4x8
//    int i = 0;
//    if (pixel.x %2 != 0) i = 1;
//    pixel.x /= 2;
//    for (i; i < w; i = i + 2) {
//        vector<Color3f> cx;
//        HighPass(img[i], cx);
//        rowFiltered.push_back(cx);
//    }

//    // column filter
//    vector<vector<Color3f>> byCol;
//    rowToCol(rowFiltered, byCol);
//    vector<vector<Color3f>> colFiltered; // 4x8
//    i = 0;
//    if (pixel.y %2 != 0) i = 1;
//    pixel.y /= 2;
//    for (int i = 0; i < h; i = i + 2) {
//        vector<Color3f> cx;
//        HighPass(byCol[i], cx);
//        colFiltered.push_back(cx);
//    }
//    Point2i dim = Point2i(colFiltered.size(), colFiltered[0].size());
//    vector<Color3f> arr;
//    Point2i start = Point2i(glm::max(0, pixel.x - 2), glm::max(0, pixel.y - 2));
//    Point2i end = Point2i(glm::min(pixel.x + 2, dim.x - 1), glm::min(pixel.y + 2, dim.y - 1));
//    for (int i = start.x; i <= end.x; i ++) {
//        for (int j = start.y; j <= end.y; j++) {
//            arr.push_back(colFiltered[i][j]);
//        }
//    }

//    sort(arr.begin(), arr.end(), sortR); float r = arr[0].r;
//    sort(arr.begin(), arr.end(), sortR); float g = arr[1].g;
//    sort(arr.begin(), arr.end(), sortR); float b = arr[2].b;
//    return Color3f(r,g,b) / 0.6745f;
//}

// a: signal
// b: high pass filter
// cx: return
void Wavelet::HighPass(vector<Color3f> &a, vector<Color3f> &cx) {
    int len = a.size();
    for (int i = 0; i < len; i++) {
        int start = glm::max(0, i - 2);
        int end = glm::min(len - 1, i + 3);
        Color3f pass = Color3f(0.f);
        float delta = end - start;
        for (int j = start; j <= end; j++) pass += kernel[glm::abs(j - i)] * a[j] / delta / delta;
        cx.push_back(glm::max(pass, Color3f(0.f)));
    }
}

Color3f Wavelet::Evaluate(Point2i pixel) {
    int index = ((pixel.x % 2 == 0) ? 0 : 2) + ((pixel.y % 2 == 0) ? 0 : 1);

    Point2i dim = Point2i(medians[index].size(), medians[index][0].size());
    vector<Color3f> arr;
    Point2i start = Point2i(glm::clamp(pixel.x/2 - 1, 0, dim.x - 1), glm::clamp(pixel.y/2 - 1, 0, dim.y - 1));
    Point2i end = Point2i(glm::clamp(pixel.x/2 + 1, 0, dim.x - 1), glm::clamp(pixel.y/2 + 1, 0, dim.y - 1));
    for (int i = start.x; i <= end.x; i ++) {
        for (int j = start.y; j <= end.y; j++) {
            arr.push_back(medians[index][i][j]);
        }
    }

    sort(arr.begin(), arr.end(), sortR); float r = arr[0].r;
    sort(arr.begin(), arr.end(), sortR); float g = arr[1].g;
    sort(arr.begin(), arr.end(), sortR); float b = arr[2].b;
    return Color3f(r,g,b) / 0.6745f;
}
