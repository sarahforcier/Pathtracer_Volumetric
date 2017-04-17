#include "denoise.h"

DeNoise::DeNoise(vector<vector<Color3f> > img,
                 vector<vector<Color3f> > sigma_sp,
                 Point2i dim) : noisy_image(img), w(dim.x), h(dim.y), win_size(8), g(3.f), buckets(1000) {

    Color3f max = SetNoiseMap(sigma_sp);

//    vector<map<int, float>> cdf_map;
//    ComputeCDF(max, cdf_map);

//    vector<vector<float>> sigmas;
//    vector<vector<vector<vector<float>>>> levels;
//    ComputeLevels(cdf_map, sigmas, levels);

//    CombineLayers(sigmas, levels); // write to final

//    RemoveSpikes(); // write to final
}

// return max value
Color3f DeNoise::SetNoiseMap(vector<vector<Color3f>> sigma_sp)
{
    vector<vector<Color3f>> sigma_wp;
    Color3f max_wp = CalculateMedian(sigma_wp);

    // DEBUG
    denoised_image = sigma_wp; // only blurred one way?? down

    Color3f max = Color3f(0.f);
    for (int a = 0; a < w; a++) {
        vector<Color3f> row;
        for (int b = 0; b < h; b++) {
            // dilation
            Point2i start = Point2i(glm::max(0, a - 1), glm::max(0, b - 1));
            Point2i end = Point2i(glm::min(a + 1, w - 1), glm::min(b + 1, w - 1));

            Color3f max_sp = Color3f(0.f);
            for (int i = start.x; i <= end.x; i++) {
                for (int j = start.y; j <= end.y; j++) {
                    max_sp = glm::max(sigma_sp[i][j], max_sp);
                    max_wp = glm::max(sigma_wp[i][j], max_wp);
                }
            }
            Color3f color = glm::pow(sigma_sp[a][b] / noisy_image[a][b], Vector3f(0.25f)) * sigma_wp[a][b];
            row.push_back(color);
            max = glm::max(color, max);
        }
        noiseMap.push_back(row); // too many zeros :(
    }

    L = glm::ceil(max_wp/ 10.f);

    // scale and normalize
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            noiseMap[i][j] /= max;
            noiseMap[i][j] *= g * max_wp;
        }
    }
    denoised_image = noiseMap;

    return max;
}

// return max wp
Color3f DeNoise::CalculateMedian(vector<vector<Color3f>> &sigma_wp)
{
    Color3f max;
    Wavelet *wavelet = new Wavelet(noisy_image, w, h);
    for (int i = 0; i < w; i ++) {
        vector<Color3f> row;
        for (int j = 0; j < h; j ++) {

            Color3f color = wavelet->Evaluate(Point2i(i,j)); max = glm::max(color, max);
            row.push_back(color);
        }
        sigma_wp.push_back(row);
    }
    delete wavelet;
    return max;
}

// CDF per color
void DeNoise::ComputeCDF(Color3f max, vector<map<int, float>> &cdf_map)
{
    for (int c = 0; c < 3; c++) { // per color
        vector<int> hist = vector<int> (buckets);

        std::fill(hist.begin(), hist.end(), 0);
        for (int i = 0; i < w; i++) {
            for (int j = 0; j < h; j++) {
                float value = noiseMap[i][j][c];
                int index = value == max[c] ? buckets - 1 : value * buckets / max[c];
                hist[index] ++;
            }
        }

        map<int, float> cmap;
        int count = 0.f;
        for (int j = 0; j < buckets; j ++) {
            count += hist[j];
            cmap[count] = j * max[c] / buckets;
        }
        cdf_map.push_back(cmap);
    }
}

// return list of denoised images based on sigmas computed from incoming cdf_map for each channel
// out: color, # levels, 2D array
void DeNoise::ComputeLevels(vector<map<int, float>> cdf_map, vector<vector<float>> &sigmas, vector<vector<vector<vector<float>>>> &out)
{

    for (int c = 0; c < 3; c ++) {
        // compute sigma values for each color
        vector<float> sigma_list;
        int step, stepC = w*h / L[c];
        auto i = cdf_map[c].begin();
        while (i != cdf_map[c].end()) {
            if (i->first >= step) {
                sigma_list.push_back(i->second); // save sigma
                stepC += step;
            }
            i++;
        }

        // denoise single channels based on above calculated sigmas
        vector<vector<vector<float>>> levels;
        for (int j = 0; j < sigma_list.size(); j++) { // levels
            vector<vector<float>> img_denoised;

            Gaussian(img_denoised, sigma_list[j], c);

            levels.push_back(img_denoised);
        }
        out.push_back(levels);
        sigmas.push_back(sigma_list);
    }
}

