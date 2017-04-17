#pragma once
#include <globals.h>
#include "samplers/sampler.h"

using namespace std;

struct BucketInfo {
    BucketInfo(Color3f color)
        : original(color),
          total_color(Color3f(0.f)),
          num(0) {}
    Color3f original;
    Color3f total_color;
    int num;
    Color3f GetAvg() {return total_color/(float)num;}
};

// TODO: need to eliminate fireflies
class K_MeansFilter
{
public:
    K_MeansFilter(std::vector<std::vector<Color3f>> p_colors,
                  std::vector<std::vector<Color3f>> stdev,int num);
    ~K_MeansFilter() {}
    Color3f Median(int i, int j);
    Color3f Average(int i, int j);
    Color3f Evaluate(int i, int j);

private:
    vector<vector<Color3f>> original;
    vector<vector<Color3f>> stdev;
    vector<BucketInfo> buckets;
    vector<vector<int>> bucketLocation; // which bucket each sample belongs to
    int num_samples, w, h;
    shared_ptr<Sampler> sampler;
};
