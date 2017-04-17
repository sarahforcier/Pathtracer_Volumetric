#include "kmeansfilter.h"

using namespace std;

bool sortR(Color3f i, Color3f j) { return i.r < j.r; }
bool sortG(Color3f i, Color3f j) { return i.g < j.g; }
bool sortB(Color3f i, Color3f j) { return i.b < j.b; }

vector<float> kernel = vector<float> {0.38774, 0.24477, 0.06136};

Color3f K_MeansFilter::Median(int a, int b) {
    vector<Color3f> arr;
    Point2i start = Point2i(glm::max(0, a - 2), glm::max(0, b - 2));
    Point2i end = Point2i(glm::min(a + 3, w), glm::min(b + 3, h));

    for (int i = start.x; i < end.x; i ++) {
        for (int j = start.y; j < end.y; j ++) {
            arr.push_back(original[i][j]);
        }
    }

    int tile = (end.x - start.x) * (end.y - start.y);
    sort(arr.begin(), arr.end(), sortR); float red = arr[tile/2].r;
    sort(arr.begin(), arr.end(), sortG); float green = arr[tile/2].g;
    sort(arr.begin(), arr.end(), sortB); float blue = arr[tile/2].b;

    return Color3f(red, green, blue);

}

Color3f K_MeansFilter::Average(int a, int b) {
    Color3f total = Color3f(0.f);
    Point2i start = Point2i(glm::max(0, a - 2), glm::max(0, b - 2));
    Point2i end = Point2i(glm::min(a + 2, w - 1), glm::min(b + 2, h - 1));

    for (int i = start.x; i <= end.x; i ++) {
        for (int j = start.y; j <= end.y; j ++) {
            total += kernel[i-a] * kernel[j-b] * original[i][j];
        }
    }

    return total;

}

K_MeansFilter::K_MeansFilter(vector<vector<Color3f>> p_colors,
                             vector<vector<Color3f>> stdev,int num)
    : original(p_colors), stdev(stdev)
{
    w = p_colors.size(); h = p_colors[0].size();
    sampler = make_shared<Sampler>(num, 0);
    vector<Point2f> samples = sampler->GenerateStratifiedSamples();

    for (int i = 0; i < samples.size(); i ++) buckets.push_back(BucketInfo(p_colors[w*samples[i].x][h*samples[i].y]));

    // 1) for each pixel, determine which selected color is the closest and
    // mark the bucketLocation array with the index of that bucket
    // 2) determine the average color of that bucket
    for (int i = 0; i < w; i ++) {
        vector<int> place;
        for (int j = 0; j < h; j ++) {
            Color3f average = Average(i, j);
            Color3f median = Median(i, j);
            Color3f color = median;
            float dist = glm::length(color - buckets[0].original);
            int b = 0;
            for (int k = 1; k < buckets.size(); k ++) {
                float len = glm::length(color - buckets[k].original);
                if (len < dist) {
                    b = k; dist = len;
                }
            }
            place.push_back(b);
            buckets[b].total_color += color;
            buckets[b].num ++;
        }
        bucketLocation.push_back(place);
    }
}

Color3f K_MeansFilter::Evaluate(int i, int j) {
    int index = bucketLocation[i][j];
    return buckets[index].GetAvg();
}
