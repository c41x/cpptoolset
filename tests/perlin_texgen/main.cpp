#include <base/base.hpp>

using namespace granite;
using namespace granite::base;

void signalError(const string &msg) {
    std::cout << msg << std::endl;
    logError(msg);
}

template <typename T> bool parseArg(char *s, T &o, const string &err) {
    if (strIs<T>(s)) {
        o = fromStr<T>(s);
    }
    else {
        signalError(err);
        return false;
    }
    return true;
}

int main(int argc, char**argv) {
    // initialize logger
    string programDir = fs::getUserDirectory() + "/.granite";
    fs::createFolderTree(programDir);
    log::init(programDir + "/perlin_texgen_log.txt");

    // check arguments count
    if (argc < 6) {
        signalError("not enough arguments: file.png width height persistance octaves tiled(true/false)");
    }
    else {
        timer t;
        string outFile;
        int w, h, octaves;
        float persistance;
        bool tiled;
        perlin p;
        float *ft;
        image img;
        std::tuple<float, float> amp[3];

        // parse args
        outFile = argv[1];
        if (parseArg<int>(argv[2], w, "width(int) - syntax error") &&
            parseArg<int>(argv[3], h, "height(int) - synrax error") &&
            parseArg<float>(argv[4], persistance, "persistance(float) - syntax error") &&
            parseArg<int>(argv[5], octaves, "octaves(int) - syntax error") &&
            parseArg<bool>(argv[6], tiled, "tiled(bool) - syntax error")) {
            t.init();
            t.reset();

            // generate perlin for all channels
            std::cout << "generating: " << outFile << " " << w << "x" << h
                      << " p: " << persistance << " oct: " << octaves
                      << " tiled: " << tiled << std::endl;
            p.init();
            ft = new float[w * h * 3];
            p.fbm2(ft, w, h, persistance, octaves, tiled);
            amp[0] = p.getAmplitude();
            p.randomize();
            p.fbm2(ft + w * h, w, h, persistance, octaves, tiled);
            amp[1] = p.getAmplitude();
            p.randomize();
            p.fbm2(ft + 2 * w * h, w, h, persistance, octaves, tiled);
            amp[2] = p.getAmplitude();

            // convert normalized perlin to image
            img.data.resize(w * h * 3);
            img.width = w;
            img.height = h;
            img.channels = 3;
            for (int i = 0; i < (w * h * 3); i += 3) {
                uint8 &pxr = *(img.data.data() + i);
                uint8 &pxg = *(img.data.data() + i + 1);
                uint8 &pxb = *(img.data.data() + i + 2);
                pxr = (uint8)stretch(std::get<0>(amp[0]), ft[i / 3],
                                     std::get<1>(amp[0]), 0.0f, 255.0f);
                pxg = (uint8)stretch(std::get<0>(amp[1]), ft[i / 3 + w * h],
                                     std::get<1>(amp[1]), 0.0f, 255.0f);
                pxb =  (uint8)stretch(std::get<0>(amp[2]), ft[i / 3 + 2 * w * h],
                                      std::get<1>(amp[2]), 0.0f, 255.0f);
            }
            delete []ft;

            // save to file
            fs::store(outFile, fromImage(img, imageCodecPNG));
            std::cout << "finished " << t.timeS() << "s" << std::endl;
            logOK("finished");
        }
    }

    log::shutdown();
    return 0;
}