float DeNoise::Gaussian(vector<vector<float>> &out, float sigma, int c)
{
    vector<float> kernel;
    float sigma2 = sigma * sigma;
    for (float i = 0.f; i < 4.f; i ++) {
        float weight = glm::exp(- i * i / 2.f/ sigma2) / glm::sqrt(TwoPi * sigma2);
        kernel.push_back(weight);
    }

    // horizontal blur
    for (int i = 0; i < w; i ++) {
        vector<float> row;
        for (int j = 0; j < h; j ++) {
            int start = glm::max(0, j - 3);
            int end = glm::min(j, j + 4);
            float sum = 0;
            for (int w = start; w < end; w++) sum += kernel[glm::abs(w - j)] * noisy_image[i][w][c];
            row.push_back(sum/(end - start));
        }
        out.push_back(row);
    }

    // vertical blur
    for (int j = 0; j < h; j ++) {
        vector<float> col;
        for (int i = 0; i < w; i ++) {
            int start = glm::max(0, i - 3);
            int end = glm::min(i, i + 4);
            float sum = 0;
            for (int w = start; w < end; w++) sum += kernel[glm::abs(w - j)] * noisy_image[w][j][c];
            col.push_back(sum/(end - start));
        }
        out.push_back(col);
    }
}

int index(vector<float> sigmas, float sigma) {
    for (int i = 0; i < sigmas.size(); i ++) {
        if (sigma > sigmas[i]) return i;
    }
}

void DeNoise::CombineLayers(vector<vector<float>> sigmas, vector<vector<vector<vector<float>>>> level) // color, levels, 2D array
{
    denoised_image = vector<vector<Color3f>>(w);
    for (int i = 0; i < w; i ++) {
        vector<Color3f> row;
        for (int j = 0; j < h; j ++) {
            float sigmaR = noiseMap[i][j].r; int i_r = index(sigmas[0], sigmaR);
            float sigmaG = noiseMap[i][j].g; int i_g = index(sigmas[1], sigmaG);
            float sigmaB = noiseMap[i][j].b; int i_b = index(sigmas[2], sigmaB);
            float t_r = sigmaR - sigmas[0][i_r]/(sigmas[0][i_r + 1] - sigmas[0][i_r]);
            float t_g = sigmaG - sigmas[1][i_g]/(sigmas[1][i_g + 1] - sigmas[1][i_g]);
            float t_b = sigmaB - sigmas[2][i_b]/(sigmas[2][i_b + 1] - sigmas[2][i_b]);
            float r = glm::mix(level[0][i_r][i][j], level[0][i_r+1][i][j], t_r);
            float g = glm::mix(level[1][i_g][i][j], level[1][i_g+1][i][j], t_g);
            float b = glm::mix(level[2][i_b][i][j], level[2][i_b+1][i][j], t_b);
            row.push_back(Color3f(r, g, b));
        }
        denoised_image.push_back(row);
    }
}

void DeNoise::RemoveSpikes()
{
    // calculate median, mean, std images
    vector<vector<Color3f>> mean, median, stdev;
    for (int a = 0; a < w; a ++) {
        vector<Color3f> rowMean, rowMed, rowSTD;
        for (int b = 0; b < h; b++) {
            vector<float> R, G, B;
            Color3f total_color = Color3f(0.f);
            vector<Color3f> all_colors;
            Point2i start = Point2i(glm::max(0, a - 1), glm::max(0, b - 1));
            Point2i end = Point2i(glm::min(a + 1, w - 1), glm::min(b + 1 , h - 1));
            for (int i = start.x; i <= end.x; i++) {
                for (int j = start.y; j <= end.y; j++) {
                    R.push_back(denoised_image[i][j].r);
                    G.push_back(denoised_image[i][j].g);
                    B.push_back(denoised_image[i][j].b);
                    total_color += denoised_image[i][j];
                    all_colors.push_back(denoised_image[i][j]);
                }
            }
            std::sort(R.begin(), R.end()); std::sort(G.begin(), G.end()); std::sort(G.begin(), G.end());
            rowMed.push_back(Color3f(R[3], G[3], B[3]));
            rowMean.push_back(total_color / 9.f);
            rowSTD.push_back(GetStdDev(all_colors, total_color / 9.f));
        }
        mean.push_back(rowMean); median.push_back(rowMed); stdev.push_back(rowSTD);
    }
    // replace all pixels whose (color - mean) > std with median
    for (int a = 0; a < w; a++) {
        for (int b = 0; b < h; b++) {
            if ((denoised_image[a][b].r - mean[a][b].r) > stdev[a][b].r) denoised_image[a][b].r = median[a][b].r;
            if ((denoised_image[a][b].g - mean[a][b].g) > stdev[a][b].g) denoised_image[a][b].g = median[a][b].g;
            if ((denoised_image[a][b].b - mean[a][b].b) > stdev[a][b].b) denoised_image[a][b].b = median[a][b].b;
        }
    }
}
