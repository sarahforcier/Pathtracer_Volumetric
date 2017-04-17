#pragma once
#include <globals.h>
#include "denoise.h"
#include "kmeansfilter.h"

class Film{
public:
    Film() : Film(400, 400){}
    ~Film() {}
    Film(unsigned int width, unsigned int height) : bounds(Point2i(0.f, 0.f), Point2i(width, height))
    {
        SetDimensions(width, height);
        kmeans = nullptr;
    }

    void SetDimensions(unsigned int w, unsigned int h);
    void SetPixelColor(const Point2i& pixel, const Color3f pixelColor);
    void SetSigma_Sp(const Point2i& pixel, const Color3f stdev);
    Color3f GetColor(const Point2i& pixel);
    void WriteImage(const std::string &path);
    void WriteImage(QString path);
    bool IsPixelColorSet(const Point2i& pixel);
    void cleanPixels();
    void PostProcess();

    Bounds2i bounds;
//    std::shared_ptr<DeNoise> denoise;
    std::shared_ptr<K_MeansFilter> kmeans;

private:
    std::vector<std::vector<Color3f>> pixels;//A 2D array of pixels in which we can store colors
    std::vector<std::vector<bool>> pixel_has_color;// A 2D array to indicate if there is something rendered on the pixel

    std::vector<std::vector<Color3f>> sigma_sp; // 2d array of standard deviations
};
